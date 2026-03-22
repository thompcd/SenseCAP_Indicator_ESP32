# Focus Timer for SenseCAP Indicator

A simple focus timer application for the SenseCAP Indicator with ESP32-S3.

## Features

- **Dark themed UI** with clean, minimal design
- **Timer display** showing "Heads down. Be back in X:XX" when running
- **Preset buttons**: 5 min, 15 min, 30 min
- **START/STOP button** with color change (blue → red when running)
- **I2C slave interface** on Grove connector for external status queries

## I2C Interface

The device exposes timer status via I2C slave on the Grove connector:

- **Address**: 0x42
- **SCL**: GPIO 20
- **SDA**: GPIO 19

### Registers

| Register | Description | Values |
|----------|-------------|--------|
| 0x00 | Status | 0 = stopped, 1 = running |
| 0x01 | Minutes remaining | 0-30 |
| 0x02 | Seconds remaining | 0-59 |

### Example Read (Python)

```python
import smbus2

bus = smbus2.SMBus(1)  # Adjust bus number as needed
addr = 0x42

# Read status
status = bus.read_byte_data(addr, 0x00)
minutes = bus.read_byte_data(addr, 0x01)
seconds = bus.read_byte_data(addr, 0x02)

print(f"Running: {status}, Time: {minutes}:{seconds:02d}")
```

## Building

Requires ESP-IDF v5.1.x

```bash
# Set target
idf.py set-target esp32s3

# Build
idf.py build

# Flash
idf.py -p /dev/cu.usbserial-1110 flash
```

## Files

- `main.c` - Application entry point
- `focus_timer_ui.c/h` - LVGL-based UI implementation
- `i2c_slave.c/h` - I2C slave interface for external status queries
- `lv_port.c/h` - LVGL display/input port

## Hardware

- SenseCAP Indicator (ESP32-S3 based)
- 4" Touch LCD Display
- Grove I2C connector for external access

## Version

v1.0.0
