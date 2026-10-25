// Version 4.25

#include <Arduino.h>
#include "main.h"
#include "CustomScreens.h"
#include "lv_meter.h"
#include "PeerClass.h"
#include "CompButton.h"

lv_timer_t *SingleTimer;
lv_obj_t *ui_ScrSingle;
lv_obj_t *ui_TimeArc;

void ui_ScrSingle_screen_init(void)
{
    ui_ScrSingle = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_ScrSingle, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_bg_color(ui_ScrSingle, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_ScrSingle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    #ifndef HIDE_BG
		lv_obj_set_style_bg_img_src(ui_ScrSingle, &ui_img_jeepifybackground_png, LV_PART_MAIN | LV_STATE_DEFAULT);
	#endif
    lv_obj_set_style_bg_img_opa(ui_ScrSingle, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(ui_ScrSingle, ui_event_ui_ScrSingle, LV_EVENT_ALL, NULL);

}
void Ui_Single_ClearScreen()
{
	if (CompThingArray[0]) 
	{
		delete CompThingArray[0];
		CompThingArray[0] = NULL;
	}
	if (ui_TimeArc) 
	{
		lv_obj_del(ui_TimeArc);
		ui_TimeArc = NULL;
	}

}
void Ui_Single_FillScreen()
{	
	
	switch (ActivePeriphShown->GetPrimary())
    {
        case PRIM_SWITCH:	CompThingArray[0] = new CompButton;
							CompThingArray[0]->Setup(ui_ScrSingle, 0, 0, 0, 2, ActivePeriphShown, Ui_Single_Clicked);
							break;
		case PRIM_AMP:  	CompThingArray[0] = new CompMeter;
							CompThingArray[0]->Setup(ui_ScrSingle, 0, 0, 0, SCREEN_RES_HOR, ActivePeriphShown, Ui_Single_Clicked);
                        	break;
        case PRIM_VOLT: 	CompThingArray[0] = new CompMeter;
							CompThingArray[0]->Setup(ui_ScrSingle, 0, 0, 0, SCREEN_RES_HOR, ActivePeriphShown, Ui_Single_Clicked);
                        	break;
        case PRIM_WATT: 	CompThingArray[0] = new CompMeter;
							CompThingArray[0]->Setup(ui_ScrSingle, 0, 0, 0, SCREEN_RES_HOR, ActivePeriphShown, Ui_Single_Clicked);
                        	break;
		default:			return;
    }
	
	CompThingArray[0]->Update();
	
	ui_TimeArc = lv_arc_create(ui_ScrSingle);
	lv_obj_set_width(ui_TimeArc, SCREEN_RES_HOR);
	lv_obj_set_height(ui_TimeArc, SCREEN_RES_VER);
	lv_obj_set_align(ui_TimeArc, LV_ALIGN_CENTER);
	lv_obj_clear_flag(ui_TimeArc, LV_OBJ_FLAG_CLICKABLE);      /// Flags
	lv_arc_set_value(ui_TimeArc, 0);
	lv_obj_set_style_arc_color(ui_TimeArc, lv_color_hex(0x4A4A4A), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_opa(ui_TimeArc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_width(ui_TimeArc, 6, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_set_style_arc_color(ui_TimeArc, lv_color_hex(0x398F30), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_opa(ui_TimeArc, 150, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_arc_width(ui_TimeArc, 6, LV_PART_INDICATOR | LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(ui_TimeArc, lv_color_hex(0xDBDBDB), LV_PART_KNOB | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui_TimeArc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui_TimeArc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui_TimeArc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui_TimeArc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui_TimeArc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

}
void Ui_Single_Next(lv_event_t * e)
{	
	int PrimType = ActivePeriphShown->GetPrimary();
	if (PrimType == 0) return;

	if (PrimType == PRIM_SWITCH)
	{
		ActivePeriphSwitch = FindNextPeriph(NULL, ActivePeriphShown, P_IS_PRIMARY_SWITCH, CIRCULAR);
		ActivePeriphShown  = ActivePeriphSwitch;
	}
	else  
	{
		ActivePeriphSensor = FindNextPeriph(NULL, ActivePeriphShown, P_IS_PRIMARY_SENSOR, CIRCULAR);
		ActivePeriphShown  = ActivePeriphSensor;
	}

	if (ActivePeriphShown)
	{
		Ui_Single_ClearScreen();
		Ui_Single_FillScreen();
	}
}
void Ui_Single_Prev(lv_event_t * e)
{
	int PrimType = ActivePeriphShown->GetPrimary();
	if (PrimType == 0) return;

	if (PrimType == PRIM_SWITCH)
	{
		ActivePeriphSwitch = FindPrevPeriph(NULL, ActivePeriphShown, P_IS_PRIMARY_SWITCH, CIRCULAR);
		ActivePeriphShown  = ActivePeriphSwitch;
	}
	else  
	{
		ActivePeriphSensor = FindPrevPeriph(NULL, ActivePeriphShown, P_IS_PRIMARY_SENSOR, CIRCULAR);
		ActivePeriphShown  = ActivePeriphSensor;
	}
	

	if (ActivePeriphShown)
	{
		Ui_Single_ClearScreen();
		Ui_Single_FillScreen();
	}
}
void Ui_Single_Prepare(lv_event_t * e)
{
	int Pos = 0;

	if (ActivePeriphShown)
	{
		Ui_Single_ClearScreen();

		//size noch nicht gut
		Ui_Single_FillScreen();
		
		static uint32_t user_data = 10;
		
		if (SingleTimer) 
		{
			lv_timer_resume(SingleTimer);
		}
		else 
		{
			SingleTimer = lv_timer_create(SingleUpdateTimer, 100,  &user_data);
		}
	}
}
void SingleUpdateTimer(lv_timer_t * timer)
{
	/*if (Knob.Clicked)
	{
		Knob.LastClicked = Knob.Clicked;
		Knob.Clicked     = 0;
		
		if (Knob.Diff < 0) 
		{
			Ui_Single_Prev(NULL);
		}
		else
		{
			Ui_Single_Next(NULL);
		}
	}*/
	//Serial.println("SingleTimer");
	
	if (CompThingArray[0]) 
	{
		CompThingArray[0]->Update();
		PeerClass *peer = PeerOf(ActivePeriphShown);
		
		uint32_t TimeDiff = millis() - peer->GetTSLastSeen();
		if (TimeDiff < 2000) 
		{
			if (peer->GetdBm() > -90) 
			{
				lv_obj_set_style_arc_color(ui_TimeArc, lv_color_hex(0x398F30), LV_PART_INDICATOR | LV_STATE_DEFAULT);
			}
			else 
			{
				lv_obj_set_style_arc_color(ui_TimeArc, lv_color_hex(0x7C7E26), LV_PART_INDICATOR | LV_STATE_DEFAULT);
			}
		}
		else 
		{
			lv_obj_set_style_arc_color(ui_TimeArc, lv_color_hex(0xFF0000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
		}
		
		lv_arc_set_value(ui_TimeArc, TimeDiff / 100);
	}
}
void Ui_Single_Clicked(lv_event_t * e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
	PeriphClass *Periph;

    if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) {
        lv_indev_wait_release(lv_indev_get_act());
        Ui_Single_Next(e);
    }
    else if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
        lv_indev_wait_release(lv_indev_get_act());
        Ui_Single_Prev(e);
	}
	else if (event_code == LV_EVENT_CLICKED) 
	{
		if (ActivePeriphShown->IsPrimarySwitch())
		{
			Periph = FindPeriphById(atoi(lv_label_get_text(lv_obj_get_child(target, 3))));

			Periph->SetChanged(true);
			
			if (lv_obj_get_state(target) == LV_IMGBTN_STATE_DISABLED) 		  Periph->SetValue(0.0);
			if (lv_obj_get_state(target) == LV_IMGBTN_STATE_CHECKED_RELEASED) Periph->SetValue(1.0);
			
			ToggleSwitch(Periph);
		}
		else if (ActivePeriphShown->IsPrimarySensor()) 
        {
            if (CompThingArray[0] && target == CompThingArray[0]->GetLblCentral()) 
            {
                PeriphClass *Periph = ActivePeriphShown;
				DEBUG3("Primary of %s is switched from %d ", Periph->GetName(), Periph->GetPrimary());
				switch (Periph->GetPrimary())
				{
					case PRIM_SWITCH: 	if (Periph->IsVolt() and Periph->IsAmp()) 	Periph->SetPrimary(PRIM_WATT); 
										else if (Periph->IsVolt()) 					Periph->SetPrimary(PRIM_VOLT); 
										else if (Periph->IsAmp()) 					Periph->SetPrimary(PRIM_AMP); 
										break;
					case PRIM_WATT:   	Periph->SetPrimary(PRIM_VOLT); break;
					case PRIM_VOLT:   	if (Periph->IsAmp()) 						Periph->SetPrimary(PRIM_AMP); 
										else if (Periph->IsSwitch()) 				Periph->SetPrimary(PRIM_SWITCH); 
										break;
					case PRIM_AMP:    	if (Periph->IsSwitch()) 					Periph->SetPrimary(PRIM_SWITCH); 
										else if (Periph->IsVolt()) 					Periph->SetPrimary(PRIM_WATT); 
										break;
				}
				DEBUG3("to %d\n\r", Periph->GetPrimary());
				Ui_Single_Prepare(e);
			}
            else 
            {
                // Standardverhalten: Wenn irgendwo anders auf den Sensor geklickt wird
                Ui_Single_Next(e);
            }
        }
	}
}
void Ui_Single_Leave(lv_event_t * e)
{
	lv_timer_del(SingleTimer);
	SingleTimer = NULL;

	int Pos = 0;

	if (CompThingArray[Pos]) 
	{
		delete CompThingArray[Pos];
		CompThingArray[Pos] = NULL;
	}
}
void ui_event_ui_ScrSingle(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP) {
        lv_indev_wait_release(lv_indev_get_act());
        _ui_screen_change(&ui_ScrMenu, MY_ANIM, MY_ANIM_TIME, 0, &ui_ScrMenu_screen_init);
    }
    else if(event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) {
        lv_indev_wait_release(lv_indev_get_act());
        Ui_Single_Next(e);
    }
    else if(event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
        lv_indev_wait_release(lv_indev_get_act());
        Ui_Single_Prev(e);
    }
	else if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM) 
	{
		//ändere Primary ???
		
	}	
    if(event_code == LV_EVENT_SCREEN_LOAD_START) {
        Ui_Single_Prepare(e);
    }
    if(event_code == LV_EVENT_SCREEN_UNLOAD_START) {
        Ui_Single_Leave(e);
    }
	
}
