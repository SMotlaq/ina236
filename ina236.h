/*!
 * @file INA236.h
 *
 * @mainpage INA236 current/voltage/power monitor library
 *
 * @section intro_sec Introduction
 *
 * The INA236 device is a 16-bit digital current monitor with an I2C/SMBus-compatible interface that is
 * compliant with a wide range of digital bus voltages such as 1.2 V, 1.8 V, 3.3 V, and 5.0 V. The device
 * monitors the voltage across an external sense resistor and reports values for current, bus voltage, and
 * power.
 * 
 * The INA236 features programmable ADC conversion times and averaging. The device also has a
 * programmable calibration value with an internal multiplier that enables direct readouts of current in
 * amperes and power in watts. The device monitors the bus voltage present on the IN- pin and can
 * alert on overcurent and undercurrent conditions as well as overvoltage and undervoltage conditions. High
 * input impedance while in current measurement mode allows use of larger current sense resistors needed to
 * measure small value system currents.
 * 
 * The INA236 senses current on common-mode bus
 * voltages that can vary from -0.3 V to 28 V,
 * independent of the supply voltage. The device
 * operates from a single 1.7-V to 5.5-V supply, drawing
 * a typical supply current of 300 uA in normal operation.
 * The device can be placed in a low-power standby
 * mode where the typical operating current is 2.2 uA.
 *
 * @section author Author
 *
 * Written by Salman Motlaq (<a href="https://github.com/SMotlaq">@SMotlaq</a> on Github)
 *
 * @section license License
 *
 * MIT License
 *
 * @section start Getting Started
 * Go to [my Github page](https://github.com/SMotlaq/ina236) to get started. You can also see the [functions list](./INA236_8c.html) to learn more.
 *
 */

#ifndef __INA236_H_
#define __INA236_H_

#include "main.h"
#include "i2c.h"

#define MAXIMUM_EXPECTED_CURRENT	5.0
#define CURRENT_LSB_MINIMUM				(MAXIMUM_EXPECTED_CURRENT / ((float)(1<<15)))
#define CURRENT_LSB								(CURRENT_LSB_MINIMUM * 1.0) // in A
#define BUS_VOLTAGE_LSB						0.0016 		// in V
#define SHUNT_VOLTAGE_81_92mv_LSB	0.0025  	// in mV		
#define SHUNT_VOLTAGE_20_48mv_LSB	0.000625  // in mV
#define POWER_LSB									(CURRENT_LSB*32) // in W

#define CONFIGURATION_REGISTER	0x00
#define SHUNT_VOLTAGE_REGISTER	0x01
#define BUS_VOLTAGE_REGISTER		0x02
#define POWER_REGISTER					0x03
#define CURRENT_REGISTER				0x04
#define CALIBRATION_REGISTER		0x05
#define MASK_ENABLE_REGISTER		0x06
#define ALERT_LIMIT_REGISTER		0x07
#define MANUFACTURERID_REGISTER	0x3E
#define DEVICEID_REGISTER				0x3F

typedef enum ADCRange				{RANGE_81_92mV, RANGE_20_48mV} ADCRange;
typedef enum NumSamples			{NADC_1, NADC_4, NADC_16, NADC_64, NADC_128, NADC_256, NADC_512, NADC_1024} NumSamples;
typedef enum ConvTime				{CTIME_140us, CTIME_204us, CTIME_332us, CTIME_588us, CTIME_1100us, CTIME_2116us, CTIME_4156us, CTIME_8244us} ConvTime;
typedef enum Mode						{MODE_SHUTDOWN, MODE_SINGLESHOT_SUNT, MODE_SINGLESHOT_BUS, MODE_SINGLESHOT_BOTH_SHUNT_BUS, MODE_SHUTDOWN2, MODE_CONTINUOUS_SHUNT, MODE_CONTINUOUS_BUS, MODE_CONTINUOUS_BOTH_SHUNT_BUS} Mode;
typedef enum INA236_Status	{INA236_OK, INA236_TimeOut} INA236_Status;
typedef enum AlertOn				{ALERT_NONE, ALERT_SHUNT_OVER_LIMIT, ALERT_SHUNT_UNDER_LIMIT, ALERT_BUS_OVER_LIMIT, ALERT_BUS_UNDER_LIMIT, ALERT_POWER_OVER_LIMIT} AlertOn;
typedef enum AlertPolarity	{ALERT_ACTIVE_LOW, ALERT_ACTIVE_HIGH} AlertPolarity;
typedef enum AlertLatch			{ALERT_TRANSPARENT, ALERT_LATCHED} AlertLatch;
typedef enum AlertConvReady	{ALERT_CONV_DISABLE, ALERT_CONV_ENABLE} AlertConvReady;
typedef enum AlertSource		{ALERT_DATA_READY, ALERT_LIMIT_REACHED} AlertSource;
typedef enum ErrorType			{ERROR_NONE, ERROR_MEMORY, ERROR_OVF, ERROR_BOTH_MEMORY_OVF} ErrorType;

