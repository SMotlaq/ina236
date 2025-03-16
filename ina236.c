/*!
 * @file INA236.c
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

#include "ina236.h"


/*!
    @brief  Initialize the INA236 with the given config
    @param  self
            A pointer to the INA236 object (struct)
		@param  I2C_ADDR
						The I2C address of the INA236. It depends on the state of A0 pin of the chip.
		@param  hi2c
		        A pointer to the I2C handler that is connected to INA236
		@param  ShuntResistor
						The resistance of your shunt resistor (in mOhm) connected to IN+ and IN- of the INA236
		@param  adc_range
						The full scale range of ADC. It can be one of these values:
						- ::RANGE_81_92mV for 81.92 mV
						- ::RANGE_20_48mV for 20.48 mV
		@param	numer_of_adc_samples
						Numer of ADC samples to calculate the average. The higher ADC samples leads to lower noises and higher latency.
						- ::NADC_1 for one sample (no average)
						- ::NADC_4 4 samples
						- ::NADC_16 16 samples
						- ::NADC_64 64 samples
						- ::NADC_128 128 samples
						- ::NADC_256 256 samples
						- ::NADC_512 512 samples
						- ::NADC_1024 1024 samples
		@param	vbus_conversion_time
						The conversion time of VBus measurment.
						- ::CTIME_140us
						- ::CTIME_204us
						- ::CTIME_332us
						- ::CTIME_588us
						- ::CTIME_1100us
						- ::CTIME_2116us
						- ::CTIME_4156us
						- ::CTIME_8244us
		@param	vshunt_conversion_time
						The conversion time of VShunt measurment.
						- ::CTIME_140us
						- ::CTIME_204us
						- ::CTIME_332us
						- ::CTIME_588us
						- ::CTIME_1100us
						- ::CTIME_2116us
						- ::CTIME_4156us
						- ::CTIME_8244us
		@param	mode
						Operating mode:
						- ::MODE_SHUTDOWN shutdown mode
						- ::MODE_SINGLESHOT_SUNT only measure the shunt voltage once
						- ::MODE_SINGLESHOT_BUS only measure the bus voltage once
						- ::MODE_SINGLESHOT_BOTH_SHUNT_BUS measure the bus and shunt voltage once
						- ::MODE_CONTINUOUS_SHUNT only measure the shunt voltage continuously
						- ::MODE_CONTINUOUS_BUS only measure the bus voltage continuously
						- ::MODE_CONTINUOUS_BOTH_SHUNT_BUS measure the bus and shunt voltage continuously
		@return	Ths status of initialization
		@retval ::INA236_OK in case of success
		@retval ::INA236_TimeOut in case of failure
*/
INA236_Status INA236_init(INA236* self, uint8_t I2C_ADDR, I2C_HandleTypeDef* hi2c, float ShuntResistor, ADCRange adc_range, NumSamples numer_of_adc_samples, ConvTime vbus_conversion_time, ConvTime vshunt_conversion_time, Mode mode){

	// Init Variables -----------------------
	self->hi2c = hi2c;
	self->I2C_ADDR = I2C_ADDR << 1;
	self->ShuntResistor = ShuntResistor;
	self->adc_range = adc_range;
	self->number_of_adc_samples = numer_of_adc_samples;
	self->vbus_conversion_time = vbus_conversion_time;
	self->vshunt_conversion_time = vshunt_conversion_time;
	self->mode = mode;
	
	// Write Configurations -----------------
	self->reg.config_register.RST = 0;
	self->reg.config_register.ACDRANGE = adc_range;
	self->reg.config_register.AVG = numer_of_adc_samples;
	self->reg.config_register.VBUSCT = vbus_conversion_time;
	self->reg.config_register.VSHCT = vshunt_conversion_time;
	self->reg.config_register.MODE = mode;

	if(INA236_OK != __INA236_writeTwoBytes(self, CONFIGURATION_REGISTER))
		return INA236_TimeOut;
	
	// Write Calibration Value --------------
	uint16_t shunt_cal = (uint16_t)((self->adc_range == RANGE_81_92mV ? 5.12 : 1.28) / (CURRENT_LSB * self->ShuntResistor));
	self->reg.calibration_register.SHUNT_CAL = shunt_cal;
	
	if(INA236_OK != __INA236_writeTwoBytes(self, CALIBRATION_REGISTER))
		return INA236_TimeOut;
	
	return INA236_OK;
}

