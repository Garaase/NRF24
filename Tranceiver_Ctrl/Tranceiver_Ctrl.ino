/*
* Arduino Wireless Communication Tutorial
*     Example 1 - Transmitter Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <arduino-timer.h>

RF24 radio(7, 8); // CE, CSN

enum state_t {send, receive, validate, execution, done} STATE;

auto timer1 = timer_create_default(); 
auto timer2 = timer_create_default(); 
Timer<16, millis, state_t *> t_timer;

const byte address1[6] = "00001";
const byte address2[6] = "00002";

const char NODE[9] = {1,2,3,4,5,6,7,8,9};
const char ML = 10;
const char DL = 4;
const char DS = 6;

const char CMD[5] = {'T', 'H', 'P', 'M', 'C'};

char sendData[20] = "";
char receiverData[32] = "";

unsigned long prevTime;
unsigned long currTime;
unsigned long prevMatchTime;
unsigned long currMatchTime;
int matchInterval = 10;
boolean sent = false;
boolean resent = false;
//char resentCnt = '0';
state_t tempstate = send;
char sendFailureCnt = 0;

bool run(state_t *_state);
void onSending();
void onReceiving();
boolean compare(char *s, char *d);
void onValidating(); 
void onEXECUTION();
void onPause();
//void sendCmd();
//void resend(char cnt);
void sendCmd(char cnt, char *_sendData);
void resend(char cnt, char *_sendData);

void setup() 
{
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address1);
  radio.openReadingPipe(1, address2);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  while (!Serial) ; 
  Serial.println("Transmitter ...");
  tempstate = send;
  t_timer.every(5000, run, &(tempstate));
  prevTime = millis();
  prevMatchTime = millis();
}

void loop() 
{

  t_timer.tick();
}

/***  ENTRY FUNCTION   ***/
bool run(state_t *_state)
{ 
  //onSending();
  currTime = millis()-prevTime;    
  prevTime = millis();
	Serial.print("==============\n");
  Serial.print("Period: ");
  Serial.println(currTime);
	Serial.print("==============\n");

  STATE = *_state;
  while (STATE != done)
  {    
    switch (STATE)
    {
      case send:
        onSending();
        break;
      case receive:
        onReceiving();
        break;
      case validate:
        onValidating();
        break;
      case execution:
        onEXECUTION();
        break;
      case done:
        onPause();
        break;
      default:
        onPause();
        break;
    }
  }
  return true;
}
/*************************/

/***  MAIN FUNCTIONS   ***/
void onSending()
{
  Serial.println("onSending()\n");
  
  if(!resent)
  { 
    Serial.println("Sending...");   
    sendCmd(sendFailureCnt, &sendData[0]);
    sendFailureCnt = '0';
  }
  else
  {
    Serial.println("Resending...");
    sendFailureCnt++;
    resend(sendFailureCnt, &sendData[0]);
  }

  STATE = receive;
}

void onReceiving()
{
  delay(10);
  if (radio.available()) {
    radio.read(&receiverData, ML);
  } 
  // Serial.print("Received: ");
  // Serial.println(receiverData);

  STATE = validate;
}

void onValidating()
{
    boolean match = compare(&sendData[0],&receiverData[0]);
      Serial.print("onValidating()\n");

  if(match || sendFailureCnt >= '5')
  {
    sendFailureCnt = '0';
    resent = false;
    STATE = execution;
    // Serial.print(sendData);
    // Serial.print(" ");
    // Serial.print(receiverData);
    Serial.print(" Rec OK\n");
  }
  else
  { 
    resent = true;
    STATE = send;
    Serial.print(sendData);
    Serial.print(" ");
    Serial.print(receiverData);
    Serial.print("Rec NOK\n");
  }
}

void onEXECUTION()
{
	Serial.print("onEXECUTION()\n");
  int _DL = receiverData[5];
  Serial.print("DATA: ");
  for (int i = DS;  i < DS+_DL; i++)
  {    
    Serial.print(receiverData[i]);
  }
  Serial.println(' ');
  delay(10);
  STATE = done;
}

void onPause()
{
	Serial.print("onPause()\n");
	Serial.print("--------------\n");
  delay(10);
  STATE = done;
}
/*******************************/

/***  SUPPORTING FUNCTIONS   ***/
void sendCmd(char cnt, char *_sendData)
{  
  
    if(cnt == '0') 
      Serial.println("----------- NEW --------->"); 
    else 
      Serial.println("Resent");
     
    radio.stopListening();

    _sendData[0] = NODE[0];
    _sendData[1] = CMD[0];
    _sendData[2] = random('A', 'Z');
    _sendData[3] = random('0', '9');
    _sendData[4] = cnt;
    
    radio.write(&_sendData, sizeof(_sendData));
    radio.startListening();
    prevMatchTime = millis();
  Serial.print("sendCmd: ");
  Serial.println(_sendData);
    sent = true;   
}

boolean compare(char *s, char *d)
{
  for (int i = 0;  i < 5; i++)
  {
    if( s[i] != d[i]) 
    {
      Serial.println("not equal");
      return false;
    }
  }
  return true;
}

void resend(char cnt, char *_sendData)
{
  Serial.print("Resent count: ");
  Serial.println(cnt);
  _sendData[7] = cnt;
  if(cnt > 0) sendCmd(cnt, _sendData); 
}