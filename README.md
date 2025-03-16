[![Doxygen Action](https://github.com/SMotlaq/ina234/actions/workflows/main2.yml/badge.svg)](https://github.com/SMotlaq/ina234/actions/workflows/main2.yml)
[![pages-build-deployment](https://github.com/SMotlaq/ina234/actions/workflows/pages/pages-build-deployment/badge.svg)](https://github.com/SMotlaq/ina234/actions/workflows/pages/pages-build-deployment)


# INA234 HAL Based Library

The INA234 device is a 12-bit digital current monitor with an I2C/SMBus-compatible interface that is compliant with a wide range of digital bus voltages such as 1.2 V, 1.8 V, 3.3 V, and 5.0 V. The device monitors the voltage across an external sense resistor and reports values for current, bus voltage, and power. ([Click for more info](https://www.ti.com/product/INA234))

This library is a software library that works with the INA234 current, voltage, and power monitor chip. This library provides a convenient and efficient way to access the I2C interfaces of the chip, allowing developers to easily integrate this power meter into their systems.

The library is designed to be easy to use and provides a simple, intuitive API for accessing the I2C interfaces of the INA234. It includes a range of functions for performing common I2C operations, such as sending and receiving data, querying the status of the chip, reading the measured parameters, and configuring the INA234 settings.

With this library, developers can quickly and easily integrate the INA234 into their systems, enabling them to take full advantage of the chip's capabilities.

<p align="center">
  <img src="https://github.com/SMotlaq/ina234/blob/images/images/all.png"/>
</p>

## Key Features

* Easy-to-use API for accessing the I2C interfaces of the INA234
* Support for common I2C operations, such as sending and receiving data, querying the status of the chip, reading the measured parameters, and configuring the INA234 settings
* Full feature library

## Documentations

The full documents are available [here](https://smotlaq.github.io/ina234/)

## Schematic Symbol and Footprint

Footprint and schematic symbols are available in [my Altium library](https://github.com/SMotlaq/altium-library).

## Donate
Is it helpfull?

<p align="left">
  <a href="http://smotlaq.ir/LQgQF">
  <img src="https://raw.githubusercontent.com/SMotlaq/LoRa/master/bmc.png" width="200" alt="Buy me a Coffee"/>
  </a>
</p>

# Getting Started

## Quick Start

1. Download the library source from the [latest release](http://github.com/smotlaq/ina234/releases/latest)

2. Copy `ina234.c` and `ina234.h` file to your project directory and add them to your IDE if necessary.

3. Inclued the library into your project:
   ```C
   #include "ina234.h"
   ```

4. Create an object (instanse) from INA234 struct with desired name:
   ```C
   INA234 ina234;
   ```

5. Initialize the chip:
   ```C
   INA234_init(&ina234, 0x48, &hi2c1, 1, RANGE_20_48mV, NADC_16, CTIME_1100us, CTIME_140us, MODE_CONTINUOUS_BOTH_SHUNT_BUS);
   ```
   Each argument is described on the [doc page](https://smotlaq.github.io/ina234/ina234_8c.html#a21acc30b187445a98f711a54b8678c7c).

6. Now you can call `INA234_readAll` function to read the meassured data:
   ```C
   INA234_readAll(&ina234);
   shunt_voltage = ina234.ShuntVoltage;
   bus_voltage = ina234.BusVoltage;
   current = ina234.Current;
   power = ina234.Power;
   ```

Here is the whole code:
```C
#include "ina234.h"

INA234 ina234;
float shunt_voltage, bus_voltage, current, power;

if(STATUS_OK == INA234_init(&ina234, 0x48, &hi2c1, 1, RANGE_20_48mV, NADC_16, CTIME_1100us, CTIME_140us, MODE_CONTINUOUS_BOTH_SHUNT_BUS)){

  INA234_readAll(&ina234);
  shunt_voltage = ina234.ShuntVoltage;
  bus_voltage = ina234.BusVoltage;
  current = ina234.Current;
  power = ina234.Power;
}
```


If you want to use UART or virtual USB COM port on youe microcontroller, it is recommended to use this print function:
```C
// Print setting -------------------
#define DEBUG_ENABLE  1
#define USB_DEBUG     0
#define DEBUG_UART    (&huart1)
// ---------------------------------

#if DEBUG_ENABLE
  #include "stdarg.h"
  #include "string.h"
  #include "stdlib.h"

  #if USB_DEBUG
    #include "usbd_cdc_if.h"
  #endif
#endif

void DEBUG(const char* _str, ...){
  #if DEBUG_ENABLE
    va_list args;
    va_start(args, _str);
    char buffer[150];
    memset(buffer, 0, 150);
    int buffer_size = vsprintf(buffer, _str, args);
    #if USB_DEBUG
      CDC_Transmit_FS((uint8_t*) buffer, buffer_size);
    #else
      HAL_UART_Transmit(DEBUG_UART, (uint8_t*)buffer, buffer_size, 5000);
    #endif
  #endif
}
```


By applying the above trick, you can simply use this one to see the variables on the serial terminal:
```C
#include "ina234.h"

INA234 ina234;

if(STATUS_OK == INA234_init(&ina234, 0x48, &hi2c1, 1, RANGE_20_48mV, NADC_16, CTIME_1100us, CTIME_140us, MODE_CONTINUOUS_BOTH_SHUNT_BUS)){

  INA234_readAll(&ina234);
  DEBUG("Shunt Voltage: %.3fmV \t Bus Voltage: %.2fV \t Current: %.2fA \t Power: %.2fW\r\n", ina234.ShuntVoltage, ina234.BusVoltage, ina234.Current, ina234.Power);

}
else{

  DEBUG("----- INA234 init failed -----\r\n");

}
```
## Advanced Options

### Using Alert

INA234 can assert an alert on several situations like convertion ready, over power, over current, bus over voltage, bus under voltage, etc. To initialize alert functionality, use `INA234_alert_init` function:
```C
INA234_alert_init(&ina234, ALERT_SHUNT_OVER_LIMIT, ALERT_ACTIVE_LOW, ALERT_TRANSPARENT, ALERT_CONV_DISABLE, 2.5)
```
Each argument is described on the [doc page](https://smotlaq.github.io/ina234/ina234_8c.html#afb44437883ad8f8d08aaf695815da7ed).

** *NOTE1* **  If you choose `ALERT_LATCHED` for alert latch mode, you have to reset the alert pin by calling `INA234_resetAlert` function after each alert assertion. ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#a5810f9a740226a39ba5cc2afa6b64f77))

** *NOTE2* **  If you enabled convertion ready alert as well as limit reach functions (like shunt over voltage etc), you have to distinguish the alert source bt calling `INA234_getAlertSource` function. ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#a52cc3b785dea1f5af6f0803f02fcefdb))

** *NOTE3* **  The alert pin is open-drain. So don not forget to add a pull-up resistor on this pin.

### Read Parameters Individually

You can read each parameter individually instead of `INA234_readAll` by calling each of these functions:
* `INA234_getShuntVoltage(&ina234);` to read shunt voltage (in mV)
* `INA234_getBusVoltage(&ina234);` to read bus voltage (in V)
* `INA234_getPower(&ina234);` to read power (in W)
* `INA234_getCurrent(&ina234);` to read current (in A)

Example:
```C
#include "ina234.h"

INA234 ina234;
float shunt_voltage, bus_voltage, current, power;

if(STATUS_OK == INA234_init(&ina234, 0x48, &hi2c1, 1, RANGE_20_48mV, NADC_16, CTIME_1100us, CTIME_140us, MODE_CONTINUOUS_BOTH_SHUNT_BUS)){

  shunt_voltage = INA234_getShuntVoltage(&ina234);
  bus_voltage = INA234_getBusVoltage(&ina234);
  current = INA234_getCurrent(&ina234);;
  power = INA234_getPower(&ina234);;
}
```

### Soft Reset

You can send a reset command to all of the INA234 chips on the same bus by calling `INA234_SoftResetAll` function. ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#af3d939ea27371b17fd265f19957234b2))

### Change Settings On The Fly

You can change each of the configurations on the fly using these functions:
* `INA234_setADCRange` to change the ADC full scale range ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#aba71c63deed65a0abdbf7269b5f382d8))
* `INA234_setNumberOfADCSamples` to change the number of averaging ADC samples ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#a84ff6173bf6cfa44348ba259a503c804))
* `INA234_setVBusConversionTime` to change the conversion period of VBus ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#a94ec7dc7cd10748c4ed822266174d0ff))
* `INA234_setVShuntConversionTime` to change the conversion period of VBus ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#ad19627414a2465c9cf1fac54f54eaa39))
* `INA234_setMode` to change the operating mode ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#ac85c8e736ffae6d248971091b374d00f))

### Getting Manufacturer and Device ID

If you want to get the manufacturer or device ID, you can use these functions:
* `INA234_getManID` ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#ae646f51adec51af1aa6377c3dffeeb6a))
* `INA234_getDevID` ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#a88ff1503798836270a41d3b9f3913ca7))

For example:
```C
printf("Manufacturer ID is 0x%4X \r\n", INA234_getManID(&ina234));
printf("      Device ID is 0x%3X \r\n", INA234_getDevID(&ina234));
```

### Get Internal Errors

INA234 can also give the state of internal modules like CPU and memory. By calling `INA234_getErrors` function you can see if there is any error or not. ([see more](https://smotlaq.github.io/ina234/ina234_8c.html#a14a3383eba06ce784ed526585a0cef9a))
