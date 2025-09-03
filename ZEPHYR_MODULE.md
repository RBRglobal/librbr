# libRBR as a Zephyr Module

This repository can be used as a Zephyr RTOS module, providing native integration with Zephyr's build system and configuration tools.

## Module Structure

```
zephyr/
├── module.yml          # Module metadata
├── Kconfig            # Configuration options
├── CMakeLists.txt     # Zephyr build integration
├── README.md          # Zephyr-specific documentation
└── sample/            # Sample application
    ├── CMakeLists.txt
    ├── prj.conf
    └── src/main.c
```

## Quick Integration

### Method 1: West Manifest
Add to your `west.yml`:
```yaml
manifest:
  projects:
    - name: librbr
      url: https://github.com/your-org/librbr
      path: modules/lib/librbr
```

### Method 2: Local Module
```bash
# In your Zephyr workspace
git clone https://github.com/your-org/librbr modules/lib/librbr
```

### Method 3: Submodule
```bash
# In your application repository
git submodule add https://github.com/your-org/librbr deps/librbr
```

Then set the module path:
```cmake
# In your app's CMakeLists.txt
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/deps/librbr)
```

## Usage in Applications

### Enable in Configuration
```ini
# prj.conf
CONFIG_LIBRBR=y
CONFIG_LIBRBR_DYNAMIC_CORRECTION=y
```

### Use in Code
```c
#include "RBRInstrument.h"

int main(void) {
    RBRInstrument instrument;
    RBRInstrument_open(&instrument, NULL);
    // ... use instrument
    RBRInstrument_close(&instrument);
    return 0;
}
```

## Features

✅ **Native Zephyr Integration**: Uses Zephyr's CMake and Kconfig systems  
✅ **Configurable**: Enable/disable features via Kconfig  
✅ **Memory Efficient**: Only builds what you configure  
✅ **Sample Application**: Complete working example  
✅ **Documentation**: Comprehensive integration guide  

For detailed documentation, see `zephyr/README.md`.