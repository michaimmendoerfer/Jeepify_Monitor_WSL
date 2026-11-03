#ifndef CUSTOM_FUNCTIONS_H
#define CUSTOM_FUNCTIONS_H

#include <lvgl.h>

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



#ifdef __cplusplus
}
#endif

#endif 
