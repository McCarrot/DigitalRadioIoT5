#include "mbed.h"
#include "EthernetInterface.h"
#include <stdlib.h>
#include "DS1820.h"

// networking stuff
EthernetInterface eth;
TCPSocket TCP; 
char* ntpServerUrl = "0.ca.pool.ntp.org";
char* thingSpeakUrl = "instebo/iot/index.php";
char* thingSpeakKey = "8L7K45MLGZTB4JB3";
//Serial test(p9,p10);
char urlBuffer[256];
char timeBuffer[64];
const int addr = 0x90;      //adress of TC74 
char i2cread[2]  = {0x00, 0x01};
uint8_t input;
float input2;
uint8_t input3;

// pin defs
AnalogIn light(p20);
DigitalOut rst(p30);
DigitalIn btn(p12);

// Serials
Serial pc(USBTX,USBRX); 
Serial zigbee(p9,p10);
I2C i2c(p28,p27);
DS1820  ds1820(p21);    // substitute PA_9 with actual mbed pin name connected to the DS1820 data pin    

//timers
Timer timer;

// Funksions declarations
void data(float v1,float v2,uint8_t v3,int variables);
void send(char *arr,int lenght);
void startup();
float checkTemp(int address, char cmd[]);
// finished initsializing \\

int main ()
{
  /*
  int timerdata = 0;
  pc.printf("starter\r\n");
  startup();
  pc.printf("gi input\r\n");
  input = pc.getc();
  pc.printf("gi input\r\n");
  input2 = pc.getc();
  pc.printf("gi input\r\n");
  input3 = pc.getc();
  data(input,input2,input3,3);
  pc.printf("ferdig\r\n");
  */
  startup();
  while(true)
  {
    input = btn;
    input2 = light;
    data(checkTemp(addr,i2cread),input2,NULL,2);
    wait_us(250000000);
  }
}


 
void data(float v1,float v2,uint8_t v3,int variables)
{
  char recieve[256];         //recieve register
  // making a string to send
  switch (variables)
  {
    case 1:  
      sprintf(recieve, "GET %s&field1=%c\r\n",thingSpeakUrl,v1);
      break; 
    case 2: 
      sprintf(recieve, "GET %s&temp=%.2f&lys=%.2f\r\n",thingSpeakUrl,v1,v2);
      pc.printf(recieve, "GET %s&temp=%.2f&lys=%.2f\r\n",thingSpeakUrl,v1,v2);
      break;
    case 3: 
      sprintf(recieve, "GET %s?temp=%.2f&lys=%.2f&field3=%d\r\n",thingSpeakUrl,v1,v2,v3);
      break;
    default:
      pc.printf("Not a supported ammount of variables\r\n");
      break;
  }
  send(recieve,256);      //sendig to thingspeak
  pc.printf("sendte: %.2f,%.2f,%d",v1,v2,v3);
}

void send(char *arr,int lenght)
{
  TCP.open(&eth);
  TCP.connect("instebo.no",80);
  TCP.send(arr,lenght);
  pc.printf("sente %d antall bit: %s",lenght, *arr);
  TCP.close();
}

void startup()
{ 
  eth.connect();
  const char *ip = eth.get_ip_address();
  pc.printf("ip: %s",ip ? ip:"no ip");
}

float checkTemp(int address,char cmd[])
{
 bool temp = ds1820.begin();  
  wait_us(5000);                           
  if(temp) 
  {
    ds1820.startConversion();   // start temperature conversion
    wait_us(5000);                  // let DS1820 complete the temperature conversion
    float tmp = ds1820.read();
    pc.printf("temp = %3.1f\r\n", tmp);     // read temperature
    return tmp;
  } 
  else
  {
    pc.printf("No DS1820 sensor found!\r\n");
    return -1;
  }
}