//Version 4.01
#include <Arduino.h>
#include "PeerClass.h"
#include "LinkedList.h"

PeerClass   *ActivePeer;
PeriphClass *ActivePeriph;

MyLinkedList<PeerClass*>   PeerList   = MyLinkedList<PeerClass*>();
MyLinkedList<PeriphClass*> PeriphList = MyLinkedList<PeriphClass*>();

int  PeriphClass::_ClassId = 1;
int  PeerClass::_ClassId   = 1;

char ExportImportBuffer[1000];

extern void PrintMAC(const uint8_t * mac_addr);

#pragma region PeriphClass::Declaration
PeriphClass::PeriphClass()
{
    _Id = _ClassId;
    _ClassId++;

    strncpy(_Name, "n.n.", sizeof(_Name) - 1);
    _Name[sizeof(_Name) - 1] = '\0';

    _Type = 0;  
    _Pos = 0;       
    for (int i=0; i<4; i++) _IOPort[i]  = -1;
    for (int i=0; i<4; i++) _I2CPort[i] = -1;
    
    _Nullwert = 0;
    _VperAmp = 0;
    _Vin = 0;
    for (int i=0; i<4; i++) _Value[i] = 0;
    for (int i=0; i<4; i++) _OldValue[i] = 0;
    _Changed = false;
    _PeerId = 0;
    _SavedValueIndex = 0;
}
void  PeriphClass::Setup(const char* Name, int Type, bool isADS, 
                         int I2CPort0, int I2CPort1, int I2CPort2, int I2CPort3, 
                         int IOPort0, int IOPort1, int IOPort2, int IOPort3,  
                         float Nullwert, float VperAmp, float Vin, int PeerId)
{
    strncpy(_Name, Name, sizeof(_Name) - 1);
    _Name[sizeof(_Name) - 1] = '\0';

    _Type = Type;
    
    _IOPort[0] = IOPort0;
    _IOPort[1] = IOPort1;
    _IOPort[2] = IOPort2;
    _IOPort[3] = IOPort3;
    
    _I2CPort[0] = I2CPort0;
    _I2CPort[1] = I2CPort1;
    _I2CPort[2] = I2CPort2;
    _I2CPort[3] = I2CPort3;

    _Nullwert = Nullwert;
    _VperAmp = VperAmp;
    _Vin = Vin;
    _PeerId = PeerId;
}
void  PeriphClass::Setup(const char* Name, int Type, int PeerId)
{
    strncpy(_Name, Name, sizeof(_Name) - 1);
    _Name[sizeof(_Name) - 1] = '\0';
    _Type = Type;
    _PeerId = PeerId;
}

