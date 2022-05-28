/*
  Arduino Wireless Communication Tutorial
        Example 1 - Receiver Code

  by Dejan Nedelkovski, www.HowToMechatronics.com

  Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <stdlib.h>

#define int2arr(n,m) ((n%m)/(m/10)+'0')

//RF24 radio(7, 8); // CE, CSN, Nano 33 ble
RF24 radio(48, 49); // CE, CSN, MEGA

const byte address1[6] = "00001";
const byte address2[6] = "00002";

const char NODE_ID = 1;

char sendData[32] = "";
char receiverData[32] = "";
char message[32] = "";

const char ML = 10;
const char DL = 4;
int moistValuePerc = 012;
char moistValuePercStr[4];

int moistSense();
void onReceiver();
void onEXECUTION(char *cmdData);
void onDataCollection(char *cmdData);
void onSender(char *cmdData);

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address1);
  radio.openWritingPipe(address2);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {

  //Serial.println("Receiver: ");

  onReceiver();

  if (receiverData[0] == NODE_ID)
  {
    if (receiverData[1] == 'T')
    {
      onEXECUTION(&receiverData[0]);
      receiverData[0] = ' ';
    }
    else
    {
      onDataCollection(&receiverData[0]);
      receiverData[0] = ' ';
    }
  }
}

void onReceiver()
{
  if (radio.available()) {
    radio.read(&receiverData, sizeof(receiverData));
    Serial.println(receiverData);
  }
}


void onDataCollection(char *cmdData)
{
  noInterrupts();
  moistValuePerc = moistSense();
  interrupts();

  Serial.println("onDataCollection\n");

  Serial.print(moistValuePerc);
  Serial.println("\n");

  Serial.print(int2arr(moistValuePerc, 1000)-'0');
  Serial.print(int2arr(moistValuePerc, 100)-'0');
  Serial.print(int2arr(moistValuePerc, 10)-'0');
  Serial.println("\n");

  receiverData[5] = DL;
  receiverData[6] = int2arr(moistValuePerc, 1000);
  receiverData[7] = int2arr(moistValuePerc, 100);
  receiverData[8] = int2arr(moistValuePerc, 10);
  receiverData[9] = '%';
  onSender(&receiverData[0]);
}

void onEXECUTION(char *cmdData)
{
  Serial.println("onEXECUTION\n");
  receiverData[5] = DL;
  receiverData[6] = 'E';
  receiverData[7] = 'X';
  receiverData[8] = 'E';
  receiverData[9] = 'C';
  onSender(&receiverData[0]);
}

void onSender(char *cmdData)
{
  radio.stopListening();
  delay(1);
  radio.write(cmdData, ML);
  radio.startListening();
}

int moistSense()
{
  const int AirValue = 900;   //you need to replace this value with Value_1
  const int WaterValue = 780;  //you need to replace this value with Value_2
  int soilMoistureValue = analogRead(A1);  //put Sensor insert into soil
  int soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  if (soilmoisturepercent >= 100)
  {
    soilmoisturepercent = 100;
  }
  else if (soilmoisturepercent <= 0)
  {
    soilmoisturepercent = 0;
  }
  return soilmoisturepercent;
}
