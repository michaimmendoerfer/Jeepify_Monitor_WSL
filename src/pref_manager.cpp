#include <Arduino.h>
#include "PeerClass.h"
#include <Preferences.h>
#include <esp_now.h>
#include <WiFi.h>
#include <pref_manager.h>

extern Preferences preferences;

void   PrintMAC(const uint8_t * mac_addr);

MultiMonitorClass Screen[MULTI_SCREENS];
MultiMonitorClass MultiGaugeScreen;

char ScreenExportImportBuffer[1000];

int  MultiMonitorClass::_ClassId = 1;

MultiMonitorClass::MultiMonitorClass() 
{ 
    _Id = _ClassId;
    _ClassId++;
    
    snprintf(_Name, sizeof(_Name), "Screen-%d", _Id);
    _Changed = false;
    
    for (int i=0; i<PERIPH_PER_SCREEN; i++)
    {
        _Peer[i] = NULL;
        _PeerId[i] = -1;
        _Periph[i] = NULL;
        _PeriphId[i] = -1;
        _Component[i] = NULL;
    }
    _Used = false;
}
char* MultiMonitorClass::Export() 
{
    char ReturnBufferScreen[50];

    // Ersten Teil sicher kopieren und geschriebene Bytes merken
    int written = snprintf(ScreenExportImportBuffer, sizeof(ScreenExportImportBuffer), "%s", _Name);
                        
    for (int Si=0; Si<PERIPH_PER_SCREEN; Si++)
    {   
        // Abbrechen, falls der Puffer voll sein sollte
        if (written >= (int)sizeof(ScreenExportImportBuffer) - 1) break;

        snprintf(ReturnBufferScreen, sizeof(ReturnBufferScreen), ";%d", _PeriphId[Si]);
        
        int res = snprintf(ScreenExportImportBuffer + written, sizeof(ScreenExportImportBuffer) - written, "%s", ReturnBufferScreen);
        if (res > 0) {
            written += res;
        }
    }

    return ScreenExportImportBuffer;
}
void MultiMonitorClass::Import(char *Buf) 
{
    if (!Buf) return;
    
    char *token = strtok(Buf, ";");
    if (token != NULL) {
        strncpy(_Name, token, sizeof(_Name) - 1);
        _Name[sizeof(_Name) - 1] = '\0';
    }
    
    for (int Si = 0; Si < PERIPH_PER_SCREEN; Si++)
    {  
        char *nextTok = strtok(NULL, ";");
        if (!nextTok) 
        {
            _PeriphId[Si] = -1; // Abbrechen/Absichern, falls String zu kurz
        } 
        else 
        {
            _PeriphId[Si] = atoi(nextTok);
        }

        if (_PeriphId[Si] >= 0)
        {
            _Periph[Si] = FindPeriphById(_PeriphId[Si]);
            
            // HIER: Der lebenswichtige Schutz vor dem Boot-Crash
            if (_Periph[Si] != NULL) 
            {
                _PeerId[Si] = _Periph[Si]->GetPeerId();
                _Peer[Si]   = FindPeerById(_PeerId[Si]);
            } 
            else 
            {
                // Peripherie existiert nicht (z.B. ID gelöscht oder geändert)
                _Periph[Si]   = NULL;
                _PeerId[Si]   = -1;
                _Peer[Si]     = NULL;
                _PeriphId[Si] = -1;
            }
        }
        else
        {
            _Periph[Si]   = NULL;
            _PeerId[Si]   = -1;
            _Peer[Si]     = NULL;
        }
    }
}

