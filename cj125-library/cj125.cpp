/* Author: Michał Byrecki
 * License: GPLv2
 */

#include "Arduino.h"
#include "cj125.h"
#include "table.h"

//CJ125 registers addresses
#define         IDENT_REG_RD            0x4800

#define         INIT_REG1_RD            0x6C00
#define         INIT_REG1_WR            0x5600

#define         INIT_REG2_RD            0x7E00
#define         INIT_REG2_WR            0x5A00

#define         DIAG_REG_RD             0x7800

CJ125::CJ125(uint16_t CS_PIN, uint16_t HTR_PIN, uint16_t UR_PIN, uint16_t UA_PIN): pid(&Input, &Output, &Setpoint, kp, ki, kd, mode)
{

	pinMode(CS_PIN, OUTPUT);
	pinMode(HTR_PIN,OUTPUT);
	digitalWrite(CS_PIN,HIGH);
	digitalWrite(HTR_PIN,LOW);
	this->CS_PIN=CS_PIN;
	this->HTR_PIN=HTR_PIN;
	this->UR_PIN=UR_PIN;
	this->UA_PIN=UA_PIN;
	double Input,Output,Setpoint;
	this->kp=1; this->ki=0.3; this->kd=0.1;
	this->pid=PID(&this->Input,&this->Output,&this->Setpoint,this->kp,this->ki,this->kd,REVERSE);
	this->pid.SetMode(AUTOMATIC);
	SPI.begin();  //added, without shield does not start

}
int CJ125::check_id(void)
{
	unsigned char result=0;
	result = COM_SPI(IDENT_REG_RD);
	if ((result&0xF8)!=0x60) return -1;
	else return (result&0x07);
}

int CJ125::check_stat(void)
{
	unsigned char result=0;
       	result=COM_SPI(DIAG_REG_RD);
	result=result>>6;
	switch (result)
	{
		case 0: return CJ125_E_SHORTCIRCUITGND; break;
		case 1: return CJ125_E_NOPOWER; break;
		case 2: return CJ125_E_SHORTCIRCUITBAT; break;
		case 3: return CJ125_OK; break;
	}	
	return -1;
}

int CJ125::calibrate(float UBAT)
{
//There is a risk of water condensed into the O2 sensor, so the proper pre-heating procedure must be maintainted.
//From what Bosch says, it folllows:
// - enter the CJ125 calibration mode
// - apply 2V to the heater for 4 seconds
// - apply 8.5V to the heater, and within each consecutive second increase by 0.4V up to the battery voltage
// - store UR value as a PWM reference point, quit CJ125 calibration
// - change over to PWM heater control mode

	COM_SPI(INIT_REG1_WR|0x9D);	//entering calibration mode
	if (UBAT<8.5) return -1;	// UBAT is less than 8.5V, hardware problem
	//two volts are equal to  136 from ADC but for convenience, lets stick to the float calculation :)
  	float pwm_factor=(2/UBAT)*255;
	analogWrite(this->HTR_PIN, byte(pwm_factor)); 
	delay(1000); delay(1000); delay (1000); delay (1000);
	float UHTR=8.5;
	while (UHTR<UBAT)
	{	
       		pwm_factor = (UHTR / UBAT) * 255;	//o2 sensor preheating sequence, starting from 8.5V and increasing 0.4V per second
		UHTR+=0.4;
		delay(1000);
		analogWrite(this->HTR_PIN,byte(pwm_factor));
	}
	analogWrite(this->HTR_PIN,0);			//end of pre-heating, power off the heater
	this->Setpoint=analogRead(this->UR_PIN);
	COM_SPI(INIT_REG1_WR|0x89);	//quit the calibration mode
	return 0;
}

void CJ125::run(void)
{
	this->Input=analogRead(this->UR_PIN);
	this->pid.Compute();
	analogWrite(this->HTR_PIN,byte(this->Output));
}

int CJ125::get_pinput(void)
{
	return this->Input;
}
int CJ125::get_psetp(void)
{
	return this->Setpoint;
}
int CJ125::get_poutput(void)
{
	return this->Output;
}
float CJ125::get_oxygen(void) 
{
  uint16_t value;
  value=analogRead(this->UA_PIN);
    //Declare and set default return value.
    float afr = 0;

    //Validate ADC range for lookup table.
    if (value > 854) value = 854;
    
    if (value >= 307 && value <= 854) {
      afr = pgm_read_float_near(afr_table + (value - 307));
    }
    
    //Return value.
    return afr;
    
}

//Function for transfering SPI data to the CJ125.
uint16_t CJ125::COM_SPI(uint16_t TX_data) 
{
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE1);
  SPI.setClockDivider(SPI_CLOCK_DIV128);

  //Set chip select pin low, chip in use.
  digitalWrite(this->CS_PIN, LOW);

  //Transmit and receive.
  byte highByte = SPI.transfer(TX_data >> 8);
  byte lowByte = SPI.transfer(TX_data & 0xff);

  //Set chip select pin high, chip not in use.
  digitalWrite(this->CS_PIN, HIGH);


  //Assemble response in to a 16bit integer and return the value.
  uint16_t Response = (highByte << 8) + lowByte;
  return Response;

}
