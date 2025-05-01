#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>
const uint64_t pipeOut = 000322;
const uint64_t pipeIn = 000321;
#define RS 2
#define EN 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define TRIM_BUTTON 8
#define TRIM 0
#define TRANSMIT 1
#define CENTER 2
uint8_t state;

RF24 radio(9, 10);
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

int throttleOffset = 0;
int pitchOffset = 0;
int rollOffset = 0;
int yawOffset = 0;


struct Signal{
  byte throttle;
  byte pitch;
  byte roll;
  byte yaw;
  byte aux1;
  byte aux2;
  byte aux3;
  byte aux4;
};

struct Telemetry{
  uint16_t id;
  uint16_t bat;
};

Signal data;
Telemetry telemetry;
void ResetData(){
  data.throttle = 0;
  data.pitch = 127;
  data.roll = 127;
  data.yaw = 127;
  data.aux1 = 0;
  data.aux2 = 0;
  data.aux3 = 0;
  data.aux4 = 0;
}

uint16_t ToMilliVolts(uint16_t adcValue){

  return (adcValue * 5000UL) >> 10;
}

uint16_t recieved = 0;
unsigned long start = millis();
uint16_t firstPacket;
void setup(){
  Serial.begin(9600);
  while(!Serial){}
  radio.begin();
  radio.openWritingPipe(pipeOut);
  radio.openReadingPipe(1, pipeIn);
  radio.setChannel(109);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
  ResetData();
  recieved = 0;
  start = millis();
  lcd.begin(16, 2);
  lcd.print("hello");
  Serial.println("hello");
  state = TRANSMIT;
}

int Border_Map(int val, int lower, int middle, int upper, bool reverse){
  val = constrain(val, lower, upper);
  if ( val < middle ){
    val = map(val, lower, middle, 0, 128);
  } else {
    val = map(val, middle, upper, 128, 255);
  }
  return ( reverse ? 255 - val : val );
}

void loop(){                          
  switch(state){
    case TRIM:
    {
      // Enter trimming by pressing button
      if(digitalRead(TRIM_BUTTON)){
        // Make sure button stopped bouncing
        delay(50);
        if(digitalRead(TRIM_BUTTON)){
          state = TRANSMIT;
          lcd.print("Transmitting");
          break;
        }     
      }

      break;
    }      
    case TRANSMIT:
    {
      // Enter trimming by pressing button
      if(digitalRead(TRIM_BUTTON)){
        // Make sure button stopped bouncing
        delay(50);
        if(digitalRead(TRIM_BUTTON)){
           state = TRIM;
           lcd.print("Trimming");
           break;
        }
      }
      data.roll = Border_Map( analogRead(A3) - rollOffset, 0, 512, 1023, true );
      data.pitch = Border_Map( analogRead(A2) - pitchOffset, 0, 512, 1023, true );      
      data.throttle = Border_Map( analogRead(A1) - throttleOffset,570, 800, 1023, false );
      data.yaw = Border_Map( analogRead(A0) - yawOffset, 0, 512, 1023, true );
      data.aux1 = Border_Map( analogRead(A4), 0, 512, 1023, true );
      data.aux2 = Border_Map( analogRead(A5), 0, 512, 1023, true );
      data.aux3 = 0;
      data.aux4 = 0;

      radio.write(&data, sizeof(Signal));
      
      while(radio.available()){
        radio.read(&telemetry, sizeof(telemetry));
        if(recieved == 0) firstPacket = telemetry.id;
        recieved++;
      }
      if(millis() - start >= 1000){
        Serial.print(recieved);
        Serial.print("/");
        Serial.print(telemetry.id - firstPacket);
        Serial.print("/");
        Serial.println(telemetry.id);
        recieved = 0;
        start = millis();
      }
      break;
    }
    case CENTER:
    {
      lcd.clear();
      lcd.print("Center the joysticks");
      unsigned long centerStart = millis();
      if(millis() - centerStart > 1000){
        throttleOffset = analogRead(A1);
        pitchOffset = analogRead(A2);
        rollOffset = analogRead(A3);
        yawOffset = analogRead(A0);
        state = TRANSMIT;
      }
      break;
    }
  }
}