void SavePeers() 
// writes [Peer-0] - [Name;Type;BroadcastAddress[0-5];SleepMode;DebugMode;DemoMode;Periph0Name;Periph0Type;Periph0Pos;Periph0PeerId...]
{
    PeerClass *P;
    
    char Buf[20];
    //char ExportBuffer[50];
    String ExportStringMulti;
    String ExportStringPeer;

    preferences.begin("JeepifyPeers", false);
    
    preferences.putInt("PeerCount", PeerList.size());
    Serial.printf("SavePeers(): PeerList.size() = %lu, gelesenes PeerCount = %lu\n\r", (unsigned long)PeerList.size(), (unsigned long)preferences.getInt("PeerCount"));

    for(int i = 0; i < PeerList.size(); i++){
      P = PeerList.get(i);
      snprintf(Buf, sizeof(Buf), "Peer-%d", i);

      ExportStringPeer = String(P->Export());

      Serial.printf("putSring = %d", preferences.putString(Buf, ExportStringPeer));
      Serial.printf("schreibe: [%s]: %s", Buf, ExportStringPeer.c_str());
      Serial.println();
      Serial.printf("Lesekontrolle %s: %s\n\r", Buf, preferences.getString(Buf).c_str());
    }
  
    Serial.println("jetzt kommt Multi");

    for (int s=0; s<MULTI_SCREENS; s++) {
      snprintf(Buf, sizeof(Buf), "Screen-%d", s);
      
      ExportStringMulti = String(Screen[s].Export());

      preferences.putString(Buf, ExportStringMulti);
      Serial.printf("schreibe: [%s]: %s", Buf, ExportStringMulti.c_str());
      Serial.println();
    }

    Serial.println("jetzt kommt MultiGaugeScreen");
    snprintf(Buf, sizeof(Buf), "MGScreen");
    
    ExportStringMulti = String(MultiGaugeScreen.Export());

    preferences.putString(Buf, ExportStringMulti);
    Serial.printf("schreibe: [%s]: %s", Buf, ExportStringMulti.c_str());
    Serial.println();

    preferences.end();
}
int  GetPeers() 
{
    PeerClass *P;
    
    char Buf[40];
    String ImportStringMulti = "";
    String ImportStringPeer = "";

    preferences.begin("JeepifyPeers", true);

    PeerList.clear();

    int PeerCount = preferences.getInt("PeerCount", 0);
    Serial.printf("GetPeers(): Peercount = %d\n\r", PeerCount);
    
    for (int Pi=0 ; Pi<PeerCount; Pi++)
    {
        snprintf(Buf, sizeof(Buf), "Peer-%d", Pi);
        ImportStringPeer = preferences.getString(Buf, "");
        //Serial.printf("GetPeers(): %s : ImportStringPeer = %s\n\r",Buf, ImportStringPeer.c_str());
        strncpy(ScreenExportImportBuffer, ImportStringPeer.c_str(), sizeof(ScreenExportImportBuffer) - 1);
        ScreenExportImportBuffer[sizeof(ScreenExportImportBuffer) - 1] = '\0';
        Serial.printf("gelesen: [%s]: %s\n\r", Buf, ScreenExportImportBuffer);
        
        if (strcmp(ScreenExportImportBuffer, "") != 0)
        {
            P = new PeerClass();
            P->Import(ScreenExportImportBuffer);
            Serial.printf("GetPeers(): Peer angelegt mit Name: %s\n\r\r", P->GetName());
            PeerList.add(P);
            for (int Si=0; Si<MAX_PERIPHERALS; Si++) 
            {
                if (P->GetPeriphType(Si)) PeriphList.add(P->GetPeriphPtr(Si));
            }
        }
    }
  
    Serial.println("importing Multi-Screens:\n\r");

    for (int s=0; s<MULTI_SCREENS; s++) 
    {
        snprintf(Buf, sizeof(Buf), "Screen-%d", s);
        
        ImportStringMulti = preferences.getString(Buf, "");
        if (ImportStringMulti != "") 
        {   
            Serial.printf("%s - %d Bytes gelesen: %s\n\r", Buf, sizeof(ImportStringMulti), ImportStringMulti.c_str());
            strncpy(ScreenExportImportBuffer, ImportStringMulti.c_str(), sizeof(ScreenExportImportBuffer) - 1);
            ScreenExportImportBuffer[sizeof(ScreenExportImportBuffer) - 1] = '\0';

            //ReportAll();
            
            Serial.println("jetzt kommt import");
            Screen[s].Import(ScreenExportImportBuffer);
        }
    }

    Serial.println("importing MultiGaugeScreen:\n\r");
    snprintf(Buf, sizeof(Buf), "MGScreen");

    ImportStringMulti = preferences.getString(Buf, "");
    if (ImportStringMulti != "") 
    {   
        Serial.printf("%s - %d Bytes gelesen: %s\n\r", Buf, sizeof(ImportStringMulti), ImportStringMulti.c_str());
        strncpy(ScreenExportImportBuffer, ImportStringMulti.c_str(), sizeof(ScreenExportImportBuffer) - 1);
        ScreenExportImportBuffer[sizeof(ScreenExportImportBuffer) - 1] = '\0';

        Serial.println("jetzt kommt import");
        MultiGaugeScreen.Import(ScreenExportImportBuffer);
    }

    ReportAll();
    preferences.end();

    return PeerCount;
}
void ClearPeers() 
{
    char Buf[20];

    preferences.begin("JeepifyPeers", false);
    for (int Pi=0; Pi<MAX_PEERS; Pi++)
    {
        snprintf(Buf, sizeof(Buf), "Peer-%d", Pi);
        preferences.remove(Buf);
    }
    for (int s=0; s<MULTI_SCREENS; s++) 
    {
        snprintf(Buf, sizeof(Buf), "Screen-%d", s);
        preferences.remove(Buf);
    }
    snprintf(Buf, sizeof(Buf), "MultiGaugeScreen");
    preferences.remove(Buf);

    preferences.clear();
    Serial.println("JeepifyPeers cleared...");
    Serial.printf("free entries in JeepifyPeers now: %d\n\r", preferences.freeEntries());
  preferences.end();
}
void ClearInit() 
{
  preferences.begin("JeepifyInit", false);
    preferences.remove("Module");
    preferences.clear();
    Serial.println("JeepifyInit cleared...");
    Serial.printf("free entries in JeepifyInit now: %d\n\r", preferences.freeEntries());
  preferences.end();
}
void DeletePeer(PeerClass *P) 
{
    PeriphClass *Periph;
    
    for (int s=0; s<MULTI_SCREENS; s++) {
      for (int Si=0; Si<PERIPH_PER_SCREEN; Si++)
      {
          if (Screen[s].GetPeerId(Si) == P->GetId())
          {
              Screen[s].SetPeerId(Si, -1);
              Screen[s].SetPeer(Si, NULL);
              Screen[s].SetPeriphId(Si, -1);
              Screen[s].SetPeer(Si, NULL);
              Screen[s].SetChanged(true);
          }
      }
    }
    for (int Si=0; Si<PERIPH_PER_SCREEN; Si++)
    {
        if (MultiGaugeScreen.GetPeerId(Si) == P->GetId())
        {
            MultiGaugeScreen.SetPeerId(Si, -1);
            MultiGaugeScreen.SetPeer(Si, NULL);
            MultiGaugeScreen.SetPeriphId(Si, -1);
            MultiGaugeScreen.SetPeriph(Si, NULL);
            MultiGaugeScreen.SetChanged(true);
        }
    }

    int PSize = PeriphList.size();
    for (int Si=PSize-1; Si>=0; Si--)
    {
        Periph = PeriphList.get(Si);
        if (Periph->GetPeerId() == P->GetId()) PeriphList.remove(Si);
    }

    PSize = PeerList.size();
    for(int i = PSize-1; i >=0; i--){
        if (PeerList.get(i) == P) 
        {
            PeerList.remove(i);
            Serial.printf("Peer: %s deleted and removed from list.", P->GetName());
            delete P;
            P = NULL;
        }
    }
    SavePeers();
    ESP.restart();
}
void RegisterPeers() 
{
    PeerClass *P;

    esp_now_peer_info_t peerInfo;
    memset(&peerInfo, 0, sizeof(peerInfo));
    peerInfo.channel = 1;
    peerInfo.encrypt = false;
    
    // Register BROADCAST
    for (int b=0; b<6; b++) peerInfo.peer_addr[b] = 0xff;
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        PrintMAC(peerInfo.peer_addr); Serial.println(": Failed to add peer");
        }
        else {
        Serial.print (" ("); PrintMAC(peerInfo.peer_addr);  Serial.println(") added...");
        }

    // Register Peers
    for (int i=0; i<PeerList.size(); i++) 
    {
        P = PeerList.get(i);
        if (P == NULL) continue;
        memcpy(peerInfo.peer_addr, P->GetBroadcastAddress(), 6);
        
        if (esp_now_add_peer(&peerInfo) != 0) 
        {
            PrintMAC(peerInfo.peer_addr); Serial.println(": Failed to add peer");
        }
        else {
            Serial.print("Peer: "); Serial.print(P->GetName()); 
            Serial.print (" ("); PrintMAC(peerInfo.peer_addr); Serial.println(") added...");
        }
    }
}
  
