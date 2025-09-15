/*
 * Author: QUBYX Software Technologies LTD HK
 * Copyright: QUBYX Software Technologies LTD HK
 */

#include "QubyxProfile.h"

#include "ICCProfLib/IccTagBasic.h"
#include "ICCProfLib/IccUtil.h"
#include "ICCProfLib/IccTagLut.h"
#include "ICCProfLib/IccEval.h"
#include "ICCProfLib/IccPrmg.h"

#include "qubyxprofilechain.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include <type_traits>

#define TRCSIZE 1024

QubyxProfile::QubyxProfile()
{
    devDim_ = 3;
    maxY_ = 1;
    savable_ = true;

    for (int i = 0;i < 9;++i) 
    {
        convertChroma_[i] = 0;
        convertRevChroma_[i] = 0;
    }

    inColorSpace_ = icSigRgbData;
    outColorSpace_ = icSigXYZData;

    spec_ = ICCSpec::ICCv4;
    skipDescrType_ = false;
}

QubyxProfile::QubyxProfile(std::string profilePath)
{
    devDim_ = 3;
    maxY_ = 1;
    savable_ = true;
    setFileName(profilePath);

    for (int i = 0;i < 9;++i) 
    {
        convertChroma_[i] = 0;
        convertRevChroma_[i] = 0;
    }

    inColorSpace_ = icSigRgbData;
    outColorSpace_ = icSigXYZData;

    spec_ = ICCSpec::ICCv4;
    skipDescrType_ = false;
}

QubyxProfile::QubyxProfile(const QubyxProfile& other)
    : devDim_(other.devDim_),
    inColorSpace_(other.inColorSpace_),
    outColorSpace_(other.outColorSpace_),
    maxY_(other.maxY_),
    optDescription_(other.optDescription_),
    skipDescrType_(other.skipDescrType_),
    spec_(other.spec_),
    filename_(other.filename_),
    savable_(other.savable_),
    profile_(other.profile_),
    deviceName_(other.deviceName_)
{
    std::copy(other.convertChroma_, other.convertChroma_ + 9, convertChroma_);
    std::copy(other.convertRevChroma_, other.convertRevChroma_ + 9, convertRevChroma_);
}

QubyxProfile::~QubyxProfile()
{
}

QubyxProfile& QubyxProfile::operator=(const QubyxProfile& other)
{
    if (this == &other)
        return *this;

    devDim_ = other.devDim_;
    inColorSpace_ = other.inColorSpace_;
    outColorSpace_ = other.outColorSpace_;
    maxY_ = other.maxY_;
    optDescription_ = other.optDescription_;
    skipDescrType_ = other.skipDescrType_;
    spec_ = other.spec_;
    filename_ = other.filename_;
    savable_ = other.savable_;
    profile_ = other.profile_;
    deviceName_ = other.deviceName_;

    std::copy(other.convertChroma_, other.convertChroma_ + 9, convertChroma_);
    std::copy(other.convertRevChroma_, other.convertRevChroma_ + 9, convertRevChroma_);

    return *this;
}

void QubyxProfile::setFileName(std::string filename)
{
    filename_ = filename;
}

std::string QubyxProfile::profilePath()
{
    return filename_;
}

std::string QubyxProfile::makeProfileTitle(std::string type)
{
    if (!optDescription_.empty()) 
    {
        if (!skipDescrType_)
            return optDescription_ + " " + type;
        else
            return optDescription_ + " " /*+ QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm")*/;
    }

    std::string res = "";
    if (!deviceName_.empty())
        res += deviceName_ + " -";

    if (!skipDescrType_)
        res += " " + type;

    res += " " /*+ QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm")*/;
    return res;
}

bool QubyxProfile::isLabProfile()
{
    return profile_.m_Header.colorSpace == icSigLabData
        || profile_.m_Header.pcs == icSigLabData
        || inColorSpace_ == icSigLabData
        || outColorSpace_ == icSigLabData;
}

CIccTag* QubyxProfile::getIccTag(icTagSignature signature)
{
    return profile_.FindTag(signature);
}

