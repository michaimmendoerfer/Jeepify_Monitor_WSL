#ifndef CUSTOM_FUNCTIONS_H
#define CUSTOM_FUNCTIONS_H

#include <lvgl.h>
#include "Jeepify.h"

#ifdef __cplusplus
extern "C" {
#endif

void Scr_Top_Gesture(lv_event_t * e);
void Btn_Top_Prev_clicked();
void Btn_Top_Next_clicked();

void Scr_Peer_loaded();
void Scr_Peer_unloaded();
void Scr_Peer_Timer_update(lv_timer_t * timer);
void Scr_Peer_pressed(lv_event_t * e);
void Btn_Peer_Next_clicked();
void Btn_Peer_Prev_clicked();
void Btn_Peer_Up_clicked();
void Btn_Peer_System_clicked();
void Btn_Peer_Volt_clicked();
void Btn_Peer_Eichen_clicked();
void Btn_Peer_Restart_clicked();
void Btn_Peer_Sleep_clicked();
void Btn_Peer_Pair_clicked();
void Btn_Peer_Reset_clicked();
void Btn_Peer_Delete_clicked();
void Scr_Peer_Gesture(lv_event_t * e);

void Scr_Eichen_loaded();
void Scr_Eichen_unloaded();
void Btn_Eichen_Eichen_clicked();
void Scr_Eichen_Gesture(lv_event_t * e);

void Scr_Volt_loaded();
void Scr_Volt_unloaded();
void Arc_Volt_change();
void Btn_Volt_Save_clicked();
void Scr_Volt_Gesture(lv_event_t * e);

void Scr_Multi_unloaded(lv_event_t * e);
void Scr_Multi_loaded(lv_event_t * e);
void Scr_Multi_FillScreen();
void Scr_Multi_ClearScreen();
void Scr_Multi_pressed(lv_event_t * e);
void Btn_Multi_Next_clicked(lv_event_t * e);
void Btn_Multi_Prev_clicked(lv_event_t * e);
void Btn_Multi_Up_clicked(lv_event_t * e);
void Scr_Multi_Timer_update(lv_timer_t *timer);
void Btn_Multi_Change_Pos(int Pos);
void Btn_Multi1_clicked(lv_event_t * e);
void Btn_Multi2_clicked(lv_event_t * e);
void Btn_Multi3_clicked(lv_event_t * e);
void Btn_Multi4_clicked(lv_event_t * e);
void Scr_Multi_Gesture(lv_event_t * e);
void Scr_Multi_clicked(lv_event_t * e);


void Scr_Settings_loaded();
void Scr_Settings_unloaded();
void Scr_Settings_Timer_update(lv_timer_t *timer);
void Btn_Settings_Restart_clicked();
void Btn_Settings_Reset_clicked();
void Btn_Settings_Peers_clicked();
void Btn_Settings_Pair_clicked();
void Btn_Settings_WebSvr_clicked();
void Btn_Settings_Save_clicked();
void Scr_Settings_Gesture(lv_event_t * e);

void Scr_JSON_loaded();
void Scr_JSON_unloaded();
void Scr_JSON_Gesture(lv_event_t * e);

void Scr_Start_loaded();
void Scr_Start_unloaded();
void Btn_Start_1_clicked();
void Btn_Start_2_clicked();
void Scr_Start_Gesture(lv_event_t * e);

void Scr_Simple_loaded();
void Scr_Simple_unloaded();
void Scr_Simple_FillScreen();
void Scr_Simple_ClearScreen();
void Scr_Simple_pressed(lv_event_t * e);
void Btn_Simple_Prev_clicked();
void Btn_Simple_Next_clicked();
void Scr_Simple_Timer_update(lv_timer_t * timer);
void Btn_Simple_Up_clicked();
void Scr_Simple_clicked(lv_event_t * e);

void Scr_PeriphChoice_loaded();
void Scr_PeriphChoice_unloaded();
void Scr_PeriphChoice_Timer_update(lv_timer_t *timer);
void Btn_PeriphChoice_clicked(lv_event_t * e);
void Scr_PeriphChoice_FillScreen();
void Scr_PeriphChoice_Return();
void Btn_PeriphChoice_Next_clicked(lv_event_t * e);
void Btn_PeriphChoice_Prev_clicked(lv_event_t * e);
void Btn_PeriphChoice_Up_clicked(lv_event_t * e);
void Scr_PeriphChoice_Gesture(lv_event_t * e);


extern char jsondataBuf[ESP_MAX_LEN+1];
#ifdef __cplusplus
}
#endif

#endif 
