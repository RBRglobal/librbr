# libRBR Zephyr Module

This directory contains the Zephyr RTOS module integration for the RBR Instrument Library.

## Overview

The libRBR Zephyr module provides:
- **RBR Instrument Library**: Communication and control of RBR oceanographic instruments
- **RBR Dynamic Correction Library**: Post-processing with dynamic corrections
- **Zephyr Integration**: Native Kconfig and CMake integration
- **Sample Application**: Example usage in Zephyr applications

## Quick Start

### 1. Add as a Zephyr Module

Add this repository as a module in your Zephyr workspace:

```bash
# In your Zephyr workspace
west config manifest.project-filter -- +librbr
# Or add to west.yml:
```

```yaml
manifest:
  projects:
    - name: librbr
      url: https://github.com/your-org/librbr
      path: modules/lib/librbr
```

### 2. Enable in Your Application

Add to your `prj.conf`:
```ini
CONFIG_LIBRBR=y
CONFIG_LIBRBR_DYNAMIC_CORRECTION=y
CONFIG_LIBRBR_LOG_LEVEL=3
```

### 3. Use in Your Code

```c
#include "RBRInstrument.h"
#include "RBRParser.h"

int main(void)
{
    RBRInstrument instrument;
    RBRInstrumentError err;
    
    err = RBRInstrument_open(&instrument, NULL);
    if (err == RBRINSTRUMENT_SUCCESS) {
        // Use the instrument...
        RBRInstrument_close(&instrument);
    }
    
    return 0;
}
```

## Configuration Options

### Kconfig Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `CONFIG_LIBRBR` | bool | n | Enable RBR Instrument Library |
| `CONFIG_LIBRBR_DYNAMIC_CORRECTION` | bool | y | Enable Dynamic Correction Library |
| `CONFIG_LIBRBR_LOG_LEVEL` | int | 3 | Log level (0=Off, 1=Error, 2=Warning, 3=Info, 4=Debug) |

### CMake Integration

The module automatically integrates with Zephyr's build system:
- Creates `rbr` library target
- Creates `rbr_dynamic_correction` library target (if enabled)
- Provides all necessary include paths
- Applies appropriate compiler flags

## Sample Application

A complete sample application is provided in `zephyr/sample/`:

```bash
cd zephyr/sample
west build -b <your_board>
west flash
```

## Hardware Requirements

### UART/Serial Communication
Most RBR instruments communicate via serial interface. Ensure your Zephyr board configuration includes:

```ini
CONFIG_SERIAL=y
CONFIG_UART_CONSOLE=y
# Additional UART configuration as needed
```

### Memory Requirements
- **Flash**: ~450KB for main library + ~20KB for dynamic correction
- **RAM**: Varies by usage, typically 4-16KB for instrument structures
- **Stack**: Minimum 4KB recommended for main thread

## API Documentation

The full API documentation is available in the parent directory:
- Use `make docs` or `./build.sh docs` to generate Doxygen documentation
- Headers are in `../include/`

## Integration Examples

### Basic Instrument Communication
```c
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include "RBRInstrument.h"

static const struct device *uart_dev;

int setup_rbr_instrument(void)
{
    uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    if (!device_is_ready(uart_dev)) {
        return -ENODEV;
    }
    
    RBRInstrument instrument;
    RBRInstrumentError err;
    
    // Configure instrument with UART device
    err = RBRInstrument_open(&instrument, uart_dev);
    return (err == RBRINSTRUMENT_SUCCESS) ? 0 : -1;
}
```

### Data Parsing
```c
#include "RBRParser.h"

void parse_instrument_data(const char *data, size_t len)
{
    RBRParser parser;
    RBRParserError err;
    
    err = RBRParser_init(&parser, NULL);
    if (err == RBRPARSER_SUCCESS) {
        // Parse data...
        RBRParser_destroy(&parser);
    }
}
```

## Troubleshooting

### Build Issues
1. **Missing headers**: Ensure `CONFIG_LIBRBR=y` is set
2. **Linker errors**: Check that required libraries are enabled
3. **Memory issues**: Increase stack size or heap size

### Runtime Issues
1. **Communication failures**: Verify UART configuration and wiring
2. **Parsing errors**: Check data format and parser configuration
3. **Memory corruption**: Increase stack size, check buffer sizes

## Contributing

This module follows the same contribution guidelines as the main libRBR project. See `../CONTRIBUTING.md` for details.

## License

Licensed under the Apache License, Version 2.0. See `../LICENSE` for details.