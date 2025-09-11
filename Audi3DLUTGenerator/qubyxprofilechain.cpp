/* 
 * Author: QUBYX Software Technologies LTD HK
 * Copyright: QUBYX Software Technologies LTD HK
 */

#include "qubyxprofilechain.h"

#include <algorithm>

#include "QubyxProfile.h"
#include "ICCProfLib/IccCmm.h"

QubyxProfileChain::QubyxProfileChain()
    : ri_(QubyxProfileChain::RI::RelativeColorimetric),
      in_(QubyxProfileChain::SpaceType::DeviceSpecific),
      out_(QubyxProfileChain::SpaceType::Lab),
      started_(false),
      hasLastChad_(false),
      hasLastLuminance_(false),
      lastLuminance_(0),
      lastOutput_(SpaceType::DeviceSpecific)
{

}

QubyxProfileChain::QubyxProfileChain(QubyxProfileChain::SpaceType in, QubyxProfileChain::SpaceType out, QubyxProfileChain::RI renderingIntent)
    : ri_(renderingIntent),
      in_(in),
      out_(out),
      started_(false),
      hasLastChad_(false),
      hasLastLuminance_(false),
      lastLuminance_(0),
      lastOutput_(SpaceType::DeviceSpecific)
{

}


void QubyxProfileChain::setRenderingIntent(QubyxProfileChain::RI renderingIntent)
{
    ri_ = renderingIntent;
}

void QubyxProfileChain::setTransformationType(QubyxProfileChain::SpaceType in, QubyxProfileChain::SpaceType out)
{
    in_ = in;
    out_ = out;
}

void QubyxProfileChain::clearChain()
{
    cmms_.clear();
    started_ = false;
    hasLastChad_ = false;
    lastChad_.clear();
    hasLastLuminance_=false;
    lastLuminance_=0;
}

bool QubyxProfileChain::addProfile(QubyxProfile *profile)
{
    return addProfile(*profile, ri_);
}

bool QubyxProfileChain::addProfile(QubyxProfile *profile, QubyxProfileChain::RI renderingIntent)
{    
    return addProfile(*profile, renderingIntent);
}

bool QubyxProfileChain::addProfile(const QubyxProfile &profile)
{
    return addProfile(profile, ri_);
}

bool QubyxProfileChain::addProfile(const QubyxProfile &profile, QubyxProfileChain::RI renderingIntent)
{
    bool first = cmms_.empty();
    if(first)
        cmms_.push_back(CMM(in_, out_));
    unsigned index = cmms_.size()-1;

    //divide cmms if xyz/lab inside realistic chain
    if(!first
       && (renderingIntent==RI::RealisticColorimetric || renderingIntent==RI::RealisticColorimetricWithLuminance)
       && (lastOutput_==SpaceType::Lab || lastOutput_==SpaceType::XYZ))
    {
        cmms_[index].changeOutput(SpaceType::XYZ);

        cmms_[index].outputChad_ = lastChad_;
        cmms_[index].hasOutputChad_ = hasLastChad_;
        cmms_[index].outputLum_ = lastLuminance_;
        cmms_[index].hasOutputLuminance_ = hasLastLuminance_;

        cmms_.push_back(CMM(SpaceType::XYZ, out_));
        first = true;
        ++index;
    }


    bool res = (cmms_[index].cmm_->AddXform(profile.profile_, iccProfLibRI(renderingIntent), icInterpTetrahedral)==icCmmStatOk);

    if(res)
        if(renderingIntent==RI::RealisticColorimetric || renderingIntent==RI::RealisticColorimetricWithLuminance)
        {
            if(first && cmms_[index].in_==SpaceType::XYZ)
            {
                cmms_[index].inputChad_ = profile.getChromaticAdaptationAsVector();
                cmms_[index].hasInputChad_ = (cmms_[index].inputChad_.size()>=9);
            }

            if(first
               && renderingIntent==RI::RealisticColorimetricWithLuminance
               && (cmms_[index].in_==SpaceType::XYZ || cmms_[index].in_==SpaceType::Lab))
                cmms_[index].hasInputLuminance_ = profile.getLuminance(cmms_[index].inputLum_);

            cmms_[index].hasOutputChad_ = false;
            if(cmms_[index].out_==SpaceType::XYZ)
            {
                cmms_[index].outputChad_ = profile.getRevertChromaticAdaptationAsVector();
                cmms_[index].hasOutputChad_ = (cmms_[index].outputChad_.size()>=9);
            }

            if(renderingIntent==RI::RealisticColorimetricWithLuminance
               && (cmms_[index].out_==SpaceType::XYZ || cmms_[index].out_==SpaceType::Lab))
                cmms_[index].hasOutputLuminance_ = profile.getLuminance(cmms_[index].outputLum_);

            //save added values for dividing on next step
            lastChad_ = profile.getRevertChromaticAdaptationAsVector();
            hasLastChad_ = (lastChad_.size()>=9);
            hasLastLuminance_ = profile.getLuminance(lastLuminance_);
            lastOutput_ = spaceType(cmms_[index].cmm_->GetLastSpace());
        }
    return res;

}