/*!
    @brief  Initialize the alert functionality of INA236 with the given configurations
    @param  self
            A pointer to the INA236 object (struct)

		@param  alert_on
						determines the event that you want to assert the alert:
						- ::ALERT_SHUNT_OVER_LIMIT over voltage of the shunt voltage
						- ::ALERT_SHUNT_UNDER_LIMIT under voltage of the shunt voltage
						- ::ALERT_BUS_OVER_LIMIT over voltage of the bus voltage
						- ::ALERT_BUS_UNDER_LIMIT under voltage of the bus voltage
						- ::ALERT_POWER_OVER_LIMIT over power
						- ::ALERT_NONE disable the alert for events

		@param  alert_polarity
						The alert polarity determines the polarity of alert assertion:
						- ::ALERT_ACTIVE_LOW Alert pin goes low on alert
						- ::ALERT_ACTIVE_HIGH Alert pin goes HiZ on alert (alert pin is open-drain so you have to add a pull-up resistor to get high state)

		@param  alert_latch
						Determine the alert pin behaviour. If the latch is enabled, you have to reset the pin by calling the ::INA236_resetAlert() function.
						- ::ALERT_TRANSPARENT for normal behaviour
						- ::ALERT_LATCHED for latched behaviour
						
		@param	alert_conv_ready
						Shows if you want to get alert on "conversion done"/"data ready" too or not. If enabled, you have to distinguish the alert sourece by calling ::INA236_getAlertSource() function.
						- ::ALERT_CONV_ENABLE assert the alert pin on data ready event
						- ::ALERT_CONV_DISABLE disable the data ready assertion
						
		@param	alert_limit
						This the limit value. It automatically maps to the alert function you have chosen.
						The unit of this limit is related to the alert_on argument:
						- for ::ALERT_SHUNT_OVER_LIMIT or ::ALERT_SHUNT_UNDER_LIMIT : mili Volts
						- for ::ALERT_BUS_OVER_LIMIT or ::ALERT_BUS_UNDER_LIMIT : Volts
						- for ::ALERT_POWER_OVER_LIMIT : Watt
						
						For example if the alert_limit was 10.4 and you give ::ALERT_SHUNT_OVER_LIMIT for the alert_on argument, it means you will get alert if the shunt voltage reaches over the 10.4mV.
						If you give ::ALERT_BUS_OVER_LIMIT to alert_on, it means you will get alert if the bus voltage reaches the 10.4V
		@return	Ths status of initialization
		@retval ::INA236_OK in case of success
		@retval ::INA236_TimeOut in case of failure
*/
INA236_Status INA236_alert_init(INA236* self, AlertOn alert_on, AlertPolarity alert_polarity, AlertLatch alert_latch, AlertConvReady alert_conv_ready, float alert_limit){
	
	self->alert_on = alert_on;
	self->alert_polarity = alert_polarity;
	self->alert_latch = alert_latch;
	self->alert_conv_ready = alert_conv_ready;
	self->alert_limit = alert_limit;
	
	// Calculate Alert Limit
	switch (alert_on) {
		case ALERT_NONE:
			self->alert_limit_int = 0x7FFF;
			break;
		case ALERT_BUS_OVER_LIMIT:
			self->alert_limit_int = (int32_t)(alert_limit / BUS_VOLTAGE_LSB);
			break;
		case ALERT_BUS_UNDER_LIMIT:
			self->alert_limit_int = (int32_t)(alert_limit / BUS_VOLTAGE_LSB);
			break;
		case ALERT_SHUNT_OVER_LIMIT:
			self->alert_limit_int = (int32_t)(alert_limit / ((self->adc_range==RANGE_20_48mV) ? SHUNT_VOLTAGE_20_48mv_LSB : SHUNT_VOLTAGE_81_92mv_LSB));
			break;
		case ALERT_SHUNT_UNDER_LIMIT:
			self->alert_limit_int = (int32_t)(alert_limit / ((self->adc_range==RANGE_20_48mV) ? SHUNT_VOLTAGE_20_48mv_LSB : SHUNT_VOLTAGE_81_92mv_LSB));
			break;
		case ALERT_POWER_OVER_LIMIT:
			self->alert_limit_int = (int32_t)(alert_limit / POWER_LSB);
			break;
	}
	
	// Write Alert Limit
	self->reg.alert_limit_register.LIMIT = self->alert_limit_int & 0x0000FFFF;
	if(INA236_OK != __INA236_writeTwoBytes(self, ALERT_LIMIT_REGISTER))
		return INA236_TimeOut;
	
	// Write Alert Setting
	self->reg.mask_enable_register.SOL  = alert_on == ALERT_SHUNT_OVER_LIMIT  ? 1 : 0;
	self->reg.mask_enable_register.SUL  = alert_on == ALERT_SHUNT_UNDER_LIMIT ? 1 : 0;
	self->reg.mask_enable_register.BOL  = alert_on == ALERT_BUS_OVER_LIMIT    ? 1 : 0;
	self->reg.mask_enable_register.BUL  = alert_on == ALERT_BUS_UNDER_LIMIT   ? 1 : 0;
	self->reg.mask_enable_register.POL  = alert_on == ALERT_POWER_OVER_LIMIT  ? 1 : 0;
	self->reg.mask_enable_register.CNVR = alert_conv_ready;
	self->reg.mask_enable_register.APOL = alert_polarity;
	self->reg.mask_enable_register.LEN  = alert_latch;
	
	return __INA236_writeTwoBytes(self, MASK_ENABLE_REGISTER);
	
}

