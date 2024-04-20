#ifndef WioMQTT_h
#define WioMQTT_h
#include <stdint.h>
#include "rpcWiFi.h"
#include "Network.h"
#include <Time.h>
// #include <WiFiClient.h>
// #include <Ethernet.h>
#include <PubSubClient.h>

class WioMQTT {
  private:
    Network *_network;
  public:
    WioMQTT();
    WioMQTT(Network &_network);
    void Begin();
    void Reconnect();
    void Publish(char* topic, char* msg);
    bool isConnected = false;
    const char* root_topic = "jake/work";
    bool in_meeting;
    uint32_t minutes_rem;
    time_t cob;
};
#endif