bool PeriphClass::IsType(int Type)
{
    switch (Type) { 
                case SENS_TYPE_ALL:                                                                return true; break;
                case SENS_TYPE_SENS:    if ((_Type == SENS_TYPE_VOLT) or (_Type == SENS_TYPE_AMP)) return true; break;
                case SENS_TYPE_VOLT:    if  (_Type == SENS_TYPE_VOLT)                              return true; break;
                case SENS_TYPE_AMP:     if  (_Type == SENS_TYPE_AMP)                               return true; break;
                case SENS_TYPE_SW_ALL:  if ((_Type == SENS_TYPE_SWITCH) or
                                            (_Type == SENS_TYPE_SW_AMP) or
                                            (_Type == SENS_TYPE_LT)     or
                                            (_Type == SENS_TYPE_LT_AMP))                           return true; break;
                case SENS_TYPE_SWITCH:  if  (_Type == SENS_TYPE_SWITCH)                            return true; break;
                case SENS_TYPE_SW_AMP:  if  (_Type == SENS_TYPE_SW_AMP)                            return true; break;
                case SENS_TYPE_LT_AMP:  if  (_Type == SENS_TYPE_LT_AMP)                            return true; break;
                case SENS_TYPE_LT:      if  (_Type == SENS_TYPE_LT)                                return true; break;
            }
    return false;
}
void PeriphClass::AddSavedValue(float V0, float V1, float V2, float V3)
{   
    _SavedValueIndex++;
    if (_SavedValueIndex == RECORDED_VALUES) _SavedValueIndex = 0;
    
    _SavedValue[_SavedValueIndex][V_ON]   = V0; 
    _SavedValue[_SavedValueIndex][V_OFF]  = V1; 
    _SavedValue[_SavedValueIndex][V_VOLT] = V2; 
    _SavedValue[_SavedValueIndex][V_AMP]  = V3; 
    _SavedValueTS[_SavedValueIndex]       = millis(); 
    
}
#pragma endregion PeriphClass::Declaration
#pragma region PeerClass::Declaration
PeerClass::PeerClass()
{
    _Id = _ClassId;
    _ClassId++;

    strncpy(_Name, "n.n.", sizeof(_Name) - 1);
    _Name[sizeof(_Name) - 1] = '\0';

    _Type = 0;  
    _SleepMode = false;
    _DebugMode = false;
    _DemoMode = false;
    _PairMode = false;
    _Changed = false;
    _TSLastSeen = 0;
    memset(_BroadcastAddress, 0, 6);
    _Brightness = 50;
    _dBm = 0;
}
void  PeerClass::Setup(const char* Name, int Type, const char *Version, const uint8_t *BroadcastAddress, 
                       bool SleepMode, bool DebugMode, bool DemoMode, bool PairMode)
{
    strncpy(_Name, Name, sizeof(_Name) - 1);
    _Name[sizeof(_Name) - 1] = '\0';

    _Type = Type;
    strncpy(_Version, Version, sizeof(_Version) - 1);
    _Version[sizeof(_Version) - 1] = '\0';

    if (BroadcastAddress) memcpy(_BroadcastAddress, BroadcastAddress, 6);
    _SleepMode = SleepMode;
    _DebugMode = DebugMode;
    _DemoMode  = DemoMode;
    _PairMode  = PairMode;
    
    for (int Si=0; Si<MAX_PERIPHERALS; Si++) Periph[Si].SetPos(Si);
}     

char* PeerClass::Export() 
{
    int UsedPeriph = 0;
    for (int Si=0; Si<MAX_PERIPHERALS; Si++) { 
        if (Periph[Si].GetType() > 0) UsedPeriph++;
    } 

    // Ersten Teil schreiben und die Anzahl der geschriebenen Bytes merken
    int written = snprintf(ExportImportBuffer, sizeof(ExportImportBuffer), "%s;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d", 
                        _Name, _Type, 
                        _BroadcastAddress[0], _BroadcastAddress[1], _BroadcastAddress[2],
                        _BroadcastAddress[3], _BroadcastAddress[4], _BroadcastAddress[5],
                        _SleepMode, _DebugMode, _DemoMode, UsedPeriph);
                        
    for (int Si=0; Si<MAX_PERIPHERALS; Si++)
    { 
        if (Periph[Si].GetType() > 0) 
        {
            // Abbrechen, falls der Puffer voll ist oder kein Platz für ein weiteres Zeichen bleibt
            if (written >= (int)sizeof(ExportImportBuffer) - 1) break;

            // Schreibt direkt an das aktuelle Ende des Puffers unter Beachtung des Restplatzes
            int res = snprintf(ExportImportBuffer + written, sizeof(ExportImportBuffer) - written, 
                               ";%s;%d;%.3f;%.2f", 
                               Periph[Si].GetName(), Periph[Si].GetType(), Periph[Si].GetNullwert(), Periph[Si].GetVin());
            
            if (res > 0) {
                written += res;
            }
        }
    }
    return ExportImportBuffer;
}

