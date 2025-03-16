[![Doxygen Action](https://github.com/SMotlaq/ina236/actions/workflows/main2.yml/badge.svg)](https://github.com/SMotlaq/ina236/actions/workflows/main2.yml)
[![pages-build-deployment](https://github.com/SMotlaq/ina236/actions/workflows/pages/pages-build-deployment/badge.svg)](https://github.com/SMotlaq/ina236/actions/workflows/pages/pages-build-deployment)


# INA236 HAL Based Library

The INA236 device is a 16-bit digital current monitor with an I2C/SMBus-compatible interface that is compliant with a wide range of digital bus voltages such as 1.2 V, 1.8 V, 3.3 V, and 5.0 V. The device monitors the voltage across an external sense resistor and reports values for current, bus voltage, and power. ([Click for more info](https://www.ti.com/product/INA236))

This library is a software library that works with the INA236 current, voltage, and power monitor chip. This library provides a convenient and efficient way to access the I2C interfaces of the chip, allowing developers to easily integrate this power meter into their systems.

The library is designed to be easy to use and provides a simple, intuitive API for accessing the I2C interfaces of the INA236. It includes a range of functions for performing common I2C operations, such as sending and receiving data, querying the status of the chip, reading the measured parameters, and configuring the INA236 settings.

With this library, developers can quickly and easily integrate the INA236 into their systems, enabling them to take full advantage of the chip's capabilities.

<p align="center">
  <img src="https://github.com/SMotlaq/ina236/blob/images/images/all.png"/>
</p>

## Key Features

* Easy-to-use API for accessing the I2C interfaces of the INA236
* Support for common I2C operations, such as sending and receiving data, querying the status of the chip, reading the measured parameters, and configuring the INA236 settings
* Full feature library

## Documentations

The full documents are available [here](https://smotlaq.github.io/ina236/)

## Schematic Symbol and Footprint

Footprint and schematic symbols are available in [my Altium library](https://github.com/SMotlaq/altium-library).

<p align="center">
  <img src="https://github.com/SMotlaq/ina236/blob/images/images/symbols.png"/>
</p>

## Donate
Is it helpfull?

<p align="left">
  <a href="http://smotlaq.ir/LQgQF">
  <img src="https://raw.githubusercontent.com/SMotlaq/LoRa/master/bmc.png" width="200" alt="Buy me a Coffee"/>
  </a>
</p>

# Getting Started

## Quick Start

1. Download the library source from the [latest release](http://github.com/smotlaq/ina236/releases/latest)

2. Copy `ina236.c` and `ina236.h` file to your project directory and add them to your IDE if necessary.

3. Inclued the library into your project:
   ```C
   #include "ina236.h"
   ```

4. Create an object (instanse) from INA236 struct with desired name:
   ```C
   INA236 ina236;
   ```

5. Initialize the chip:
   ```C
   INA236_init(&ina236, 0x48, &hi2c1, 1, RANGE_20_48mV, NADC_16, CTIME_1100us, CTIME_140us, MODE_CONTINUOUS_BOTH_SHUNT_BUS);
   ```
   Each argument is described on the [doc page](https://smotlaq.github.io/ina236/ina236_8c.html#afac863dce34fc9ce66f222eb697ffa0d).

6. Now you can call `INA236_readAll` function to read the meassured data:
   ```C
   INA236_readAll(&ina236);
   shunt_voltage = ina236.ShuntVoltage;
   bus_voltage = ina236.BusVoltage;
   current = ina236.Current;
   power = ina236.Power;
   ```

Here is the whole code:
```C
#include "ina236.h"

INA236 ina236;
float shunt_voltage, bus_voltage, current, power;

if(STATUS_OK == INA236_init(&ina236, 0x48, &hi2c1, 1, RANGE_20_48mV, NADC_16, CTIME_1100us, CTIME_140us, MODE_CONTINUOUS_BOTH_SHUNT_BUS)){

  INA236_readAll(&ina236);
  shunt_voltage = ina236.ShuntVoltage;
  bus_voltage = ina236.BusVoltage;
  current = ina236.Current;
  power = ina236.Power;
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
#include "ina236.h"

INA236 ina236;

if(STATUS_OK == INA236_init(&ina236, 0x48, &hi2c1, 1, RANGE_20_48mV, NADC_16, CTIME_1100us, CTIME_140us, MODE_CONTINUOUS_BOTH_SHUNT_BUS)){

  INA236_readAll(&ina236);
  DEBUG("Shunt Voltage: %.3fmV \t Bus Voltage: %.2fV \t Current: %.2fA \t Power: %.2fW\r\n", ina236.ShuntVoltage, ina236.BusVoltage, ina236.Current, ina236.Power);

}
else{

  DEBUG("----- INA236 init failed -----\r\n");

}
```
## Advanced Options

### Using Alert

INA236 can assert an alert on several situations like convertion ready, over power, over current, bus over voltage, bus under voltage, etc. To initialize alert functionality, use `INA236_alert_init` function:
```C
INA236_alert_init(&ina236, ALERT_SHUNT_OVER_LIMIT, ALERT_ACTIVE_LOW, ALERT_TRANSPARENT, ALERT_CONV_DISABLE, 2.5)
```
Each argument is described on the [doc page](https://smotlaq.github.io/ina236/ina236_8c.html#a6da58328ac79c68a8471940fbeacacde).

** *NOTE1* **  If you choose `ALERT_LATCHED` for alert latch mode, you have to reset the alert pin by calling `INA236_resetAlert` function after each alert assertion. ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#a37f4e9a052a8f8b6e85619372ec627db))

** *NOTE2* **  If you enabled convertion ready alert as well as limit reach functions (like shunt over voltage etc), you have to distinguish the alert source bt calling `INA236_getAlertSource` function. ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#a72a83e615d43b675c1a53d1e8572475a))

** *NOTE3* **  The alert pin is open-drain. So don not forget to add a pull-up resistor on this pin.

### Read Parameters Individually

You can read each parameter individually instead of `INA236_readAll` by calling each of these functions:
* `INA236_getShuntVoltage(&ina236);` to read shunt voltage (in mV)
* `INA236_getBusVoltage(&ina236);` to read bus voltage (in V)
* `INA236_getPower(&ina236);` to read power (in W)
* `INA236_getCurrent(&ina236);` to read current (in A)

Example:
```C
#include "ina236.h"

INA236 ina236;
float shunt_voltage, bus_voltage, current, power;

if(STATUS_OK == INA236_init(&ina236, 0x48, &hi2c1, 1, RANGE_20_48mV, NADC_16, CTIME_1100us, CTIME_140us, MODE_CONTINUOUS_BOTH_SHUNT_BUS)){

  shunt_voltage = INA236_getShuntVoltage(&ina236);
  bus_voltage = INA236_getBusVoltage(&ina236);
  current = INA236_getCurrent(&ina236);;
  power = INA236_getPower(&ina236);;
}
```

### Soft Reset

You can send a reset command to all of the INA236 chips on the same bus by calling `INA236_SoftResetAll` function. ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#aa4ba955e3e7793601ba52b28a2c67443))

### Change Settings On The Fly

You can change each of the configurations on the fly using these functions:
* `INA236_setADCRange` to change the ADC full scale range ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#a54228dec1296b2b38dd6eee21f147503))
* `INA236_setNumberOfADCSamples` to change the number of averaging ADC samples ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#a6be904fbbeffed1725841018138a7cdd))
* `INA236_setVBusConversionTime` to change the conversion period of VBus ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#abe15ca640e6d27e9945e50e0c3d60b8a))
* `INA236_setVShuntConversionTime` to change the conversion period of VBus ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#acd99e0c1c70ae96ffc4276297d70b952))
* `INA236_setMode` to change the operating mode ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#a9a2673191922b6b2e22c56bebcdb58d6))

### Getting Manufacturer and Device ID

If you want to get the manufacturer or device ID, you can use these functions:
* `INA236_getManID` ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#a76087166e06d502fff4fcc50b60192b0))
* `INA236_getDevID` ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#a1d07bedaf1f7ae7acfce73bd49f5ac85))

For example:
```C
printf("Manufacturer ID is 0x%4X \r\n", INA236_getManID(&ina236));
printf("      Device ID is 0x%3X \r\n", INA236_getDevID(&ina236));
```

### Get Internal Errors

INA236 can also give the state of internal modules like CPU and memory. By calling `INA236_getErrors` function you can see if there is any error or not. ([see more](https://smotlaq.github.io/ina236/ina236_8c.html#afb2d6eb60b3498ab85ac3784413be133))
