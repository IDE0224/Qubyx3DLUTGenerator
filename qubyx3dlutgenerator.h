/*
 * Author: QUBYX Software Technologies LTD HK
 * Copyright: QUBYX Software Technologies LTD HK
 */

#ifndef QUBYX3DLUTGENERATOR_H
#define QUBYX3DLUTGENERATOR_H

enum Q3dLut_Status
{
    Q3dLut_Ok = 0,
    Q3dLut_Error_CantOpenGA,
    Q3dLut_Error_CantOpenDisplay,
    Q3dLut_Error_WrongGridValue,
    Q3dLut_Error_NullPointerForOutput,
    Q3dLut_Error_Other
};

extern "C" __declspec(dllexport)
Q3dLut_Status generate3dLut(char* ga_profile, char* display_profile, int grid, unsigned int* rlut, unsigned int* glut, unsigned int* blut);

#endif // QUBYX3DLUTGENERATOR_H
