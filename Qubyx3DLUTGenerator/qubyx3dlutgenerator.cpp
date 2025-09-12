/* 
 * Author: QUBYX Software Technologies LTD HK
 * Copyright: QUBYX Software Technologies LTD HK
 */

#include "qubyx3dlutgenerator.h"

#include <cmath>

#include "QubyxProfile.h"
#include "qubyxprofilechain.h"

A3dLut_Status __attribute__((visibility("default"))) generate3dLut(char* ga_profile, char* display_profile, int grid, unsigned int *rlut, unsigned int *glut, unsigned int *blut)
{
    if(grid<2)
        return A3dLut_Error_WrongGridValue;

    if(rlut==nullptr || glut==nullptr || blut==nullptr)
        return A3dLut_Error_NullPointerForOutput;

    QubyxProfile ga(ga_profile);
    if(!ga.LoadFromFile())
        return A3dLut_Error_CantOpenGA;

    QubyxProfile display(display_profile);
    if(!display.LoadFromFile())
        return A3dLut_Error_CantOpenDisplay;

    QubyxProfileChain chain(QubyxProfileChain::SpaceType::DeviceSpecific,
                            QubyxProfileChain::SpaceType::DeviceSpecific,
                            QubyxProfileChain::RI::RealisticColorimetricWithLuminance);
    chain.addProfile(&ga);
    chain.addProfile(&display);

    unsigned index = 0;
    for (int R = 0; R < grid; R++)
        for (int G = 0; G < grid; G++)
            for (int B = 0; B < grid; B++)
            {
                std::vector<double> in(3), out;

                in[0] = R/(grid - 1.0);
                in[1] = G/(grid - 1.0);
                in[2] = B/(grid - 1.0);

                chain.transform(in, out);

                int maxValue = 256*256-1;
                rlut[index] = round(out[0]*maxValue);
                glut[index] = round(out[1]*maxValue);
                blut[index] = round(out[2]*maxValue);

                ++index;
            }

    return A3dLut_Ok;
}

