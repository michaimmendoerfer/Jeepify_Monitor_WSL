#include <Arduino.h>
#include "custom_functions.h"
#include "picoUI/actions.h"
#include "main.h"


#pragma region Scr_Peer
lv_timer_t *Timer_Scr_Peer = NULL;

void Scr_Peer_loaded()
{
    static uint32_t user_data = 10;
	
	if (Timer_Scr_Peer) 
	{
		lv_timer_resume(Timer_Scr_Peer);
	}
	else 
	{
		Timer_Scr_Peer = lv_timer_create(Scr_Peer_Timer_update, 100,  &user_data);
	}

	if (!ActivePeer) ActivePeer = FindNextPeer(NULL, MODULE_ALL, CIRCULAR);
	lv_label_set_recolor(objects.lbl_peer_scrname, true);
}
void Scr_Peer_unloaded()
{
    if (Timer_Scr_Peer) lv_timer_del(Timer_Scr_Peer);
	Timer_Scr_Peer = NULL;
}
void Scr_Peer_Timer_update(lv_timer_t * timer)
{
    if (ActivePeer) 
    {
		char buf[50];
		if ((ActivePeer->GetTSLastSeen() == 0) or (millis()- ActivePeer->GetTSLastSeen() > OFFLINE_INTERVAL))
		{
			strcpy(buf, "#f000000");
			strcat(buf, ActivePeer->GetName());
			strcat(buf, "#");
		}
		else
		{
			strcpy(buf, ActivePeer->GetName());
		}
		lv_label_set_text(objects.lbl_peer_scrname, buf);	
		
        if (ActivePeer->GetSleepMode()) {
			lv_obj_add_state(objects.btn_peer_sleep, LV_STATE_CHECKED);
		}
		else {
			lv_obj_clear_state(objects.btn_peer_sleep, LV_STATE_CHECKED);
		}
		
        if (ActivePeer->GetPairMode()) {
			lv_obj_add_state(objects.btn_peer_pair, LV_STATE_CHECKED);
		}
		else {
			lv_obj_clear_state(objects.btn_peer_pair, LV_STATE_CHECKED);
		}
    }
    else 
    {
        lv_label_set_text(objects.lbl_peer_scrname, "No Peer");
        lv_obj_clear_state(objects.btn_peer_sleep, LV_STATE_CHECKED);
        lv_obj_clear_state(objects.btn_peer_pair, LV_STATE_CHECKED);
	}

}
void Scr_Peer_Next()
{
	ActivePeer = FindNextPeer(ActivePeer, MODULE_ALL, CIRCULAR); 
	if (ActivePeer) Scr_Peer_Timer_update(NULL);
}
void Scr_Peer_Prev()
{
	ActivePeer = FindPrevPeer(ActivePeer, MODULE_ALL, CIRCULAR); 
	if (ActivePeer) Scr_Peer_Timer_update(NULL);
}
void Btn_Peer_System_clicked()
{
    //if (ActivePeer) lv_scr_load_anim(objects.scr_system, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Peer_Volt_clicked()
{
    if (ActivePeer) lv_scr_load_anim(objects.scr_volt, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Peer_Eichen_clicked()
{
    if (ActivePeer) lv_scr_load_anim(objects.scr_eichen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Peer_Restart_clicked()
{
    if (ActivePeer) SendCommand(ActivePeer, SEND_CMD_RESTART);
}
void Btn_Peer_Sleep_clicked()
{
    if (ActivePeer) SendCommand(ActivePeer, SEND_CMD_SLEEPMODE_TOGGLE);
}
void Btn_Peer_Pair_clicked()
{
    if (ActivePeer) SendCommand(ActivePeer, SEND_CMD_PAIRMODE_TOGGLE);
}
void Btn_Peer_Reset_clicked()
{
    if (ActivePeer) SendCommand(ActivePeer, SEND_CMD_RESET);
}
void Btn_Peer_Delete_clicked()
{
    if (ActivePeer) DeletePeer(ActivePeer);
    ActivePeer = NULL;
    lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
#pragma endregion Scr_Peer
#pragma region Scr_Eichen
void Scr_Eichen_loaded()
{
}
void Scr_Eichen_unloaded()
{
}
void Btn_Eichen_Eichen_clicked()
{
    if (ActivePeer) 
    {
        CalibAmp();
        lv_scr_load_anim(objects.scr_peer, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
    }
}
#pragma endregion Scr_Eichen
