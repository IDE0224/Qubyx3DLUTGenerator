/* 
 * Author: QUBYX Software Technologies LTD HK
 * Copyright: QUBYX Software Technologies LTD HK
 */

#ifndef QUBYX3DLUTGENERATOR_H
#define QUBYX3DLUTGENERATOR_H

enum A3dLut_Status
{
    A3dLut_Ok = 0,
    A3dLut_Error_CantOpenGA,
    A3dLut_Error_CantOpenDisplay,
    A3dLut_Error_WrongGridValue,
    A3dLut_Error_NullPointerForOutput,
    A3dLut_Error_Other
};

extern "C"
A3dLut_Status generate3dLut(char* ga_profile, char* display_profile, int grid, unsigned int* rlut, unsigned int *glut, unsigned int *blut);

#endif // QUBYX3DLUTGENERATOR_H