void PeerClass::Import(char *Buf) 
{
    if (!Buf) return;

    char *token = strtok(Buf, ";");
    if (!token) return;
    strncpy(_Name, token, sizeof(_Name) - 1);
    _Name[sizeof(_Name) - 1] = '\0';
    
    #define GET_NEXT_TOKEN() token = strtok(NULL, ";"); if (!token) return;

    GET_NEXT_TOKEN(); _Type = atoi(token);
    GET_NEXT_TOKEN(); _BroadcastAddress[0] = (byte) atoi(token);
    GET_NEXT_TOKEN(); _BroadcastAddress[1] = (byte) atoi(token);
    GET_NEXT_TOKEN(); _BroadcastAddress[2] = (byte) atoi(token);
    GET_NEXT_TOKEN(); _BroadcastAddress[3] = (byte) atoi(token);
    GET_NEXT_TOKEN(); _BroadcastAddress[4] = (byte) atoi(token);
    GET_NEXT_TOKEN(); _BroadcastAddress[5] = (byte) atoi(token);
   
    GET_NEXT_TOKEN(); _SleepMode = (bool) atoi(token);
    GET_NEXT_TOKEN(); _DebugMode = (bool) atoi(token);
    GET_NEXT_TOKEN(); _DemoMode  = (bool) atoi(token);
    GET_NEXT_TOKEN(); int UsedPeriph = atoi(token);

    // Schutz vor Buffer Overflow/Underrun bei der Array-Zuweisung
    if (UsedPeriph > MAX_PERIPHERALS) UsedPeriph = MAX_PERIPHERALS;

    for (int Si=0; Si<UsedPeriph; Si++)
    {
        token = strtok(NULL, ";"); if (!token) break;
        Periph[Si].SetName(token);

        GET_NEXT_TOKEN(); Periph[Si].SetType(atoi(token));
        GET_NEXT_TOKEN(); Periph[Si].SetNullwert(atof(token));
        GET_NEXT_TOKEN(); Periph[Si].SetVin(atof(token));
        
        Periph[Si].SetPos(Si);
        Periph[Si].SetPeerId(_Id);
    }
    #undef GET_NEXT_TOKEN
}
        
void  PeerClass::PeriphSetup(int Pos, const char* Name, int Type, 
                             int I2CPort0, int I2CPort1, int I2CPort2, int I2CPort3, 
                             int IOPort0, int IOPort1, int IOPort2, int IOPort3, 
                             float Nullwert, float VperAmp, float Vin, int PeerId)
{
    Periph[Pos].Setup(Name, Type, 0, I2CPort0, I2CPort1, I2CPort2, I2CPort3, IOPort0, IOPort1, IOPort2, IOPort3, Nullwert, VperAmp, Vin, PeerId);
}

void  PeerClass::PeriphSetup(int Pos, const char* Name, int Type, int PeerId)
{
    Periph[Pos].Setup(Name, Type, PeerId);
}
#pragma endregion PeerClass::Declaration
#pragma region MAC-Things
PeerClass *FindPeerByMAC(const uint8_t *BroadcastAddress)
{
    PeerClass *Peer;
    for(int i = 0; i < PeerList.size(); i++){

		Peer = PeerList.get(i);

		if (memcmp(Peer->GetBroadcastAddress(), BroadcastAddress, 6) == 0) return Peer;
	}
    return NULL;
}
PeerClass *FindPeerById(int Id)
{
    PeerClass *Peer;
    for(int i = 0; i < PeerList.size(); i++){

		Peer = PeerList.get(i);

		if (Peer->GetId() == Id) return Peer;
    }
    return NULL;
}
PeerClass *FindPeerByName(char *Name)
{
    PeerClass *Peer;
    for(int i = 0; i < PeerList.size(); i++) 
    {   
        Peer = PeerList.get(i);
        if (strcmp(Peer->GetName(), Name) == 0) return Peer;
    }
    return NULL;
}
PeerClass *PeerOf(PeriphClass *P)
{
    return FindPeerById(P->GetPeerId());
}
bool IsOnline(PeerClass *P)
{
    if (P == NULL) return 0;
    if ((P->GetTSLastSeen() != 0) and (millis() - P->GetTSLastSeen() < OFFLINE_INTERVAL)) return 1;
    return 0;
}
bool IsOnline(PeriphClass *P)
{
    if (P == NULL) return 0;
    PeerClass *Peer = PeerOf(P);
    if (Peer == NULL) return 0;

    if ((Peer->GetTSLastSeen() != 0) and (millis() - Peer->GetTSLastSeen() < OFFLINE_INTERVAL)) return 1;
    return 0;
}
int FindPeriphListPos(PeriphClass *Periph)
{
    if ((PeriphList.size() == 0)  or (Periph == NULL)) return -1;

    for(int i = 0; i < PeriphList.size(); i++) 
    {   
        if (PeriphList.get(i) == Periph) return i;
    }
    return -1;
}
int FindPeerListPos(PeerClass *Peer)
{
    if ((PeerList.size() == 0) or (Peer == NULL)) return -1;
    for (int i = 0; i < PeerList.size(); i++) 
    {   
        if (PeerList.get(i) == Peer) return i;
    }
    return -1;
}