// Privates
/*!
    @brief  Read two bytes (a 16bit register) from INA236 and stores in the INA236::_reg::raw_data
    @param  self
            A pointer to the INA236 object (struct)
		@param  MemAddress
		        Address of the register
		@return	Ths status of reading
		@retval ::INA236_OK in case of success
		@retval ::INA236_TimeOut in case of failure
*/
INA236_Status __INA236_readTwoBytes(INA236* self, uint8_t MemAddress){
	if(HAL_OK == HAL_I2C_Mem_Read(self->hi2c, self->I2C_ADDR, MemAddress, I2C_MEMADD_SIZE_8BIT, self->reg.raw_data, 2, 100)){
		
		self->reg.raw_data[0] ^= self->reg.raw_data[1];
		self->reg.raw_data[1] ^= self->reg.raw_data[0];
		self->reg.raw_data[0] ^= self->reg.raw_data[1];

		return INA236_OK;
	}
	else{
		return INA236_TimeOut;
	}
}

/*!
    @brief  Write two bytes (a 16bit register) to INA236 from the INA236::_reg::raw_data
    @param  self
            A pointer to the INA236 object (struct)
		@param  MemAddress
		        Address of the register
		@return	Ths status of writing
		@retval ::INA236_OK in case of success
		@retval ::INA236_TimeOut in case of failure
*/
INA236_Status __INA236_writeTwoBytes(INA236* self, uint8_t MemAddress){

	self->reg.raw_data[0] ^= self->reg.raw_data[1];
	self->reg.raw_data[1] ^= self->reg.raw_data[0];
	self->reg.raw_data[0] ^= self->reg.raw_data[1];
	
	if(HAL_OK == HAL_I2C_Mem_Write(self->hi2c, self->I2C_ADDR, MemAddress, I2C_MEMADD_SIZE_8BIT, self->reg.raw_data, 2, 100))
		return INA236_OK;
	else
		return INA236_TimeOut;
}

// Configurations
/*!
    @brief  Set the ADC full scale range of INA236
    @param  self
            A pointer to the INA236 object (struct)
		@param  adc_range
						The full scale range of ADC. It can be one of these values:
						- ::RANGE_81_92mV for 81.92 mV
						- ::RANGE_20_48mV for 20.48 mV
		@return	Ths status of config
		@retval ::INA236_OK in case of success
		@retval ::INA236_TimeOut in case of failure
*/
INA236_Status INA236_setADCRange(INA236* self, ADCRange adc_range){
	if(INA236_OK == __INA236_readTwoBytes(self, CONFIGURATION_REGISTER)){
		
		self->reg.config_register.ACDRANGE = adc_range;
		return __INA236_writeTwoBytes(self, CONFIGURATION_REGISTER);
		
	}
	else{
		return INA236_TimeOut;
	}
}

