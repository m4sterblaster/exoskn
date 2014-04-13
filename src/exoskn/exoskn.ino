#include <Servo.h>
#include <TimedAction.h>

// WRITE TO
const int MOTOR_PIN = 9;

// READ FROM
const int FLEX_PIN = 0;
const int TEMP_PIN = 1;
const int GALV_PIN = 2;
const int LED_R_PIN = 3;
const int LED_G_PIN = 5;
const int LED_B_PIN = 6;

// HEALTHINESS RETURN CODE
const int HEALTHY = 0;
const int FATAL = 666;
// index_warning: 1..10 (10 is condition)
const int WARN_MAX = 10;

Servo servo;

TimedAction motorThread = TimedAction(15,motorize);
TimedAction monitorThread = TimedAction(3000,monitor);
  
void setup()
{
  Serial.begin(9600);
  servo.attach(MOTOR_PIN);
}

void loop()
{
  motorThread.check();
  monitorThread.check();
}

void motorize()
{
  int flex_val = analogRead(FLEX_PIN);
  
  //Serial.print("flex: ");
  //Serial.println(flex_val);
  
  // normalise value to use servo range
  flex_val = map(flex_val, 50, 300, 0, 179);
  servo.write(flex_val);
}

void monitor()
{
  if (isHealthy()) 
  {
     isComfortable();
  }
}

// herustics for determining status of health
// @returns true if healthy
boolean isHealthy()
{
  // human body temp is 37
  // .. but since the sensor is strapped on the surface of skin
  // .. this threshold is to be tweaked after basic tests
  // .. value currently set for the purpose of demo
  // TODO set threshold in initialisation procedure that can tailor to each explorer
  float fatal_min_temp = 18;
  float warn_min_temp = 22;
  float warn_max_temp = 23.5;
  float fatal_max_temp = 30;
  float warn_conductance = 0.5; 
  
  // Array of health indexes for all metrics [temp, conductance]
  int healthIndexes[] = {HEALTHY, HEALTHY};

  float temp_val = getBodyTempreature();
  if (temp_val < fatal_min_temp || temp_val > fatal_max_temp)
  { 
    healthIndexes[0] = FATAL;
  } 
  else if (temp_val < warn_min_temp)
  {
     healthIndexes[0] = min((warn_min_temp-temp_val)*10, WARN_MAX);
  }
  else if (temp_val > warn_max_temp)
  {
     healthIndexes[0] = min((temp_val-warn_max_temp)*10, WARN_MAX);
  }  
  
  // TODO use conductance to guesstimate dehydration through sweat
  float cond_val = getConductance();
  if (cond_val > warn_conductance)
  { 
    // max conductance value from sensor is 5
    healthIndexes[1] = min((cond_val-warn_conductance)*2, WARN_MAX);
  }
  
  int condition = getMaxInArray(healthIndexes);
  showHealth(condition, healthIndexes);
  
  Serial.print("condition: ");
  Serial.println(condition);

  return (condition == HEALTHY);
}

// herustics for determining status of comfortness
// @returns true if comfortable
boolean isComfortable()
{
  //TODO
  return true;
}

// @return a value that is proportional to conductance
float getConductance()
{
  float retval = 5 - voltageRead(GALV_PIN);
  Serial.print("galv: ");
  Serial.println(retval);
  return retval;
}

// @return tempreature in degree celcius
float getBodyTempreature()
{
  // voltage to temp conversion
  // .. 0.5V at 0 degree celcius
  // .. 0.01V per degree celcius
  float retval = voltageRead(TEMP_PIN);
  retval = (retval - 0.5) / 0.01;
  Serial.print("temp: ");
  Serial.println(retval);
  return retval;
}

// @return the voltage from given pin in volts 
float voltageRead(int pin)
{
  // pin_val range: 0-1023 
  // voltage range: 0-5
  float pin_val = analogRead(pin);
  return (pin_val * 5 / 1024);
}

// update health monitoring mechanism
// simple indicator: show the overall (worst) index
void showHealth(int condition, int healthIndexes[])
{
  // TODO add more output devices to show metric per device
  if (condition == HEALTHY)
  {
    setLED(0,255,0);
  } 
  else if (condition == FATAL)
  {
    setLED(255,0,0);
  }
  else
  {
    // the healthier the sharper the green (less amber/red-ish)
    int g = map(condition, 1, WARN_MAX, 50, 255);
    setLED(255,g,0);
  }
}

void setLED(int r, int g, int b)
{
  analogWrite(LED_R_PIN, r);
  analogWrite(LED_G_PIN, g);
  analogWrite(LED_B_PIN, b);
}

// get max in an array 
int getMaxInArray(int arr[])
{
  int retval = arr[0];
  for (int i = 1; i < sizeof(arr); i++)
  {
    if (arr[i] > retval)
    {
      retval = arr[i];
    }
  }

  return retval; 
}

