#include <ESP8266WiFi.h>
#include "./functions.h"

int8 chans[3] = {1, 6, 11};
int8 Z = 0;
int8 chan = 1;
void setup() 
{
  Serial.begin(9600);
  Serial.printf("\n\t           ♥ SDK version: %s\n\r", system_get_sdk_version());
  Serial.println(F("\n\r        ♥ CGD network scanner using ESP8266, by winners team"));  
  Serial.println(F("\n\r        |=========MAC=========|=====WiFi Access Point SSID=====|  |======MAC======|  Chnl  RSSI  APs  DEVs"));
  //Magic, dont touch those
  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(chan);
  wifi_set_promiscuous_rx_cb(promisc_cb);   
  wifi_promiscuous_enable(1);
  //start scanning
}

void loop() 
{
 for (int i = 0; i<10000000;){i+=1;}
 chan = chans[Z];
 Z++;
 if (Z>2){
  Z=0;
 }
 
}