/*!
    @brief  Set the number of ADC samples to calculate the average
    @param  self
            A pointer to the INA236 object (struct)
		@param	numer_of_adc_samples
						Numer of ADC samples to calculate the average. The higher ADC samples leads to lower noises and higher latency.
						- ::NADC_1 for one sample (no average)
						- ::NADC_4 4 samples
						- ::NADC_16 16 samples
						- ::NADC_64 64 samples
						- ::NADC_128 128 samples
						- ::NADC_256 256 samples
						- ::NADC_512 512 samples
						- ::NADC_1024 1024 samples
		@return	Ths status of config
		@retval ::INA236_OK in case of success
		@retval ::INA236_TimeOut in case of failure
*/
INA236_Status INA236_setNumberOfADCSamples(INA236* self, NumSamples numer_of_adc_samples){
	if(INA236_OK == __INA236_readTwoBytes(self, CONFIGURATION_REGISTER)){
		
		self->reg.config_register.AVG = numer_of_adc_samples;
		return __INA236_writeTwoBytes(self, CONFIGURATION_REGISTER);
		
	}
	else{
		return INA236_TimeOut;
	}
}

/*!
    @brief  Set the VBus convertion period
    @param  self
            A pointer to the INA236 object (struct)
		@param	vbus_conversion_time
						The conversion time of VBus measurment.
						- ::CTIME_140us
						- ::CTIME_204us
						- ::CTIME_332us
						- ::CTIME_588us
						- ::CTIME_1100us
						- ::CTIME_2116us
						- ::CTIME_4156us
						- ::CTIME_8244us
		@return	Ths status of config
		@retval ::INA236_OK in case of success
		@retval ::INA236_TimeOut in case of failure
*/
INA236_Status INA236_setVBusConversionTime(INA236* self, ConvTime vbus_conversion_time){
	if(INA236_OK == __INA236_readTwoBytes(self, CONFIGURATION_REGISTER)){
		
		self->reg.config_register.VBUSCT = vbus_conversion_time;
		return __INA236_writeTwoBytes(self, CONFIGURATION_REGISTER);
		
	}
	else{
		return INA236_TimeOut;
	}
}

/*!
    @brief  Set the VShunt convertion period
    @param  self
            A pointer to the INA236 object (struct)
		@param	vshunt_conversion_time
						The conversion time of VShunt measurment.
						- ::CTIME_140us
						- ::CTIME_204us
						- ::CTIME_332us
						- ::CTIME_588us
						- ::CTIME_1100us
						- ::CTIME_2116us
						- ::CTIME_4156us
						- ::CTIME_8244us
		@return	Ths status of config
		@retval ::INA236_OK in case of success
		@retval ::INA236_TimeOut in case of failure
*/
INA236_Status INA236_setVShuntConversionTime(INA236* self, ConvTime vshunt_conversion_time){
	if(INA236_OK == __INA236_readTwoBytes(self, CONFIGURATION_REGISTER)){
		
		self->reg.config_register.VSHCT = vshunt_conversion_time;
		return __INA236_writeTwoBytes(self, CONFIGURATION_REGISTER);
		
	}
	else{
		return INA236_TimeOut;
	}
}

/*!
    @brief  Set the operating mode
    @param  self
            A pointer to the INA236 object (struct)
		@param	mode
						Operating mode:
						- ::MODE_SHUTDOWN shutdown mode
						- ::MODE_SINGLESHOT_SUNT only measure the shunt voltage once
						- ::MODE_SINGLESHOT_BUS only measure the bus voltage once
						- ::MODE_SINGLESHOT_BOTH_SHUNT_BUS measure the bus and shunt voltage once
						- ::MODE_CONTINUOUS_SHUNT only measure the shunt voltage continuously
						- ::MODE_CONTINUOUS_BUS only measure the bus voltage continuously
						- ::MODE_CONTINUOUS_BOTH_SHUNT_BUS measure the bus and shunt voltage continuously
		@return	Ths status of config
		@retval ::INA236_OK in case of success
		@retval ::INA236_TimeOut in case of failure
*/
INA236_Status INA236_setMode(INA236* self, Mode mode){
	if(INA236_OK == __INA236_readTwoBytes(self, CONFIGURATION_REGISTER)){
		
		self->reg.config_register.MODE = mode;
		return __INA236_writeTwoBytes(self, CONFIGURATION_REGISTER);
		
	}
	else{
		return INA236_TimeOut;
	}
}

/*!
    @brief  Get the ADC full scale range of INA236
    @param  self
            A pointer to the INA236 object (struct)
		@return The full scale range of ADC.
		@retval ::RANGE_81_92mV for 81.92 mV
		@retval ::RANGE_20_48mV for 20.48 mV
*/
ADCRange INA236_getADCRange(INA236* self){
	return self->adc_range;
}

