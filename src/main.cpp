/**
 * GARAGENTOR STEUERUNG PER HC-12 - Arduino in der Garage mit 2 Relais und 2 Reed-Schaltern
 */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Bounce2.h>
#include <Ticker.h>

SoftwareSerial hc12(6, 5);

Bounce bounceGate1Pin = Bounce();
Bounce bounceGate2Pin = Bounce();

void switchOffRelay1();
Ticker relay1Timer(switchOffRelay1, 1000, 1);

void switchOffRelay2();
Ticker relay2Timer(switchOffRelay2, 1000, 1);

// Variables and functions for Sending
long sendInterval = 60000;
unsigned long lastSentMillis = 0;
void publishStates();
void checkGateStateChange();

// Variables and functions for Receiving
const byte numChars = 64;
char receivedChars[numChars];
char tempChars[numChars];
boolean newData = false;

void recvWithStartEndMarkers();
void parseData();
void setRelay1(int newState);
void setRelay2(int newState);

// PIN definition
int RELAY_1_PIN = 7;
int RELAY_2_PIN = 8;
int GATE_1_PIN = 9;
int GATE_2_PIN = 10;

void setup()
{
  Serial.begin(9600);
  hc12.begin(9600);

  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);

  bounceGate1Pin.attach(GATE_1_PIN, INPUT);
  bounceGate1Pin.interval(5000);
  bounceGate2Pin.attach(GATE_2_PIN, INPUT);
  bounceGate2Pin.interval(5000);
}

void loop()
{
  relay1Timer.update();
  relay2Timer.update();

  bounceGate1Pin.update();
  bounceGate2Pin.update();
  checkGateStateChange();

  // Sending
  unsigned long now = millis();
  if (now >= lastSentMillis + sendInterval)
  {
    publishStates();
    lastSentMillis = now;
  }

  // Receiving
  recvWithStartEndMarkers();
  if (newData == true)
  {
    strcpy(tempChars, receivedChars);
    parseData();
    newData = false;
  }
}

// https://forum.arduino.cc/t/serial-input-basics-updated/382007/2
void recvWithStartEndMarkers()
{
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (hc12.available() > 0 && newData == false)
  {
    rc = hc12.read();

    if (recvInProgress == true)
    {
      if (rc != endMarker)
      {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars)
        {
          ndx = numChars - 1;
        }
      }
      else
      {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker)
    {
      recvInProgress = true;
    }
  }
}

void parseData()
{
  Serial.print("Parsing ");
  Serial.println(tempChars);

  char *strtokIndx;
  char command[numChars] = {0};

  strtokIndx = strtok(tempChars, "=");
  strtokIndx = strtok(NULL, ",");
  strcpy(command, strtokIndx);

  if (strcmp(command, "setrelay1") == 0)
  {
    strtokIndx = strtok(NULL, "=");
    strtokIndx = strtok(NULL, ",");
    setRelay1(atoi(strtokIndx));
  }
  else if (strcmp(command, "setrelay2") == 0)
  {
    strtokIndx = strtok(NULL, "=");
    strtokIndx = strtok(NULL, ",");
    setRelay2(atoi(strtokIndx));
  }
}

void setRelay1(int newState)
{
  digitalWrite(RELAY_1_PIN, newState);
  publishStates();
  relay1Timer.start();
}

void switchOffRelay1()
{
  digitalWrite(RELAY_1_PIN, 0);
  publishStates();
}

void setRelay2(int newState)
{
  digitalWrite(RELAY_2_PIN, newState);
  publishStates();
  relay2Timer.start();
}

void switchOffRelay2()
{
  digitalWrite(RELAY_2_PIN, 0);
  publishStates();
}

void checkGateStateChange()
{
  if (bounceGate1Pin.changed() || bounceGate2Pin.changed())
  {
    publishStates();
  }
}

void publishStates()
{
  Serial.println("Publish states ");
  hc12.print('<');
  hc12.print("command");
  hc12.print("=");
  hc12.print("publishstates");
  hc12.print(",");
  hc12.print("relay1");
  hc12.print("=");
  hc12.print(digitalRead(RELAY_1_PIN));
  hc12.print(",");
  hc12.print("relay2");
  hc12.print("=");
  hc12.print(digitalRead(RELAY_2_PIN));
  hc12.print(",");
  hc12.print("gate1");
  hc12.print("=");
  hc12.print(digitalRead(GATE_1_PIN));
  hc12.print(",");
  hc12.print("gate2");
  hc12.print("=");
  hc12.print(digitalRead(GATE_2_PIN));
  hc12.println('>');
}