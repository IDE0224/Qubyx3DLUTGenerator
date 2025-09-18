/*
 * File:   IccVCGTTag.cpp
 * Author: Vitaly Bondar
 *
 * Created on 15.11.2012
 */

#include "IccVcgtTag.h"

#include "IccIO.h"

#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <cmath>
#include <algorithm>

// Prevent Windows min/max macros from conflicting with std::min/std::max
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef USESAMPLEICCNAMESPACE
namespace sampleICC {
#endif

const icUInt32Number cmVideoCardGammaTableType = 0;
const icUInt32Number cmVideoCardGammaFormulaType = 1;


/**
****************************************************************************
* Name: IccVCGTTag::IccVCGTTag
*
* Purpose: Constructor
*****************************************************************************
*/
IccVCGTTag::IccVCGTTag() : channels_(0), entryCount_(0), curves_(NULL)
{
}

/**
****************************************************************************
* Name: IccVCGTTag::IccVCGTTag
*
* Purpose: Copy constructor
*
*****************************************************************************
*/
IccVCGTTag::IccVCGTTag(const IccVCGTTag& orig)
{
    channels_ = orig.channels_;
    entryCount_ = orig.entryCount_;
    if (channels_ && entryCount_ && orig.curves_)
    {
        curves_ = (icUInt16Number*)calloc(channels_ * entryCount_, sizeof(icUInt16Number));
        memcpy(curves_, orig.curves_, channels_ * entryCount_ * sizeof(icUInt16Number));
    }
    else
        channels_ = entryCount_ = 0;
}

/**
****************************************************************************
* Name: IccVCGTTag::operator=
*
* Purpose: copy operator
*****************************************************************************
*/
IccVCGTTag& IccVCGTTag::operator=(const IccVCGTTag& tag)
{
    if (&tag == this)
        return *this;

    channels_ = tag.channels_;
    entryCount_ = tag.entryCount_;
    if (channels_ && entryCount_ && tag.curves_)
    {
        curves_ = (icUInt16Number*)calloc(channels_ * entryCount_, sizeof(icUInt16Number));
        memcpy(curves_, tag.curves_, channels_ * entryCount_ * sizeof(icUInt16Number));
    }
    else
        channels_ = entryCount_ = 0;

    return *this;
}

/**
****************************************************************************
* Name: IccVCGTTag::~IccVCGTTag
*
* Purpose: Destructor
*
*****************************************************************************
*/
IccVCGTTag::~IccVCGTTag()
{
    if (curves_)
        free(curves_);
}

/**
****************************************************************************
* Name: IccVCGTTag::SetSize
*
* Purpose: Sets the size of the correction lut.
*
* Args:
*  channels - number of channels,
*  entryCount - number of elements per channel
*****************************************************************************
*/
void IccVCGTTag::SetSize(icUInt16Number channels, icUInt16Number entryCount)
{
    if (channels_ == channels && entryCount_ == entryCount)
        return;

    if (channels == 0 || entryCount == 0)
        return;

    channels_ = channels;
    entryCount_ = entryCount;
    if (curves_)
        free(curves_);
    curves_ = (icUInt16Number*)malloc(channels_ * entryCount_ * sizeof(icUInt16Number));
}

/**
****************************************************************************
* Name: IccVCGTTag::operator[]
*
* Purpose: Array-like access to channels
*
* Return:
*  pointer to the array of values which represent channel
*****************************************************************************
*/
icUInt16Number* IccVCGTTag::operator[](icUInt32Number channel)
{
    return curves_ + channel * entryCount_;
}

/**
****************************************************************************
* Name: IccVCGTTag::Read
*
* Purpose: Read in the tag contents into a data block
*
* Args:
*  size - # of bytes in tag,
*  pIO - IO object to read tag from
*
* Return:
*  true = successful, false = failure
*****************************************************************************
*/
bool IccVCGTTag::Read(icUInt32Number size, CIccIO* pIO)
{
    icTagTypeSignature sig;

    if (sizeof(icTagTypeSignature) +
        2 * sizeof(icUInt32Number) +
        3 * sizeof(icUInt16Number) > size)
        return false;

    if (!pIO)
    {
        return false;
    }

    if (!pIO->Read32(&sig))
    {
        return false;
    }

    if (!pIO->Read32(&m_nReserved))
    {
        return false;
    }

    icUInt32Number type;

    if (!pIO->Read32(&type))
    {
        return false;
    }


    if (type == cmVideoCardGammaTableType)
    {
        //sizes
        icUInt16Number ch, ec, esz;
        if (!pIO->Read16(&ch))
            return false;

        if (!pIO->Read16(&ec))
            return false;

        if (!pIO->Read16(&esz))
            return false;

        SetSize(ch, ec);

        if (curves_)
            free(curves_);
        curves_ = (icUInt16Number*)malloc(channels_ * entryCount_ * sizeof(icUInt16Number));
        if (esz == sizeof(icUInt16Number))
        {
            if (pIO->Read16(curves_, channels_ * entryCount_) != (icInt32Number)(channels_ * entryCount_))
                return false;
        }
        else
        {
            icInt8Number t;
            const unsigned long ONE_COEF_16 = 1 << 16;
            const unsigned long ONE_COEF_esz = 1 << (8 * esz);
            for (unsigned i = 0;i < channels();++i)
                for (unsigned j = 0;j < entryCount();++j)
                {
                    unsigned long long v = 0;
                    for (unsigned k = 0;k < esz;++k)
                    {
                        if (pIO->Read8(&t, 1) != 1)
                            return false;
                        v = v * 2 + t;
                    }
                    curves_[i * entryCount_ + j] = icUInt16Number((long double)(v) / (ONE_COEF_esz - 1) * (ONE_COEF_16 - 1));
                }

        }
    }
    else if (type == cmVideoCardGammaFormulaType)
    {
        SetSize(3, 256);

        double vcgt[3][3]; // red gamma, red min, red max; green ...; blue ...
        const unsigned long ONE_COEF = 1 << 16;
        for (unsigned ch = 0; ch < 3; ch++)
            for (unsigned i = 0; i < 3; i++)
            {
                unsigned long value;
                if (!pIO->Read32(&value))
                {
                    return false;
                }

                vcgt[ch][i] = value / static_cast<double>(ONE_COEF);
            }

        if (curves_)
            free(curves_);
        curves_ = (icUInt16Number*)malloc(channels_ * entryCount_ * sizeof(icUInt16Number));

        for (int ch = 0;ch < 3;ch++)
        {
            if (vcgt[ch][0] < 0.1 || vcgt[ch][0] > 10.0)
            {
                return false;
            }

            double range = vcgt[ch][2] - vcgt[ch][1];

            for (int i = 0; i < entryCount_; i++)
            {
                double y = vcgt[ch][1] + range * std::pow(i / 255.0, vcgt[ch][0]);
                y = std::min(y, 1.0);
                curves_[ch * entryCount_ + i] = static_cast<icUInt16Number>(y * (ONE_COEF - 1));
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

/**
****************************************************************************
* Name: IccVCGTTag::Write
*
* Purpose: Write the tag to a file
*
* Args:
*  pIO - The IO object to write tag to.
*
* Return:
*  true = succesful, false = failure
*****************************************************************************
*/
bool IccVCGTTag::Write(CIccIO* pIO)
{
    icTagTypeSignature sig = GetType();

    if (!pIO)
        return false;

    if (!pIO->Write32(&sig))
        return false;

    if (!pIO->Write32(&m_nReserved))
        return false;

    // VideoCardGamma
    icUInt32Number tagtype = cmVideoCardGammaTableType;
    if (!pIO->Write32(&tagtype))
        return false;

    // VideoCardGammaTable
    if (!pIO->Write16(&channels_))
        return false;

    if (!pIO->Write16(&entryCount_))
        return false;

    icUInt16Number entrysz = sizeof(icUInt16Number);
    if (!pIO->Write16(&entrysz))
        return false;

    if (curves_)
        if (pIO->Write16(curves_, channels_ * entryCount_) != (icInt32Number)(channels_ * entryCount_))
            return false;

    return true;
}

/**
****************************************************************************
* Name: IccVCGTTag::Describe
*
* Purpose: Dump data associated with the tag to a string
*
* Args:
*  sDescription - string to concatenate tag dump to
*****************************************************************************
*/
void IccVCGTTag::Describe(std::string& sDescription)
{
    std::stringstream ss;
    ss << "VCGT LUT" << std::endl;
    for (int i = 0;i < channels_;++i)
    {
        ss << "Channel#" << i << std::endl;
        for (int j = 0;j < entryCount_;++j)
            ss << curves_[j + i * entryCount_] << std::endl;
        ss << std::endl;
    }
    ss << std::endl;
    sDescription += ss.str();
}

/**
******************************************************************************
* Name: IccVCGTTag::Validate
*
* Purpose: Check tag data validity. (not implemented yet)
*
* Args:
*  sig = signature of tag being validated,
*  sReport = String to add report information to
*
* Return:
*  icValidateStatusOK if valid, or other error status.
******************************************************************************
*/
icValidateStatus IccVCGTTag::Validate(icTagSignature sig, std::string& sReport, const CIccProfile* pProfile) const
{
    return icValidateOK;
}

/**
****************************************************************************
* Name: IccVCGTTag::channels
*
* Purpose: get channels number
*
* Return:
*  channels count
*****************************************************************************
*/
icUInt16Number IccVCGTTag::channels() const
{
    return channels_;
}

/**
****************************************************************************
* Name: IccVCGTTag::entryCount
*
* Purpose: get elements count per channel
*
* Return:
*  elements count
*****************************************************************************
*/
icUInt16Number IccVCGTTag::entryCount() const
{
    return entryCount_;
}

#ifdef USESAMPLEICCNAMESPACE
} //namespace sampleICC
#endif
