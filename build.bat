@echo off
REM Qubyx3DLUTGenerator Build Script
REM Author: QUBYX Software Technologies LTD HK
REM Copyright: QUBYX Software Technologies LTD HK

echo Building Qubyx3DLUTGenerator...

REM Check if Visual Studio compiler is available
where cl >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Visual Studio compiler not found
    echo Please run this from a Visual Studio Developer Command Prompt
    echo or install Visual Studio Build Tools
    echo.
    echo To set up Visual Studio environment manually:
    echo 1. Open "Developer Command Prompt for VS 2019 (or 2022)" from Start Menu
    echo 2. Navigate to this directory
    echo 3. Run this script again
    pause
    exit /b 1
)

REM Create output directories
if not exist bin mkdir bin
if not exist obj mkdir obj

REM Clean up old object files
del obj\*.obj 2>nul
del *.obj 2>nul

REM Compile all source files
echo Compiling source files...
cl /LD /std:c++14 /EHsc /O2 /W3 /D_CRT_SECURE_NO_WARNINGS /DWINDOWS_IGNORE_PACKING_MISMATCH /DNOMINMAX ^
    /Foobj\ ^
    qubyx3dlutgenerator.cpp ^
    QubyxProfile.cpp ^
    qubyxprofilechain.cpp ^
    ICCProfLib\*.cpp ^
    /Fe:bin\Qubyx3DLUTGenerator.dll ^
    /link /SUBSYSTEM:WINDOWS /DEF:Qubyx3DLUTGenerator.def

if %errorlevel% neq 0 (
    echo ERROR: Compilation failed
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo.
echo Output files:
echo - bin\Qubyx3DLUTGenerator.dll
echo - bin\Qubyx3DLUTGenerator.lib
echo.

REM Test the DLL
echo Testing DLL...
dumpbin /exports bin\Qubyx3DLUTGenerator.dll | findstr generate3dLut
if %errorlevel% equ 0 (
    echo SUCCESS: generate3dLut function exported correctly
) else (
    echo WARNING: generate3dLut function not found in exports
)

echo.
echo Build script completed.
pause
