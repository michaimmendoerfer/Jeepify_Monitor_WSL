#ifndef CUSTOM_FUNCTIONS_H
#define CUSTOM_FUNCTIONS_H

#include <lvgl.h>
#include "Jeepify.h"

#ifdef __cplusplus
extern "C" {
#endif

void Scr_Peer_loaded();
void Scr_Peer_unloaded();
void Scr_Peer_Timer_update(lv_timer_t * timer);
void Scr_Peer_Next();
void Scr_Peer_Prev();
void Btn_Peer_System_clicked();
void Btn_Peer_Volt_clicked();
void Btn_Peer_Eichen_clicked();
void Btn_Peer_Restart_clicked();
void Btn_Peer_Sleep_clicked();
void Btn_Peer_Pair_clicked();
void Btn_Peer_Reset_clicked();
void Btn_Peer_Delete_clicked();

void Scr_Eichen_loaded();
void Scr_Eichen_unloaded();
void Btn_Eichen_Eichen_clicked();

void Scr_Volt_loaded();
void Scr_Volt_unloaded();
void Arc_Volt_change();
void Btn_Volt_Save_clicked();

void Scr_Multi_Unloaded(lv_event_t * e);
void Scr_Multi_Loaded(lv_event_t * e);
void Scr_Multi_FillScreen();
void Scr_Multi_Clicked(lv_event_t * e);
void Scr_Multi_Next(lv_event_t * e);
void Scr_Multi_Prev(lv_event_t * e);
void Btn_Multi_1_clicked(void);
void Scr_Multi_Timer_update(lv_timer_t *timer);

void Scr_Settings_loaded();
void Scr_Settings_unloaded();
void Scr_Settings_Timer_update(lv_timer_t *timer);
void Btn_Settings_Restart_clicked();
void Btn_Settings_Reset_clicked();
void Btn_Settings_Peers_clicked();
void Btn_Settings_Pair_clicked();
void Btn_Settings_WebSvr_clicked();
void Btn_Settings_Save_clicked();

void Scr_JSON_loaded();
void Scr_JSON_unloaded();

void Scr_Start_loaded();
void Scr_Start_unloaded();
void Btn_Start_1_clicked();
void Btn_Start_2_clicked();

void Scr_Simple_loaded();
void Scr_Simple_unloaded();

void Scr_PeerChoice_loaded();
void Scr_PeerChoice_unloaded();
void Btn_PeerChoice_Open_clicked();
void Scr_PeerChoice_Timer_update(lv_timer_t *timer);
    
void Scr_PeriphChoice_loaded();
void Scr_PeriphChoice_unloaded();
void Scr_PeriphChoice_Timer_update(lv_timer_t *timer);
void Scr_PeriphChoice_Click(lv_event_t * e);
void Scr_PeriphChoice_FillScreen();
void Scr_PeriphChoice_Return();
void Scr_PeriphChoice_Next(lv_event_t * e);
void Scr_PeriphChoice_Prev(lv_event_t * e);


extern char jsondataBuf[ESP_MAX_LEN+1];
#ifdef __cplusplus
}
#endif

#endif 