PeerClass *FindNextPeer(PeerClass *P, int Type, bool circular, int Status)
// returns next Peer, tries PeerList.size() times, otherwise returns NULL
{
    PeerClass *Peer;
    int ActualPeerIndex = FindPeerListPos(P);

    for (int i=0; i<PeerList.size(); i++)
    {       
        ActualPeerIndex++;
        if (ActualPeerIndex == PeerList.size()) 
        {   
            if (!circular) return NULL;
            ActualPeerIndex = 0;
        }

        Peer = PeerList.get(ActualPeerIndex);
        
        if (Peer) switch (Status)
        {
            case 0: if ((!IsOnline(Peer)) and ((Type == Peer->GetType()) or (Type == MODULE_ALL))) return Peer; break; // Peer ist offline
            case 1: if (( IsOnline(Peer)) and ((Type == Peer->GetType()) or (Type == MODULE_ALL))) return Peer; break; // Peer ist  online
            case 2: if ((Type == Peer->GetType()) or (Type == MODULE_ALL)) return Peer; break;
        }
    }
    return NULL;
}
PeerClass *FindPrevPeer(PeerClass *P, int Type, bool circular, int Status)
// returns previous Peer, tries PeerList.size() times, otherwise returns NULL
{
    PeerClass *Peer;
    int ActualPeerIndex = FindPeerListPos(P);
    if (ActualPeerIndex == -1) ActualPeerIndex = PeerList.size();
    
    for (int i=0; i<PeerList.size(); i++)
    {       
        ActualPeerIndex--;
        if (ActualPeerIndex == -1) 
        {   
            if (!circular) return NULL;
            ActualPeerIndex = PeerList.size()-1;
        }

        Peer = PeerList.get(ActualPeerIndex);
        
        if (Peer) switch (Status)
        {
            case 0: if ((!IsOnline(Peer)) and ((Type == Peer->GetType()) or (Type == MODULE_ALL))) return Peer; break; // Peer ist offline
            case 1: if (( IsOnline(Peer)) and ((Type == Peer->GetType()) or (Type == MODULE_ALL))) return Peer; break; // Peer ist  online
            case 2: if (Type == Peer->GetType() or (Type == MODULE_ALL)) return Peer; break;
        }
    }
    return NULL;
}
PeriphClass *FindPeriphById(int Id)
{
    for(int i = 0; i < PeriphList.size(); i++) 
    {   
        if (PeriphList.get(i)->GetId() == Id) return PeriphList.get(i);
    }
    return NULL;
}