bool QubyxProfileChain::addProfiles(const std::vector<QubyxProfile *> &profiles)
{
    bool r=true;
    for(auto v : profiles)
        r = r && addProfile(v, ri_);
    return r;
}

bool QubyxProfileChain::addProfiles(const std::vector<QubyxProfile *> &profiles, QubyxProfileChain::RI renderingIntent)
{
    bool r=true;
    for(auto v : profiles)
        r = r && addProfile(v, renderingIntent);
    return r;
}

bool QubyxProfileChain::isChainComplete()
{
    if(cmms_.empty())
    {
//        qDebug() << "no cmm!!!";
        return false;
    }

    if(!started_)
    {
        started_ = true;
        for(unsigned i=0;i<cmms_.size() && started_;++i)
            started_ = (cmms_[i].cmm_->Begin()==icCmmStatOk)
                       && (cmms_[i].cmm_->GetApply()!=nullptr);
    }

    return started_;
}

std::shared_ptr<QubyxProfileChain> QubyxProfileChain::singleProfileChain(QubyxProfile *profile, QubyxProfileChain::SpaceType in, QubyxProfileChain::SpaceType out, QubyxProfileChain::RI renderingIntent)
{
    return singleProfileChain(*profile, in, out, renderingIntent);
}

std::shared_ptr<QubyxProfileChain> QubyxProfileChain::singleProfileChain(const QubyxProfile &profile, QubyxProfileChain::SpaceType in, QubyxProfileChain::SpaceType out, QubyxProfileChain::RI renderingIntent)
{
    std::shared_ptr<QubyxProfileChain> res(new QubyxProfileChain(in, out, renderingIntent));
    res->addProfile(profile);
    return res;
}

template<typename T>
bool QubyxProfileChain::transform(const std::vector<T> &in, std::vector<T> &out)
{
    using std::copy;
    using std::begin;
    using std::end;

    if(!isChainComplete())
    {
//        qDebug() << "!!! chain is not complete";
        return false;
    }

    bool res = true;
    icFloatNumber sPixel[16], rPixel[16];
    int inCount, outCount;
    for(int i=0;i<cmms_.size() && res;++i)
    {
        inCount = colorsCountByType(cmms_[i].cmm_->GetFirstXformSource());
        outCount = colorsCountByType(cmms_[i].cmm_->GetLastXformDest());
        if(i==0)
            for(int i=0;i<inCount;i++)
                sPixel[i] = in[i];
        else
            for(int i=0;i<inCount;i++)
                sPixel[i] = rPixel[i];

        if(cmms_[i].hasInputLuminance_)
        {
            if(cmms_[i].in_==QubyxProfileChain::SpaceType::XYZ)
                for(int j=0;j<inCount;++j)
                    sPixel[j] /= cmms_[i].inputLum_;

            if(cmms_[i].in_==QubyxProfileChain::SpaceType::Lab)
            {

            }
        }

        if(cmms_[i].hasInputChad_)
            QubyxProfile::applyChromaticAdaptation(cmms_[i].inputChad_, sPixel);

        if(cmms_[i].in_==QubyxProfileChain::SpaceType::Lab)
            icLabToPcs(sPixel);
        if(cmms_[i].in_==QubyxProfileChain::SpaceType::XYZ)
            icXyzToPcs(sPixel);

        res = res && (cmms_[i].cmm_->Apply(rPixel, sPixel)==icCmmStatOk);

        if(cmms_[i].out_==QubyxProfileChain::SpaceType::Lab)
            icLabFromPcs(rPixel);
        if(cmms_[i].out_==QubyxProfileChain::SpaceType::XYZ)
            icXyzFromPcs(rPixel);


        if(cmms_[i].hasOutputChad_)
            QubyxProfile::applyChromaticAdaptation(cmms_[i].outputChad_, rPixel);

        if(cmms_[i].hasOutputLuminance_)
        {
            if(cmms_[i].out_==QubyxProfileChain::SpaceType::XYZ)
                for(int j=0;j<outCount;++j)
                    rPixel[j] *= cmms_[i].outputLum_;

            if(cmms_[i].out_==QubyxProfileChain::SpaceType::Lab)
            {

            }
        }
    }
    out.resize(outCount);
    copy(rPixel, rPixel+outCount, out.begin());

    return res;
}

