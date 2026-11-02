#include <Arduino.h>
#include "main.h"
#include "CompButton.h"

// Version 4.25

#define VALUE_V_MIN     9
#define VALUE_V_MAX     15
#define VALUE_V_GREEN   13
#define VALUE_V_YELLOW  14.4
#define VALUE_A_MIN     0
#define VALUE_A_MAX     40
#define VALUE_A_GREEN   20
#define VALUE_A_YELLOW  25
#define VALUE_W_MIN     0
#define VALUE_W_MAX     400
#define VALUE_W_GREEN   300
#define VALUE_W_YELLOW  350

extern lv_obj_t *ui_LblMenuBatt;
float testvalue;

CompThing::CompThing()
{
    _PeerVisible      = true;
    _PeriphVisible    = true;
    _ValueVisible     = false;
    _SystemVisible    = false;
    _GraphVisible     = false;
    _PeriphValueCombo = true;
    _dBmVisible       = false;
    _GraphValuePos    = -1;

    _Chart= NULL;
    _Button = NULL;
    _LblPeer = NULL;
    _LblPeriph = NULL;
    _LblPeriphId = NULL;
    _LblPos = NULL;
    _LblValue = NULL;
    _LblCentral = NULL;

    _Periph = NULL;

    _ChartSerie = NULL;
    _event_cb = NULL;
}
CompThing::~CompThing()
{
	Serial.println("CompThing::~CompThing()");
    if (_Chart)       { Serial.println("Delete _Chart"); lv_obj_del(_Chart);     _Chart      = NULL; }
    if (_Button)      { Serial.println("Delete _Button"); lv_obj_del(_Button);    _Button     = NULL; }
}
void CompThing::Update()
{ 	
}
void CompThing::Setup(lv_obj_t * comp_parent, int x, int y, int Pos, int size, PeriphClass *Periph, lv_event_cb_t event_cb)
{
}
void CompThing::ChartInit(int x, int y, int Width, int Height, int Anz)
{    
    _Chart = lv_chart_create(lv_scr_act());
    if (_Periph->GetType() == SENS_TYPE_AMP)  lv_chart_set_range(_Chart,  LV_CHART_AXIS_PRIMARY_Y, 0, 40);
    if (_Periph->GetType() == SENS_TYPE_VOLT) lv_chart_set_range(_Chart,  LV_CHART_AXIS_PRIMARY_Y, 0, 15);
    if (_Periph->IsSwitch())                  lv_chart_set_range(_Chart,  LV_CHART_AXIS_PRIMARY_Y, 0,  1);
    
    _ChartSerie = lv_chart_add_series(_Chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_SECONDARY_Y);

    lv_obj_center(_Chart);
    lv_obj_set_size(_Chart, Width, Height);
    lv_obj_set_pos(_Chart, x, y);
    lv_obj_set_style_size(_Chart, 0, LV_PART_INDICATOR);

    lv_chart_set_type(_Chart, LV_CHART_TYPE_LINE); 
    lv_chart_set_point_count(_Chart, Anz);
    lv_chart_set_div_line_count(_Chart, 0, 0);

    lv_obj_set_style_bg_opa(_Chart, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(_Chart, 0, LV_PART_MAIN);

    //lv_obj_set_style_bg_opa(_Chart, LV_OPA_0, 0);
}
void CompThing::ChartUpdate()
{
    int ValueIndex = _Periph->GetSavedValueIndex();
    
    DEBUG3 ("GraphUpdate: GraphIndex: %d\n\r", ValueIndex);
    for (int ChartIndex=RECORDED_VALUES-1; ChartIndex>=0; ChartIndex--)
    {
        //_Periph->AddSavedValue((float) random(0,1), (float) random(0,1), (float) random(0,15), (float) random(0,35));

        //DEBUG3 ("Graph[%d].value[%d][%d] = %.2f\n\r", _Periph->GetPos(), GraphIndex, _GraphValuePos, Graph[_Periph->GetPos()].Value[GraphIndex][_GraphValuePos]);
        //lv_chart_set_next_value(_Chart, _ChartSerie, _Periph->GetSavedValue(GraphIndex, _GraphValuePos));
        _ChartSerie->y_points[ChartIndex] = _Periph->GetSavedValue(ValueIndex, _GraphValuePos);
        ValueIndex++;
        if (ValueIndex == RECORDED_VALUES) ValueIndex = 0;
    }
    lv_chart_refresh(_Chart); 
}
void CompThing::SetStyle(lv_obj_t *obj)
{
    ui_object_set_themeable_style_property(obj, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_TEXT_COLOR, _ui_theme_color_BtnTxt);
    ui_object_set_themeable_style_property(obj, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_TEXT_OPA, _ui_theme_alpha_BtnTxt);
    lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_object_set_themeable_style_property(obj, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR, _ui_theme_color_BtnBg);
    ui_object_set_themeable_style_property(obj, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_OPA, _ui_theme_alpha_BtnBg);
    ui_object_set_themeable_style_property(obj, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BORDER_COLOR, _ui_theme_color_BtnBorder);
    ui_object_set_themeable_style_property(obj, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BORDER_OPA, _ui_theme_alpha_BtnBorder);
    lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(obj, LV_ALIGN_CENTER);	
}

CompButton::CompButton()
{
    _ClassType = 1;
	_SpinnerVisible = false;
    _GraphVisible = false;
}
CompButton::~CompButton()
{
    Serial.println("CompButton::~CompButton()");

    lv_obj_remove_event_cb(_Button, _event_cb);
    
	lv_obj_invalidate(_LblPeer);
    lv_obj_invalidate(_LblPeriph);
    lv_obj_invalidate(_LblValue);
	
    if (_Spinner) { lv_obj_del(_Spinner); _Spinner = NULL; }
    #ifdef BATTERY_PORT
		lv_obj_clear_flag(ui_LblMenuBatt, LV_OBJ_FLAG_HIDDEN);
	#endif
    //if (_Button)     { Serial.println("~CompButton: Delete _Button"); lv_obj_del(_Button);    _Button     = NULL; }
}
void CompButton::Setup(lv_obj_t * comp_parent, int x, int y, int Pos, int size, PeriphClass *Periph, lv_event_cb_t event_cb)
{
    // Size 1: klein, 2: groß, 
    _Periph = Periph;
    _event_cb = event_cb;
    _Pos = Pos;
    _Size = size;
    _GraphValuePos = V_ON;
   
    _x = x;
    _y = y;			
	
    switch (size)
    {
        case 1:
            _Width  = WIDTH_BTN1;
            _Height = HEIGHT_BTN1;
            _PeerVisible = false;
            _PeriphVisible = true;
            _ValueVisible = true;
            _PeriphValueCombo = true;
            break;
        case 2:
            _Width  = WIDTH_BTN2;
            _Height = HEIGHT_BTN2;
            _PeerVisible = true;
            _PeriphVisible = true;
            _ValueVisible = false;
            _PeriphValueCombo = true;

            #ifdef BATTERY_PORT
		        lv_obj_add_flag(ui_LblMenuBatt, LV_OBJ_FLAG_HIDDEN);
	        #endif
            break; 
    }     

    _Spinner = lv_spinner_create(comp_parent, 1000, 90);
    
    lv_obj_set_align(_Spinner, LV_ALIGN_CENTER);
    lv_obj_set_width(_Spinner,  _Width+SCREEN_RES_HOR/12);
    lv_obj_set_height(_Spinner, _Width+SCREEN_RES_VER/12);
	
    lv_obj_set_pos(_Spinner, _x, _y);
    lv_obj_add_flag(_Spinner, LV_OBJ_FLAG_HIDDEN);     
    lv_obj_clear_flag(_Spinner, LV_OBJ_FLAG_CLICKABLE);     
    lv_obj_set_style_arc_color(_Spinner, lv_color_hex(0x83061F), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(_Spinner, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(_Spinner, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(_Spinner, 5, LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_set_style_arc_color(_Spinner, lv_color_hex(0x31020B), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(_Spinner, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(_Spinner, 5, LV_PART_INDICATOR | LV_STATE_CHECKED);
	
    _Button = lv_imgbtn_create(comp_parent);
    if (size == 1)
    {
        lv_imgbtn_set_src(_Button, LV_IMGBTN_STATE_RELEASED, NULL, IMG_BTN1_RELEASED, NULL);
        lv_imgbtn_set_src(_Button, LV_IMGBTN_STATE_PRESSED,  NULL, IMG_BTN1_PRESSED,  NULL);
        lv_imgbtn_set_src(_Button, LV_IMGBTN_STATE_CHECKED_PRESSED,  NULL, IMG_BTN1_CHK_PRESSED,  NULL);
        lv_imgbtn_set_src(_Button, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, IMG_BTN1_CHK_RELEASED, NULL);
    }
    if (size == 2)
    {
	    lv_imgbtn_set_src(_Button, LV_IMGBTN_STATE_RELEASED, NULL, IMG_BTN2_RELEASED, NULL);
        lv_imgbtn_set_src(_Button, LV_IMGBTN_STATE_PRESSED,  NULL, IMG_BTN2_PRESSED,  NULL);
        lv_imgbtn_set_src(_Button, LV_IMGBTN_STATE_CHECKED_PRESSED,  NULL, IMG_BTN2_CHK_PRESSED,  NULL);
        lv_imgbtn_set_src(_Button, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, IMG_BTN2_CHK_RELEASED, NULL);
    }

    //set Switch-state
    if (Periph->GetValue() == 0) lv_imgbtn_set_state(_Button, LV_IMGBTN_STATE_RELEASED);
    else                         lv_imgbtn_set_state(_Button, LV_IMGBTN_STATE_CHECKED_RELEASED);
    
    lv_obj_set_align(_Button, LV_ALIGN_CENTER);
    lv_obj_set_height(_Button, _Height);
    lv_obj_set_width (_Button, _Width);   /// 1
    lv_obj_set_pos(_Button, _x, _y);
        
    lv_obj_add_flag(_Button, LV_OBJ_FLAG_CHECKABLE);     
    lv_obj_clear_flag(_Button, LV_OBJ_FLAG_SCROLLABLE);        
    lv_obj_add_flag(_Button, LV_OBJ_FLAG_OVERFLOW_VISIBLE);  

	_LblPeer = lv_label_create(_Button);
	SetStyle(_LblPeer);
    lv_obj_add_flag(_LblPeer, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_width (_LblPeer, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblPeer, LV_SIZE_CONTENT);    /// 1
	
	switch (size)
    {
        case 1:
            SetPeerPos(-SCREEN_RES_HOR/7, -5);
            lv_obj_set_style_text_font(_LblPeer, MY_FONT2, LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        case 2:
            SetPeerPos(0, lv_pct(58));
            lv_obj_set_style_text_font(_LblPeer, MY_FONT3, LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
	}
    
	_LblPeriph = lv_label_create(_Button);
	SetStyle(_LblPeriph);
    lv_obj_add_flag(_LblPeriph, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(_LblPeriph, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblPeriph, LV_SIZE_CONTENT);    /// 1
    
    switch (size)
    {
        case 1:
            SetPeriphPos(0, lv_pct(-70));
            lv_obj_set_style_text_font(_LblPeriph, MY_FONT2, LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        case 2:
            SetPeriphPos(0, lv_pct(-70));
            lv_obj_set_style_text_font(_LblPeriph, MY_FONT3, LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
	}

    _LblValue = lv_label_create(_Button);
    SetStyle(_LblValue);
    lv_obj_add_flag(_LblValue, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(_LblValue, LV_SIZE_CONTENT);
    lv_obj_set_height(_LblValue, LV_SIZE_CONTENT);  
	
	switch (size)
    {
        case 1:
            SetValuePos(50, lv_pct(20));
            lv_obj_set_style_text_font(_LblValue, MY_FONT2, LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
	    case 2:
            SetValuePos(lv_pct(20), lv_pct(30));
		    lv_obj_set_style_text_font(_LblValue, MY_FONT3, LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
    }

	lv_label_set_text(_LblValue, "--.- A");

    _LblPeriphId = lv_label_create(_Button);
    if (!_SystemVisible) lv_obj_add_flag(_LblPeriphId, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(_LblPeriphId, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblPeriphId, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_pos(_LblPeriphId, 50, 0);
    lv_obj_set_align(_LblPeriphId, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(_LblPeriphId, "%d", _Periph->GetId());
    lv_obj_set_style_text_color(_LblPeriphId, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    _LblPos = lv_label_create(_Button);
    if (!_SystemVisible) lv_obj_add_flag(_LblPos, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(_LblPos, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblPos, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(_LblPos, LV_ALIGN_CENTER);
    lv_obj_set_pos(_LblPos, 50, -15);
    lv_label_set_text_fmt(_LblPos, "%d", _Pos);
    lv_obj_set_style_text_color(_LblPos, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    lv_obj_add_event_cb(_Button, _event_cb, LV_EVENT_ALL, NULL);  	
}
void CompButton::Update()
{
    lv_obj_set_pos(_Spinner, _x, _y);
    lv_obj_set_pos(_Button,  _x, _y);

    //set Switch-state
    if (_Periph->GetValue(0) == 0) 
    {
        lv_imgbtn_set_state(_Button, LV_IMGBTN_STATE_RELEASED);
    }
    else 
    {			  
        lv_imgbtn_set_state(_Button, LV_IMGBTN_STATE_CHECKED_RELEASED);
    }

    // show Spinner if Value is in changed state
    if (_Periph->GetChanged()) ShowSpinner();
    else HideSpinner();
    
	// - show Peer if named and visible
	if ((PeerOf(_Periph)->GetName() == NULL) or (!_PeerVisible))
	{
	    lv_obj_add_flag(_LblPeer, LV_OBJ_FLAG_HIDDEN);
	}
	else
	{
	    lv_label_set_text_fmt(_LblPeer, "%.6s", PeerOf(_Periph)->GetName());
     	lv_obj_clear_flag(_LblPeer, LV_OBJ_FLAG_HIDDEN);
	}

	char Buf_Periph[20] = {};
	char Buf_Value[10] = {};

	// fill Buf_Value if switch is on
	sprintf(Buf_Periph, "%.6s", _Periph->GetName());
	if ((_ValueVisible) and (_Periph->IsCombo()) and (_Periph->GetValue(0) == 1))
	{
		if (_PeriphValueCombo) 
        {
            sprintf(Buf_Value, " %0.1fA", _Periph->GetValue(3)); // amp-wert
        }
        else    
        {
            sprintf(Buf_Value, " %0.1fA", _Periph->GetValue(3)); // amp-wert
            lv_obj_clear_flag(_LblValue, LV_OBJ_FLAG_HIDDEN);
        }
	}

	// add Value to periph if desired, hide value then
	if (_PeriphValueCombo)
	{	
		strcat(Buf_Periph, Buf_Value);
		lv_obj_add_flag(_LblValue, LV_OBJ_FLAG_HIDDEN);
	}

	// Show Periph (and evtl value) if visible
    if ((_Periph->GetName() == NULL) or (!_PeriphVisible)) 
    {
        lv_obj_add_flag(_LblPeriph, LV_OBJ_FLAG_HIDDEN);
    }
    else 
    {
        if ((_Size == 1) or (_Size == 3))
        {
            if (_Periph->GetValue(0) == 1) SetPeriphPos(0,  lv_pct(33));
            else                           SetPeriphPos(0,  lv_pct(-33));
        }

        lv_label_set_text(_LblPeriph, Buf_Periph);
        lv_obj_clear_flag(_LblPeriph, LV_OBJ_FLAG_HIDDEN);
    
    }

    // show value only if visible, not in periph, switch is on and brother is existent
    if ((_ValueVisible) and (_Periph->IsCombo()) and (!_PeriphValueCombo) and (_Periph->GetValue(0) == 1))
    {
        lv_label_set_text(_LblValue, Buf_Value);
        lv_obj_clear_flag(_LblValue, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(_LblValue, LV_OBJ_FLAG_HIDDEN);
    }  
}

CompSensor::CompSensor()
{
    _ClassType = 2;
    _GraphVisible = false;
}
CompSensor::~CompSensor()
{
    lv_obj_remove_event_cb(_Button, _event_cb);
    //if (_Arc)     { Serial.println("~CompSensor: Delete _Arc"); lv_obj_del(_Arc);     _Arc      = NULL; }
}
void CompSensor::Setup(lv_obj_t * comp_parent, int x, int y, int Pos, int size, PeriphClass *Periph, lv_event_cb_t event_cb)
{
    // Size 1: klein, 2: groß
    _Periph = Periph;
    _event_cb = event_cb;
    _Pos = Pos;
    if (Periph->GetType() == SENS_TYPE_VOLT) _GraphValuePos = V_VOLT;
    if (Periph->GetType() == SENS_TYPE_AMP)  _GraphValuePos = V_AMP;
    _x = x;
    _y = y;		

    _Width  = SCREEN_RES_HOR * 0.28;
    _Height = SCREEN_RES_VER * 0.28;
    
    _PeerVisible = true;
    _PeriphVisible = true;
    _ValueVisible = true;
    _PeriphValueCombo = false;
    _SystemVisible = false;
    
    _Button = lv_btn_create(comp_parent);
    lv_obj_set_width(_Button, _Width);
    lv_obj_set_height(_Button, _Height);
    lv_obj_set_pos(_Button, _x, _y);
    lv_obj_set_align(_Button, LV_ALIGN_CENTER);
    lv_obj_add_flag(_Button, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags
    lv_obj_add_flag(_Button, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_clear_flag(_Button, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_radius(_Button, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(_Button, lv_color_hex(0x165420), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_Button, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(_Button, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(_Button, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    _LblPeer = lv_label_create(_Button);
    lv_obj_set_width(_LblPeer, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblPeer, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(_LblPeer, LV_ALIGN_CENTER);
    SetStyle(_LblPeer);
    SetPeerPos(0,lv_pct(75));

    lv_obj_set_style_text_font(_LblPeer, MY_FONT2, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    _LblPeriph = lv_label_create(_Button);
    lv_obj_set_width(_LblPeriph, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblPeriph, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(_LblPeriph, LV_ALIGN_CENTER);
    
    lv_obj_set_style_text_font(_LblPeriph, MY_FONT2, LV_PART_MAIN | LV_STATE_DEFAULT);
    SetPeriphPos(0, -SCREEN_RES_VER * 0.06);
    
    _LblValue = lv_label_create(_Button);
    lv_obj_set_width(_LblValue, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblValue, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(_LblValue, LV_ALIGN_CENTER);
    
    lv_obj_set_style_text_font(_LblValue, MY_FONT3, LV_PART_MAIN | LV_STATE_DEFAULT);
    SetValuePos(0, 5);
    
    _LblPeriphId = lv_label_create(_Button);
    lv_obj_set_width(_LblPeriphId, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblPeriphId, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_pos(_LblPeriphId, 80, 0);
    lv_obj_set_align(_LblPeriphId, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(_LblPeriphId, "%d", _Periph->GetId());
    lv_obj_set_style_text_color(_LblPeriphId, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    if (!_SystemVisible) lv_obj_add_flag(_LblPeriphId, LV_OBJ_FLAG_HIDDEN);

    _LblPos = lv_label_create(_Button);
    lv_obj_set_width(_LblPos, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblPos, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_pos(_LblPos, 80, 18);
    lv_obj_set_align(_LblPos, LV_ALIGN_CENTER);
    lv_label_set_text_fmt(_LblPos, "%d", _Pos);
    lv_obj_set_style_text_color(_LblPos, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(_LblPos, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    if (!_SystemVisible) lv_obj_add_flag(_LblPos, LV_OBJ_FLAG_HIDDEN);

    _Arc = lv_arc_create(_Button);
    lv_obj_set_width(_Arc, _Width);
    lv_obj_set_height(_Arc, _Height);
    lv_obj_set_align(_Arc, LV_ALIGN_CENTER);
    lv_obj_clear_flag(_Arc, LV_OBJ_FLAG_CLICKABLE);      /// Flags
    lv_arc_set_value(_Arc, 0);
    lv_obj_set_style_arc_color(_Arc, lv_color_hex(0x4A4A4A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(_Arc, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(_Arc, 6, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(_Arc, lv_color_hex(0x398F30), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(_Arc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(_Arc, 6, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(_Arc, lv_color_hex(0xDBDBDB), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(_Arc, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(_Arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(_Arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(_Arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(_Arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    if (_GraphVisible) ChartInit(x, y+_Height*0.25, _Width*0.7, _Height*0.3, RECORDED_VALUES);
        
    lv_obj_add_event_cb(_Button, _event_cb, LV_EVENT_ALL, NULL);  
}
void CompSensor::Update()
{
    lv_obj_set_pos(_Button, _x, _y);
    
    //testvalue++;
    //if (testvalue > 20) testvalue = 0;

	if ((PeerOf(_Periph)->GetName() == NULL) or (!_PeerVisible))
	{
	    lv_obj_add_flag(_LblPeer, LV_OBJ_FLAG_HIDDEN);
	}
	else
	{
	    lv_label_set_text_fmt(_LblPeer, "%.8s", PeerOf(_Periph)->GetName());
     	lv_obj_clear_flag(_LblPeer, LV_OBJ_FLAG_HIDDEN);
	}

    if ((_Periph->GetName() == NULL) or (!_PeriphVisible)) 
    {
	lv_obj_add_flag(_LblPeriph, LV_OBJ_FLAG_HIDDEN);
    }
    else 
    {
	lv_label_set_text_fmt(_LblPeriph, "%.8s", _Periph->GetName());
	lv_obj_clear_flag(_LblPeriph, LV_OBJ_FLAG_HIDDEN);
    
    }

    if (_ValueVisible) 
    {
        char buf[10] = {};
        int nk = 0;
        lv_color_t bg;
        float value;

        if (Screen[ActiveMultiScreen].GetPrimToShow(_Pos) == PRIM_AMP)
        {
            value = _Periph->GetValue(V_AMP);
            //value = testvalue; // for testing only

            if      (value<10)  nk = 2;
            else if (value<100) nk = 1;
            else                nk = 0;

            if (value == -99) strcpy(buf, "--"); 
            else dtostrf(value, 0, nk, buf);
            strcat(buf, " A");
            
            if      (value < VALUE_A_GREEN)  bg = lv_color_hex(0x135A25);
            else if (value < VALUE_A_YELLOW) bg = lv_color_hex(0x7C7E26);
            else 		                     bg = lv_color_hex(0x88182C);
            lv_obj_set_style_bg_color(_Button, bg, LV_PART_MAIN | LV_STATE_DEFAULT);
            
            lv_arc_set_range(_Arc, VALUE_A_MIN*10, VALUE_A_MAX*10);
            lv_arc_set_value(_Arc, value*10);
        }
        else if (Screen[ActiveMultiScreen].GetPrimToShow(_Pos) == PRIM_VOLT)
        {
            float value = _Periph->GetValue(V_VOLT);
        
            if      (value<10)  nk = 2;
            else if (value<100) nk = 1;
            else                nk = 0;

            if (value == -99) strcpy(buf, "--"); 
            else dtostrf(value, 0, nk, buf);
            strcat(buf, " V");
                        
            if 	    (value < VALUE_V_GREEN)  bg = lv_color_hex(0x135A25);
            else if (value < VALUE_V_YELLOW) bg = lv_color_hex(0x7C7E26);
            else 	 		                 bg = lv_color_hex(0x88182C);
            lv_obj_set_style_bg_color(_Button, bg, LV_PART_MAIN | LV_STATE_DEFAULT);
            
            lv_arc_set_range(_Arc, VALUE_V_MIN*10, VALUE_V_MAX*10);
            lv_arc_set_value(_Arc, value*10);
        }
        else if (Screen[ActiveMultiScreen].GetPrimToShow(_Pos) == PRIM_WATT)
        {
            float value = _Periph->GetValue(V_VOLT) * _Periph->GetValue(V_AMP);
        
            if      (value<10)  nk = 2;
            else if (value<100) nk = 1;
            else                nk = 0;

            if (value == -99) strcpy(buf, "--"); 
            else dtostrf(value, 0, nk, buf);
            strcat(buf, " W");
                        
            if 	    (value < VALUE_W_GREEN)  bg = lv_color_hex(0x135A25);
            else if (value < VALUE_W_YELLOW) bg = lv_color_hex(0x7C7E26);
            else 	 		                 bg = lv_color_hex(0x88182C);
            lv_obj_set_style_bg_color(_Button, bg, LV_PART_MAIN | LV_STATE_DEFAULT);
            
            lv_arc_set_range(_Arc, VALUE_W_MIN*10, VALUE_W_MAX*10);
            lv_arc_set_value(_Arc, value*10);
        }
            
        lv_label_set_text(_LblValue, buf);
        lv_obj_clear_flag(_LblValue, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(_LblValue, LV_OBJ_FLAG_HIDDEN);
    }  
    if (_GraphVisible) ChartUpdate();
}

CompMeter::CompMeter() 
{
    _PeerVisible = true;
    _PeriphVisible = true;
    _ValueVisible = true;
    _SystemVisible = false;
    _PeriphValueCombo = false;
    _GraphVisible = false;
    _dBmVisible = true;
}
CompMeter::~CompMeter() 
{
	lv_obj_del(ui_ImgMeter2);
	lv_obj_del(ui_ImgZeiger);
    lv_obj_del(_LblPeer);
    lv_obj_del(_LblPeriph); 
    lv_obj_del(_LblValue);
    lv_obj_del(_LblCentral);
}
void CompMeter::Setup(lv_obj_t * comp_parent, int x, int y, int Pos, int size, PeriphClass *Periph, lv_event_cb_t event_cb) 
{
	_Periph = Periph;
    _event_cb = event_cb;
    _Pos = Pos;
    _x = x;
    _y = y;		
    _Width  = size;
    _Height = size;
    _Size = size;	
    _X_Value =  20;
    _Y_Value =  0;
    uint16_t newSize = 256*SCREEN_RES_HOR/360; //+5
    
    ui_ImgMeter2 = lv_img_create(comp_parent);
    if (_Periph->GetPrimary() == PRIM_VOLT)
    { 
        lv_img_set_src(ui_ImgMeter2, &ui_img_voltmeter_360_png);
    }
    else if (_Periph->GetPrimary() == PRIM_AMP)
    { 
        lv_img_set_src(ui_ImgMeter2, &ui_img_ampmeter_360_png);
    }
    else if (_Periph->GetPrimary() == PRIM_WATT)
    { 
        lv_img_set_src(ui_ImgMeter2, &ui_img_wattmeter_360_png);
    }
    lv_img_set_zoom(ui_ImgMeter2,     newSize);
    lv_obj_set_width(ui_ImgMeter2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_ImgMeter2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_ImgMeter2, LV_ALIGN_CENTER);
    lv_obj_set_x(ui_ImgMeter2, lv_pct(1));
    lv_obj_add_flag(ui_ImgMeter2, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_ImgMeter2, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_CHAIN);     /// Flags
    lv_obj_move_background(ui_ImgMeter2);
          
    ui_ImgZeiger = lv_img_create(comp_parent);
    lv_img_set_src(ui_ImgZeiger, &ui_img_zeiger_png);
    newSize = 256*SCREEN_RES_HOR/360;
    lv_img_set_zoom(ui_ImgZeiger,     newSize);
    lv_obj_set_width(ui_ImgZeiger, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_ImgZeiger, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_ImgZeiger, LV_ALIGN_CENTER);
    lv_obj_set_x(ui_ImgZeiger, 0);
    
    int pivotY;

    switch (SCREEN_RES_VER)
    {
        case 240:
            lv_obj_set_y(ui_ImgZeiger, lv_pct(-17)); 
            break;
        case 360:
            lv_obj_set_y(ui_ImgZeiger, lv_pct(-4));    
            break;
        case 466:
            lv_obj_set_y(ui_ImgZeiger, lv_pct(2));    
            break;
        default:
            lv_obj_set_y(ui_ImgZeiger, lv_pct(-4));    
            break;
    }   

    lv_img_set_pivot(ui_ImgZeiger, 5, 150); 
    lv_obj_add_flag(ui_ImgZeiger, LV_OBJ_FLAG_ADV_HITTEST);     /// Flags
    lv_obj_clear_flag(ui_ImgZeiger, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_img_set_angle(ui_ImgZeiger, 0);
	
	if (_Periph->GetPrimary() == PRIM_VOLT)
	{	
        if (_GraphVisible) ChartInit(SCREEN_RES_HOR/2, SCREEN_RES_VER*0.7, SCREEN_RES_HOR/3, SCREEN_RES_VER/15, RECORDED_VALUES);
    }

    _LblPeer = lv_label_create(comp_parent);
    lv_obj_set_width(_LblPeer, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblPeer, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(_LblPeer, LV_ALIGN_CENTER);
    SetPeerPos(0, lv_pct(-35));
    lv_obj_set_style_text_font(_LblPeer, MY_FONT2, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_object_set_themeable_style_property(_LblPeer, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_TEXT_COLOR, _ui_theme_color_BtnTxtInaktive);
    //ui_object_set_themeable_style_property(_LblPeer, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_TEXT_OPA, _ui_theme_alpha_BtnTxt);

    _LblPeriph = lv_label_create(comp_parent);
    lv_obj_set_width(_LblPeriph, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblPeriph, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(_LblPeriph, LV_ALIGN_CENTER);
    SetPeriphPos(0, lv_pct(7));
    SetStyle(_LblPeriph);   
    lv_obj_set_style_text_font(_LblPeriph, MY_FONT3, LV_PART_MAIN | LV_STATE_DEFAULT);
    //ui_object_set_themeable_style_property(_LblPeriph, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_TEXT_COLOR, _ui_theme_color_BtnTxtInaktive);
    //ui_object_set_themeable_style_property(_LblPeriph, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_TEXT_OPA, _ui_theme_alpha_BtnTxt);
    
    _LblCentral = lv_label_create(comp_parent);
    lv_obj_set_width(_LblCentral, lv_pct(20));   /// 1
    lv_obj_set_height(_LblCentral, lv_pct(20));    /// 1
    lv_obj_set_align(_LblCentral, LV_ALIGN_CENTER);
    lv_obj_set_y(_LblCentral, lv_pct(-10));
    //SetStyle(_LblCentral);   
    lv_obj_add_event_cb(_LblCentral, _event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(_LblCentral, LV_OBJ_FLAG_CLICKABLE);
    lv_label_set_text(_LblCentral, "");

    _LblValue = lv_label_create(comp_parent);
    lv_obj_set_width(_LblValue, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(_LblValue, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(_LblValue, LV_ALIGN_CENTER);
    SetValuePos(0, lv_pct(5));
    lv_obj_set_style_text_font(_LblValue, MY_FONT4, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_object_set_themeable_style_property(_LblValue, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_TEXT_COLOR, _ui_theme_color_BtnTxtInaktive);
    ui_object_set_themeable_style_property(_LblValue, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_TEXT_OPA, _ui_theme_alpha_BtnTxt);


    lv_obj_move_foreground(ui_ImgZeiger);
}
void CompMeter::Update() 
{    
    uint16_t newSize = 256*SCREEN_RES_HOR/360+5;
    
    lv_obj_set_pos(_LblPeer,   _X_Peer,   _Y_Peer);
    lv_obj_set_pos(_LblPeriph, _X_Periph, _Y_Periph);
    
	if ((PeerOf(_Periph)->GetName() == NULL) or (!_PeerVisible))
	{
	    lv_obj_add_flag(_LblPeer, LV_OBJ_FLAG_HIDDEN);
	}
	else
	{
        lv_label_set_text_fmt(_LblPeer, "%.6s (%d dBm)", PeerOf(_Periph)->GetName(), PeerOf(_Periph)->GetdBm());
        lv_obj_clear_flag(_LblPeer, LV_OBJ_FLAG_HIDDEN);
	}
    
    if ((_Periph->GetName() == NULL) or (!_PeriphVisible)) 
    {
		lv_obj_add_flag(_LblPeriph, LV_OBJ_FLAG_HIDDEN);
    }
    else 
    {
		lv_label_set_text_fmt(_LblPeriph, "%.6s", _Periph->GetName());
		lv_obj_clear_flag(_LblPeriph, LV_OBJ_FLAG_HIDDEN);
    
    }

    char buf[10];
	int nk = 0;
	float value = 0;
    int Range = 485;	
    int Angle = 0;
    int Tick = 0;
    
    switch (_Periph->GetPrimary())
    {
        case PRIM_AMP:  value = _Periph->GetValue(V_AMP); 
                        Angle = -Range + (int)((value * 2.0f * 30) / 15.0f); //KI
                        break;
        case PRIM_VOLT: value = _Periph->GetValue(V_VOLT); 
                        Tick = 2*Range/15;
                        Angle = -Range+value*Tick;
                        break;
        case PRIM_WATT: value = _Periph->GetValue(V_VOLT) * _Periph->GetValue(V_AMP); 
                        Tick = 2*Range/400;
                        Angle = -Range+value*Tick;
                        break;
    }
    lv_img_set_angle(ui_ImgZeiger, Angle);
    
	if (_ValueVisible) 
	{
		//Serial.printf("Sensor: %s: %f\n\r", _Periph->GetName(), value);
		if (abs(value) < SCHWELLE) value = 0;

		if      (value<10)  nk = 2;
		else if (value<100) nk = 1;
		else                nk = 0;

		if (value == -99) strcpy(buf, "--"); 
		else dtostrf(value, 0, nk, buf);
        
        if (_Periph->GetPrimary() == PRIM_AMP)  
        { 
            strcat(buf, " A");
            if (value > 20) lv_obj_set_pos(_LblValue,   lv_pct(-_X_Value), lv_pct(_Y_Value));
            else            lv_obj_set_pos(_LblValue,   lv_pct( _X_Value), lv_pct(_Y_Value));
        }
		else if (_Periph->GetPrimary() == PRIM_VOLT)
        { 
            strcat(buf, " V");
            if (value > 7.5) lv_obj_set_pos(_LblValue,  lv_pct(-_X_Value), lv_pct(_Y_Value));
            else             lv_obj_set_pos(_LblValue,   lv_pct(_X_Value), lv_pct(_Y_Value));
        }
        else if (_Periph->GetPrimary() == PRIM_WATT)
        { 
            strcat(buf, " W");
            if (value > 200) lv_obj_set_pos(_LblValue,  lv_pct(-_X_Value), lv_pct(_Y_Value));
            else             lv_obj_set_pos(_LblValue,   lv_pct(_X_Value), lv_pct(_Y_Value));
        }
        lv_label_set_text(_LblValue, buf);
        
        if (Angle < 0)  
        {
            SetValuePos(lv_pct(25), lv_pct(12));
        }
        else            
        {
            SetValuePos(lv_pct(-25), lv_pct(12));
        }   

		lv_obj_clear_flag(_LblValue, LV_OBJ_FLAG_HIDDEN);
	}
	else 
	{
		lv_obj_add_flag(_LblValue, LV_OBJ_FLAG_HIDDEN);
	}

    if (_GraphVisible) ChartUpdate();
}