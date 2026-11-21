#include <Arduino.h>
#include "picoUI/actions.h"
#include "picoUI/screens.h"
#include "picoUI/images.h"
#include "picoUI/colors.h"
#include "main.h"
#include "picoButton.h"
#include "custom_functions.h"
#include <ArduinoJson.h>

uint8_t MultiPosToChange;
MultiMonitorClass *ActiveScreen;

int ActiveRollerId;
int MaxRollerItems;
int Scr_Peers_Mode;

CompThing   *CompThingArray[4] = {NULL, NULL, NULL, NULL};

#pragma region Scr_Navi
lv_timer_t *Timer_Scr_Navi = NULL;
lv_obj_t *Img_Wifi;

void Scr_Navi_init()
{
	lv_obj_t *parent_obj = lv_layer_top();
	{
		// img_top_wifi
		lv_obj_t *obj = lv_img_create(parent_obj);
		Img_Wifi = obj;
		lv_obj_set_pos(obj, 165, 332);
		lv_obj_set_size(obj, 31, 26);
		lv_obj_set_style_img_recolor(obj, lv_color_hex(common_dc1010), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_img_recolor_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_img_set_src(obj, &icon_wlan);
		lv_img_set_size_mode(obj, LV_IMG_SIZE_MODE_REAL);
		lv_img_set_zoom(obj, 220);
		lv_img_set_antialias(obj, true);
	}

	static uint32_t user_data = 10;
		
	if (Timer_Scr_Navi) 
	{
		lv_timer_resume(Timer_Scr_Navi);
	}
	else 
	{
		Timer_Scr_Navi = lv_timer_create(Scr_Navi_Timer_update, 100,  &user_data);
		DEBUG3("Navi-Timer created\n\r");
	}
    tick_screen_scr_navi();
}
void Scr_Navi_SendIcon(int color)
{
	switch (color)
	{
		case 0: //off
				lv_obj_set_style_img_recolor(Img_Wifi, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT); break;
		case 1: //red
				lv_obj_set_style_img_recolor(Img_Wifi, lv_color_hex(0x990000), LV_PART_MAIN | LV_STATE_DEFAULT); break;
		case 2: //green
				lv_obj_set_style_img_recolor(Img_Wifi, lv_color_hex(0x009900), LV_PART_MAIN | LV_STATE_DEFAULT); break;
		case 3: //blue
				lv_obj_set_style_img_recolor(Img_Wifi, lv_color_hex(0x000099), LV_PART_MAIN | LV_STATE_DEFAULT); break;
	}
}
void Scr_Navi_Timer_update(lv_timer_t * timer)
{
	uint32_t aktZeit = millis();
	int color = 0;

	if (aktZeit - TSMsgSnd < MSGLIGHT_INTERVAL) color = 1; 
	if (aktZeit - TSMsgRcv < MSGLIGHT_INTERVAL) color = 2;
	if (aktZeit - TSPair   < PAIR_INTERVAL)     color = 3;
	
	Scr_Navi_SendIcon(color);

	#ifdef BATTERY_PORT
		char buf[10];
		static int BattCount;
		static int BattRaw;

		BattRaw += analogRead(BATTERY_PORT);
		BattCount++;

		if (BattCount == 20)
		{
			float Batt = 3.3/4095 * BattRaw/BattCount * BATTERY_DEVIDER;
			dtostrf(Batt, 0, 2, buf);
			strcat(buf, "V");
			lv_label_set_text(objects.lbl_start_bat, buf);
			BattCount = 0;
			BattRaw   = 0;
		}
	#endif
}
void tick_screen_scr_navi() {
}
#pragma endregion Scr_Navi
#pragma region Scr_Top
#pragma endregion Scr_Top
#pragma region Scr_PeriphChoice
lv_timer_t *Timer_Scr_PeriphChoice = NULL;
void Scr_PeriphChoice_loaded(lv_event_t * e)
{
    static uint32_t user_data = 10;
	
	if (!ActivePeriph) ActivePeriph = FindNextPeriph(NULL, NULL, SENS_TYPE_ALL, CIRCULAR);
	ActivePeriph->SetPrimHelper(ActivePeriph->GetPrimary());

	Scr_PeriphChoice_FillScreen(e);

	if (Timer_Scr_PeriphChoice) 
	{
		lv_timer_resume(Timer_Scr_PeriphChoice);
	}
	else 
	{
		Timer_Scr_PeriphChoice = lv_timer_create(Scr_PeriphChoice_Timer_update, 100,  &user_data);
	}
}
void Scr_PeriphChoice_unloaded(lv_event_t * e)
{
    if (Timer_Scr_PeriphChoice) lv_timer_del(Timer_Scr_PeriphChoice);
	Timer_Scr_PeriphChoice = NULL;
}
void Scr_PeriphChoice_Return(lv_event_t * e)
{
	//if (ActiveScreen == &MultiGaugeScreen)
		//_ui_screen_change(&ui_ScrMultiGauge, MY_ANIM, MY_ANIM_TIME, 0, &ui_ScrMultiGauge_screen_init);
	//else
		lv_scr_load_anim(objects.scr_multi, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Scr_PeriphChoice_FillScreen(lv_event_t * e)
{
	PeerClass *P;
	if (!ActivePeriph) ActivePeriph = FindNextPeriph(NULL, NULL, SENS_TYPE_ALL, CIRCULAR);
	
	if (!ActivePeriph) return;
	
	P = PeerOf(ActivePeriph);

	switch (ActivePeriph->GetPrimHelper()) 
	{
		case PRIM_SWITCH: lv_label_set_text_fmt(objects.lbl_periphchoice_periphname, "%s: (SW)", ActivePeriph->GetName()); break;
		case PRIM_VOLT:   lv_label_set_text_fmt(objects.lbl_periphchoice_periphname, "%s: (V)",  ActivePeriph->GetName()); break;
		case PRIM_AMP:    lv_label_set_text_fmt(objects.lbl_periphchoice_periphname, "%s: (A)",  ActivePeriph->GetName()); break;
		case PRIM_WATT:   lv_label_set_text_fmt(objects.lbl_periphchoice_periphname, "%s: (W)",  ActivePeriph->GetName()); break;
		default:          lv_label_set_text_fmt(objects.lbl_periphchoice_periphname, "%s: (?)",  ActivePeriph->GetName()); break;
	}
	
	lv_label_set_text(objects.lbl_periphchoice_scrname, P->GetName());

	if (millis()- P->GetTSLastSeen() > OFFLINE_INTERVAL) 
		lv_label_set_text(objects.lbl_periphchoice_online, "Offline");
	else 
		lv_label_set_text(objects.lbl_periphchoice_online, "Online");

	lv_label_set_text(objects.lbl_periphchoice_type, TypeInText(ActivePeriph->GetType())); 

	if (ActivePeriph->GetPrimHelper() == PRIM_SWITCH) lv_img_set_src(objects.img_periphchoice, &kipp_1_on_70); 
	else lv_img_set_src(objects.img_periphchoice, &menubtn1_png);
}
void Btn_PeriphChoice_Next_clicked(lv_event_t * e)
{
	if (!ActivePeriph) return;
	
	int OldPrim = ActivePeriph->GetPrimHelper();
	int NewPrim = OldPrim;

	static int StartPrim = 0;
	if (StartPrim == 0) StartPrim = OldPrim;

	if      ((OldPrim == PRIM_SWITCH) && (ActivePeriph->GetType() & P_IS_VOLT)) NewPrim = PRIM_VOLT;
	else if ((OldPrim == PRIM_SWITCH) && (ActivePeriph->GetType() & P_IS_AMP))  NewPrim = PRIM_AMP;
	else if ((OldPrim == PRIM_VOLT) && (ActivePeriph->GetType() & P_IS_AMP))    NewPrim = PRIM_AMP;
	else if ((OldPrim == PRIM_VOLT) && (ActivePeriph->GetType() & P_IS_SWITCH)) NewPrim = PRIM_SWITCH;
	else if ((OldPrim == PRIM_AMP) && (ActivePeriph->GetType() & P_IS_VOLT))    NewPrim = PRIM_WATT;
	else if ((OldPrim == PRIM_AMP) && (ActivePeriph->GetType() & P_IS_SWITCH))  NewPrim = PRIM_SWITCH;
	else if ((OldPrim == PRIM_WATT) && (ActivePeriph->GetType() & P_IS_SWITCH)) NewPrim = PRIM_SWITCH;
	else if ( OldPrim == PRIM_WATT)   											NewPrim = PRIM_VOLT;

	DEBUG3("STartPrim = %d, OldPrim=%d, NewPrim=%d, getType: %d, ActivePeriph->GetPrimary()=%d\n\r", StartPrim, OldPrim, NewPrim, ActivePeriph->GetType(), ActivePeriph->GetPrimary());

	if (NewPrim != StartPrim)
	{
		ActivePeriph->SetPrimHelper(NewPrim);
		DEBUG3("ActivePeriph->SetPrimHelper(NewPrim)=%d\n\r", NewPrim);
	}
	else
	{
		PeriphClass *PeriphT = FindNextPeriph(NULL, ActivePeriph, SENS_TYPE_ALL, CIRCULAR);
		if (PeriphT) 
		{
			ActivePeriph = PeriphT;
			ActivePeriph->SetPrimHelper(ActivePeriph->GetPrimary());
			StartPrim = 0;
			DEBUG3("new: StartPrim: %d, ActivePeriph: %s, ActivePeriph->GetPrimary()=%d\n\r", StartPrim, ActivePeriph->GetName(), ActivePeriph->GetPrimary());
		}
	}
	Scr_PeriphChoice_FillScreen(e);

}
void Btn_PeriphChoice_Prev_clicked(lv_event_t * e)
{
	if (!ActivePeriph) return;
	
	int OldPrim = ActivePeriph->GetPrimHelper();
	int NewPrim = OldPrim;

	static int StartPrim = 0;
	if (StartPrim == 0) StartPrim = OldPrim;

	if      ( OldPrim == PRIM_WATT) NewPrim = PRIM_AMP;
  	else if ((OldPrim == PRIM_AMP) && (ActivePeriph->GetType() & P_IS_VOLT))    NewPrim = PRIM_VOLT;
	else if ((OldPrim == PRIM_AMP) && (ActivePeriph->GetType() & P_IS_SWITCH))  NewPrim = PRIM_SWITCH;								
	else if ((OldPrim == PRIM_VOLT) && (ActivePeriph->GetType() & P_IS_SWITCH)) NewPrim = PRIM_SWITCH;
	else if ((OldPrim == PRIM_VOLT) && (ActivePeriph->GetType() & P_IS_AMP))    NewPrim = PRIM_WATT;
	else if ((OldPrim == PRIM_SWITCH) && (ActivePeriph->GetType() & P_IS_VOLT) && (ActivePeriph->GetType() & P_IS_AMP)) NewPrim = PRIM_WATT;
	else if ((OldPrim == PRIM_SWITCH) && (ActivePeriph->GetType() & P_IS_AMP)) NewPrim = PRIM_AMP;
	else if ((OldPrim == PRIM_SWITCH) && (ActivePeriph->GetType() & P_IS_VOLT)) NewPrim = PRIM_VOLT;
	
	DEBUG3("StartPrim = %d, OldPrim=%d, NewPrim=%d, ActivePeriph->GetPrimary()=%d\n\r", StartPrim, OldPrim, NewPrim, ActivePeriph->GetPrimary());

	if (NewPrim != StartPrim)
	{
		ActivePeriph->SetPrimHelper(NewPrim);
		DEBUG3("ActivePeriph->SetPrimHelper(NewPrim)=%d\n\r", NewPrim);
	}
	else
	{
		PeriphClass *PeriphT = FindPrevPeriph(NULL, ActivePeriph, SENS_TYPE_ALL, CIRCULAR);
		if (PeriphT) 
		{
			ActivePeriph = PeriphT;
			ActivePeriph->SetPrimHelper(ActivePeriph->GetPrimary());
			StartPrim = 0;
			DEBUG3("new: StartPrim: %d,  ActivePeriph: %s, ActivePeriph->GetPrimary()=%d\n\r", StartPrim, ActivePeriph->GetName(), ActivePeriph->GetPrimary());

		}
	}
	Scr_PeriphChoice_FillScreen(e);
}
void Btn_PeriphChoice_Up_clicked(lv_event_t * e)
{
	Scr_PeriphChoice_Return(e);
}
void Btn_PeriphChoice_clicked(lv_event_t * e)
{		
	if (ActiveScreen == &MultiGaugeScreen)
	{
		//vielleicht Primary in Activescreen festlegen
		ActiveScreen->AddPeriph(MultiPosToChange, ActivePeriph);
		ActiveScreen->SetPrimToShow(MultiPosToChange, ActivePeriph->GetPrimHelper());
	}
	else
	{
		Screen[ActiveMultiScreen].AddPeriph(MultiPosToChange, ActivePeriph);
		Screen[ActiveMultiScreen].SetPrimToShow(MultiPosToChange, ActivePeriph->GetPrimHelper());
	}	
	
	Scr_PeriphChoice_Return(e);
}
void Scr_PeriphChoice_Timer_update(lv_timer_t * timer)
{
	
}
void Scr_PeriphChoice_Gesture(lv_event_t * e)
{
	lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
#pragma endregion Scr_PeriphChoice
#pragma region Scr_Start
void Scr_Start_loaded(lv_event_t * e)
{
}
void Scr_Start_unloaded(lv_event_t * e)
{
}
void Btn_Start_1_clicked(lv_event_t * e)
{
	if (!ActivePeriphSensor) ActivePeriphSensor = FindNextPeriph(NULL, NULL, P_IS_PRIMARY_SENSOR, CIRCULAR, FUCKLINE);
	if (ActivePeriphSensor) 
	{	
		DEBUG3("PeriphName = %s\n\r", ActivePeriphSensor->GetName());
		//ActivePeriphShown = ActivePeriphSensor;
		lv_scr_load_anim(objects.scr_meter, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
	}
}
void Btn_Start_2_clicked(lv_event_t * e)
{
	if (!ActivePeriphSwitch) ActivePeriphSwitch = FindNextPeriph(NULL, NULL, P_IS_PRIMARY_SWITCH, CIRCULAR, FUCKLINE);
	if (ActivePeriphSwitch) 
	{
		//ActivePeriphShown = ActivePeriphSwitch;
		Serial.printf("Menu - 2 - ActivePeriphSwitch is %s\n\r", ActivePeriphSwitch->GetName());
		lv_scr_load_anim(objects.scr_meter, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
	}
}
void Scr_Start_Gesture(lv_event_t * e)
{

}
#pragma endregion Scr_Start
#pragma region Scr_Meter
lv_timer_t *Timer_Scr_Meter = NULL;
uint32_t TSLastClickMeter = -1;

void Scr_Meter_loaded(lv_event_t * e)
{
	if (ActivePeriphSensor)
	{
		uint16_t newSize = 256*SCREEN_RES_HOR/360; //+5
    
		if (ActivePeriphSensor->GetPrimary() == PRIM_VOLT)      lv_img_set_src(objects.img_meter_top, &meter_scale_v);
		else if (ActivePeriphSensor->GetPrimary() == PRIM_AMP)  lv_img_set_src(objects.img_meter_top, &meter_scale_a);
		else if (ActivePeriphSensor->GetPrimary() == PRIM_WATT) lv_img_set_src(objects.img_meter_top, &meter_scale_w);
		
		lv_img_set_zoom(objects.img_meter_top, newSize);
		lv_obj_set_width(objects.img_meter_top, LV_SIZE_CONTENT);   /// 1
		lv_obj_set_height(objects.img_meter_top, LV_SIZE_CONTENT);    /// 1
		
		lv_obj_clear_flag(objects.img_meter_top, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
		lv_obj_move_background(objects.img_meter_top);
			
		lv_img_set_zoom(objects.img_meter_zeiger, newSize);
		lv_obj_set_width(objects.img_meter_zeiger, LV_SIZE_CONTENT);   /// 1
		lv_obj_set_height(objects.img_meter_zeiger, LV_SIZE_CONTENT);    /// 1
		
		lv_img_set_pivot(objects.img_meter_zeiger, 5, 150); 
		
		lv_obj_add_flag(objects.img_meter_zeiger, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
		lv_obj_clear_flag(objects.img_meter_zeiger, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
		
		switch (SCREEN_RES_VER)
		{
			case 240:
				lv_obj_set_y(objects.img_meter_zeiger, lv_pct(-17)); 
				break;
			case 360:
				lv_obj_set_y(objects.img_meter_zeiger, lv_pct(30));    
				break;
			case 466:
				lv_obj_set_y(objects.img_meter_zeiger, lv_pct(2));    
				break;
			default:
				lv_obj_set_y(objects.img_meter_zeiger, lv_pct(-4));    
				break;
		}   

		static uint32_t user_data = 10;
		
		if (Timer_Scr_Meter) 
		{
			lv_timer_resume(Timer_Scr_Meter);
		}
		else 
		{
			Timer_Scr_Meter = lv_timer_create(Scr_Meter_Timer_update, 100,  &user_data);
		}
	}
}
void Scr_Meter_Timer_update(lv_timer_t * timer)
{
	if (millis() - TSLastClickMeter > 1000)
	{
		lv_obj_add_flag(objects.btn_meter_prev, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objects.btn_meter_next, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objects.btn_meter_up,   LV_OBJ_FLAG_HIDDEN);
		TSLastClickMeter = -1;
	}
	
	PeerClass *peer = PeerOf(ActivePeriphSensor);
	if (peer->GetName()) lv_label_set_text_fmt(objects.lbl_meter_peername, "%.6s (%d dBm)", peer->GetName(), peer->GetdBm());
	if (ActivePeriphSensor->GetName()) lv_label_set_text_fmt(objects.lbl_meter_periphname, "%.6s", ActivePeriphSensor->GetName());
    
	char buf[10];
	int nk = 0;
	float value = 0;
    int Range = 485;	
    int Angle = 0;
    int Tick = 0;
    
    switch (ActivePeriphSensor->GetPrimary())
    {
        case PRIM_AMP:  value = ActivePeriphSensor->GetValue(V_AMP); 
                        Angle = -Range + (int)((value * 2.0f * 30) / 15.0f); //KI
                        break;
        case PRIM_VOLT: value = ActivePeriphSensor->GetValue(V_VOLT); 
                        Tick = 2*Range/15;
                        Angle = -Range+value*Tick;
                        break;
        case PRIM_WATT: value = ActivePeriphSensor->GetValue(V_VOLT) * ActivePeriphSensor->GetValue(V_AMP); 
                        Tick = 2*Range/400;
                        Angle = -Range+value*Tick;
                        break;
    }
	static int TestV;
	TestV++;
	if (TestV > 300) TestV=0;

    lv_img_set_angle(objects.img_meter_zeiger, Angle);
    
	if (abs(value) < SCHWELLE) value = 0;

	if      (value<10)  nk = 2;
	else if (value<100) nk = 1;
	else                nk = 0;

	if (value == -99) strcpy(buf, "--"); 
	else dtostrf(value, 0, nk, buf);
	
	if (ActivePeriphSensor->GetPrimary() == PRIM_AMP)       strcat(buf, " A");
	else if (ActivePeriphSensor->GetPrimary() == PRIM_VOLT) strcat(buf, " V");
	else if (ActivePeriphSensor->GetPrimary() == PRIM_WATT) strcat(buf, " W");
	
	lv_label_set_text(objects.lbl_meter_value, buf);
	
	uint32_t TimeDiff = millis() - peer->GetTSLastSeen();
	if (TimeDiff < 2000) 
	{
		if (peer->GetdBm() > -90) 
		{
			lv_obj_set_style_arc_color(objects.arc_meter, lv_color_hex(0x398F30), LV_PART_INDICATOR | LV_STATE_DEFAULT);
		}
		else 
		{
			lv_obj_set_style_arc_color(objects.arc_meter, lv_color_hex(0x7C7E26), LV_PART_INDICATOR | LV_STATE_DEFAULT);
		}
	}
	else 
	{
		lv_obj_set_style_arc_color(objects.arc_meter, lv_color_hex(0xFF0000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	}
	
	lv_arc_set_value(objects.arc_meter, TimeDiff / 100);
}
void Scr_Meter_unloaded(lv_event_t * e)
{
	//Scr_Simple_ClearScreen(e);
}
void Scr_Meter_pressed(lv_event_t * e)
{
	DEBUG3("Meter - pressed\n\r");
	lv_event_code_t event_code = lv_event_get_code(e);
	
	lv_obj_clear_flag(objects.btn_meter_prev, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.btn_meter_next, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.btn_meter_up,   LV_OBJ_FLAG_HIDDEN);
	TSLastClickMeter = millis();

}
void Btn_Meter_Prev_clicked(lv_event_t * e)
{
	ActivePeriphSensor = FindPrevPeriph(NULL, ActivePeriphSensor, P_IS_PRIMARY_SENSOR, CIRCULAR);
	//ActivePeriphShown  = ActivePeriphSensor;
	lv_scr_load_anim(objects.scr_meter, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Meter_Next_clicked(lv_event_t * e)
{	
	ActivePeriphSensor = FindNextPeriph(NULL, ActivePeriphSensor, P_IS_PRIMARY_SENSOR, CIRCULAR);
	//ActivePeriphShown  = ActivePeriphSensor;
	DEBUG3("Next Periph will be: %s", ActivePeriphSensor->GetName());
	lv_scr_load_anim(objects.scr_meter, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Meter_Up_clicked(lv_event_t * e)
{
	lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Scr_Meter_clicked(lv_event_t * e)
{
	DEBUG3("Meter - clicked\n\r");
	Btn_Meter_Next_clicked(e);
}
void Scr_Meter_long_clicked(lv_event_t * e)
{
	PeriphClass *Periph = ActivePeriphSensor;
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
	lv_scr_load_anim(objects.scr_meter, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Scr_Meter_Gesture(lv_event_t * e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
    
    if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Meter_Next_clicked(e);
		DEBUG3("Meter - Gesture - left\n\r");
    }
    else if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Meter_Prev_clicked(e);
		DEBUG3("Meter - Gesture - right\n\r");
	}
	else
	{
		DEBUG3("Meter - Gesture - not recognized !\n\r");
	}
}
#pragma endregion Scr_Meter
#pragma region Scr_Switch
lv_timer_t *Timer_Scr_Switch = NULL;
uint32_t TSLastClickSwitch = -1;

void Scr_Switch_loaded(lv_event_t * e)
{
	CompThingArray[0] = new CompButton;
	CompThingArray[0]->Setup(lv_scr_act(), 0, 0, 0, 2, ActivePeriphSwitch, Scr_Switch_clicked);
	CompThingArray[0]->Update();
	
	static uint32_t user_data = 10;
	
	if (Timer_Scr_Switch) 
	{
		lv_timer_resume(Timer_Scr_Switch);
	}
	else 
	{
		Timer_Scr_Switch = lv_timer_create(Scr_Switch_Timer_update, 100,  &user_data);
	}
}
void Scr_Switch_Timer_update(lv_timer_t * timer)
{
	if (millis() - TSLastClickSwitch > 1000)
	{
		lv_obj_add_flag(objects.btn_switch_prev, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objects.btn_switch_next, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objects.btn_switch_up,   LV_OBJ_FLAG_HIDDEN);
		TSLastClickSwitch = -1;
	}
	if (CompThingArray[0]) 
	{
		CompThingArray[0]->Update();
		PeerClass *peer = PeerOf(ActivePeriphSwitch);
		
		uint32_t TimeDiff = millis() - peer->GetTSLastSeen();
		if (TimeDiff < 2000) 
		{
			if (peer->GetdBm() > -90) 
			{
				lv_obj_set_style_arc_color(objects.arc_switch, lv_color_hex(0x398F30), LV_PART_INDICATOR | LV_STATE_DEFAULT);
			}
			else 
			{
				lv_obj_set_style_arc_color(objects.arc_switch, lv_color_hex(0x7C7E26), LV_PART_INDICATOR | LV_STATE_DEFAULT);
			}
		}
		else 
		{
			lv_obj_set_style_arc_color(objects.arc_switch, lv_color_hex(0xFF0000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
		}
		
		lv_arc_set_value(objects.arc_switch, TimeDiff / 100);
	}
}
void Scr_Switch_unloaded(lv_event_t * e)
{
	if (CompThingArray[0]) 
	{
		delete CompThingArray[0];
		CompThingArray[0] = NULL;
	}
}
void Scr_Switch_pressed(lv_event_t * e)
{
	lv_obj_clear_flag(objects.btn_switch_prev, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.btn_switch_next, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.btn_switch_up,   LV_OBJ_FLAG_HIDDEN);
	TSLastClickSwitch = millis();
}
void Btn_Switch_Prev_clicked(lv_event_t * e)
{
	ActivePeriphSwitch = FindPrevPeriph(NULL, ActivePeriphSwitch, P_IS_PRIMARY_SWITCH, CIRCULAR);
	//ActivePeriphShown  = ActivePeriphSwitch;

	if (ActivePeriphSwitch) lv_scr_load_anim(objects.scr_switch, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
	//Update??
}
void Btn_Switch_Next_clicked(lv_event_t * e)
{	
	ActivePeriphSwitch = FindNextPeriph(NULL, ActivePeriphSwitch, P_IS_PRIMARY_SWITCH, CIRCULAR);
	//ActivePeriphShown  = ActivePeriphSwitch;

	if (ActivePeriphSwitch) lv_scr_load_anim(objects.scr_switch, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
	//Update??
}
void Btn_Switch_Up_clicked(lv_event_t * e)
{
	lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Scr_Switch_Gesture(lv_event_t * e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
    
    if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Switch_Next_clicked(e);
    }
    else if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Switch_Prev_clicked(e);
	}
	else if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Switch_Up_clicked(e);
	}
}
void Scr_Switch_clicked(lv_event_t * e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
	
	if (event_code == LV_EVENT_CLICKED) 
	{
		ActivePeriphSwitch->SetChanged(true);
			
			if (lv_obj_get_state(target) == LV_IMGBTN_STATE_DISABLED) 		  ActivePeriphSwitch->SetValue(0.0);
			if (lv_obj_get_state(target) == LV_IMGBTN_STATE_CHECKED_RELEASED) ActivePeriphSwitch->SetValue(1.0);
			
			ToggleSwitch(ActivePeriphSwitch);
	}	
}
#pragma endregion Scr_Switch
#pragma region Scr_JSON
void Scr_JSON_loaded(lv_event_t * e)
{
    if (jsondataBuf[0] != '\0') 
    { 
        JsonDocument doc;
        
        DeserializationError error = deserializeJson(doc, jsondataBuf);
        if (!error) {
            if (doc["Node"] != NODE_NAME) { 
                //lv_textarea_set_placeholder_text(ui_TxtJSON1, jsondataBuf);
                lv_textarea_set_placeholder_text(objects.txt_json_1, jsondataBuf);
            }
        }
        jsondataBuf[0] = '\0'; 
    }
}
void Scr_JSON_unloaded(lv_event_t * e)
{
}   
void Scr_JSON_Gesture(lv_event_t * e)
{
	lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
#pragma endregion Scr_JSON
#pragma region Scr_Simple
/*lv_timer_t *Timer_Scr_Simple = NULL;
uint32_t TSLastClickSimple = -1;

void Scr_Simple_loaded(lv_event_t * e)
{
	if (ActivePeriphShown)
	{
		Scr_Simple_ClearScreen(e);

		//size noch nicht gut
		Scr_Simple_FillScreen(e);
		
		static uint32_t user_data = 10;
		
		if (Timer_Scr_Simple) 
		{
			lv_timer_resume(Timer_Scr_Simple);
		}
		else 
		{
			Timer_Scr_Simple = lv_timer_create(Scr_Simple_Timer_update, 100,  &user_data);
		}
	}
}
void Scr_Simple_Timer_update(lv_timer_t * timer)
{
	if (millis() - TSLastClickSimple > 1000)
	{
		lv_obj_add_flag(objects.btn_simple_prev, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objects.btn_simple_next, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objects.btn_simple_up,   LV_OBJ_FLAG_HIDDEN);
		TSLastClickSimple = -1;
	}
	if (CompThingArray[0]) 
	{
		CompThingArray[0]->Update();
		PeerClass *peer = PeerOf(ActivePeriphShown);
		
		uint32_t TimeDiff = millis() - peer->GetTSLastSeen();
		if (TimeDiff < 2000) 
		{
			if (peer->GetdBm() > -90) 
			{
				lv_obj_set_style_arc_color(objects.arc_simple, lv_color_hex(0x398F30), LV_PART_INDICATOR | LV_STATE_DEFAULT);
			}
			else 
			{
				lv_obj_set_style_arc_color(objects.arc_simple, lv_color_hex(0x7C7E26), LV_PART_INDICATOR | LV_STATE_DEFAULT);
			}
		}
		else 
		{
			lv_obj_set_style_arc_color(objects.arc_simple, lv_color_hex(0xFF0000), LV_PART_INDICATOR | LV_STATE_DEFAULT);
		}
		
		lv_arc_set_value(objects.arc_simple, TimeDiff / 100);
	}
}
void Scr_Simple_unloaded(lv_event_t * e)
{
	Scr_Simple_ClearScreen(e);
}
void Scr_Simple_FillScreen(lv_event_t * e)
{	
	
	switch (ActivePeriphShown->GetPrimary())
    {
        case PRIM_SWITCH:	CompThingArray[0] = new CompButton;
							CompThingArray[0]->Setup(lv_scr_act(), 0, 0, 0, 2, ActivePeriphShown, Scr_Simple_clicked);
							break;
		case PRIM_AMP:  	CompThingArray[0] = new CompMeter;
							CompThingArray[0]->Setup(lv_scr_act(), 0, 0, 0, SCREEN_RES_HOR, ActivePeriphShown, Scr_Simple_clicked);
                        	break;
        case PRIM_VOLT: 	CompThingArray[0] = new CompMeter;
							CompThingArray[0]->Setup(lv_scr_act(), 0, 0, 0, SCREEN_RES_HOR, ActivePeriphShown, Scr_Simple_clicked);
                        	break;
        case PRIM_WATT: 	CompThingArray[0] = new CompMeter;
							CompThingArray[0]->Setup(lv_scr_act(), 0, 0, 0, SCREEN_RES_HOR, ActivePeriphShown, Scr_Simple_clicked);
                        	break;
		default:			return;
    }
	
	CompThingArray[0]->Update();
}
void Scr_Simple_ClearScreen(lv_event_t * e)
{
	if (CompThingArray[0]) 
	{
		delete CompThingArray[0];
		CompThingArray[0] = NULL;
	}
}
void Scr_Simple_pressed(lv_event_t * e)
{
	lv_obj_clear_flag(objects.btn_simple_prev, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.btn_simple_next, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.btn_simple_up,   LV_OBJ_FLAG_HIDDEN);
	TSLastClickSimple = millis();
}
void Btn_Simple_Prev_clicked(lv_event_t * e)
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
		Scr_Simple_ClearScreen(e);
		Scr_Simple_FillScreen(e);
	}
}
void Btn_Simple_Next_clicked(lv_event_t * e)
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
		Scr_Simple_ClearScreen(e);
		Scr_Simple_FillScreen(e);
	}
}
void Btn_Simple_Up_clicked(lv_event_t * e)
{
	lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Scr_Simple_clicked(lv_event_t * e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t * target = lv_event_get_target(e);
	PeriphClass *Periph;

    if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Simple_Next_clicked(e);
    }
    else if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Simple_Prev_clicked(e);
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
				Scr_Simple_loaded(e);
			}
            else 
            {
                // Standardverhalten: Wenn irgendwo anders auf den Sensor geklickt wird
                Btn_Simple_Next_clicked(e);
            }
        }
	}
}*/
#pragma endregion Scr_Simple
#pragma region Scr_Peer
lv_timer_t *Timer_Scr_Peer = NULL;
uint32_t TSLastClickPeer = -1;

void Scr_Peer_loaded(lv_event_t * e)
{
    if (!ActivePeer) ActivePeer = FindNextPeer(NULL, MODULE_ALL, CIRCULAR);
	
	lv_label_set_recolor(objects.lbl_peer_scrname, true);
	lv_label_set_text(objects.lbl_peer_scrname, ActivePeer->GetName());

	static uint32_t user_data = 10;
	
	if (Timer_Scr_Peer) 
	{
		lv_timer_resume(Timer_Scr_Peer);
	}
	else 
	{
		Timer_Scr_Peer = lv_timer_create(Scr_Peer_Timer_update, 100,  &user_data);
	}
}
void Scr_Peer_unloaded(lv_event_t * e)
{
    if (Timer_Scr_Peer) lv_timer_del(Timer_Scr_Peer);
	Timer_Scr_Peer = NULL;
}
void Scr_Peer_Timer_update(lv_timer_t * timer)
{
    if ((TSLastClickPeer != -1) && (millis() - TSLastClickPeer > 1000))
	{
		lv_obj_add_flag(objects.btn_multi_prev, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objects.btn_multi_next, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objects.btn_multi_up,   LV_OBJ_FLAG_HIDDEN);
		TSLastClickPeer = -1;
	}	
	if (ActivePeer) 
    {
		Serial.printf("Scr_Peer_Timer_update: ActivePeer is %s\n\r", ActivePeer->GetName());
		char buf[50];
		if ((ActivePeer->GetTSLastSeen() == 0) or (millis()- ActivePeer->GetTSLastSeen() > OFFLINE_INTERVAL))
		{
			strcpy(buf, "#f00000 ");
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
void Scr_Peer_pressed(lv_event_t * e)
{
	lv_obj_clear_flag(objects.btn_peer_prev, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.btn_peer_next, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.btn_peer_up,   LV_OBJ_FLAG_HIDDEN);
	TSLastClickPeer = millis();
}
void Btn_Peer_Next_clicked(lv_event_t * e)
{
	ActivePeer = FindNextPeer(ActivePeer, MODULE_ALL, CIRCULAR); 
	if (ActivePeer) Scr_Peer_Timer_update(NULL);
}
void Btn_Peer_Prev_clicked(lv_event_t * e)
{
	ActivePeer = FindPrevPeer(ActivePeer, MODULE_ALL, CIRCULAR); 
	if (ActivePeer) Scr_Peer_Timer_update(NULL);
}
void Btn_Peer_Up_clicked(lv_event_t * e)
{
	lv_scr_load_anim(objects.scr_settings, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Peer_System_clicked(lv_event_t * e)
{
    //if (ActivePeer) lv_scr_load_anim(objects.scr_system, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Peer_Volt_clicked(lv_event_t * e)
{
    if (ActivePeer) lv_scr_load_anim(objects.scr_volt, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Peer_Eichen_clicked(lv_event_t * e)
{
    if (ActivePeer) lv_scr_load_anim(objects.scr_eichen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Peer_Restart_clicked(lv_event_t * e)
{
    if (ActivePeer) SendCommand(ActivePeer, SEND_CMD_RESTART);
}
void Btn_Peer_Sleep_clicked(lv_event_t * e)
{
    if (ActivePeer) SendCommand(ActivePeer, SEND_CMD_SLEEPMODE_TOGGLE);
}
void Btn_Peer_Pair_clicked(lv_event_t * e)
{
    if (ActivePeer) SendCommand(ActivePeer, SEND_CMD_PAIRMODE_TOGGLE);
}
void Btn_Peer_Reset_clicked(lv_event_t * e)
{
    if (ActivePeer) SendCommand(ActivePeer, SEND_CMD_RESET);
}
void Btn_Peer_Delete_clicked(lv_event_t * e)
{
    if (ActivePeer) DeletePeer(ActivePeer);
    ActivePeer = NULL;
    lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Scr_Peer_Gesture(lv_event_t * e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	lv_obj_t * target = lv_event_get_target(e);  
	
	if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Peer_Next_clicked(e);
    }
    else if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Peer_Prev_clicked(e);
	}
	else if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Peer_Up_clicked(e);
	}
}

#pragma endregion Scr_Peer
#pragma region Scr_Eichen
void Scr_Eichen_loaded(lv_event_t * e)
{
	if (ActivePeer) lv_label_set_text(objects.lbl_eichen_scrname, ActivePeer->GetName());
	else lv_scr_load_anim(objects.scr_peer, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Scr_Eichen_unloaded(lv_event_t * e)
{
}
void Scr_Eichen_Gesture(lv_event_t * e)
{
	lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Eichen_Eichen_clicked(lv_event_t * e)
{
    if (ActivePeer) 
    {
        CalibAmp();
        lv_scr_load_anim(objects.scr_peer, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
    }
}
#pragma endregion Scr_Eichen
#pragma region Scr_Volt
void Scr_Volt_loaded(lv_event_t * e)
{
    if (ActivePeer) lv_label_set_text(objects.lbl_volt_scrname, "12.00 V");
}
void Arc_Volt_change(lv_event_t * e)
{
    lv_label_set_text_fmt(objects.lbl_volt_volt_value, "%.2f V", lv_arc_get_value(objects.arc_volt));
}
void Btn_Volt_Save_clicked(lv_event_t * e)
{
    if (ActivePeer) {
        float volt = lv_arc_get_value(objects.arc_volt);
        CalibVolt(volt);
    }
    lv_scr_load_anim(objects.scr_peer, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Scr_Volt_unloaded(lv_event_t * e)
{
}
void Scr_Volt_Gesture(lv_event_t * e)
{
	lv_scr_load_anim(objects.scr_peer, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
#pragma endregion Scr_Volt
#pragma region Scr_Settings
lv_timer_t *Timer_Scr_Settings = NULL;
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
void Scr_Settings_loaded(lv_event_t * e)
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
void Scr_Settings_unloaded(lv_event_t * e)
{
    if (Timer_Scr_Settings) lv_timer_del(Timer_Scr_Settings);
	Timer_Scr_Settings = NULL;
}   
void Btn_Settings_Restart_clicked(lv_event_t * e)
{
	ESP.restart();
}	
void Btn_Settings_Reset_clicked(lv_event_t * e)
{
}	
void Btn_Settings_Peers_clicked(lv_event_t * e)
{
	 lv_scr_load_anim(objects.scr_peer, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}	
void Btn_Settings_Pair_clicked(lv_event_t * e)
{
	TogglePairMode();
}	
void Btn_Settings_WebSvr_clicked(lv_event_t * e)
{
	ToggleWebServer();
}	
void Btn_Settings_Save_clicked(lv_event_t * e)
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
void Scr_Settings_Gesture(lv_event_t * e)
{
	lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
#pragma endregion Scr_Settings
#pragma region Scr_Multi
lv_timer_t *Timer_Scr_Multi = NULL;
uint32_t TSLastClickMulti = -1;

void Scr_Multi_Timer_update(lv_timer_t *timer)
{
    for (int Pos=0; Pos<PERIPH_PER_SCREEN; Pos++) 
		if (Screen[ActiveMultiScreen].GetPeriphId(Pos) >= 0) CompThingArray[Pos]->Update();
	
	if ((TSLastClickMulti != -1) && (millis() - TSLastClickMulti > 1000))
	{
		lv_obj_add_flag(objects.btn_multi_prev, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objects.btn_multi_next, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(objects.btn_multi_up,   LV_OBJ_FLAG_HIDDEN);
		TSLastClickMulti = -1;
	}	
	if (TSLastClickMulti != -1) DEBUG3("millis() - TSLastClickMulti = %u\n\r", millis() - TSLastClickMulti);
}
void Scr_Multi_pressed(lv_event_t * e)
{
	DEBUG3("Scr_Multi_pressed fires\n\r");
	TSLastClickMulti = millis();
	lv_obj_clear_flag(objects.btn_multi_prev, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.btn_multi_next, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(objects.btn_multi_up,   LV_OBJ_FLAG_HIDDEN);

}
void Scr_Multi_ClearScreen(lv_event_t * e)
{
	for (int Pos = 0; Pos<PERIPH_PER_SCREEN; Pos++)
	{
		if (CompThingArray[Pos])
		{
			delete CompThingArray[Pos];
			CompThingArray[Pos] = NULL;
		}
	}
	lv_obj_invalidate(lv_scr_act());
}
void Scr_Multi_FillScreen(lv_event_t * e)
{
	lv_label_set_text(objects.lbl_multi_scrname, Screen[ActiveMultiScreen].GetName());
					
	for (int Pos=0; Pos<PERIPH_PER_SCREEN; Pos++)
	{
		int x; int y;
		
		switch (Pos) {
			case 0: x= (int)-SCREEN_RES_HOR/5; y=(int)-SCREEN_RES_VER/4.7; break;
			case 1: x= (int) SCREEN_RES_HOR/5; y=(int)-SCREEN_RES_VER/4.7; break;
			case 2: x= (int)-SCREEN_RES_HOR/5; y=(int) SCREEN_RES_VER/5.3; break;
			case 3: x= (int) SCREEN_RES_HOR/5; y=(int) SCREEN_RES_VER/5.3; break;
		}

		PeriphClass *Periph =  Screen[ActiveMultiScreen].GetPeriph(Pos);
		if (Periph)
		{
			DEBUG3("Pos: %u, PeriphName:%s\n\r", Pos, Periph->GetName());
		
			lv_obj_add_flag(lv_obj_get_child(objects.cnt_multi, Pos), LV_OBJ_FLAG_HIDDEN);
			
			if (CompThingArray[Pos]) 
			{
				delete CompThingArray[Pos];
				CompThingArray[Pos] = NULL;
			}

			if (Screen[ActiveMultiScreen].GetPrimToShow(Pos) == PRIM_SWITCH)
			{
				DEBUG3("erstelle: Setup x:%u, y:%u, Pos:%u\n\r", x, y, Pos);
				CompThingArray[Pos] = new CompButton;
				CompThingArray[Pos]->Setup(objects.scr_multi, x, y, Pos, 1, Periph, Scr_Multi_clicked);
				CompThingArray[Pos]->Update();
			}
			else if (Screen[ActiveMultiScreen].GetPrimToShow(Pos) == PRIM_VOLT or
			         Screen[ActiveMultiScreen].GetPrimToShow(Pos) == PRIM_AMP or
				     Screen[ActiveMultiScreen].GetPrimToShow(Pos) == PRIM_WATT)
			{	
				CompThingArray[Pos] = new CompSensor;
				CompThingArray[Pos]->Setup(objects.scr_multi, x, y, Pos, 1, Periph, Scr_Multi_clicked);
				CompThingArray[Pos]->Update();
			}
		}
		else
		{
			lv_obj_clear_flag(lv_obj_get_child(objects.cnt_multi, Pos), LV_OBJ_FLAG_HIDDEN);
			CompThingArray[Pos] = NULL;
			DEBUG3("Pos: %u, no Periph\n\r", Pos);
		
		}
	}
}
void Scr_Multi_loaded(lv_event_t * e)
{
	static uint32_t user_data = 10;
	
	Scr_Multi_FillScreen(e);
	
	if (Timer_Scr_Multi) 
	{
		lv_timer_resume(Timer_Scr_Multi);
	}
	else 
	{
		Timer_Scr_Multi = lv_timer_create(Scr_Multi_Timer_update, 100,  &user_data);
	}
	ActiveScreen = &Screen[ActiveMultiScreen];
	TSLastClickMulti = millis();
}
void Scr_Multi_unloaded(lv_event_t * e)
{
	Scr_Multi_ClearScreen(e);

	if (Timer_Scr_Multi) lv_timer_del(Timer_Scr_Multi);
	Timer_Scr_Multi = NULL;

}
void Btn_Multi_Next_clicked(lv_event_t * e)
{
	Scr_Multi_ClearScreen(e);
	ActiveMultiScreen++;
	if (ActiveMultiScreen == MULTI_SCREENS) ActiveMultiScreen = 0;
	Scr_Multi_FillScreen(e);
}
void Btn_Multi_Prev_clicked(lv_event_t * e)
{
	Scr_Multi_ClearScreen(e);
	ActiveMultiScreen--;
	if (ActiveMultiScreen == -1) ActiveMultiScreen = MULTI_SCREENS-1;
	Scr_Multi_FillScreen(e);
}
void Btn_Multi_Up_clicked(lv_event_t * e)
{
	lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
}
void Btn_Multi_Change_Pos(int Pos)
{	
	if (PeriphList.size() > 0) 
	{
		MultiPosToChange = Pos;
		ActiveScreen = &Screen[ActiveMultiScreen];
		lv_scr_load_anim(objects.scr_periphchoice, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
	}
}
void Btn_Multi1_clicked(lv_event_t * e) { Btn_Multi_Change_Pos(0); }
void Btn_Multi2_clicked(lv_event_t * e) { Btn_Multi_Change_Pos(1); }
void Btn_Multi3_clicked(lv_event_t * e) { Btn_Multi_Change_Pos(2); }
void Btn_Multi4_clicked(lv_event_t * e) { Btn_Multi_Change_Pos(3); }	
void Scr_Multi_Gesture(lv_event_t * e)
{
	lv_event_code_t event_code = lv_event_get_code(e);
	lv_obj_t * target = lv_event_get_target(e);  
	
	if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_LEFT) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Multi_Next_clicked(e);
    }
    else if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT) {
        lv_indev_wait_release(lv_indev_get_act());
        Btn_Multi_Prev_clicked(e);
	}
	else if (event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_TOP) {
        lv_indev_wait_release(lv_indev_get_act());
        lv_scr_load_anim(objects.scr_start, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
	}
}
void Scr_Multi_clicked(lv_event_t * e) //Handler für picoButtons und picoMeter
{
	lv_event_code_t event_code = lv_event_get_code(e);
	lv_obj_t * target          = lv_event_get_target(e); 
	CompThing *TargetCompThing = (CompThing *) lv_obj_get_user_data(target);
	PeriphClass *Periph        = NULL;

	if (!TargetCompThing) return;

	Periph = TargetCompThing->GetPeriph();
	MultiPosToChange = TargetCompThing->GetPos();
	DEBUG3("Scr_Multi_clicked: Periph=%s, Pos=%d\n\r", Periph->GetName(), MultiPosToChange);
	
	if (event_code == LV_EVENT_LONG_PRESSED)
	{
		DEBUG3("Scr_Multi_longclicked: Periph=%s, Pos=%d\n\r", Periph->GetName(), MultiPosToChange);
		Btn_Multi_Change_Pos(MultiPosToChange);
    }
	else if (event_code == LV_EVENT_CLICKED) 
	{
		if (Screen[ActiveMultiScreen].GetPrimToShow(MultiPosToChange) == PRIM_SWITCH)
		{
			Periph->SetChanged(true);
			
			if (lv_obj_get_state(target) == LV_IMGBTN_STATE_DISABLED) 		  Periph->SetValue(0.0);
			if (lv_obj_get_state(target) == LV_IMGBTN_STATE_CHECKED_RELEASED) Periph->SetValue(1.0);
			
			ToggleSwitch(Periph);
		}
		else 
		{
			ActivePeriphSensor = Periph;
			ActivePeer   = PeerOf(Periph);
			lv_scr_load_anim(objects.scr_meter, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, false);
		}
		
    }	
}
#pragma endregion Scr_Multi