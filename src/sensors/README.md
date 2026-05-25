# Sensors Subsystem

This directory contains all joystick sensor-related functionality, including support for motion sensors (accelerometer, gyroscope) and their UI representations.

## Components

### Core Sensor Classes
- **joysensor.h/cpp** - Base abstract class for all joystick sensors
- **joysensorfactory.h/cpp** - Factory pattern implementation for creating sensor instances
- **joysensordirection.h** - Enumeration and utility for sensor direction definitions
- **joysensortype.h** - Enumeration defining supported sensor types

### IMU Sensors
- **joyaccelerometersensor.h/cpp** - Accelerometer sensor implementation (3-axis linear acceleration)
- **joygyroscopesensor.h/cpp** - Gyroscope sensor implementation (3-axis angular velocity)

### Sensor Configuration & Presets
- **joysensorpreset.h/cpp** - Preset configurations and profiles for sensors

### UI Components
- **joysensorpushbutton.h/cpp** - Push button widget for sensor input
- **joysensorbuttonpushbutton.h/cpp** - Button variant for sensor-based button inputs
- **joysensorcontextmenu.h/cpp** - Context menu for sensor configuration
- **joysensorstatusbox.h/cpp** - Status display widget showing sensor data

## Architecture

The sensor subsystem follows a factory pattern:
1. `joysensorfactory` creates sensor instances based on type
2. Each sensor type extends the base `joysensor` class
3. UI components provide visual representation and user interaction
4. Presets allow saving and loading sensor configurations

## Supported Sensors

- Accelerometer (3-axis motion detection)
- Gyroscope (rotation/angular velocity detection)

## Future Extensions

This modular design allows for easy addition of new sensor types:
- Magnetometer (3-axis magnetic field)
- Barometer (altitude/pressure)
- Proximity sensors
- Custom OEM sensors
