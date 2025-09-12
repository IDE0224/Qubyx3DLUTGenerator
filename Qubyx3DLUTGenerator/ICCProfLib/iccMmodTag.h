/* 
 * File:   iccMmodTag.h
 * Author: Roman
 *
 * Created on 16 серпня 2013, 14:06
 */

#ifndef ICCMMODTAG_H
#define	ICCMMODTAG_H

#include "IccTagBasic.h"
#include <vector>

#ifdef USESAMPLEICCNAMESPACE
namespace sampleICC {
#endif

enum
{
    icSigMMODType                      = 0x6D6D6F64  /* 'vcgt' */
};

class ICCPROFLIB_API iccMmodTag : public CIccTag
{
public:
    iccMmodTag();
    iccMmodTag(const iccMmodTag& orig);
    iccMmodTag &operator=(const iccMmodTag &tag);
    virtual CIccTag *NewCopy() const { return new iccMmodTag(*this);}
    virtual ~iccMmodTag();

    virtual icTagTypeSignature GetType() { return icTagTypeSignature(icSigMMODType); }
    virtual const icChar *GetClassName() { return "IccMMODTag"; }
    
    bool SetEdid(const unsigned char *edid, unsigned int edidSize);

    virtual bool Read(icUInt32Number size, CIccIO *pIO);
    virtual bool Write(CIccIO *pIO);

    virtual icValidateStatus Validate(icTagSignature sig, std::string &sReport, const CIccProfile* pProfile) const;
private:
    std::vector<unsigned char> data_;

};

#ifdef USESAMPLEICCNAMESPACE
}
#endif

#endif	/* ICCMMODTAG_H */