void QubyxProfile::calcChromaticAdaptation(icFloatNumber* PCS, icFloatNumber* src, icFloatNumber* resMatrix, ChromaType type/*=ChromaBradford*/)
{
    // linearized Bradford transformation, see ICC spec Annex E
    icFloatNumber fwdLinBfd[] =
    {
        (icFloatNumber)0.8951,
        (icFloatNumber)0.2664,
        (icFloatNumber)-0.1614,
        (icFloatNumber)-0.7502,
        (icFloatNumber)1.7135,
        (icFloatNumber)0.0367,
        (icFloatNumber)0.0389,
        (icFloatNumber)-0.0685,
        (icFloatNumber)1.0296
    };

    // inverse is from Mathematica since the spec doesn't provide it
    icFloatNumber invLinBfd[] =
    {
        (icFloatNumber)0.9869929,
        (icFloatNumber)-0.1470543,
        (icFloatNumber)0.1599627,
        (icFloatNumber)0.4323053,
        (icFloatNumber)0.5183603,
        (icFloatNumber)0.0492912,
        (icFloatNumber)-0.00852866,
        (icFloatNumber)0.0400428,
        (icFloatNumber)0.9684867
    };

    // CIECAM/CAT02 forward matrix
    icFloatNumber fwdCAT02[] =
    {
        (icFloatNumber)0.7328,
        (icFloatNumber)0.4296,
        (icFloatNumber)-0.1624,
        (icFloatNumber)-0.7036,
        (icFloatNumber)1.6975,
        (icFloatNumber)0.0061,
        (icFloatNumber)0.0030,
        (icFloatNumber)0.0136,
        (icFloatNumber)0.9834
    };

    // inverse is from MatLab
    icFloatNumber invCAT02[] =
    {
        (icFloatNumber)1.096123820835514,
        (icFloatNumber)-0.278869000218287,
        (icFloatNumber)0.182745179382773,
        (icFloatNumber)0.454369041975359,
        (icFloatNumber)0.473533154307412,
        (icFloatNumber)0.072097803717229,
        (icFloatNumber)-0.009627608738429,
        (icFloatNumber)-0.005698031216113,
        (icFloatNumber)1.015325639954543
    };

    //von Kries transformation
    icFloatNumber fwdvKr[] =
    {
        (icFloatNumber)0.4002400,
        (icFloatNumber)0.7076,
        (icFloatNumber)-0.08081,
        (icFloatNumber)-0.2263,
        (icFloatNumber)1.16532,
        (icFloatNumber)0.0457,
        (icFloatNumber)0.,
        (icFloatNumber)0.,
        (icFloatNumber)0.91822
    };

    // inversed von Kries matrix
    icFloatNumber invvKr[] =
    {
        (icFloatNumber)1.8599364,
        (icFloatNumber)-1.1293816,
        (icFloatNumber)0.2198974,
        (icFloatNumber)0.3611914,
        (icFloatNumber)0.6388125,
        (icFloatNumber)-0.0000064,
        (icFloatNumber)0.,
        (icFloatNumber)0.,
        (icFloatNumber)1.0890636
    };

    icFloatNumber* fwd = fwdLinBfd;
    icFloatNumber* inv = invLinBfd;

    switch (type)
    {
    case ChromaBradford:
        fwd = fwdLinBfd;
        inv = invLinBfd;
        break;

    case ChromaCAT02:
        fwd = fwdCAT02;
        inv = invCAT02;
        break;

    case ChromaVonKries:
        fwd = fwdvKr;
        inv = invvKr;
        break;
    }


    icFloatNumber rhoPCS = fwd[0] * PCS[0] + fwd[1] * PCS[1] + fwd[2] * PCS[2];
    icFloatNumber gamPCS = fwd[3] * PCS[0] + fwd[4] * PCS[1] + fwd[5] * PCS[2];
    icFloatNumber betPCS = fwd[6] * PCS[0] + fwd[7] * PCS[1] + fwd[8] * PCS[2];
    icFloatNumber rhoSrc = fwd[0] * src[0] + fwd[1] * src[1] + fwd[2] * src[2];
    icFloatNumber gamSrc = fwd[3] * src[0] + fwd[4] * src[1] + fwd[5] * src[2];
    icFloatNumber betSrc = fwd[6] * src[0] + fwd[7] * src[1] + fwd[8] * src[2];
    icFloatNumber scaling[9];
    scaling[0] = rhoPCS / rhoSrc;
    scaling[1] = 0;
    scaling[2] = 0;
    scaling[3] = 0;
    scaling[4] = gamPCS / gamSrc;
    scaling[5] = 0;
    scaling[6] = 0;
    scaling[7] = 0;
    scaling[8] = betPCS / betSrc;
    icFloatNumber tmp[9];
    icMatrixMultiply3x3(tmp, scaling, fwd);
    icMatrixMultiply3x3(resMatrix, inv, tmp);
}

