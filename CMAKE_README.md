# CMake Build System for libRBR

This directory now includes a CMake build system as an alternative to the original Makefile.

## Quick Start

### Using the build script (recommended):
```bash
# Build all libraries
./build.sh

# Build specific targets
./build.sh lib                    # Build main library only
./build.sh libdynamiccorrection   # Build dynamic correction library only
./build.sh docs                   # Generate documentation (if Doxygen available)
./build.sh clean                  # Clean build directory
```

### Using CMake directly:
```bash
# Create build directory and configure
mkdir build && cd build
cmake ..

# Build libraries
make all                          # Build both libraries
make RBR                         # Build main library only
make RBRDynamicCorrection        # Build dynamic correction library only

# Generate documentation (if Doxygen available)
make docs                        # Regular documentation
make devdocs                     # Developer documentation
```

## Output

The built libraries will be placed in:
- `build/bin/libRBR.a` - Main RBR instrument library
- `build/bin/libRBRDynamicCorrection.a` - Dynamic correction library

## Configuration Options

You can customize the build by setting CMake variables:

```bash
cmake -DLIB_NAME="MyCustomRBR" -DCMAKE_BUILD_TYPE=Debug ..
```

Available options:
- `LIB_NAME`: Library name (default: "libRBR")
- `CMAKE_BUILD_TYPE`: Build type (Release, Debug, etc.)
- `BUILD_DOCS`: Enable documentation generation (ON/OFF, default: ON if Doxygen found)

## Tests

Tests are intentionally disabled in this CMake configuration for simplicity. 
To run tests, use the original Makefile:

```bash
make tests
```

## Compatibility

This CMake configuration:
- ✅ Builds the same libraries as the original Makefile
- ✅ Uses the same compiler flags and warnings
- ✅ Supports the same preprocessor definitions
- ✅ Generates documentation with Doxygen
- ✅ Handles version information from VERSION file
- ✅ Cross-platform compatible
- ❌ Does not build tests (use original Makefile for tests)

## Migration from Makefile

| Makefile Command | CMake Equivalent |
|------------------|------------------|
| `make lib` | `./build.sh lib` or `make RBR` |
| `make libdynamiccorrection` | `./build.sh libdynamiccorrection` or `make RBRDynamicCorrection` |
| `make docs` | `./build.sh docs` or `make docs` |
| `make devdocs` | `./build.sh devdocs` or `make devdocs` |
| `make clean` | `./build.sh clean` |
| `make tests` | Use original Makefile: `make tests` |

## Installation

To install the libraries and headers system-wide:

```bash
cd build
make install
```

This will install:
- Libraries to `/usr/local/lib/`
- Headers to `/usr/local/include/`

You can change the installation prefix:
```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/rbr ..
```