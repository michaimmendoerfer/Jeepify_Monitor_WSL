#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "main.h"
#include "webserver.h"

bool WebServerActive = true;
const char *ActivePage;

AsyncWebServer server(80);
const char* ssid = "Jeepify_Monitor";
const char* password = "";

const char* MESSAGE_ROOT   = "root";
const char* MESSAGE_PEER   = "peer";
const char* MESSAGE_PERIPH = "periph";

PeerClass   *ActiveWebPeer = NULL;
PeriphClass *ActiveWebPeriph = NULL;

String processor(const String& var)
{
    String ReturnString = "";
    char Buf[30];

    if (var == "TYPE")        if (ActiveWebPeriph) return "text";
                              else return "hidden";
    
    if (var == "ModulName") return Module.GetName();
    
    if (var == "Peers")
    {
        ReturnString += "<tr><td><input name='root' type='submit' value='";
        ReturnString += Module.GetName();
        ReturnString += "'/</td></tr>";

        for (int p=0; p<PeerList.size(); p++)
        {
        
            ReturnString += "<tr><td><input name='root' type='submit' value='";
            ReturnString += PeerList.get(p)->GetName();
            ReturnString += "'/</td></tr>";

        }
        return ReturnString;
    }

    if (var == "Periphs")
    {
        for (int Si=0; Si<MAX_PERIPHERALS; Si++)
        {
            if (!ActiveWebPeer->isPeriphEmpty(Si))
            {
                ReturnString += "<tr><td><input name='periph' type='submit' value='";
                ReturnString += ActiveWebPeer->GetPeriphName(Si);
                ReturnString += "'/</td></tr>";
            }
        }
        return ReturnString;
    }

    if (var == "PeriphValues")
    {
        
        ReturnString += "<div class='part'><input name='Nullwert' type='%TYPE%' placeholder='%Nullwert%' />";
        ReturnString += "<input name='periph' type='submit' value='UpdNullwert' /></div>";
        ReturnString += "<div class='part'><input name='VperAmp' type='%TYPE%' placeholder='%VperAmp%' />";
        ReturnString += "<input name='periph' type='submit' value='UpdVperAmp' /></div>";
        ReturnString += "<div class='part'><input name='Vin' type='%TYPE%' placeholder='%Vin%' />";
        ReturnString += "<input name='periph' type='submit' value='UpdVin' /></div>";

        return ReturnString;
    }
    
    if (var == "PeerName")    return ActiveWebPeer->GetName();
    if (var == "PeriphName")  if (ActiveWebPeriph) return ActiveWebPeriph->GetName();
    if (var == "Nullwert")    if (ActiveWebPeriph) { dtostrf(ActiveWebPeriph->GetNullwert(), 0, 3, Buf); return String(Buf); }
    if (var == "VperAmp")     if (ActiveWebPeriph) { dtostrf(ActiveWebPeriph->GetVperAmp(), 0, 3, Buf); return String(Buf); }
    if (var == "Vin")         if (ActiveWebPeriph) return String(ActiveWebPeriph->GetVin());
    
    return String();
}
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
bool SendWebPeriphNameChange()
{
    if (!ActiveWebPeriph) return false;
    
    JsonDocument doc; String jsondata; 
    char mac[13];

    doc[SEND_CMD_JSON_FROM]  = MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]    = MacByteToChar(mac, ActiveWebPeer->GetBroadcastAddress());
    doc[SEND_CMD_JSON_TS]    = millis();
    doc[SEND_CMD_JSON_TTL]   = SEND_CMD_MSG_TTL;
    doc[SEND_CMD_JSON_ORDER] = SEND_CMD_UPDATE_NAME;
    doc[SEND_CMD_JSON_VALUE] = ActiveWebPeriph->GetName();
    doc[SEND_CMD_JSON_PERIPH_POS] = ActiveWebPeriph->GetPos();
    
    serializeJson(doc, jsondata);  
    
    TSMsgSnd = millis();
    JeepifySend(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250, true);  

    return true;
}
bool SendWebPeerNameChange()
{
    if (!ActiveWebPeer) return false;
    
    JsonDocument doc; String jsondata; 
    char mac[13];

    doc[SEND_CMD_JSON_FROM]  = MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]    = MacByteToChar(mac, ActiveWebPeer->GetBroadcastAddress());
    doc[SEND_CMD_JSON_TS]    = millis();
    doc[SEND_CMD_JSON_TTL]   = SEND_CMD_MSG_TTL;
    doc[SEND_CMD_JSON_ORDER] = SEND_CMD_UPDATE_NAME;
    doc[SEND_CMD_JSON_VALUE] = ActiveWebPeer->GetName();
    doc[SEND_CMD_JSON_PERIPH_POS] = 99;
    
    serializeJson(doc, jsondata);  
    
    TSMsgSnd = millis();
    JeepifySend(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250, true);  
    
    return true;
}
bool SendWebVinChange()
{
    if (!ActiveWebPeriph) return false;
    
    JsonDocument doc; String jsondata; 
    char mac[13];

    doc[SEND_CMD_JSON_FROM]  = MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]    = MacByteToChar(mac, ActiveWebPeer->GetBroadcastAddress());
    doc[SEND_CMD_JSON_TS]    = millis();
    doc[SEND_CMD_JSON_TTL]   = SEND_CMD_MSG_TTL;
    doc[SEND_CMD_JSON_ORDER] = SEND_CMD_UPDATE_VIN;
    doc[SEND_CMD_JSON_VALUE] = ActiveWebPeriph->GetVin();
    doc[SEND_CMD_JSON_PERIPH_POS] = ActiveWebPeriph->GetPos();
    
    serializeJson(doc, jsondata);  
    
    TSMsgSnd = millis();
    JeepifySend(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250, true);  
    
    return true;
}
bool SendWebVperAmpChange()
{
    if (!ActiveWebPeriph) return false;
    
    JsonDocument doc; String jsondata; 
    char mac[13];

    doc[SEND_CMD_JSON_FROM]  = MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]    = MacByteToChar(mac, ActiveWebPeer->GetBroadcastAddress());
    doc[SEND_CMD_JSON_TS]    = millis();
    doc[SEND_CMD_JSON_TTL]   = SEND_CMD_MSG_TTL;
    doc[SEND_CMD_JSON_ORDER] = SEND_CMD_UPDATE_VPERAMP;
    doc[SEND_CMD_JSON_VALUE] = ActiveWebPeriph->GetVperAmp();
    doc[SEND_CMD_JSON_PERIPH_POS] = ActiveWebPeriph->GetPos();
        
    serializeJson(doc, jsondata);  
    
    TSMsgSnd = millis();
    JeepifySend(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250, true);  

    return true;
}
bool SendWebNullwertChange()
{
    if (!ActiveWebPeriph) return false;
    
    JsonDocument doc; String jsondata; 
    char mac[13];

    doc[SEND_CMD_JSON_FROM]  = MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]    = MacByteToChar(mac, ActiveWebPeer->GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]    = mac;
    doc[SEND_CMD_JSON_TS]    = millis();
    doc[SEND_CMD_JSON_TTL]   = SEND_CMD_MSG_TTL;
    doc[SEND_CMD_JSON_ORDER] = SEND_CMD_UPDATE_NULLWERT;
    doc[SEND_CMD_JSON_VALUE] = ActiveWebPeriph->GetNullwert();
    doc[SEND_CMD_JSON_PERIPH_POS] = ActiveWebPeriph->GetPos();
    
    serializeJson(doc, jsondata);  
    
    TSMsgSnd = millis();
    JeepifySend(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250, true);  

    return true;
}
void ToggleWebServer()
{   
    WebServerActive = !WebServerActive;
    if (WebServerActive) 
    {
        ActiveWebPeer = PeerList.get(0);
        ActiveWebPeriph = PeriphList.get(0);
        server.begin();
    }
    else 
    {
        server.end();
    }
}

