/* Author: Michał Byrecki
 * License: GPLv2
 */

#ifndef CJ125_h
#define CJ125_h

#include "Arduino.h"
#include <SPI.h>
#include <PID_v1.h>

#define		CJ125_OK		0
#define		CJ125_E_NOPOWER		1
#define		CJ125_E_SHORTCIRCUITGND	2
#define		CJ125_E_SHORTCIRCUITBAT	3
class CJ125
{
  public:
	CJ125(uint16_t CS_PIN, uint16_t HTR_PIN, uint16_t UR_PIN, uint16_t UA_PIN);
	int check_id (void);
	int check_stat(void);
	int calibrate(float UBAT);
	int get_psetp(void);
	int get_pinput(void);
	int get_poutput(void);
	float get_oxygen(void);

	void run(void);
  private:
		uint16_t UR_PIN;
		uint16_t UA_PIN;

		double HTR_SETP;
		uint16_t CS_PIN;
		uint16_t HTR_PIN;
	        uint16_t COM_SPI(uint16_t TX_data);
		double Input,Output,Setpoint,kp, ki,kd;
		uint16_t mode,pon;
		PID pid;
		

};
#endif
