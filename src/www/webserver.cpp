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
bool SaveNeeded = false;

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
                ReturnString += "<tr><td><input name='peer' type='submit' value='";
                ReturnString += ActiveWebPeer->GetPeriphName(Si);
                ReturnString += ": ";
                if (ActiveWebPeer->isPeriphSwitch(Si)) 
                {
                    if (ActiveWebPeer->GetPeriphValue(Si, 0) ==  1) 
                        ReturnString += "on";
                    else
                        ReturnString += "off";
                }
                if (ActiveWebPeer->isPeriphCombo(Si)) 
                {
                    dtostrf(ActiveWebPeer->GetPeriphValue(Si,3), 0, 2, Buf);
                    ReturnString += " (";
                    ReturnString += Buf;
                    ReturnString += " A)";
                }  
                if (ActiveWebPeer->GetPeriphType(Si) == SENS_TYPE_AMP) 
                {
                    dtostrf(ActiveWebPeer->GetPeriphValue(Si,3), 0, 2, Buf);
                    ReturnString += Buf;
                    ReturnString += " A";
                }
                if (ActiveWebPeer->GetPeriphType(Si) == SENS_TYPE_VOLT) 
                {
                    dtostrf(ActiveWebPeer->GetPeriphValue(Si,2), 0, 2, Buf);
                    ReturnString += Buf;
                    ReturnString += " V";
                }

                ReturnString += "'/</td></tr>";
            }
        }
        return ReturnString;
    }
    
    if (var == "Screens")
    {
        if (ActiveWebPeer == &Module)
        for (int i=0; i<MULTI_SCREENS; i++)
        {
                ReturnString += "<form action='/submit' method='POST'>";
                ReturnString += "<input type='text' name='Screen-" + String(i+1) + "' placeholder='";
                ReturnString += Screen[i].GetName();
                ReturnString += "'/>";
                ReturnString += "<input type='submit' value='Update Screen-'" + String(i + 1) + "'/></form>";
        }
        return ReturnString;
    }
    
    if (var == "PeerName")    return ActiveWebPeer->GetName();
    if (var == "PeriphName")  if (ActiveWebPeriph) return ActiveWebPeriph->GetName();
    if (var == "Nullwert")    if (ActiveWebPeriph) { dtostrf(ActiveWebPeriph->GetNullwert(), 0, 3, Buf); return String(Buf); }
    if (var == "VperAmp")     if (ActiveWebPeriph) { dtostrf(ActiveWebPeriph->GetVperAmp(), 0, 3, Buf); return String(Buf); }
    if (var == "Vin")         if (ActiveWebPeriph) return String(ActiveWebPeriph->GetVin());
    
    if (var == "AnzPeers")    return String(PeerList.size()+1);
    if (var == "AnzPeriphs")  return String(PeriphList.size());

    if (var == "PeerStatus") 
    {
        ReturnString += "<table><tr><td width=90>";
        ReturnString += "<b>Periph</b></td><td><b>on</b></td><td><b>off</b></td><td><b>Volt</b></td><td><b>Amp</b></td></tr><tr></tr>";
                
        for (int Si=0; Si<MAX_PERIPHERALS; Si++)
        {
            if (!ActiveWebPeer->isPeriphEmpty(Si))
            {
                ReturnString += "<tr><td>";
                ReturnString += ActiveWebPeer->GetPeriphName(Si);
                ReturnString += "</td><td>";
                if (ActiveWebPeer->GetPeriphValue(Si,0) == 1) ReturnString += "*";
                ReturnString += "</td><td>";
                if (ActiveWebPeer->GetPeriphValue(Si,1) == 1) ReturnString += "*";
                ReturnString += "</td><td>";
                dtostrf(ActiveWebPeer->GetPeriphValue(Si,2), 0, 2, Buf);
                ReturnString += Buf;
                ReturnString += "</td><td>";
                dtostrf(ActiveWebPeer->GetPeriphValue(Si,3), 0, 2, Buf);
                ReturnString += Buf;
                ReturnString += "</td></tr>";
            }
        }
        ReturnString += "</table>";
        return ReturnString;
    }
    
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
        Serial.println("schicke index_html");
        request->send(200, "text/html", index_html, processor);
        });
    server.on("/peer", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("schicke peer_html");
        if (ActiveWebPeer) request->send(200, "text/html", peer_html, processor);
        });
    server.on("/periph", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("schicke periph.html");
        if (ActiveWebPeriph) request->send(200, "text/html", periph_html, processor);
        });
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        Serial.println("schicke peerstatus.html");
        if (ActiveWebPeer) request->send(200, "text/html", peerstatus_html, processor);
        });
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String message = "dumm";
        String WebBuffer;
        Serial.println("in get");
        
        if      (request->hasParam(MESSAGE_ROOT)   and !request->getParam(MESSAGE_ROOT)->value().isEmpty())
        {
            message = request->getParam(MESSAGE_ROOT)->value();
            DEBUG3("/get: Server: root-message->value: %s\n\r", message);
            
            if (message == Module.GetName()) 
            {
                ActiveWebPeer = &Module;
                request->redirect("/peer");

            }
            else 
            {
                ActiveWebPeer = FindPeerByName((char *)message.c_str());
                request->redirect("/peer");
            }            
        } 
        else if (request->hasParam(MESSAGE_PEER)   and !request->getParam(MESSAGE_PEER)->value().isEmpty()) 
        {
            message = request->getParam(MESSAGE_PEER)->value();
            DEBUG3("/get: Server: peer-message->value: %s\n\r", message);
            
            for (int Si=0; Si<MAX_PERIPHERALS; Si++)
            {
                if (strncmp(message.c_str(), ActiveWebPeer->GetPeriphName(Si), strlen(ActiveWebPeer->GetPeriphName(Si))) == 0)
                {
                    ActiveWebPeriph = ActiveWebPeer->GetPeriphPtr(Si);
                    request->redirect("/periph");
                    break;
                }
            }
            if (message == "Update")
            {
                if (request->hasParam("PeerName") and !request->getParam("PeerName")->value().isEmpty())
                {
                    WebBuffer = request->getParam("PeerName")->value();
                    DEBUG3 ("Received from web: NewPeerName = %s\n\r", WebBuffer.c_str());  
                    
                    if (ActiveWebPeer) 
                    {
                        SaveNeeded = true;
                        ActiveWebPeer->SetName(WebBuffer.c_str());
                        if (ActiveWebPeer != &Module) SendWebPeerNameChange();
                        request->redirect("/peer");
                    }
                }
                else
                {
                    request->redirect("/peer");
                }
            }
            if (message == "Update Screen-1") 
            {    
                if (request->hasParam("Screen-1") and !request->getParam("Screen-1")->value().isEmpty())
                {
                    WebBuffer = request->getParam("Screen-1")->value();
                    DEBUG3 ("Received from web: NewScreen-1-Name = %s\n\r", WebBuffer.c_str());  
                    if (ActiveWebPeer == &Module)    
                    {
                        SaveNeeded = true;
                        Screen[0].SetName((char*) WebBuffer.c_str());
                        request->redirect("/peer");
                    }
                }
            }
            if (message == "back") 
            {   
                request->redirect("/");
            } 
            if (message == "status") 
            {   
                request->redirect("/status");
            } 
        }
        else if (request->hasParam(MESSAGE_PERIPH) and !request->getParam(MESSAGE_PERIPH)->value().isEmpty()) 
        {
            message = request->getParam(MESSAGE_PERIPH)->value();
            if (message == "Update Name") 
            {    
                if (request->hasParam("PeriphName") and !request->getParam("PeriphName")->value().isEmpty())
                {
                    WebBuffer = request->getParam("PeriphName")->value();
                    DEBUG3 ("Received from web: NewPeriphName = %s\n\r", WebBuffer.c_str());  
                    if (ActiveWebPeriph) 
                    {
                        SaveNeeded = true;
                        ActiveWebPeriph->SetName(WebBuffer.c_str());
                        if (ActiveWebPeer != &Module) SendWebPeriphNameChange();
                        request->redirect("/peer");
                    }
                }
            }
            if (message == "Update Null") 
            {
                if (request->hasParam("Nullwert") and !request->getParam("Nullwert")->value().isEmpty())
                {
                    WebBuffer = request->getParam("Nullwert")->value();
                    DEBUG3 ("Received from web: NewNullwert = %s\n\r", WebBuffer.c_str());  
                    if (ActiveWebPeriph) 
                    {
                        SaveNeeded = true;
                        ActiveWebPeriph->SetNullwert(atof(WebBuffer.c_str()));
                        SendWebNullwertChange();
                        request->redirect("/periph");
                    }
                }
            }
            if (message == "Update VpA") 
            {   
                if (request->hasParam("VperAmp")  and !request->getParam("VperAmp")->value().isEmpty())
                {
                    WebBuffer = request->getParam("VperAmp")->value();
                    DEBUG3 ("Received from web: NewVperAmp = %s\n\r", WebBuffer.c_str());  
                    if (ActiveWebPeriph) 
                    {
                        SaveNeeded = true;
                        ActiveWebPeriph->SetVperAmp(atof(WebBuffer.c_str()));
                        if (ActiveWebPeer != &Module) SendWebVperAmpChange();
                        request->redirect("/periph");
                    }
                }
            }   
            if (message == "back") 
            {   
                request->redirect("/peer");
            } 
        }
        else {
            request->redirect("/");
        }
        
        if (SaveNeeded)
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
        }
    });
    
  server.onNotFound(notFound);
}
