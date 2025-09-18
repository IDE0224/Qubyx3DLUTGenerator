@echo off
REM Test script for Qubyx3DLUTGenerator
REM Author: QUBYX Software Technologies LTD HK
REM Copyright: QUBYX Software Technologies LTD HK

echo Testing Qubyx3DLUTGenerator...
echo Current directory: %CD%
echo.

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

REM Check if the library exists
if not exist "bin\Qubyx3DLUTGenerator.dll" (
    echo ERROR: Qubyx3DLUTGenerator.dll not found
    echo.
    echo Please build the library first by running:
    echo   build.bat
    echo.
    echo Then run this test again.
    pause
    exit /b 1
)

REM Compile test program
echo Compiling test program...
cl /std:c++14 /EHsc /O2 /W3 /D_CRT_SECURE_NO_WARNINGS /DNOMINMAX ^
    /Foobj\ ^
    test.c ^
    /Fe:bin\test.exe ^
    /link

if %errorlevel% neq 0 (
    echo ERROR: Test compilation failed
    pause
    exit /b 1
)

echo.
echo Running test...
echo ================
cd bin
test.exe
cd ..

echo.
echo Test completed.
pause
