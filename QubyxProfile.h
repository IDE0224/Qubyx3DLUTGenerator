/*
 * Author: QUBYX Software Technologies LTD HK
 * Copyright: QUBYX Software Technologies LTD HK
 */

#ifndef QUBYXPROFILE_H
#define	QUBYXPROFILE_H

#include "ICCProfLib/IccProfile.h"
#include "ICCProfLib/IccCmm.h"
#include "ICCProfLib/IccEval.h"

#include <vector>
#include <memory>
#include <string>


class CIccMinMaxEval : public CIccEvalCompare
{
public:
    CIccMinMaxEval();

    void Compare(icFloatNumber* pixel, icFloatNumber* deviceLab, icFloatNumber* lab1, icFloatNumber* lab2);

    icFloatNumber GetMean1() { return sum1 / num1; }
    icFloatNumber GetMean2() { return sum2 / num2; }

    icFloatNumber minDE1, minDE2;
    icFloatNumber maxDE1, maxDE2;

    icFloatNumber maxLab1[3], maxLab2[3];
    icFloatNumber maxPixel1[3], maxPixel2[3];

protected:
    icFloatNumber sum1, sum2;
    icFloatNumber num1, num2;
};

class QubyxProfile
{
public:
    typedef std::vector<long double> Curve;

    struct ParamCurve
    {
        int type;
        std::vector<double> params;
    };

    struct Curves
    {
        Curves() : parametric(false) {}

        std::vector<bool> parametric;

        std::vector<ParamCurve> paramCurves;
        std::vector<Curve> curves;
    };

    enum ChromaType
    {
        ChromaBradford,
        ChromaCAT02,
        ChromaVonKries
    };


    enum class ClutElement
    {
        curveA,
        curveB,
        curveM,
        Matrix,
        Lut
    };

    enum class ICCSpec
    {
        ICCv4,
        ICCv2
    };

private:
    int devDim_;
    std::unique_ptr<CIccCmm> dev2lab_, dev2xyz_, lab2dev_, xyz2dev_;

    long double convertChroma_[9];
    long double convertRevChroma_[9];
    icColorSpaceSignature inColorSpace_, outColorSpace_;

    double maxY_;

    std::string optDescription_;
    bool skipDescrType_;
    ICCSpec spec_;

protected:
    std::string filename_;

    void addPointTag(icInt32Number* point, icSignature tag);
    bool readPointTag(icFloatNumber* point, icSignature tag);
    bool readPointTag(icInt32Number* point, icSignature tag);

    void perceptualTransform(icFloatNumber* point, icFloatNumber* white, icFloatNumber* black);


    void fillCurveTag(CIccTagCurve* curve, Curve& values);
    void fillCurveTagLinear(CIccTagCurve* curve);

    void fillParamCurveTag(CIccTagParametricCurve* curve, const ParamCurve& values);
    void fillParamCurveTagLinear(CIccTagParametricCurve* curve);

    // template<class Tag>
    // ProfileCLUT_Equidistant get3dLUTTag(icTagSignature signature, bool applyChromatic, bool applyLuminance);      

    bool savable_;
    CIccProfile profile_;
    std::string deviceName_;


    void setColorSpaces(icColorSpaceSignature inColorSpace, icColorSpaceSignature outColorSpace);

    /**
     * @brief makeProfileTitle generate string for profile name shown in color management applications
     * @param deviceName name of display/camera/printer
     * @param type some common name for printer type. e.g. "Qubyx printer profile"
     * @return title with the date, type and devicename (see @setDeviceName)
     */
    std::string makeProfileTitle(std::string type);

public:
    QubyxProfile();
    QubyxProfile(std::string profilePath);
    QubyxProfile(const QubyxProfile& other);

    virtual ~QubyxProfile();

    QubyxProfile& operator=(const QubyxProfile& other);

    void setFileName(std::string filename);

    /**
     * Calculate 3x3 matrix for chromatic adaptation. This matrix transfer src to PCS
     * @param PCS pointer to PCS white (usually D50)
     * @param src pointer to src white (real measured and normalized)
     * @param resMatrix pointer to 9 element buffer. Result matrix will be saved
     * @param type type of calculations
     */
    static void calcChromaticAdaptation(icFloatNumber* PCS, icFloatNumber* src, icFloatNumber* resMatrix, ChromaType type = ChromaBradford);

    bool validate();

    void addTRCtagDirect(icSignature tagname, const std::vector<long double>& values);

    bool haveTag(icSignature tagName);
    bool haveTagInCLUT(icSignature clutName, ClutElement element);

    // ProfileCLUT_Equidistant getAToBTag(icTagSignature signature, bool applyChromatic = false, bool applyLuminance = false);
    // ProfileCLUT_Equidistant getBToATag(icTagSignature signature);

    bool LoadFromFile();
    bool LoadFromMemory(unsigned char* buf, size_t size);
    bool SaveToMemory(unsigned char*& buf, size_t& size);

    /**
     * Calculate dimention of device space, eg. for RGB=3, for CMYK=4
     * @return dimention of device color space
     */
    int deviceColorDimention();

    /**
     * Function for checking what is exact file used for load/save.
     * @return full path to the profile
     */
    std::string profilePath();

    /**
     * Used for denying saving of profile to the  disk and applying.
     * @param deny true - to deny, false - to return to normal state
     */
    void denySaving(bool deny = true);

    void setDeviceName(std::string deviceName);

    void setLuminance(double lum);
    bool getLuminance(double& resLum) const;

    std::vector<double> getChromaticAdaptationAsVector() const;
    std::vector<double> getRevertChromaticAdaptationAsVector() const;

    template <class T>
    static bool applyChromaticAdaptation(long double m[9], T XYZ[3]);
    template <typename T>
    static bool applyChromaticAdaptation(const std::vector<double>& m, T XYZ[3]);
    template <typename T>
    static bool applyChromaticAdaptation(const std::vector<double>& m, std::vector<T>& XYZ);

    void setOptionalDescription(std::string text, bool useType = true);

    void setICCspecification(ICCSpec spec);
    ICCSpec ICCspecification();
    std::string ICCspecificationAsString();
    int ICCspecificationAsInt();
    void setTextTag(icSignature sig, std::string text);
    std::string getTextTag(icSignature sig) const;

    bool isLabProfile();

    CIccTag* getIccTag(icTagSignature signature);

    friend class QubyxProfileChain;
};



#endif	/* QUBYXPROFILE_H */