PeriphClass *FindNextPeriph(PeerClass *Peer, PeriphClass *Periph, int Type, bool circular, int Status)
// return next Periph of Type. If Peer=NULL Peer is ignored, if Periph=NULL find first - otherwise NULL, circular...
{
    PeriphClass *TPeriph;

    if (PeriphList.size() == 0) return NULL;

    int PeriphPos = -1;
    PeriphPos = FindPeriphListPos(Periph);
    
    if (Peer == NULL) circular = true;  // if Peer doesnt matter, always search circular

    for(int i = 0; i < PeriphList.size(); i++) 
    {
        PeriphPos++;
        if (PeriphPos == PeriphList.size())
        {
            if (!circular) return NULL;
            PeriphPos = 0;
        }
        TPeriph = PeriphList.get(PeriphPos);
        if (TPeriph == NULL) continue;

        if ((Peer == NULL) or (Peer->GetId() == TPeriph->GetPeerId()))
        // Peer fits
        {
            switch (Status)
            {
                case 0: if ((!IsOnline(TPeriph)) and (TPeriph->IsType(Type))) return TPeriph; break; // Periph ist offline
                case 1: if (( IsOnline(TPeriph)) and (TPeriph->IsType(Type))) return TPeriph; break; // Periph ist  online
                case 2: if (TPeriph->IsType(Type)) return TPeriph; break;
            }
        }
    }
    return NULL;
}
PeriphClass *FindPrevPeriph(PeerClass *Peer, PeriphClass *Periph, int Type, bool circular, int Status)
// return previous Periph of Type. If Peer=NULL Peer is ignored, otherwise NULL, circular...
{
    PeriphClass *TPeriph;

    if (PeriphList.size() == 0) return NULL;
    
    int PeriphPos = FindPeriphListPos(Periph);
    if (PeriphPos == -1) PeriphPos = PeriphList.size(); // if not found prev will start at last element
    
    if (Peer == NULL) circular = true;   // if Peer doesnt matter, always search circular

    for(int i = 0; i < PeriphList.size(); i++) 
    {
        PeriphPos--;
        if (PeriphPos == -1)
        {
            if (!circular) return NULL;
            PeriphPos = PeriphList.size()-1;
        }
        TPeriph = PeriphList.get(PeriphPos);
        if (TPeriph == NULL) continue;
        
        if ((Peer == NULL) or (Peer->GetId() == TPeriph->GetPeerId()))
        // Peer fits
        {
            switch (Status)
            {
                case 0: if ((!IsOnline(TPeriph)) and (TPeriph->IsType(Type))) return TPeriph; break; // Periph ist offline
                case 1: if (( IsOnline(TPeriph)) and (TPeriph->IsType(Type))) return TPeriph; break; // Periph ist  online
                case 2: if (TPeriph->IsType(Type)) return TPeriph; break;
            }
        }
    }
    return NULL;
}
#pragma endregion MAC-Things

char *TypeInText(int Type)
{
    switch (Type)
    {
        case SENS_TYPE_VOLT:    return (char*) "Voltage-Sensor";
        case SENS_TYPE_AMP:     return (char*) "Current-Sensor";
        case SENS_TYPE_SWITCH:  return (char*) "Switch";
        case SENS_TYPE_SW_AMP:  return (char*) "sensed Switch";
        case SENS_TYPE_LT:      return (char*) "Switch";
        case SENS_TYPE_LT_AMP:  return (char*) "sensed Switch";
        case SWITCH_1_WAY:      return (char*) "1-way Switch";
        case SWITCH_2_WAY:      return (char*) "2-Way Switch";
        case SWITCH_4_WAY:      return (char*) "4-way Switch";
        case SWITCH_8_WAY:      return (char*) "8-Way Switch";
        case PDC:               return (char*) "Power distributor";
        case PDC_SENSOR_MIX:    return (char*) "Mixed Device";
        case BATTERY_SENSOR:    return (char*) "Battery-Sensor";
        case MONITOR_ROUND:     return (char*) "Round Monitor";
        case MONITOR_BIG:       return (char*) "3.5' Monitor";
    }
    return (char*) "not known";
}