icRenderingIntent QubyxProfileChain::iccProfLibRI(QubyxProfileChain::RI renderingIntent)
{
    switch(renderingIntent)
    {
        case QubyxProfileChain::RI::Perceptual:
            return icPerceptual;
        case QubyxProfileChain::RI::RelativeColorimetric:
            return icRelativeColorimetric;
        case QubyxProfileChain::RI::Saturation:
            return icSaturation;
        case QubyxProfileChain::RI::AbsoluteColorimetric:
            return icAbsoluteColorimetric;

        case QubyxProfileChain::RI::RealisticColorimetric:
            return icRelativeColorimetric;

        default:
            return icRelativeColorimetric;
    }
}

icColorSpaceSignature QubyxProfileChain::iccProfLibSpace(QubyxProfileChain::SpaceType space)
{
    switch(space)
    {
        case QubyxProfileChain::SpaceType::Lab:
            return icSigLabData;
        case QubyxProfileChain::SpaceType::XYZ:
            return icSigXYZData;

        default:
            return icSigUnknownData;
    }
}

QubyxProfileChain::SpaceType QubyxProfileChain::spaceType(icColorSpaceSignature space)
{
    switch(space)
    {
        case icSigLabData:
            return QubyxProfileChain::SpaceType::Lab;
        case icSigXYZData:
            return QubyxProfileChain::SpaceType::XYZ;

        default:
            return QubyxProfileChain::SpaceType::DeviceSpecific;
    }
}

int QubyxProfileChain::colorsCountByType(icColorSpaceSignature sig)
{
    switch(sig)
    {
        case icSigGrayData:
            return 1;
        case icSigCmykData:
            return 4;
        case icSig1colorData:
            return 1;
        case icSig2colorData:
            return 2;
        case icSig4colorData:
            return 4;
        case icSig5colorData:
            return 5;
        case icSig6colorData:
            return 6;
        case icSig7colorData:
            return 7;
        case icSig8colorData:
            return 8;
        case icSig9colorData:
            return 9;
        case icSig10colorData:
            return 10;
        case icSig11colorData:
            return 11;
        case icSig12colorData:
            return 12;
        case icSig13colorData:
            return 13;
        case icSig14colorData:
            return 14;
        case icSig15colorData:
            return 15;
        case icSig16colorData:
            return 16;

        default:
            return 3;
    }
    return 3;
}

template bool QubyxProfileChain::transform<double>(const std::vector<double> &in, std::vector<double> &out);
template bool QubyxProfileChain::transform<float>(const std::vector<float> &in, std::vector<float> &out);

QubyxProfileChain::CMM::CMM(SpaceType in, SpaceType out)
    : hasInputChad_(false),
      hasOutputChad_(false),
      hasInputLuminance_(false),
      hasOutputLuminance_(false),
      inputLum_(1),
      outputLum_(1),
      in_(in),
      out_(out),
      cmm_(new CIccCmm(iccProfLibSpace(in), iccProfLibSpace(out), in==SpaceType::DeviceSpecific))
{

}

void QubyxProfileChain::CMM::changeOutput(QubyxProfileChain::SpaceType out)
{
    out_ = out;
    cmm_->SetDestSpace(iccProfLibSpace(out));
}
