//#define KILL_NVS 

#include "main.h"

#ifdef DEV_240_KLEIN
    #include "Dev_240_klein_scr_tft.h"
#endif

#ifdef DEV_360_SILVER
    #include "Dev_360_silver_scr_tft.h"
#endif
#ifdef DEV_360_KNOB
    #include "Dev_360_knob_scr_tft.h"
#endif
#ifdef DEV_466_RED
    #include "Dev_466_red_scr_tft.h" 
#endif

#pragma region Globals

const char *ArrNullwert[MAX_PERIPHERALS] = {"NW0",  "NW1",  "NW2",  "NW3",  "NW4",  "NW5",  "NW6",  "NW7",  "NW8" };
const char *ArrVperAmp[MAX_PERIPHERALS] =  {"VpA0", "VpA1", "VpA2", "VpA3", "VpA4", "VpA5", "VpA6", "VpA7", "VpA8"};
const char *ArrVin[MAX_PERIPHERALS] =      {"Vin0", "Vin1", "Vin2", "Vin3", "Vin4", "Vin5", "Vin6", "Vin7", "Vin8"};
const char *ArrPeriph[MAX_PERIPHERALS]   = {"Per0", "Per1", "Per2", "Per3", "Per4", "Per5", "Per6", "Per7", "Per8"};