void QubyxProfile::perceptualTransform(icFloatNumber* point, icFloatNumber* white, icFloatNumber* black)
{
    for (int i = 0;i < 3;i++) {
        point[i] = point[i] * (1 - black[i] / white[i]) + black[i];
    }
}

void QubyxProfile::setColorSpaces(icColorSpaceSignature inColorSpace, icColorSpaceSignature outColorSpace)
{
    inColorSpace_ = inColorSpace;
    outColorSpace_ = outColorSpace;
}

void QubyxProfile::setDeviceName(std::string deviceName)
{
    deviceName_ = deviceName;
    skipDescrType_ = false;
}

void QubyxProfile::addPointTag(icInt32Number* point, icSignature tag)
{
    CIccTagXYZ* PointTag = new CIccTagXYZ;

    (*PointTag)[0].X = point[0];
    (*PointTag)[0].Y = point[1];
    (*PointTag)[0].Z = point[2];

    profile_.DeleteTag(tag);
    profile_.AttachTag(tag, PointTag);
}

bool QubyxProfile::readPointTag(icFloatNumber* point, icSignature tag)
{
    icInt32Number intPt[3];

    if (!readPointTag(intPt, tag))
        return false;

    for (int i = 0;i < 3;++i)
        point[i] = icFtoD(intPt[i]);

    return true;
}

bool QubyxProfile::readPointTag(icInt32Number* point, icSignature tag)
{
    CIccTagXYZ* xyzTag = dynamic_cast<CIccTagXYZ*>(profile_.FindTag(tag));
    if (!xyzTag)
        return false;
    if (!xyzTag->GetSize())
        return false;

    icXYZNumber xyz = (*xyzTag)[0];
    point[0] = xyz.X;
    point[1] = xyz.Y;
    point[2] = xyz.Z;

    return true;
}

void QubyxProfile::addTRCtagDirect(icSignature tagname, const std::vector<long double>& values)
{
    CIccTagCurve* TRCTag = new CIccTagCurve(values.size());

    for (unsigned i = 0;i < values.size(); ++i)
        (*TRCTag)[i] = values[i];

    profile_.DeleteTag(tagname);
    profile_.AttachTag(tagname, TRCTag);
}

template <class T>
bool QubyxProfile::applyChromaticAdaptation(long double m[9], T XYZ[3])
{
    T X, Y, Z;
    X = m[0] * XYZ[0] + m[1] * XYZ[1] + m[2] * XYZ[2];
    Y = m[3] * XYZ[0] + m[4] * XYZ[1] + m[5] * XYZ[2];
    Z = m[6] * XYZ[0] + m[7] * XYZ[1] + m[8] * XYZ[2];

    XYZ[0] = X;
    XYZ[1] = Y;
    XYZ[2] = Z;

    return true;
}

template <typename T>
bool QubyxProfile::applyChromaticAdaptation(const std::vector<double>& m, T XYZ[3])
{
    if (m.size() < 9)
        return false;

    T X, Y, Z;
    X = m[0] * XYZ[0] + m[1] * XYZ[1] + m[2] * XYZ[2];
    Y = m[3] * XYZ[0] + m[4] * XYZ[1] + m[5] * XYZ[2];
    Z = m[6] * XYZ[0] + m[7] * XYZ[1] + m[8] * XYZ[2];

    XYZ[0] = X;
    XYZ[1] = Y;
    XYZ[2] = Z;

    return true;
}

