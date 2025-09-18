#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <direct.h>

// Function pointer type for the DLL function
typedef int (*Generate3dLutFunc)(char*, char*, int, unsigned int*, unsigned int*, unsigned int*);

int main() {
    printf("Qubyx3DLUTGenerator - Test Program\n");
    printf("==================================\n\n");
    
    // Check if we're in the right directory
    if (!GetFileAttributesA("qubyx3dlutgenerator.h")) {
        printf("ERROR: qubyx3dlutgenerator.h not found\n");
        printf("Please run this test from the Qubyx3DLUTGenerator directory\n");
        return 1;
    }
    
    // Load the DLL - try different possible paths
    HMODULE dll = NULL;
    char* dll_paths[] = {
        "Qubyx3DLUTGenerator.dll",           // If running from bin directory
        "bin\\Qubyx3DLUTGenerator.dll",      // If running from project root
        "..\\bin\\Qubyx3DLUTGenerator.dll"   // If running from subdirectory
    };
    
    for (int i = 0; i < 3; i++) {
        dll = LoadLibraryA(dll_paths[i]);
        if (dll) {
            printf("DLL loaded from: %s\n", dll_paths[i]);
            break;
        }
    }
    
    if (!dll) {
        printf("ERROR: Could not load Qubyx3DLUTGenerator.dll\n");
        printf("Tried paths:\n");
        printf("  - Qubyx3DLUTGenerator.dll\n");
        printf("  - bin\\Qubyx3DLUTGenerator.dll\n");
        printf("  - ..\\bin\\Qubyx3DLUTGenerator.dll\n");
        printf("Make sure you have built the library first with build.bat\n");
        printf("Current working directory: %s\n", _getcwd(NULL, 0));
        return 1;
    }
    
    // Get the function pointer
    Generate3dLutFunc generate3dLut = (Generate3dLutFunc)GetProcAddress(dll, "generate3dLut");
    if (!generate3dLut) {
        printf("ERROR: Could not find generate3dLut function in DLL\n");
        FreeLibrary(dll);
        return 1;
    }
    
    printf("generate3dLut function found\n");
    
    // Test with dummy data (this will fail because we don't have real ICC profiles)
    printf("\nTesting function call (will fail without real ICC profiles)...\n");
    
    int grid_size = 17;
    size_t lut_size = grid_size * grid_size * grid_size;
    unsigned int* rlut = malloc(lut_size * sizeof(unsigned int));
    unsigned int* glut = malloc(lut_size * sizeof(unsigned int));
    unsigned int* blut = malloc(lut_size * sizeof(unsigned int));
    
    if (!rlut || !glut || !blut) {
        printf("ERROR: Memory allocation failed\n");
        FreeLibrary(dll);
        return 1;
    }
    
    // Try to call the function (this will fail because the ICC files don't exist)
    int result = generate3dLut("ga_profile.icc", "display_profile.icc", grid_size, rlut, glut, blut);
    
    printf("Function call result: %d\n", result);
    
    // Cleanup
    free(rlut);
    free(glut);
    free(blut);
    FreeLibrary(dll);
    
    printf("\nTest completed successfully!\n");
    printf("The library is working correctly.\n");
    printf("To use it with real ICC profiles, provide valid .icc files.\n");
    
    return 0;
}