int PeerCount;
Preferences preferences;
uint8_t broadcastAddressAll[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
const char *broadCastAddressAllC = "FFFFFFFFFFFF";

MyLinkedList<ConfirmStruct*> ConfirmList = MyLinkedList<ConfirmStruct*>();
MyLinkedList<ReceivedMessagesStruct*> ReceivedMessagesList = MyLinkedList<ReceivedMessagesStruct*>();

PeerClass Module;
KnobStruct Knob;

String jsondataBuf;

uint32_t TSPing          = 0;

volatile uint32_t TSMsgRcv  = 0;
volatile uint32_t TSMsgSnd  = 0;
volatile uint32_t TSMsgPDC  = 0;
volatile uint32_t TSMsgBat  = 0;
volatile uint32_t TSMsgVolt = 0;
volatile uint32_t TSMsgEich = 0;
volatile uint32_t TSMsgPair = 0;
volatile uint32_t TSPair    = 0;
volatile uint32_t TSConfirm = 0;

int ActiveMultiScreen;
#pragma endregion Globals

#pragma region Main
void OnDataRecv(const esp_now_recv_info *info, const uint8_t* incomingData, int len) 
{
    PeerClass *P;
    
    char* buff = (char*) incomingData;   
    
    JsonDocument doc; 
    String jsondata = String(buff); 

    Serial.println(jsondata);
    
    String BufS; char Buf[50] = {};
    bool SaveNeeded = false;
    bool NewPeer    = false;
    char buf[100];
    
    jsondataBuf = jsondata;
    PrepareJSON();
    
    DeserializationError error = deserializeJson(doc, jsondata);

    if (!error) // erfolgreich JSON
    {
        int         _Status      = (int)doc[SEND_CMD_JSON_STATUS];
        int         _Order       = (int)doc[SEND_CMD_JSON_ORDER];   
        uint32_t    _TS          = (uint32_t)doc[SEND_CMD_JSON_TS];
        
        uint8_t _From[6];
        const char *MACF = doc[SEND_CMD_JSON_FROM];
        MacCharToByte(_From, (char *) MACF);
        uint8_t _To[6];
        const char *MACT = doc[SEND_CMD_JSON_TO];
        MacCharToByte(_To, (char *) MACT);
        
        if ( (memcmp(_To, Module.GetBroadcastAddress(), 6)) and (memcmp(_To, broadcastAddressAll, 6)) ) return;
        DEBUG2 ("%s\n\rwird verarbeitet - Order:%d\n\r", jsondata.c_str(), _Order);

        //already recevied?
        if (ReceivedMessagesList.size() > 0)
        { 
            for (int i=ReceivedMessagesList.size()-1; i>=0; i--)
            {
                ReceivedMessagesStruct *RMItem = ReceivedMessagesList.get(i);
                
                if ( (memcmp(RMItem->From, _From, 6) == 0) and (RMItem->TS ==_TS) )
                {
                    DEBUG3 ("Message schon verarbeitet\n\r");
                    return;
                }
            }
        }   
        ReceivedMessagesStruct *RMItem = new ReceivedMessagesStruct;
        memcpy(RMItem->From, _From, 6);
        RMItem->TS = _TS;
        RMItem->SaveTime = millis();
        ReceivedMessagesList.add(RMItem);
        DEBUG3 ("%d.Message registriert\n\r", ReceivedMessagesList.size());
            
        P = FindPeerByMAC(_From);
        
        if (P)
        {
            Serial.printf("bekannter Peer: %s\n\r", P->GetName());
            
            if ((Module.GetDebugMode()) and (millis() - P->GetTSLastSeen() > OFFLINE_INTERVAL)) ShowMessageBox("Peer online", P->GetName(), 1000, 200);
            P->SetTSLastSeen(millis());
        
            #ifdef MODULE_MONITOR_240
                P->SetdBm(0); 
            #else
                P->SetdBm(info->rx_ctrl->rssi); 
            #endif
        }

        TSMsgRcv = millis();
        
        switch (_Order)
        {
            case SEND_CMD_PAIR_ME:
                // new Peer wants to pair and module too - create it
                if ((!P) and Module.GetPairMode())
                {
                    const char *_PeerName    = doc[SEND_CMD_JSON_PEER_NAME];
                    int         _Type        = (int) (doc[SEND_CMD_JSON_MODULE_TYPE]);
                    const char *_PeerVersion = doc[SEND_CMD_JSON_VERSION];
                    
                    Serial.println("neuen Peer erstellen");
                    P = new PeerClass();
                    PeerList.add(P);
                    SaveNeeded = true;
                    NewPeer    = true;
                    Module.SetPairMode(false); TSPair = 0;
                    P->Setup(_PeerName, _Type, _PeerVersion, _From, (bool) bitRead(_Status, 1), (bool) bitRead(_Status, 0), (bool) bitRead(_Status, 2), (bool) bitRead(_Status, 3));                    

                    if (Module.GetDebugMode()) ShowMessageBox("Peer added...", doc[SEND_CMD_JSON_PEER_NAME], 2000, 150);

                    for (int Si=0; Si<MAX_PERIPHERALS; Si++) 
                    {
                        DEBUG3 ("Check Pairing for: %s\n\r", ArrPeriph[Si]);
                        
                        if (doc[ArrPeriph[Si]].is<JsonVariant>()) // vielleicht String (Per+Si)
                        {
                            strcpy(buf, doc[ArrPeriph[Si]]);
                            int   _PeriphType = atoi(strtok(buf, ";"));
                            char *_PeriphName = strtok(NULL, ";");
                            P->PeriphSetup(Si, _PeriphName, _PeriphType, P->GetId()); 
                            
                            P->SetPeriphChanged(Si, true);
                            PeriphList.add(P->GetPeriphPtr(Si));
                            SaveNeeded = true;
                            DEBUG2 ("%s->Periph[%d].Name is now: %s\n\r", P->GetName(), Si, P->GetPeriphName(Si));
                        }
                    }
                }
                if (P) // already known or just created - confirm
                {
                    SendPairingConfirm(P);
                }
                
                break;
            case SEND_CMD_STATUS:
                if (P)
                {
                    // check for module name change
                    if (doc[SEND_CMD_JSON_PEER_NAME].is<JsonVariant>())
                    {
                        const char *_PeerName    = doc[SEND_CMD_JSON_PEER_NAME];
                        Serial.printf("PeerName: %s - ", _PeerName);
                        if (strcmp(_PeerName, P->GetName())) P->SetName(_PeerName);
                    }    
                    
                    for (int Si=0; Si<MAX_PERIPHERALS; Si++) 
                    {
                        //DEBUG3 ("Check values of: %s\n\r", ArrPeriph[Si]);
                        
                        if (doc[ArrPeriph[Si]].is<JsonVariant>())
                        {
                            strcpy(buf, doc[ArrPeriph[Si]]);
                            int   _PeriphType = atoi(strtok(buf, ";"));
                            char *_PeriphName = strtok(NULL, ";");
                            float _Value0     = atof(strtok(NULL, ";"));
                            float _Value1     = atof(strtok(NULL, ";"));
                            float _Value2     = atof(strtok(NULL, ";"));
                            float _Value3     = atof(strtok(NULL, ";"));
                            
                            // check for periph name change
                            if (strcmp(_PeriphName, P->GetPeriphName(Si))) P->SetPeriphName(Si, _PeriphName);
                            
                            P->SetPeriphOldValue(Si, P->GetPeriphValue(Si, 0), 0);// überflüssig?
                            P->SetPeriphValue(Si, _Value0, 0);
                            P->SetPeriphOldValue(Si, P->GetPeriphValue(Si, 1), 1);
                            P->SetPeriphValue(Si, _Value1, 1);
                            P->SetPeriphOldValue(Si, P->GetPeriphValue(Si, 2), 2);
                            P->SetPeriphValue(Si, _Value2, 2);
                            P->SetPeriphOldValue(Si, P->GetPeriphValue(Si, 3), 3);
                            P->SetPeriphValue(Si, _Value3, 3);

                            P->AddPeriphSavedValue(Si, _Value0, _Value1, _Value2, _Value3);

                            P->SetPeriphChanged(Si, false); //werte wieder uptodate

                            if (_Status)
                            {
                                P->SetDebugMode ((bool) bitRead(_Status, 0));
                                P->SetSleepMode ((bool) bitRead(_Status, 1));
                                P->SetDemoMode  ((bool) bitRead(_Status, 2));
                                P->SetPairMode  ((bool) bitRead(_Status, 3));
                            } 
                            
                            DEBUG3 ("%s->%s values are: %.2f - %.2f - %.2f - %.2f\n\r", P->GetName(), P->GetPeriphName(Si), 
                                P->GetPeriphValue(Si, 0), P->GetPeriphValue(Si, 1), P->GetPeriphValue(Si, 2), P->GetPeriphValue(Si, 3));
                        }
                    }
                }
                break;
    
            case SEND_CMD_CONFIRM:
                if (P) // and (doc[SEND_CMD_JSON_CONFIRM].is<JsonVariant>()))
                {                    
                    DEBUG2 ("Confirm (%lu) empfangen von %s\n\r", _TS, P->GetName());
                    for (int i=0; i<ConfirmList.size(); i++)
                    {
                        ConfirmStruct *TempConfirm;
                        TempConfirm = ConfirmList.get(i);
                        DEBUG3 ("empfangener TS ist: %lu - durchsuchter TS (List[%d]) ist: %lu\n\r", _TS, i, TempConfirm->TSMessage);
                        if (TempConfirm->TSMessage == _TS)
                        {
                            TempConfirm->Confirmed = true;
                            DEBUG2 ("Found at list[%d] - DELETED\n\r", i);
                        }
                    }
                }
                break;
        } 
        
        if (SaveNeeded)
        {
            SavePeers();
            SaveNeeded = false;
            if (Module.GetDebugMode()) ShowMessageBox("Saving...", "complete", 1000, 200);
            delay(500);
            ESP.restart();
        }
    }
    else // Error bei JSON
    {        
        Serial.print(F("deserializeJson() failed: ")); 
        Serial.println(error.f_str());
        Serial.printf("jsondata was: %s\n\r", jsondata);
        return;
    }
}

void setup() 
{
    //delay(2000);
    Serial.begin(115200);
    scr_lvgl_init();

    #ifdef KILL_NVS
        nvs_flash_erase(); nvs_flash_init();
        while(1)
        {}
    #endif
    
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin();
    uint8_t MacTemp[6];
    WiFi.macAddress(MacTemp);
    
    Module.Setup(NODE_NAME, NODE_TYPE, MODULE_VERSION, MacTemp, false, true, false, false);
    
    InitWebServer();
    
    WebServerActive = !WebServerActive;
    ToggleWebServer();

    if (esp_now_init() != ESP_OK) { DEBUG1 ("Error initializing ESP-NOWn\r"); return; }

    esp_now_register_send_cb((esp_now_send_cb_t) OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);   
    
    //Get saved Peers  
    preferences.begin("JeepifyInit", true);
        Module.SetDebugMode(preferences.getBool("DebugMode", true));
        Module.SetSleepMode(preferences.getBool("SleepMode", false));
        Module.SetName(preferences.getString("ModuleName", NODE_NAME).c_str());
    preferences.end();
    
    GetPeers();
    RegisterPeers();
    ReportAll();
  
    static uint32_t user_data = 10;
    lv_timer_t * TimerPing    = lv_timer_create(SendPing, PING_INTERVAL,  &user_data);
    //lv_timer_t * TimerStatus  = lv_timer_create(SendStatus, PING_INTERVAL,  &user_data);
    lv_timer_t * TimerGarbage = lv_timer_create(GarbageMessages, PING_INTERVAL,  &user_data);

    #ifdef MON_HAS_PERIPH
        lv_timer_t * TimerPeriph = lv_timer_create(SendStatus, PING_INTERVAL,  &user_data);
    #endif

    ui_init();
}
void loop() 
{
    #ifdef DEV_360_KNOB
        static auto lv_last_tick = millis();
        auto const  now = millis();
        lv_tick_inc(now - lv_last_tick);
        lv_last_tick = now;
        delay(5);
    #endif

    lv_timer_handler(); 
    //delay(5);
}
#pragma endregion Main

#pragma region Send-Things
void GarbageMessages(lv_timer_t * timer)
{
    //Serial.printf("free Heap: %d\n\r", ESP.getFreeHeap());

    //Serial.println("Garbage-Kollektion");
    if (ReceivedMessagesList.size() > 0)
    {  
        for (int i=ReceivedMessagesList.size()-1; i>=0; i--)
        {
            ReceivedMessagesStruct *RMItem = ReceivedMessagesList.get(i);
            
            if (millis() > RMItem->SaveTime + SEND_CMD_MSG_HOLD*1000)
            {
                DEBUG3 ("Garbage-Kollektion: Message aus RMList entfernt\n\r");
                ReceivedMessagesList.remove(i);
                delete RMItem;
            }
        }
    }
}
esp_err_t  JeepifySend(const uint8_t *peer, const uint8_t *data, size_t len, bool ConfirmNeeded = false)
{
    if (!ConfirmNeeded)
    {
        esp_err_t SendStatus = esp_now_send(broadcastAddressAll, data, 250);
        return SendStatus;
    }
    else
    {
        JsonDocument doc;
        
        char* buff = (char*) data;   
        String jsondata = String(buff); 
        
        DeserializationError error = deserializeJson(doc, jsondata);

        if (!error) // erfolgreich JSON
        {
            doc[SEND_CMD_JSON_CONFIRM] = 1;
        
            serializeJson(doc, jsondata);  
            
            esp_err_t SendStatus = esp_now_send(broadcastAddressAll, (uint8_t *)jsondata.c_str(), 250);
            
            DEBUG3 ("SendStatus was %d, ConfirmNeeded = %d\n\r", SendStatus, ConfirmNeeded);
            uint8_t _To[6];
            const char *MacToS = doc[SEND_CMD_JSON_TO];
            MacCharToByte(_To, (char *) MacToS);

            ConfirmStruct *Confirm = new ConfirmStruct;
            memcpy(Confirm->Address, _To, 6);
            strcpy(Confirm->Message, (const char *)data);
            Confirm->Confirmed = false;
            Confirm->TSMessage = doc[SEND_CMD_JSON_TS];
            Confirm->Try = 1;

            ConfirmList.add(Confirm);

            DEBUG2 ("added Msg: %s to ConfirmList\n\r", Confirm->Message, Confirm->Try);   
    
            return SendStatus;
        }
        else
        {   
            DEBUG1 ("JSON-error while adding doc[SEND_CMD_JSON_CONFIRM]\n\r");
            return -1;
        }
    }
}
void SendPing(lv_timer_t * timer) {
    JsonDocument doc; String jsondata;
    char mac[13];

    PeerClass *P;
    
    MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_FROM]  = mac;
    doc[SEND_CMD_JSON_TO]    = broadCastAddressAllC;
    doc[SEND_CMD_JSON_TS]    = millis();
    doc[SEND_CMD_JSON_TTL]   = SEND_CMD_MSG_TTL;
    doc[SEND_CMD_JSON_ORDER] = SEND_CMD_STAY_ALIVE;

    if (Module.GetPairMode())
    {
        doc[SEND_CMD_JSON_PAIRING] = "aktiv";
    }

    serializeJson(doc, jsondata);  
    
    esp_now_send(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250);  
    
    if (ConfirmList.size() > 0)
    { 
        for (int i=ConfirmList.size()-1; i>=0; i--)
        {
            ConfirmStruct *Confirm = ConfirmList.get(i);
            Confirm->Try++;
            
            if (Confirm->Confirmed == true)
            {
                char TxtBuf[100];
                if (Module.GetDebugMode()) sprintf(TxtBuf, "SUCCESS - Message to %s successful confirmed after %d tries!", FindPeerByMAC(Confirm->Address)->GetName(), Confirm->Try);
                if (Module.GetDebugMode()) ShowMessageBox("SUCCESS", TxtBuf, 1000, 200);
                ConfirmList.remove(i);
                delete Confirm;
            }
            else if (Confirm->Try == JEEPIFY_SEND_MAX_TRIES+1)
            {
                char TxtBuf[100];
                if (Module.GetDebugMode()) sprintf(TxtBuf, "FAILED - Message to %s deleted after %d tries!", FindPeerByMAC(Confirm->Address)->GetName(), Confirm->Try);
                if (Module.GetDebugMode()) ShowMessageBox("FAILED", TxtBuf, 1000, 200);
                ConfirmList.remove(i);
                delete Confirm;
            }
            else
            {
                DEBUG3 ("%d: reSending Msg: %s from ConfirmList Try: %d\n\r", millis(), Confirm->Message, Confirm->Try);
                esp_err_t SendStatus = esp_now_send(broadcastAddressAll, (uint8_t*) Confirm->Message, 250); 
            }     
        }
    }

    if (ReceivedMessagesList.size() > 0)
    {  
        for (int i=ReceivedMessagesList.size()-1; i>=0; i--)
        {
            ReceivedMessagesStruct *RMItem = ReceivedMessagesList.get(i);
            
            if (millis() > RMItem->TS + SEND_CMD_MSG_HOLD*1000)
            {
                DEBUG3 ("Message aus RMList entfernt\n\r");
                ReceivedMessagesList.remove(i);
                delete RMItem;
            }
        }
    }
}
void SendPairingConfirm(PeerClass *P) {
    JsonDocument doc; String jsondata; 

    char mac[13];

    MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_FROM]        = mac;
    MacByteToChar(mac, P->GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]          = mac;
    doc[SEND_CMD_JSON_TS]          = millis();
    doc[SEND_CMD_JSON_ORDER]       = SEND_CMD_YOU_ARE_PAIRED;
    doc[SEND_CMD_JSON_MODULE_TYPE] = Module.GetType();
    doc[SEND_CMD_JSON_TTL]         = SEND_CMD_MSG_TTL;
    doc[SEND_CMD_JSON_PEER_NAME]   = Module.GetName();
    
    serializeJson(doc, jsondata);  
  
    TSMsgSnd = millis();
    esp_now_send(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250); 
    DEBUG2 ("Sent you are paired\n\r%s\n\r", jsondata.c_str());  
     
}
void SendStatus()
{

}
bool ToggleSwitch(PeerClass *P, int PerNr)
{
    return ToggleSwitch(P->GetPeriphPtr(PerNr));
}
bool ToggleSwitch(PeriphClass *Periph)
{
    JsonDocument doc; String jsondata; 
    
    char mac[13];

    MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_FROM]        = mac;
    MacByteToChar(mac, FindPeerById(Periph->GetPeerId())->GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]          = mac;
    doc[SEND_CMD_JSON_TS]          = millis();
    doc[SEND_CMD_JSON_ORDER]       = SEND_CMD_SWITCH_TOGGLE;
    doc[SEND_CMD_JSON_PERIPH_NAME] = Periph->GetName();
    doc[SEND_CMD_JSON_PERIPH_POS]  = Periph->GetPos();
    doc[SEND_CMD_JSON_TTL]         = SEND_CMD_MSG_TTL;
    
    serializeJson(doc, jsondata);  
    
    TSMsgSnd = millis();
    esp_now_send(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250);  //Sending "jsondata"  
    DEBUG3 ("%s", jsondata.c_str());
    
    return true;
}
void SendCommand(PeerClass *P, int Cmd, String Value) {
    JsonDocument doc; String jsondata; 
    char mac[13];

    MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_FROM]      = mac;
    MacByteToChar(mac, P->GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]        = mac;
    doc[SEND_CMD_JSON_TS]        = millis();
    doc[SEND_CMD_JSON_TTL]       = SEND_CMD_MSG_TTL;
    
    doc[SEND_CMD_JSON_ORDER]     = Cmd;
    if (Value != "") 
        doc[SEND_CMD_JSON_VALUE] = Value;
    
    serializeJson(doc, jsondata);  
    
    TSMsgSnd = millis();
    esp_now_send(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250);  //Sending "jsondata"  
    DEBUG3 ("%s", jsondata.c_str());
}