template <typename T>
bool QubyxProfile::applyChromaticAdaptation(const std::vector<double>& m, std::vector<T>& XYZ)
{
    if (m.size() < 9)
        return false;

    T X, Y, Z;
    X = m[0] * XYZ[0] + m[1] * XYZ[1] + m[2] * XYZ[2];
    Y = m[3] * XYZ[0] + m[4] * XYZ[1] + m[5] * XYZ[2];
    Z = m[6] * XYZ[0] + m[7] * XYZ[1] + m[8] * XYZ[2];

    XYZ[0] = X;
    XYZ[1] = Y;
    XYZ[2] = Z;

    return true;
}

void QubyxProfile::fillCurveTag(CIccTagCurve* curve, Curve& values)
{
    unsigned sz = values.size();
    if (sz > 2 && curve) {
        curve->SetSize(sz, icInitNone);
        for (unsigned j = 0;j < sz;j++)
            (*curve)[j] = values[j];
    }
}

void QubyxProfile::fillCurveTagLinear(CIccTagCurve* curve)
{
    if (curve) 
    {
        curve->SetSize(2);
        (*curve)[0] = 0;
        (*curve)[1] = 1;
    }
}

void QubyxProfile::fillParamCurveTag(CIccTagParametricCurve* curve, const QubyxProfile::ParamCurve& values)
{
    curve->SetFunctionType(values.type);
    for (unsigned i = 0;i < values.params.size();++i) {
        (*curve)[i] = values.params[i];
    }
}

void QubyxProfile::fillParamCurveTagLinear(CIccTagParametricCurve* curve)
{
    curve->SetFunctionType(0);
    (*curve)[0] = 1;
}

bool QubyxProfile::LoadFromFile()
{
    CIccFileIO in;
    bool isOpen = false;
    isOpen = in.Open(filename_.c_str(), "r");

    if (isOpen) 
    {
        bool res = profile_.Read(&in);
        in.Close();

        inColorSpace_ = profile_.m_Header.colorSpace;
        outColorSpace_ = profile_.m_Header.pcs;

        spec_ = (profile_.m_Header.version < icVersionNumberV4) ? ICCSpec::ICCv2 : ICCSpec::ICCv4;

        return res;
    }
    else {
        return false;
    }
}

bool QubyxProfile::LoadFromMemory(unsigned char* buf, size_t size)
{
    CIccMemIO in;
    in.Attach(buf, size, false);
    bool res = profile_.Read(&in);

    inColorSpace_ = profile_.m_Header.colorSpace;
    outColorSpace_ = profile_.m_Header.pcs;

    return res;
}

bool QubyxProfile::SaveToMemory(unsigned char*& buf, size_t& size)
{
    size = profile_.m_Header.size;
    buf = new unsigned char[size];
    CIccMemIO out;
    out.Attach(buf, size, true);

    bool res = profile_.Write(&out);

    return res;
}

bool QubyxProfile::validate()
{
#ifdef _DEBUG

    std::string validationReport;
    icValidateStatus validationStatus = profile_.Validate(validationReport);

    switch (validationStatus)
    {
    case icValidateOK: break;
    case icValidateWarning: break;
    case icValidateNonCompliant: break;
    }

    return validationStatus == icValidateOK || validationStatus == icValidateWarning;

#endif
    return true;
}

int QubyxProfile::deviceColorDimention()
{
    return icGetSpaceSamples(profile_.m_Header.colorSpace);
}

CIccMinMaxEval::CIccMinMaxEval()
{
    minDE1 = minDE2 = 10000;
    maxDE1 = maxDE2 = -1;
    sum1 = sum2 = 0;
    num1 = num2 = 0;

    memset(&maxLab1[0], 0, sizeof(maxLab1));
    memset(&maxLab2[0], 0, sizeof(maxLab2));

    memset(&maxPixel1[0], 0, sizeof(maxPixel1));
    memset(&maxPixel2[0], 0, sizeof(maxPixel2));
}