void ReportAll()
{
    PeerClass *P;
    Serial.printf("REPORT: PeerList.size() = %d\n\r", PeerList.size());

    for(int i=0; i < PeerList.size(); i++)
    {
        P = PeerList.get(i);
        if (P == NULL) continue;

        Serial.printf("[%d] %s, Type:%d, MAC:", P->GetId(), P->GetName(), P->GetType());
        PrintMAC(P->GetBroadcastAddress());
        Serial.println();
        
        for (int Si=0; Si<MAX_PERIPHERALS; Si++)
        {
            if (P->GetPeriphType(Si) > 0)
            {
                Serial.printf(" Si=%d - PeriphId=%d: %s(%d) at position %d\n\r", Si, P->GetPeriphId(Si), P->GetPeriphName(Si), P->GetPeriphType(Si), P->GetPeriphPos(Si));
            }
        }
    }
    for (int s=0; s<MULTI_SCREENS; s++) 
    {
        Serial.printf("Screen[%d]: %s", s, Screen[s].GetName());
        for (int Si=0; Si<PERIPH_PER_SCREEN; Si++)
        {
            if (Screen[s].GetPeriphId(Si) > -1)
            { 
                PeriphClass *periphPtr = Screen[s].GetPeriph(Si);
                if (periphPtr != NULL) 
                {
                    Serial.printf("    %d: %s(%d) at position %d\n\r", Screen[s].GetPeriphId(Si), periphPtr->GetName(), periphPtr->GetType(), Si);
                }
                else 
                {
                    Serial.printf("    %d: [Nicht gefunden/NULL] at position %d\n\r", Screen[s].GetPeriphId(Si), Si);
                }
            }
        }
    }
    for (int Si=0; Si<PERIPH_PER_SCREEN; Si++)
    {
        if (MultiGaugeScreen.GetPeriphId(Si) > -1)
        {
            PeriphClass *periphPtr = MultiGaugeScreen.GetPeriph(Si);
            if (periphPtr != NULL) 
            {
                Serial.printf("    %d: %s(%d) at position %d\n\r", MultiGaugeScreen.GetPeriphId(Si), periphPtr->GetName(), periphPtr->GetType(), Si);
            }
            else 
            {
                Serial.printf("    %d: [Nicht gefunden/NULL] at position %d\n\r", MultiGaugeScreen.GetPeriphId(Si), Si);
            }
        }
    }
}