/*!
    @brief  Get the number of ADC samples to calculate the average
    @param  self
            A pointer to the INA236 object (struct)
		@return	Numer of ADC samples to calculate the average. The higher ADC samples leads to lower noises and higher latency.
		@retval ::NADC_1 for one sample (no average)
		@retval ::NADC_4 4 samples
		@retval ::NADC_16 16 samples
		@retval ::NADC_64 64 samples
		@retval ::NADC_128 128 samples
		@retval ::NADC_256 256 samples
		@retval ::NADC_512 512 samples
		@retval ::NADC_1024 1024 samples
*/
NumSamples INA236_getNumberOfADCSamples(INA236* self){
	return self->number_of_adc_samples;
}

/*!
    @brief  Get the VBus convertion period
    @param  self
            A pointer to the INA236 object (struct)
		@return	The conversion time of VBus measurment.
		@retval ::CTIME_140us
		@retval ::CTIME_204us
		@retval ::CTIME_332us
		@retval ::CTIME_588us
		@retval ::CTIME_1100us
		@retval ::CTIME_2116us
		@retval ::CTIME_4156us
		@retval ::CTIME_8244us
*/
ConvTime INA236_getVBusConversionTime(INA236* self){
	return self->vbus_conversion_time;
}

/*!
    @brief  Get the VShunt convertion period
    @param  self
            A pointer to the INA236 object (struct)
		@return	The conversion time of VShunt measurment.
		@retval ::CTIME_140us
		@retval ::CTIME_204us
		@retval ::CTIME_332us
		@retval ::CTIME_588us
		@retval ::CTIME_1100us
		@retval ::CTIME_2116us
		@retval ::CTIME_4156us
		@retval ::CTIME_8244us
*/
ConvTime INA236_getVShuntConversionTime(INA236* self){
	return self->vshunt_conversion_time;
}

/*!
    @brief  Set the operating mode
    @param  self
            A pointer to the INA236 object (struct)
		@return	Operating mode
		@retval ::MODE_SHUTDOWN shutdown mode
		@retval ::MODE_SINGLESHOT_SUNT only measure the shunt voltage once
		@retval ::MODE_SINGLESHOT_BUS only measure the bus voltage once
		@retval ::MODE_SINGLESHOT_BOTH_SHUNT_BUS measure the bus and shunt voltage once
		@retval ::MODE_CONTINUOUS_SHUNT only measure the shunt voltage continuously
		@retval ::MODE_CONTINUOUS_BUS only measure the bus voltage continuously
		@retval ::MODE_CONTINUOUS_BOTH_SHUNT_BUS measure the bus and shunt voltage continuously
*/
Mode INA236_getMode(INA236* self){
	return self->mode;
}

/*!
    @brief  Send a reset command to all of the INA236s on the bus
    @param  self
            A pointer to the INA236 object (struct)
*/
void INA236_SoftResetAll(INA236* self){
	uint8_t data = 0x06;
	HAL_I2C_Master_Transmit(self->hi2c, 0x00, &data, 1, 100);
}

// Getting Data
/*!
    @brief  Get the manufacturer ID
    @param  self
            A pointer to the INA236 object (struct)
		@return	a 16bit manufacturer ID
*/
uint16_t INA236_getManID(INA236* self){
	__INA236_readTwoBytes(self, MANUFACTURERID_REGISTER);
	return self->reg.manufacture_id_register.MANUFACTURE_ID;
}

/*!
    @brief  Get the device ID
    @param  self
            A pointer to the INA236 object (struct)
		@return	a 12bit device ID
*/
uint16_t INA236_getDevID(INA236* self){
	__INA236_readTwoBytes(self, DEVICEID_REGISTER);
	return self->reg.devide_id_register.DIEID;	
}

/*!
    @brief  Read all of the measured values: Shunt voltage, bus voltage, power, and current. Then store the values to the INA236 object (struct) variables.
						Then you can read variables: INA236#ShuntVoltage, INA236#BusVoltage, INA236#Power, and INA236#Current
    @param  self
            A pointer to the INA236 object (struct)
*/
void INA236_readAll(INA236* self){
	INA236_getShuntVoltage(self);
	INA236_getBusVoltage(self);
	INA236_getPower(self);
	INA236_getCurrent(self);
}

/*!
    @brief  Read the current from INA236
    @param  self
            A pointer to the INA236 object (struct)
		@return	a float value in **Amps** representing the current
*/
float INA236_getCurrent(INA236* self){ // In A
	__INA236_readTwoBytes(self, CURRENT_REGISTER);
	self->Current = self->reg.current_register.CURRENT * CURRENT_LSB;
	return self->Current;
}

