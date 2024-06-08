extern "C" {
#include "user_interface.h"
  typedef void (*freedom_outside_cb_t)(uint8 status);
  int  wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
  void wifi_unregister_send_pkt_freedom_cb(void);
  int  wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
}

#include <ESP8266WiFi.h>
#include "./structures.h"

const int MAX_APS_TRACKED = 100;
const int MAX_CLIENTS_TRACKED = 500;

beaconinfo aps_known[MAX_APS_TRACKED];                   
int aps_known_count = 0;           
clientinfo clients_known[MAX_CLIENTS_TRACKED];            
int clients_known_count = 0;                              

int register_beacon(beaconinfo beacon)
{
  int known = 0;
  for (int u = 0; u < aps_known_count; u++)
  {
    if (!memcmp(aps_known[u].bssid, beacon.bssid, 6)) {
      known = 1;
      break;
    }
  }
  if (! known & beacon.err == 0)
  {
    memcpy(&aps_known[aps_known_count], &beacon, sizeof(beacon));
    aps_known_count++;
    if ((unsigned int) aps_known_count >=
        sizeof (aps_known) / sizeof (aps_known[0]) ) {
      Serial.printf("exceeded max aps_known\n");
      aps_known_count = 0;
    }
  }
  return known;
}

int register_client(clientinfo ci)
{
  int known = 0;   // Clear known flag
  for (int u = 0; u < clients_known_count; u++)
  {
    if (!memcmp(clients_known[u].station, ci.station, 6)) {
      known = 1;
      break;
    }
  }
  if (! known)
  {
    memcpy(&clients_known[clients_known_count], &ci, sizeof(ci));
    clients_known_count++;
    if ((unsigned int) clients_known_count >=
        sizeof (clients_known) / sizeof (clients_known[0]) ) {
      Serial.printf("exceeded max clients_known\n");
      clients_known_count = 0;
    }
  }
  return known;
}

void print_beacon(beaconinfo beacon)
{
  if (beacon.err == 0) {
    Serial.printf("ROUTER: <-------------------- {%32s}  ", beacon.ssid);
    for (int i = 0; i < 5; i++) Serial.printf("%02x:", beacon.bssid[i]);
    Serial.printf("%02x", beacon.bssid[5]);
    Serial.printf("   %2d", beacon.channel);
    Serial.printf("   %4d", beacon.rssi);
    Serial.printf("\r\n");
  }
}

void print_client(clientinfo ci)
{
  int u = 0;
  int known = 0;   
  if (ci.err ==0) {
    Serial.printf("DEVICE: ");
    for (int i = 0; i < 5; i++) Serial.printf("%02x:", ci.station[i]);
    Serial.printf("%02x", ci.station[5]);
    Serial.printf(" --> ");

    for (u = 0; u < aps_known_count; u++)
    {
      if (!memcmp(aps_known[u].bssid, ci.bssid, 6)) {
        Serial.printf("{%32s}", aps_known[u].ssid);
        known = 1;
        break;
      }
    }
    
    if (! known)  {
      Serial.printf("{%32s}", "AP UNKNOWN");
      Serial.printf("%2s", " ");
      Serial.printf("%17s","  ???");
      Serial.printf("  ???");
      Serial.printf("   %4d", ci.rssi);      
    } else {
      Serial.printf("%2s", " ");
      for (int i = 0; i < 5; i++) Serial.printf("%02x:", ci.bssid[i]);
      Serial.printf("%02x", ci.bssid[5]);
      Serial.printf("  %3d", aps_known[u].channel);
      Serial.printf("   %4d", ci.rssi);
    }
  }
  Serial.printf("\r\n");

}

void promisc_cb(uint8_t *buf, uint16_t len)
{
  signed ssi;
  if (len == 12) {
    struct RxControl *sniffer = (struct RxControl*) buf;
    ssi = sniffer->rssi;
  } else if (len == 128) {
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    struct beaconinfo beacon = parse_beacon(sniffer->buf, 112, sniffer->rx_ctrl.rssi);
    ssi = sniffer->rx_ctrl.rssi;
    
    if (register_beacon(beacon) == 0) {
      print_beacon(beacon);
    }    
    
  } else {
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    ssi = sniffer->rx_ctrl.rssi;

    //Is data or QOS
    if ((sniffer->buf[0] == 0x08) || (sniffer->buf[0] == 0x88)) {
      struct clientinfo ci = parse_data(sniffer->buf, 36, sniffer->rx_ctrl.rssi, sniffer->rx_ctrl.channel);
      if (memcmp(ci.bssid, ci.station, 6)) {
        if (register_client(ci) == 0) {
          print_client(ci);
        }
      }
    }
  }
  //В 12 позиции хранятся типы пакета
  // https://ilovewifi.blogspot.mx/2012/07/80211-frame-types.html
  if((buf[12]==0x88)||(buf[12]==0x40)||(buf[12]==0x94)||(buf[12]==0xa4)||(buf[12]==0xb4)||(buf[12]==0x08))
  {
    
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    ssi = sniffer->rx_ctrl.rssi;
      struct clientinfo ci = parse_data(sniffer->buf, 36, sniffer->rx_ctrl.rssi, sniffer->rx_ctrl.channel);
      if (memcmp(ci.bssid, ci.station, 6)) {
        if (register_client(ci) == 0) {
          print_client(ci);
        }
      }
  }
}
