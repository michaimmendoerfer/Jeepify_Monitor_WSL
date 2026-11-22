#ifndef CUSTOM_FUNCTIONS_H
#define CUSTOM_FUNCTIONS_H

#include <lvgl.h>
#include "Jeepify.h"

#ifdef __cplusplus
extern "C" {
#endif

void Scr_Navi_init();
void Scr_Navi_Timer_update(lv_timer_t * timer);
void tick_screen_scr_navi();
void Scr_Navi_SendIcon(int color);

void Scr_Peer_loaded(lv_event_t * e);
void Scr_Peer_unloaded(lv_event_t * e);
void Scr_Peer_Timer_update(lv_timer_t * timer);
void Scr_Peer_pressed(lv_event_t * e);
void Btn_Peer_Next_clicked(lv_event_t * e);
void Btn_Peer_Prev_clicked(lv_event_t * e);
void Btn_Peer_Up_clicked(lv_event_t * e);
void Btn_Peer_System_clicked(lv_event_t * e);
void Btn_Peer_Volt_clicked(lv_event_t * e);
void Btn_Peer_Eichen_clicked(lv_event_t * e);
void Btn_Peer_Restart_clicked(lv_event_t * e);
void Btn_Peer_Sleep_clicked(lv_event_t * e);
void Btn_Peer_Pair_clicked(lv_event_t * e);
void Btn_Peer_Reset_clicked(lv_event_t * e);
void Btn_Peer_Delete_clicked(lv_event_t * e);
void Scr_Peer_Gesture(lv_event_t * e);

void Scr_Eichen_loaded(lv_event_t * e);
void Scr_Eichen_unloaded(lv_event_t * e);
void Btn_Eichen_Eichen_clicked(lv_event_t * e);
void Scr_Eichen_Gesture(lv_event_t * e);

void Scr_Volt_loaded(lv_event_t * e);
void Scr_Volt_unloaded(lv_event_t * e);
void Arc_Volt_change(lv_event_t * e);
void Btn_Volt_Save_clicked(lv_event_t * e);
void Scr_Volt_Gesture(lv_event_t * e);

void Scr_Multi_unloaded(lv_event_t * e);
void Scr_Multi_loaded(lv_event_t * e);
void Scr_Multi_FillScreen(lv_event_t * e);
void Scr_Multi_ClearScreen(lv_event_t * e);
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


void Scr_Settings_loaded(lv_event_t * e);
void Scr_Settings_unloaded(lv_event_t * e);
void Scr_Settings_Timer_update(lv_timer_t *timer);
void Btn_Settings_Restart_clicked(lv_event_t * e);
void Btn_Settings_Reset_clicked(lv_event_t * e);
void Btn_Settings_Peers_clicked(lv_event_t * e);
void Btn_Settings_Pair_clicked(lv_event_t * e);
void Btn_Settings_WebSvr_clicked(lv_event_t * e);
void Btn_Settings_Save_clicked(lv_event_t * e);
void Scr_Settings_Gesture(lv_event_t * e);

void Scr_JSON_loaded(lv_event_t * e);
void Scr_JSON_unloaded(lv_event_t * e);
void Scr_JSON_Gesture(lv_event_t * e);

void Scr_Start_loaded(lv_event_t * e);
void Scr_Start_unloaded(lv_event_t * e);
void Btn_Start_1_clicked(lv_event_t * e);
void Btn_Start_2_clicked(lv_event_t * e);
void Scr_Start_Gesture(lv_event_t * e);

void Scr_Meter_loaded(lv_event_t * e);
void Scr_Meter_unloaded(lv_event_t * e);
void Scr_Meter_pressed(lv_event_t * e);
void Btn_Meter_Prev_clicked(lv_event_t * e);
void Btn_Meter_Next_clicked(lv_event_t * e);
void Scr_Meter_Timer_update(lv_timer_t * timer);
void Btn_Meter_Up_clicked(lv_event_t * e);
void Scr_Meter_clicked(lv_event_t * e);
void Scr_Meter_Gesture(lv_event_t * e);
void Scr_Meter_long_clicked(lv_event_t * e);

void Btn_Switch_Prev_clicked(lv_event_t * e);
void Btn_Switch_Next_clicked(lv_event_t * e);
void Btn_Switch_Up_clicked(lv_event_t * e);
void Scr_Switch_Gesture(lv_event_t * e);
void Scr_Switch_loaded(lv_event_t * e);
void Scr_Switch_unloaded(lv_event_t * e);
void Scr_Switch_pressed(lv_event_t * e);
void Scr_Switch_Timer_update(lv_timer_t * timer);
void Scr_Switch_clicked(lv_event_t * e);


void Scr_PeriphChoice_loaded(lv_event_t * e);
void Scr_PeriphChoice_unloaded(lv_event_t * e);
void Scr_PeriphChoice_Timer_update(lv_timer_t *timer);
void Btn_PeriphChoice_clicked(lv_event_t * e);
void Scr_PeriphChoice_FillScreen(lv_event_t * e);
void Scr_PeriphChoice_Return(lv_event_t * e);
void Btn_PeriphChoice_Next_clicked(lv_event_t * e);
void Btn_PeriphChoice_Prev_clicked(lv_event_t * e);
void Btn_PeriphChoice_Up_clicked(lv_event_t * e);
void Scr_PeriphChoice_Gesture(lv_event_t * e);

void Scr_Prev_loaded(lv_event_t * e);
void Scr_Prev_unloaded(lv_event_t * e);
void Scr_Prev_Gesture(lv_event_t * e);

extern char jsondataBuf[ESP_MAX_LEN+1];
#ifdef __cplusplus
}
#endif

#endif 