void InitWebServer()
{
    Serial.printf("create AP = %d", WiFi.softAP(ssid, password));
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    int txPower = WiFi.getTxPower();
    Serial.print("TX power: ");
    Serial.println(txPower);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    
    ActiveWebPeer = &Module;
    ActiveWebPeriph = NULL;
    ActivePage = index_html;
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", ActivePage, processor);
        });
    server.on("/peer.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", peer_html, processor);
        });
    server.on("/periph.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", periph_html, processor);
        });


    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String message;
        String WebBuffer;
        Serial.println("in get");
        
        if (request->hasParam(MESSAGE_ROOT)) {
            message = request->getParam(MESSAGE_ROOT)->value();
            Serial.printf("Server: message->value: %s\n\r", message);
            
            if (message == Module.GetName()) 
            {
                ActiveWebPeer = &Module;
                ActivePage = peer_html;

            }
            else 
            {
                ActiveWebPeer = FindPeerByName((char *)message.c_str());
                ActivePage = peer_html;
            }
           
            /*if (message == "UpdPeriphName") 
            {    
                if (request->hasParam("PeriphName"))
                {
                  WebBuffer = request->getParam("PeriphName")->value();
                  if (WebBuffer != "")
                    { 
                        DEBUG3 ("Received from web: NewPeriphName = %s\n\r", WebBuffer.c_str());  
                        if (ActiveWebPeriph) 
                        {
                            SaveNeeded = true;
                            ActiveWebPeriph->SetName(WebBuffer.c_str());
                            if (ActiveWebPeer != &Module) SendWebPeriphNameChange();
                        }
                    }
                }
            }
            if (message == "UpdNullwert") 
            {
                if (request->hasParam("Nullwert"))
                {
                  WebBuffer = request->getParam("Nullwert")->value();
                  if (WebBuffer != "")
                    { 
                        DEBUG3 ("Received from web: NewNullwert = %s\n\r", WebBuffer.c_str());  
                        if (ActiveWebPeriph) 
                        {
                            SaveNeeded = true;
                            ActiveWebPeriph->SetNullwert(atof(WebBuffer.c_str()));
                            if (ActiveWebPeer != &Module) SendWebNullwertChange();
                        }
                    }
                }
            }
            if (message == "UpdVperAmp") 
            {   
                if (request->hasParam("VperAmp"))
                {
                  WebBuffer = request->getParam("VperAmp")->value();
                  if (WebBuffer != "")
                    { 
                        DEBUG3 ("Received from web: NewVperAmp = %s\n\r", WebBuffer.c_str());  
                        if (ActiveWebPeriph) 
                        {
                            SaveNeeded = true;
                            ActiveWebPeriph->SetVperAmp(atof(WebBuffer.c_str()));
                            if (ActiveWebPeer != &Module) SendWebVperAmpChange();
                        }
                    }
                }
            }
            if (message == "UpdVin") 
            {
                if (request->hasParam("Vin"))
                {
                  WebBuffer = request->getParam("Vin")->value();
                  if (WebBuffer != "")
                    { 
                        DEBUG3 ("Received from web: NewVin = %s\n\r", WebBuffer.c_str());  
                        if (ActiveWebPeriph) 
                        {
                            SaveNeeded = true;
                            ActiveWebPeriph->SetVperAmp(atoi(WebBuffer.c_str()));
                            if (ActiveWebPeer != &Module) SendWebVinChange();
                        }
                    }
                }  
            }
            
            if (message == "module") 
            {
                DEBUG3 ("Module aufgerufen\n\r");
                ActiveWebPeer   = &Module;
                ActiveWebPeriph = NULL;
                DEBUG2 ("aktueller Name = %s\n\r", ActiveWebPeer->GetName());
            }
            if (message == "prev") 
            {
                DEBUG3 ("Prev aufgerufen\n\r");
                if (ActiveWebPeer == &Module) 
                {
                    PeerClass *TempP = FindPrevPeer(NULL, MODULE_ALL, CIRCULAR, ONLINE);
                    if (TempP) ActiveWebPeer = TempP;
                    ActiveWebPeriph = FindNextPeriph(ActiveWebPeer, NULL, SENS_TYPE_ALL, ONLINE);  
                }
                else
                {
                    ActiveWebPeriph = FindPrevPeriph(NULL, ActiveWebPeriph, SENS_TYPE_ALL, CIRCULAR, ONLINE);
                    ActiveWebPeer   = FindPeerById(ActiveWebPeriph->GetPeerId());
                }

            }
            if (message == "next") 
            {
                DEBUG3 ("Next aufgerufen\n\r");
                if (ActiveWebPeer == &Module) 
                {
                    PeerClass *TempP = FindNextPeer(NULL, MODULE_ALL, CIRCULAR, ONLINE);
                    if (TempP) ActiveWebPeer = TempP;
                    ActiveWebPeriph = FindNextPeriph(ActiveWebPeer, NULL, SENS_TYPE_ALL, CIRCULAR, ONLINE);  
                }
                else
                {
                    ActiveWebPeriph = FindNextPeriph(NULL, ActiveWebPeriph, SENS_TYPE_ALL, CIRCULAR, ONLINE);
                    ActiveWebPeer   = FindPeerById(ActiveWebPeriph->GetPeerId());
                }
            }*/
        } 
        else if (request->hasParam(MESSAGE_PEER)) 
        {
            message = request->getParam(MESSAGE_PEER)->value();
            Serial.printf("Server: peer-message->value: %s\n\r", message);
            
            for (int Si=0; Si<MAX_PERIPHERALS; Si++)
            {
                if (message == ActiveWebPeer->GetPeriphName(Si)) 
                {
                    ActiveWebPeriph = ActiveWebPeer->GetPeriphPtr(Si);
                    ActivePage = periph_html;
                    break;
                }
            }
        }
        else if (request->hasParam(MESSAGE_PERIPH)) 
        {
            message = request->getParam(MESSAGE_PERIPH)->value();
            Serial.printf("Server: periph-message->value: %s\n\r", message);
            
            for (int Si=0; Si<MAX_PERIPHERALS; Si++)
            {
                if (message == ActiveWebPeer->GetPeriphName(Si)) 
                {
                    ActiveWebPeriph = ActiveWebPeer->GetPeriphPtr(Si);
                    ActivePage = periph_html;
                    break;
                }
            }
        }
        else {
            ActivePage = index_html;
        }
        request->send_P(200, "text/html", ActivePage, processor);
        
        /*if (SaveNeeded)
        {   
            if (ActiveWebPeer != &Module) SavePeers();
            else 
            {
                preferences.begin("JeepifyInit", false);
                preferences.putString("ModuleName", Module.GetName());
                preferences.end();
                DEBUG2 ("Neuer Module Name:%s gespeichert\n\r", Module.GetName());
            }
            SaveNeeded = false;
        }*/
    });
    
  server.onNotFound(notFound);
}
