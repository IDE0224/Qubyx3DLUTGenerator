/*
 * Author: QUBYX Software Technologies LTD HK
 * Copyright: QUBYX Software Technologies LTD HK
 */

#ifndef QUBYXPROFILECHAINS_H
#define QUBYXPROFILECHAINS_H

#include <vector>
#include <memory>

#include "QubyxProfile.h"


class QubyxProfileChain
{
public:

    enum class SpaceType
    {
        DeviceSpecific, //RGB, CMYK, CMYK+
        XYZ,
        Lab
    };

    enum class RI
    {
        Perceptual,
        RelativeColorimetric,
        Saturation,
        AbsoluteColorimetric,
        RealisticColorimetric,
        RealisticColorimetricWithLuminance
    };

    QubyxProfileChain();
    QubyxProfileChain(SpaceType in, SpaceType out, RI renderingIntent = RI::AbsoluteColorimetric);

    void setRenderingIntent(RI renderingIntent);
    void setTransformationType(SpaceType in, SpaceType out);

    void clearChain();
    bool addProfile(QubyxProfile* profile);
    bool addProfile(QubyxProfile* profile, RI renderingIntent);
    bool addProfile(const QubyxProfile& profile);
    bool addProfile(const QubyxProfile& profile, RI renderingIntent);
    bool addProfiles(const std::vector<QubyxProfile*>& profiles);
    bool addProfiles(const std::vector<QubyxProfile*>& profiles, RI renderingIntent);

    bool isChainComplete();

    template<typename T>
    bool transform(const std::vector<T>& in, std::vector<T>& out);

    static std::shared_ptr<QubyxProfileChain> singleProfileChain(QubyxProfile* profile, SpaceType in, SpaceType out, RI renderingIntent = RI::AbsoluteColorimetric);
    static std::shared_ptr<QubyxProfileChain> singleProfileChain(const QubyxProfile& profile, SpaceType in, SpaceType out, RI renderingIntent = RI::AbsoluteColorimetric);

private:
    RI ri_;
    SpaceType in_, out_;
    bool started_;
    bool hasLastChad_;
    std::vector<double> lastChad_;
    bool hasLastLuminance_;
    double lastLuminance_;
    SpaceType lastOutput_;

    struct CMM
    {
        bool hasInputChad_, hasOutputChad_;
        bool hasInputLuminance_, hasOutputLuminance_;
        std::vector<double> inputChad_, outputChad_;
        double inputLum_, outputLum_;
        SpaceType in_, out_;

        std::unique_ptr<CIccCmm> cmm_;

        CMM(SpaceType in, SpaceType out);

        void changeOutput(SpaceType out);
    };
    std::vector<CMM> cmms_;

    static icRenderingIntent iccProfLibRI(RI renderingIntent);
    static icColorSpaceSignature iccProfLibSpace(SpaceType space);
    static SpaceType spaceType(icColorSpaceSignature space);
    static int colorsCountByType(icColorSpaceSignature sig);
};
#endif // QUBYXPROFILECHAINS_H
