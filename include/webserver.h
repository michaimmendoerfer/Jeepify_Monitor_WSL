#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "www/index.h"
#include "www/peer.h"
#include "www/periph.h"

#define DEBUG1(...) if ((Module.GetDebugMode()) and (DEBUG_LEVEL > 0)) Serial.printf(__VA_ARGS__)
#define DEBUG2(...) if ((Module.GetDebugMode()) and (DEBUG_LEVEL > 1)) Serial.printf(__VA_ARGS__)
#define DEBUG3(...) if ((Module.GetDebugMode()) and (DEBUG_LEVEL > 2)) Serial.printf(__VA_ARGS__)

extern bool WebServerActive;

void InitWebServer();
void ToggleWebServer();

#endif