/*! 
    @brief  Class (struct) that stores variables for interacting with INA236
*/
typedef struct INA236{
	
	I2C_HandleTypeDef*	hi2c;										/*!< Specifies the I2C handler. */
	uint8_t 						I2C_ADDR;
	
	// Main configs
	ADCRange		adc_range;
	NumSamples	number_of_adc_samples;
	ConvTime		vbus_conversion_time;
	ConvTime		vshunt_conversion_time;
	Mode				mode;
	float				ShuntResistor;
	
	// Alert Configs
	AlertOn					alert_on;
	AlertPolarity		alert_polarity;
	AlertLatch			alert_latch;
	AlertConvReady	alert_conv_ready;
	float						alert_limit;
	int32_t 				alert_limit_int;

	float				ShuntVoltage;
	float				BusVoltage;
	float				Power;
	float				Current;
	
	union _reg {
		uint8_t raw_data[2];
		
		struct _config_register{
			uint16_t MODE:3, VSHCT:3, VBUSCT:3, AVG:3, ACDRANGE:1, RESERVED:2, RST:1;
		} config_register;
		
		struct _shunt_voltage_register{
			int16_t VSHUNT;
		} shunt_voltage_register;
		
		struct _bus_voltage_register{
			uint16_t VBUS:15, RESERVED:1;
		} bus_voltage_register;
		
		struct _power_register{
			uint16_t POWER;
		} power_register;
		
		struct _current_register{
			int16_t CURRENT;
		} current_register;
		
		struct _calibration_register{
			uint16_t SHUNT_CAL:15, RESERVED:1;
		} calibration_register;
		
		struct _mask_enable_register{
			uint16_t LEN:1, APOL:1, OVF:1, CVRF:1, AFF:1, MemError:1, RESERVED:4, CNVR:1, POL:1, BUL:1, BOL:1, SUL:1, SOL:1;
		} mask_enable_register;
		
		struct _alert_limit_register{
			int16_t LIMIT;
		} alert_limit_register;
			
		struct _manufacture_id_register{
			uint16_t MANUFACTURE_ID;
		} manufacture_id_register;
		
		struct _devide_id_register{
			uint16_t RESERVED:4, DIEID:12;
		} devide_id_register;
		
	} reg;

} INA236;

INA236_Status INA236_init(INA236* self, uint8_t I2C_ADDR, I2C_HandleTypeDef* hi2c, float ShuntResistor, ADCRange adc_range, NumSamples numer_of_adc_samples, ConvTime vbus_conversion_time, ConvTime vshunt_conversion_time, Mode mode);
INA236_Status INA236_alert_init(INA236* self, AlertOn alert_on, AlertPolarity alert_polarity, AlertLatch alert_latch, AlertConvReady alert_conv_ready, float alert_limit);

// Privates ----------------------------------

INA236_Status __INA236_readTwoBytes(INA236* self, uint8_t MemAddress);
INA236_Status __INA236_writeTwoBytes(INA236* self, uint8_t MemAddress);

// Configurations ----------------------------

INA236_Status INA236_setADCRange(INA236* self, ADCRange adc_range);
INA236_Status INA236_setNumberOfADCSamples(INA236* self, NumSamples numer_of_adc_samples);
INA236_Status INA236_setVBusConversionTime(INA236* self, ConvTime vbus_conversion_time);
INA236_Status INA236_setVShuntConversionTime(INA236* self, ConvTime vshunt_conversion_time);
INA236_Status INA236_setMode(INA236* self, Mode mode);

ADCRange		INA236_getADCRange(INA236* self);
NumSamples	INA236_getNumberOfADCSamples(INA236* self);
ConvTime		INA236_getVBusConversionTime(INA236* self);
ConvTime		INA236_getVShuntConversionTime(INA236* self);
Mode 				INA236_getMode(INA236* self);

void INA236_SoftResetAll(INA236* self);

// Getting Data ------------------------------

uint16_t	INA236_getManID(INA236* self);
uint16_t	INA236_getDevID(INA236* self);
void			INA236_readAll(INA236* self);
float			INA236_getCurrent(INA236* self);
float			INA236_getBusVoltage(INA236* self);
float			INA236_getShuntVoltage(INA236* self);
float			INA236_getPower(INA236* self);

uint8_t			INA236_isDataReady(INA236* self);
AlertSource	INA236_getAlertSource(INA236* self);
ErrorType		INA236_getErrors(INA236* self);
INA236_Status			INA236_resetAlert(INA236* self);

#endif
