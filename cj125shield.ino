#include <cj125.h>
#include <table.h>

#define version  "1.0b"

#include <Arduino.h>
#include <cj125.h>

/* CJ125 */

#define   CJ_CS     10
#define   CJ_HEATER 5
#define   CJ_UR     A1
#define   CJ_UA     A0
#define   UB_ANALOG_INPUT_PIN A2
uint16_t cj_status;
float oxy;
CJ125 cj(CJ_CS, CJ_HEATER,CJ_UR, CJ_UA);


float UBAT;
unsigned long delayTime;


float get_bat(void)
{
  UBAT = float(analogRead(UB_ANALOG_INPUT_PIN));
  UBAT = (UBAT * 15) / 1023;
  return UBAT;
}

void O2_health(void)
{
  uint16_t status;
  // O2 SENSOR HEALTH CHECK
  status=cj.check_stat();
  
  while (status != CJ125_OK)
  {
    if (status == CJ125_E_NOPOWER)
    {
      Serial.println("O2 Health check: No power!");
    }
    if (status == CJ125_E_SHORTCIRCUITGND || status==CJ125_E_SHORTCIRCUITBAT)
    {
      Serial.println("O2 Health check: Short Circuit");
    }
    delay(2000);
    status=cj.check_stat();
  }

  Serial.println("O2 Health OK");
  delay(2000);
}

void O2_calibrate(void)
{
  Serial.print ("Battery voltage:"); Serial.println(get_bat());
  Serial.println ("Calibrating, please wait");
  cj.calibrate(get_bat());
  Serial.println("Calibration process finished.");  
}


void setup() {



  //Set up digital output pins.

  pinMode(CJ_HEATER,OUTPUT);
  analogWrite(CJ_HEATER, 0); /* PWM is initially off. */

  Serial.begin(9600);
  while (!Serial);   // time to get serial running
  Serial.print("O2 Shield version "); Serial.println(version);
  
  //O2 Init
  Serial.println("Looking for O2...");
  cj_status = cj.check_id();
   
  while (cj_status==-1)
  {
    Serial.println("O2 not responding");
    delay(1000);
  }

  Serial.print("Found O2, revision: "); Serial.println(cj_status);
  delay(2000);
  O2_calibrate();
  O2_health();
  
}


void loop() {
    UBAT = analogRead(UB_ANALOG_INPUT_PIN);
    UBAT = (UBAT * 15) / 1023;
    uint16_t test;
    test=cj.check_stat();
    if (test!=CJ125_OK)
    {
      //coś się zepsuło z układem pomiarowym
      cj_status=test;
      analogWrite(CJ_HEATER,0);
      O2_health(); O2_calibrate();
    }
    if (cj_status!=CJ125_OK && cj.check_stat()==CJ125_OK)
    {
      //recovering from fault - restart of a calibration procedure
      cj.calibrate(get_bat());
      cj_status=CJ125_OK;
    }
    
    if (cj_status==CJ125_OK && cj.check_stat()==CJ125_OK)
    {
      //normal operation
      cj.run();
      oxy=cj.get_oxygen();
      Serial.print("AFR: "); Serial.println(oxy);
    }
  delay(500);

}
