#include <Arduino.h>
#include "custom_functions.h"
#include "picoUI/actions.h"
#include "picoUI/screens.h"
#include "picoUI/images.h"
#include "main.h"
#include "CompButton.h"

uint8_t MultiPosToChange;
MultiMonitorClass *ActiveScreen;
int ActiveRollerId;
int MaxRollerItems;
int Scr_Peers_Mode;

CompThing   *CompThingArray[4] = {NULL, NULL, NULL, NULL};

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
#pragma region Scr_Volt
void Scr_Volt_loaded()
{
    if (ActivePeer) {
        lv_label_set_text(objects.lbl_volt_scrname, "12.00 V");
        lv_img_set_src(objects.img_simple_top, &meter_scale_v);
    }
}
void Arc_Volt_change()
{
    lv_label_set_text_fmt(objects.lbl_volt_volt_value, "%.2f V", lv_arc_get_value(objects.arc_volt));
}
void Btn_Volt_Save_clicked()
{
    if (ActivePeer) {
        float volt = lv_arc_get_value(objects.arc_volt);
        CalibVolt(volt);
    }
    lv_scr_load_anim(objects.scr_peer, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Scr_Volt_unloaded()
{
}
#pragma endregion Scr_Volt
#pragma region Scr_Settings
lv_timer_t *Timer_Scr_Settings = NULL;

void Scr_Settings_loaded()
{
    uint32_t user_data = 10;
	if (Timer_Scr_Settings) 
		{
			lv_timer_resume(Timer_Scr_Settings);
		}
		else 
		{
			Timer_Scr_Settings = lv_timer_create(Scr_Settings_Timer_update, 500,  &user_data);
		}
}
void Scr_Settings_Timer_update(lv_timer_t *timer)
{
    if (Module.GetPairMode()) 
    {
        lv_obj_add_state(objects.btn_set_pair, LV_STATE_CHECKED);
    }
    else 
    {
        lv_obj_clear_state(objects.btn_set_peers, LV_STATE_CHECKED);
    }
    if (Module.GetChanged()) 
    {
        lv_obj_add_state(objects.btn_set_save, LV_STATE_CHECKED);
    }
    else 
    {
        lv_obj_clear_state(objects.btn_set_save, LV_STATE_CHECKED);
    }
    if (WebServerActive) 
    {
        lv_obj_add_state(objects.btn_set_websvr, LV_STATE_CHECKED);
    }
    else 
    {
        lv_obj_clear_state(objects.btn_set_websvr, LV_STATE_CHECKED);
    }

}
void Scr_Settings_unloaded()
{
    if (Timer_Scr_Settings) lv_timer_del(Timer_Scr_Settings);
	Timer_Scr_Settings = NULL;
}   
void Btn_Set_Save_clicked()
{
    SavePeers();
	Module.SetChanged(false);
	char buf1[10]  = "";
	char buf2[100] = "free entries: ";

	preferences.begin("JeepifyPeers", true);
    	itoa(preferences.freeEntries(), buf1, 10);  
    preferences.end();
	strcat(buf2, buf1);
	
	if (Module.GetDebugMode()) ShowMessageBox("Saving complete...", buf2, 1000, 200);
}
#pragma endregion Scr_Settings
#pragma region Scr_Multi
lv_timer_t *Timer_Scr_Multi = NULL;
void Scr_Multi_Timer_update(lv_timer_t *timer)
{
    for (int Pos=0; Pos<PERIPH_PER_SCREEN; Pos++) 
		if (Screen[ActiveMultiScreen].GetPeriphId(Pos) >= 0) CompThingArray[Pos]->Update();
}
void Scr_Multi_Loaded(lv_event_t * e)
{
	static uint32_t user_data = 10;
	
	Ui_Multi_FillScreen();
	
	if (Timer_Scr_Multi) 
	{
		lv_timer_resume(Timer_Scr_Multi);
	}
	else 
	{
		Timer_Scr_Multi = lv_timer_create(Scr_Multi_Timer_update, 100,  &user_data);
	}
	ActiveScreen = &Screen[ActiveMultiScreen];
}
void Scr_Multi_Unloaded(lv_event_t * e)
{
	if (Timer_Scr_Multi) lv_timer_del(Timer_Scr_Multi);
	Timer_Scr_Multi = NULL;
}

int MultiPosToChange = -1;
void Btn_Multi_1_clicked()
{
    if (PeriphList.size() > 0) 
	{
		MultiPosToChange = 0;
		Ui_Multi_Unload(e);
		ActiveScreen = &Screen[ActiveMultiScreen];
		_ui_screen_change(&ui_ScrPeriph, LV_SCR_LOAD_ANIM_NONE, 0, 0, &ui_ScrPeriph_screen_init);
	}
}
#pragma endregion Scr_Multi