void CIccMinMaxEval::Compare(icFloatNumber* pixel, icFloatNumber* deviceLab, icFloatNumber* lab1, icFloatNumber* lab2)
{
    icFloatNumber DE1 = icDeltaE(deviceLab, lab1);
    icFloatNumber DE2 = icDeltaE(lab1, lab2);

    if (DE1 < minDE1) {
        minDE1 = DE1;
    }

    if (DE1 > maxDE1) {
        maxDE1 = DE1;
        memcpy(&maxLab1[0], deviceLab, sizeof(maxLab1));
        memcpy(&maxPixel1[0], pixel, sizeof(maxPixel1));
    }

    if (DE2 < minDE2) {
        minDE2 = DE2;
    }

    if (DE2 > maxDE2) {
        maxDE2 = DE2;
        memcpy(&maxLab2[0], deviceLab, sizeof(maxLab2));
        memcpy(&maxPixel2[0], pixel, sizeof(maxPixel2));
    }

    sum1 += DE1;
    num1 += 1.0;

    sum2 += DE2;
    num2 += 1.0;
}

void QubyxProfile::denySaving(bool deny/*=true*/)
{
    savable_ = !deny;
}

void QubyxProfile::setLuminance(double lum)
{
    CIccTagXYZ* PointTag = new CIccTagXYZ;

    (*PointTag)[0].X = icDtoF(0.0);
    (*PointTag)[0].Y = icDtoF(lum);
    (*PointTag)[0].Z = icDtoF(0.0);

    profile_.DeleteTag(icSigLuminanceTag);
    profile_.AttachTag(icSigLuminanceTag, PointTag);
}

bool QubyxProfile::getLuminance(double& resLum) const
{

    CIccTagXYZ* col = dynamic_cast<CIccTagXYZ*>(profile_.FindTag(icSigLuminanceTag));
    if (!col)
        return false;

    resLum = icFtoD((*col)[0].Y);

    return true;
}

std::vector<double> QubyxProfile::getChromaticAdaptationAsVector() const
{
    CIccTagS15Fixed16* chroma = dynamic_cast<CIccTagS15Fixed16*>(profile_.FindTag(icSigChromaticAdaptationTag));
    if (!chroma || chroma->GetSize() < 9)
        return std::vector<double>();

    std::vector<double> res(9);

    for (int i = 0;i < 9;i++)
        res[i] = icFtoD((*chroma)[i]);

    return res;
}

std::vector<double> QubyxProfile::getRevertChromaticAdaptationAsVector() const
{
    CIccTagS15Fixed16* chroma = dynamic_cast<CIccTagS15Fixed16*>(profile_.FindTag(icSigChromaticAdaptationTag));
    if (!chroma || chroma->GetSize() < 9)
        return std::vector<double>();

    icFloatNumber contents[9];
    for (int i = 0;i < 9;i++)
        contents[i] = icFtoD((*chroma)[i]);
    icMatrixInvert3x3(contents);

    std::vector<double> res(9);
    for (int i = 0;i < 9;i++)
        res[i] = contents[i];

    return res;
}

bool QubyxProfile::haveTag(icSignature tagName)
{
    return (profile_.FindTag(tagName) != NULL);
}

bool QubyxProfile::haveTagInCLUT(icSignature clutName, ClutElement element)
{
    CIccMBB* a2b = dynamic_cast<CIccMBB*>(profile_.FindTag(clutName));
    if (!a2b)
        return false;

    switch (element)
    {
    case ClutElement::curveA: {
        CIccCurve** cc = a2b->GetCurvesA();
        if (!cc)
            return false;
        CIccTagCurve* c = dynamic_cast<CIccTagCurve*>(*cc);
        if (!c)
            return false;
        return (c->GetSize() > 1);
        break;
    }

    case ClutElement::curveB: {
        CIccCurve** cc = a2b->GetCurvesB();
        if (!cc)
            return false;
        CIccTagCurve* c = dynamic_cast<CIccTagCurve*>(*cc);
        if (!c)
            return false;
        return (c->GetSize() > 1);
        break;
    }

    case ClutElement::curveM: {
        CIccCurve** cc = a2b->GetCurvesM();
        if (!cc)
            return false;
        CIccTagCurve* c = dynamic_cast<CIccTagCurve*>(*cc);
        if (!c)
            return false;
        return (c->GetSize() > 1);
        break;
    }

    case ClutElement::Matrix:
        return (a2b->GetMatrix() != NULL);
        break;

    case ClutElement::Lut:
        return (a2b->GetCLUT() != NULL);
        break;
    }
    return false;
}

void QubyxProfile::setOptionalDescription(std::string text, bool useType)
{
    optDescription_ = text;
    skipDescrType_ = !useType;
}