/*!
    @brief  Read the bus voltage from INA236
    @param  self
            A pointer to the INA236 object (struct)
		@return	a float value in **Volts** representing the bus voltage
*/
float INA236_getBusVoltage(INA236* self){ // In V
	__INA236_readTwoBytes(self, BUS_VOLTAGE_REGISTER);
	self->BusVoltage = self->reg.bus_voltage_register.VBUS * BUS_VOLTAGE_LSB;
	return self->BusVoltage;
}

/*!
    @brief  Read the shunt voltage from INA236
    @param  self
            A pointer to the INA236 object (struct)
		@return	a float value in **miliVolts** representing the shunt voltage
*/
float INA236_getShuntVoltage(INA236* self){ // In mV
	__INA236_readTwoBytes(self, SHUNT_VOLTAGE_REGISTER);
	self->ShuntVoltage = self->reg.shunt_voltage_register.VSHUNT * (self->adc_range == RANGE_20_48mV ? SHUNT_VOLTAGE_20_48mv_LSB : SHUNT_VOLTAGE_81_92mv_LSB);
	return self->ShuntVoltage;
}

/*!
    @brief  Read the power from INA236
    @param  self
            A pointer to the INA236 object (struct)
		@return	a float value in **Watt** representing the power
*/
float INA236_getPower(INA236* self){ // In Watt
	__INA236_readTwoBytes(self, POWER_REGISTER);
	self->Power = self->reg.power_register.POWER * POWER_LSB;
	return self->Power;
}

/*!
    @brief  Check if the conversion is done or not. **NOTE: This function will reset the alert pin if it was in the latch mode. Exactly like calling the ::INA236_resetAlert() function.**
    @param  self
            A pointer to the INA236 object (struct)
		@retval True
		@retval False
*/
uint8_t INA236_isDataReady(INA236* self){
	__INA236_readTwoBytes(self, MASK_ENABLE_REGISTER);
	return self->reg.mask_enable_register.CVRF == 1;
}

/*!
    @brief  Get the alert source. This function is usefull when you enabled both of the alert functions and data ready alert simultaneously. **NOTE: This function will reset the alert pin if it was in the latch mode. Exactly like calling the ::INA236_resetAlert() function.**
    @param  self
            A pointer to the INA236 object (struct)
		@return	The alert source
		@retval ::ALERT_DATA_READY The alert source is convertion ready
		@retval ::ALERT_LIMIT_REACHED The alert source is limit reach
*/
AlertSource INA236_getAlertSource(INA236* self){
	__INA236_readTwoBytes(self, MASK_ENABLE_REGISTER);
	return self->reg.mask_enable_register.AFF ? ALERT_LIMIT_REACHED : ALERT_DATA_READY;
}

/*!
    @brief  Get the error flags of INA236. **NOTE: This function will reset the alert pin if it was in the latch mode. Exactly like calling the ::INA236_resetAlert() function.**
    @param  self
            A pointer to the INA236 object (struct)
		@return	Error type
		@retval ::ERROR_NONE	No error
		@retval ::ERROR_MEMORY	Memory error (CRC or ECC)
		@retval ::ERROR_OVF	Math overflow error
		@retval ::ERROR_BOTH_MEMORY_OVF	Both memory error (CRC or ECC) and math overflow error
*/
ErrorType INA236_getErrors(INA236* self){
	__INA236_readTwoBytes(self, MASK_ENABLE_REGISTER);
	if(self->reg.mask_enable_register.MemError && self->reg.mask_enable_register.OVF)
		return ERROR_BOTH_MEMORY_OVF;
	else if(self->reg.mask_enable_register.MemError)
		return ERROR_MEMORY;
	else if(self->reg.mask_enable_register.OVF)
		return ERROR_OVF;
	else
		return ERROR_NONE;
}

/*!
    @brief  Reset the alert pin. This function is useful when set the alert pin to latch mode.
    @param  self
            A pointer to the INA236 object (struct)
		@return	Ths status of reset
		@retval ::INA236_OK in case of success
		@retval ::INA236_TimeOut in case of failure
*/
INA236_Status INA236_resetAlert(INA236* self){
	return __INA236_readTwoBytes(self, MASK_ENABLE_REGISTER);
}
