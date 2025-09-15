/*
 * File:   iccMmodTag.cpp
 * Author: Roman
 *
 * Created on 16 серпня 2013, 14:06
 */

#include "iccMmodTag.h"
#include "IccIO.h"

iccMmodTag::iccMmodTag()
{
}

iccMmodTag::iccMmodTag(const iccMmodTag& orig)
{
    data_ = orig.data_;
}

iccMmodTag& iccMmodTag::operator=(const iccMmodTag& tag)
{
    data_ = tag.data_;
    return *this;
}

iccMmodTag::~iccMmodTag()
{
}

bool iccMmodTag::Read(icUInt32Number size, CIccIO* pIO)
{
    icTagTypeSignature sig;

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

    int bufLen = size - 8;
    unsigned char* buf = new unsigned char[bufLen];

    if (!pIO->Read8(buf, bufLen))
    {
        delete[]buf;
        return false;
    }

    data_ = std::vector<unsigned char>(buf, buf + bufLen);
    delete[]buf;

    return true;
}

bool iccMmodTag::Write(CIccIO* pIO)
{
    icTagTypeSignature sig = GetType();

    if (!pIO)
        return false;

    if (!pIO->Write32(&sig))
        return false;

    if (!pIO->Write32(&m_nReserved))
        return false;

    if (!pIO->Write8(data_.data(), data_.size()))
        return false;

    return true;
}

icValidateStatus iccMmodTag::Validate(icTagSignature sig, std::string& sReport, const CIccProfile* pProfile) const
{
    return icValidateOK;
}

bool iccMmodTag::SetEdid(const unsigned char* edid, unsigned int edidSize)
{
    if (!edid || edidSize < 12)
        return false;

    data_.push_back(0);
    data_.push_back(0);

    data_.push_back(edid[8]);
    data_.push_back(edid[9]);

    data_.push_back(0);
    data_.push_back(0);

    data_.push_back(edid[11]);
    data_.push_back(edid[10]);

    bool snFound = false;
    //try to find serial number
    for (unsigned i = 54;i < 126 && i + 18 <= edidSize;i += 18)
        if (edid[i] == 0 && edid[i + 1] == 0 && edid[i + 2] == 0 && edid[i + 3] == 0xff)
        {
            //save last 4 bytes of serial
            for (int j = 0;j < 4;j++)
                data_.push_back(edid[i + 5 + 8 + j]);
            snFound = true;
        }

    if (!snFound)
    {
        for (int i = 0;i < 4;i++)
            data_.push_back(0);
    }

    for (int i = 0;i < 20;i++)
        data_.push_back(0);

    return true;
}