void QubyxProfile::setICCspecification(QubyxProfile::ICCSpec spec)
{
    spec_ = spec;
}

QubyxProfile::ICCSpec QubyxProfile::ICCspecification()
{
    return spec_;
}

std::string QubyxProfile::ICCspecificationAsString()
{
    switch (spec_)
    {
    case ICCSpec::ICCv2:
        return "2.1";
    case ICCSpec::ICCv4:
        return "4.0";
    }
    return "4.0";
}

int QubyxProfile::ICCspecificationAsInt()
{
    switch (spec_)
    {
    case ICCSpec::ICCv2:
        return 21;
    case ICCSpec::ICCv4:
        return 40;
    }
    return 40;
}

void QubyxProfile::setTextTag(icSignature sig, std::string text)
{
    switch (spec_)
    {
    case ICCSpec::ICCv2:
    {
        profile_.DeleteTag(sig);

        if (sig == icSigCopyrightTag || sig == icSigCharTargetTag) 
        {
            CIccTagText* tag = new CIccTagText;
            tag->SetText(text.c_str());
            profile_.AttachTag(sig, tag);
        }
        else 
        {
            CIccTagTextDescription* tag = new CIccTagTextDescription;
            tag->SetText(text.c_str());
            profile_.AttachTag(sig, tag);
        }
    }
    break;

    case ICCSpec::ICCv4:
    {
        profile_.DeleteTag(sig);
        if (sig == icSigCharTargetTag) 
        {
            CIccTagText* tag = new CIccTagText;
            tag->SetText(text.c_str());
            profile_.AttachTag(sig, tag);
        }
        else 
        {
            CIccLocalizedUnicode USAEnglish;
            USAEnglish.SetText(text.c_str());
            CIccTagMultiLocalizedUnicode* tag = new CIccTagMultiLocalizedUnicode;
            tag->m_Strings = new CIccMultiLocalizedUnicode; // dtor does deletion
            tag->m_Strings->push_back(USAEnglish);
            profile_.AttachTag(sig, tag);
        }
    }
    break;
    }
}

std::string QubyxProfile::getTextTag(icSignature sig) const
{
    CIccTag* tag = profile_.FindTag(sig);

    if (!tag)
        return "";

    if (dynamic_cast<CIccTagMultiLocalizedUnicode*>(tag)) {
        CIccTagMultiLocalizedUnicode* mlTag = dynamic_cast<CIccTagMultiLocalizedUnicode*>(tag);
        if (!mlTag->m_Strings)
            return "";

        icChar buff[4096];
        std::string res;
        for (auto itr = mlTag->m_Strings->begin();itr != mlTag->m_Strings->end();++itr) {
            itr->GetAnsi(buff, 4096);
            res = res + std::string(buff);
        }
        return res;
    }

    if (dynamic_cast<CIccTagTextDescription*>(tag)) {
        CIccTagTextDescription* tdTag = dynamic_cast<CIccTagTextDescription*>(tag);
        return tdTag->GetText();
    }

    if (dynamic_cast<CIccTagText*>(tag)) {
        CIccTagText* tTag = dynamic_cast<CIccTagText*>(tag);
        return tTag->GetText();
    }

    return "";
}

template bool QubyxProfile::applyChromaticAdaptation<float>(long double m[9], float XYZ[3]);
template bool QubyxProfile::applyChromaticAdaptation<double>(long double m[9], double XYZ[3]);
template bool QubyxProfile::applyChromaticAdaptation<long double>(long double m[9], long double XYZ[3]);
template bool QubyxProfile::applyChromaticAdaptation<float>(const std::vector<double>& m, float XYZ[3]);
template bool QubyxProfile::applyChromaticAdaptation<double>(const std::vector<double>& m, double XYZ[3]);
template bool QubyxProfile::applyChromaticAdaptation<long double>(const std::vector<double>& m, long double XYZ[3]);
template bool QubyxProfile::applyChromaticAdaptation<float>(const std::vector<double>& m, std::vector<float>& XYZ);
template bool QubyxProfile::applyChromaticAdaptation<double>(const std::vector<double>& m, std::vector<double>& XYZ);