#pragma endregion Send-Things
#pragma region System-Screens
void PrepareJSON() {
  if (jsondataBuf) {
    JsonDocument doc;
  
    DeserializationError error = deserializeJson(doc, jsondataBuf);
    if (doc["Node"] != NODE_NAME) { 
      lv_textarea_set_placeholder_text(ui_TxtJSON1, jsondataBuf.c_str());
      jsondataBuf = "";
    }
  }
}
/*void TopUpdateTimer(lv_timer_t * timer)
{
	if ((TSPair)  and (millis() - TSPair < PAIR_INTERVAL)){
		lv_led_on(Ui_LedPair);
        #ifdef CYD_LED
            smartdisplay_led_set_rgb(255, 0, 0);
        #endif
	}
	else {
		lv_led_off(Ui_LedPair);
        #ifdef CYD_LED
            smartdisplay_led_set_rgb(0, 0, 0);
        #endif
		TSPair = 0;
		Module.SetPairMode(false);
	}
    
    if ((TSMsgSnd) and (millis() - TSMsgSnd < MSGLIGHT_INTERVAL)) {
		lv_led_on(Ui_LedSnd);
        #ifdef CYD_LED
            smartdisplay_led_set_rgb(0, 0, 255);
        #endif
	}
	else {
		lv_led_off(Ui_LedSnd);
        #ifdef CYD_LED
            if (TSPair) smartdisplay_led_set_rgb(255, 0, 0);
            else smartdisplay_led_set_rgb(0, 0, 0);
        #endif
		TSMsgSnd = 0;

	}

	if ((TSMsgRcv) and (millis() - TSMsgRcv < MSGLIGHT_INTERVAL)) {
		lv_led_on(Ui_LedRcv);
        #ifdef CYD_LED
            smartdisplay_led_set_rgb(0, 255, 0);
        #endif
	}
	else {
		lv_led_off(Ui_LedRcv);
        #ifdef CYD_LED
            if (TSPair) smartdisplay_led_set_rgb(255, 0, 0);
            else smartdisplay_led_set_rgb(0, 0, 0);
        #endif
		TSMsgRcv = 0;
	}

	#ifdef BATTERY_PORT
		lv_label_set_text_fmt(ui_LblMenuBatt, "%.2f", analogRead(BATTERY_PORT*BATTERY_DEVIDER));
	#endif
}*/
#pragma endregion System-Screens
#pragma region Other
void ShowMessageBox(const char * Titel, const char *Txt, int delay, int opa)
{
    static const char * btns[] = {""};

    lv_obj_t *MsgBox = lv_msgbox_create(lv_scr_act(), Titel, Txt, NULL, false);
    lv_obj_set_style_bg_color(MsgBox, lv_color_hex(0xAD0808), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(MsgBox, opa, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(MsgBox, lv_color_hex(0xDBDBDB), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(MsgBox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(MsgBox, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(MsgBox, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(MsgBox, lv_color_hex(0xDBDBDB), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(MsgBox, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_text_font(lv_msgbox_get_title(MsgBox), &lv_font_montserrat_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_center(MsgBox);
    lv_obj_del_delayed(MsgBox, delay);
}  
bool ToggleSleepMode() 
{
    preferences.begin("JeepifyInit", false);
        Module.SetSleepMode(!Module.GetSleepMode());
        if (preferences.getBool("SleepMode", false) != Module.GetSleepMode()) preferences.putBool("SleepMode", Module.GetSleepMode());
    preferences.end();
    return Module.GetSleepMode();
}
bool ToggleDebugMode() {
  preferences.begin("JeepifyInit", false);
      Module.SetDebugMode(!Module.GetDebugMode());
      if (preferences.getBool("DebugMode", false) != Module.GetDebugMode()) preferences.putBool("DebugMode", Module.GetDebugMode());
  preferences.end();
  return Module.GetDebugMode();
}
bool TogglePairMode() {
  if (Module.GetPairMode())
  {
      Module.SetPairMode(false);
      TSPair = 0;
  }
  else 
  {
      Module.SetPairMode(true);
      TSPair = millis();
  };

  DEBUG2 ("PairMode changed to: %d\n\r", Module.GetPairMode());
  
  return Module.GetPairMode();
}
void CalibVolt() {
    JsonDocument doc; String jsondata;

    char mac[13];

    doc[SEND_CMD_JSON_FROM]  = MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]    = MacByteToChar(mac, ActivePeer->GetBroadcastAddress());
    doc[SEND_CMD_JSON_TS]    = millis();
    doc[SEND_CMD_JSON_ORDER] = SEND_CMD_VOLTAGE_CALIB;
    doc[SEND_CMD_JSON_VALUE] = lv_textarea_get_text(ui_TxtVolt);
    doc[SEND_CMD_JSON_TTL]   = SEND_CMD_MSG_TTL;
    
    serializeJson(doc, jsondata);  

    JeepifySend(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250, true);  
        
    DEBUG3 ("%s", jsondata.c_str());
}
void CalibAmp() 
{
    JsonDocument doc; String jsondata;

    char mac[13];

    doc[SEND_CMD_JSON_FROM]  = MacByteToChar(mac, Module.GetBroadcastAddress());
    doc[SEND_CMD_JSON_TO]    = MacByteToChar(mac, ActivePeer->GetBroadcastAddress());
    doc[SEND_CMD_JSON_TS]    = millis();
    doc[SEND_CMD_JSON_ORDER] = SEND_CMD_CURRENT_CALIB;
    doc[SEND_CMD_JSON_TTL]   = SEND_CMD_MSG_TTL;
        
    serializeJson(doc, jsondata);  
    JeepifySend(broadcastAddressAll, (uint8_t *) jsondata.c_str(), 250, true);  

    DEBUG3 ("%s", jsondata.c_str());
}
void PrintMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}
void MacCharToByte(uint8_t *mac, char *MAC)
{
    sscanf(MAC, "%2x%2x%2x%2x%2x%2x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
}
char *MacByteToChar(char *MAC, uint8_t *mac)
{
    sprintf(MAC, "%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return MAC;
}

// 3.4.1 void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
void OnDataSent(const uint8_t *tx_info, esp_now_send_status_t status)
{ 
    if (status == ESP_NOW_SEND_SUCCESS)
    {
        //DEBUG3 ("Message send SUCCESS\n\r");
    }
    else 
    {
        DEBUG1 ("Message send FAILED\n\r");
    }
}

#pragma endregion Other
//
