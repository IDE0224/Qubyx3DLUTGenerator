# Qubyx3DLUTGenerator

A C++ library for generating 3D Look-Up Tables (3DLUTs) from ICC color profiles. This library provides a simple C API for creating color transformation tables used in professional color management workflows.

## Features

- **3DLUT Generation**: Create 3D Look-Up Tables from ICC profiles
- **C API**: Simple C interface for easy integration
- **Windows DLL**: Native Windows dynamic library
- **Open Source**: Licensed under GPL v3
- **ICC Profile Support**: Full support for ICC color profiles
- **Flexible Grid Sizes**: Support for various 3DLUT grid sizes (e.g., 17x17x17, 33x33x33)

## Building the Library

### Prerequisites
- Visual Studio 2019 or later with C++ development tools
- Windows SDK 10.0 or later

### Build Instructions

1. **Open a Visual Studio Developer Command Prompt:**
   - Press `Windows Key` and search for "Developer Command Prompt for VS 2022"
   - Or look for "Developer Command Prompt for VS 2019" if you have the older version

2. **Navigate to the project directory:**
   ```cmd
   cd "path\to\Qubyx3DLUTGenerator"
   ```

3. **Run the build script:**
   ```cmd
   build.bat
   ```

The build script will:
- Check if Visual Studio compiler is available
- Compile the library
- Place object files in `obj/` directory
- Generate the DLL and LIB files in `bin/` directory

### Troubleshooting Build Issues

#### If you get "Visual Studio compiler not found" error:

1. **Make sure you're using the correct command prompt:**
   - Use "Developer Command Prompt for VS 2022" (not regular Command Prompt)
   - Look for it in Start Menu under "Visual Studio 2022" folder

2. **If you don't have Visual Studio installed:**
   - Download and install **Visual Studio Build Tools** (free)
   - Or install **Visual Studio Community** (free)

3. **Verify the environment:**
   - Run `where cl` in the command prompt
   - You should see a path to `cl.exe` if the environment is set up correctly

### Output
After successful compilation, you'll find:
- `bin/Qubyx3DLUTGenerator.dll` - Dynamic library
- `bin/Qubyx3DLUTGenerator.lib` - Import library
- `obj/` - Object files (temporary build artifacts)

## Usage

### Basic Example

```c
#include "qubyx3dlutgenerator.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Prepare your data
    char* ga_profile = "sRGB.icc";
    char* display_profile = "Display.icc";
    int grid_size = 17;  // 17x17x17 grid
    
    // Allocate memory for LUT data
    size_t lut_size = grid_size * grid_size * grid_size;
    unsigned int* rlut = malloc(lut_size * sizeof(unsigned int));
    unsigned int* glut = malloc(lut_size * sizeof(unsigned int));
    unsigned int* blut = malloc(lut_size * sizeof(unsigned int));
    
    // Generate the 3DLUT
    Q3dLut_Status result = generate3dLut(ga_profile, display_profile, grid_size, rlut, glut, blut);
    
    if (result == Q3dLut_SUCCESS) {
        printf("3DLUT generated successfully!\n");
        // Use your 3DLUT data...
    } else {
        printf("Error generating 3DLUT: %d\n", result);
    }
    
    // Cleanup
    free(rlut);
    free(glut);
    free(blut);
    
    return 0;
}
```

### API Reference

#### `generate3dLut`

```c
Q3dLut_Status generate3dLut(
    char* ga_profile,        // Path to GA (Gamut Adaptation) ICC profile
    char* display_profile,   // Path to display ICC profile
    int grid,                // Grid size (e.g., 17 for 17x17x17)
    unsigned int* rlut,      // Output: Red LUT data
    unsigned int* glut,      // Output: Green LUT data
    unsigned int* blut       // Output: Blue LUT data
);
```

**Return Values:**
- `Q3dLut_SUCCESS` (0): Success
- `Q3dLut_ERROR` (1): General error
- `Q3dLut_INVALID_PARAM` (2): Invalid parameters
- `Q3dLut_FILE_ERROR` (3): File I/O error
- `Q3dLut_MEMORY_ERROR` (4): Memory allocation error

### Building Your Application

```cmd
cl your_app.cpp /I. /link Qubyx3DLUTGenerator.lib
```

## Testing

Run the test program to verify the library works:

```cmd
test.bat
```

This will build and run a simple test that verifies the DLL can be loaded and the function can be called.

## License

This project is licensed under the GNU General Public License v3.0 - see the LICENSE file for details.

## Support

- **Issues**: Report bugs and request features on GitHub
- **Email**: support@qubyx.com
- **Website**: https://qubyx.com
