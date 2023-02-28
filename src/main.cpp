#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial hc12(6, 5);

/**
 * https://forum.arduino.cc/t/serial-input-basics-updated/382007/2
 */

/**
 * Gate States: <left=0,right=1>, <L0,R1>, <0,1>, <1> (Binaer: 00=<0>, 01=<1>, 10=<2>, 11=<3>)
 * Commands for opening gates: <openleft>/<openright>, <0>/<1>
 */

// Variables and functions for Sending
int sendInterval = 5000;
unsigned long lastSentMillis = 0;
void send();

// Variables and functions for Receiving
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];
boolean newData = false;

int leftState = 0;
int rightState = 0;

char whichRelay[numChars] = {0};
int newRelayState = 0;

void recvWithStartEndMarkers();
void parseData();
void evaluateData();
void showParsedData();

// Relais
int RELAIS_LEFT_PIN = 7;
int RELAIS_RIGHT_PIN = 8;

void setup()
{
  Serial.begin(9600);
  hc12.begin(9600);

  pinMode(RELAIS_LEFT_PIN, OUTPUT);
  pinMode(RELAIS_RIGHT_PIN, OUTPUT);
}

void loop()
{
  // Sending
  unsigned long now = millis();
  if (now >= lastSentMillis + sendInterval)
  {
    send();
    lastSentMillis = now;
  }

  // Receiving
  recvWithStartEndMarkers();
  if (newData == true)
  {
    strcpy(tempChars, receivedChars);
    parseData();
    showParsedData();
    evaluateData();
    newData = false;
  }
}

void send()
{
  Serial.println("Sending...");
  hc12.print('<');
  hc12.print("openleft");
  hc12.println('>');
}

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

/**
 * <left=1,right=0> => left=1,right=0 => Save integers in variables `leftState` and `rightState`
 */
/* void parseData()
{
  char *strtokIndx;

  strtokIndx = strtok(tempChars, "=");
  strtokIndx = strtok(NULL, ",");
  leftState = atoi(strtokIndx);

  strtokIndx = strtok(NULL, "=");
  strtokIndx = strtok(NULL, ",");
  rightState = atoi(strtokIndx);
} */

/**
 * Expected format: <relayleft=0>
 */
void parseData()
{
  char *strtokIndx;

  strtokIndx = strtok(tempChars, "=");
  strcpy(whichRelay, strtokIndx);

  strtokIndx = strtok(NULL, ",");
  newRelayState = atoi(strtokIndx);
}

void evaluateData()
{
  if (strcmp(whichRelay, "relayleft") == 0)
  {
    digitalWrite(RELAIS_LEFT_PIN, newRelayState);
  }
  else if (strcmp(whichRelay, "relayright") == 0)
  {
    digitalWrite(RELAIS_RIGHT_PIN, newRelayState);
  }
}

void showParsedData()
{
  Serial.print("Received: ");
  Serial.print("whichRelay=");
  Serial.print(whichRelay);
  Serial.print(",");
  Serial.print("newRelayState=");
  Serial.println(newRelayState);
}