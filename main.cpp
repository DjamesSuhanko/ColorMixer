#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <EasyPCF8574.h>
#include <WiFi.h>
#include <math.h>
#include "EasyColor.h"

#define DISPLAY_WIDTH  240
#define display_HEIGHT 320

#define HUE_ANGLE 360

#define SSID   "SuhankoFamily"
#define PASSWD "fsjmr112"

EasyPCF8574 pcfSmart(0x27,0xFF);
TFT_eSPI tft = TFT_eSPI(); /* driver do touch */

//BUFFER DE FRAME DO DISPLAY
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

//GUARDA AS COORDENADAS X e Y DO TOUCH
uint16_t t_x = 0, t_y = 0;

//LIMITADOR DO TOQUE PARA NAO HAVER REPETICAO DE EVENTO
unsigned long time_to_next = millis();

//CONVERSOR DE CORES
EasyColor::HSVRGB hsvConverter;
EasyColor::CMYKRGB cmykConverter;
EasyColor rgb2rgb;

//PROTÓTIPOS DE FUNÇÕES
bool my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data);
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

void colorPickerLocal(void);
void CMYKselector();
void PickerSelector(lv_obj_t dst, lv_cpicker_color_mode_t new_mode);
void colorSample();
void loginScreen();
void dashboard();
void tabs();

void hsvSample();
void colorToSample();

void cmykSample();
void colorToSampleLineCMYK();

void hsv_plus_minus(lv_obj_t target);

void start_button(lv_obj_t target);
void spinbox_ink_volume(lv_obj_t target);

void type_values_hsv(lv_obj_t target);

void sliders_cmyk(lv_obj_t target);

static void hsv_matrix_button_cb(lv_obj_t * obj, lv_event_t event);
static void event_handler_hsv_switch(lv_obj_t * obj, lv_event_t event); //callback do switch do hsl
static void btn_start_cb(lv_obj_t * obj, lv_event_t event); //callback botão start

//HSV
static void txtarea_hue_cb(lv_obj_t * obj, lv_event_t event); 
static void txtarea_sat_cb(lv_obj_t * obj, lv_event_t event);
static void txtarea_val_cb(lv_obj_t * obj, lv_event_t event);

//CMYK
static void slider_c_cb(lv_obj_t * obj, lv_event_t event);
static void slider_m_cb(lv_obj_t * obj, lv_event_t event);
static void slider_y_cb(lv_obj_t * obj, lv_event_t event);
static void slider_k_cb(lv_obj_t * obj, lv_event_t event);

static void hsv_plus_minus_cb(lv_obj_t * obj, lv_event_t event);

void cpicker_cb(lv_obj_t * obj, lv_event_t event);

lv_obj_t * hsv_matrix_button;

lv_obj_t * slider_label_sat;
lv_obj_t * slider_label_hue;
lv_obj_t * slider_label_val;

lv_obj_t * slider_hue;
lv_obj_t * slider_sat;
lv_obj_t * slider_val;

lv_obj_t * btn_start_mixer;
lv_obj_t * spinbox_ink_volume_hsv;
lv_obj_t * spinbox_ink_volume_cmyk;
lv_obj_t * cpicker;

lv_obj_t *color_cmyk;
lv_obj_t *color_rgb;
lv_obj_t *color_hsl;
lv_obj_t *color_hsv;

lv_obj_t *txt_areas[3];
lv_obj_t * btn_plus_minus_hsv;

lv_obj_t * line1;
lv_obj_t *line_cmyk;

lv_obj_t * slider_label_c;
lv_obj_t * slider_label_m;
lv_obj_t * slider_label_y;
lv_obj_t * slider_label_k;

static lv_style_t style_line;
static lv_style_t style_line_cmyk;

static const char * hsv_btns[] = {"H", "\n","S", "\n","V",""};
static const char * hsv_plus_minus_btns[] = {LV_SYMBOL_MINUS,LV_SYMBOL_PLUS,""};

//========================VALORES IMPORTANTES PARA EXECUCAO=================================
//struct dos valores HSV
lv_color_hsv_t hsv_values;
//struct dos valores CMYK - guarda os valores para a execução
cmyk cmyk_values_struct;

uint32_t spinbox_ink_volume_value  = 0; //volume de tinta
//===============================================================

uint8_t txt_area_index = 0;

//callbacks

static void slider_c_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        int sample = lv_slider_get_value(obj);
        cmyk_values_struct.c = sample > 100 ? 100 : sample;
        snprintf(buf, 4, "%u", cmyk_values_struct.c);
        strcat(buf,"%");
        lv_label_set_text(slider_label_c, buf);

        lv_color_t lvgl_color_format;
        cmyk_values_struct.c = sample;
        rgb out_rgb = cmykConverter.CMYKtoRGB(cmyk_values_struct,out_rgb);
        lvgl_color_format.full = rgb2rgb.RGB24toRGB16(out_rgb.r,out_rgb.g,out_rgb.b);
        lv_style_set_line_color(&style_line_cmyk, LV_STATE_DEFAULT, lvgl_color_format);
    }
}

static void slider_m_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        int sample = lv_slider_get_value(obj);
        cmyk_values_struct.m = sample > 100 ? 100 : sample;
        snprintf(buf, 4, "%u", cmyk_values_struct.m);
        strcat(buf,"%");
        lv_label_set_text(slider_label_m, buf);

        lv_color_t lvgl_color_format;
        cmyk_values_struct.m = sample;
        rgb out_rgb = cmykConverter.CMYKtoRGB(cmyk_values_struct,out_rgb);
        lvgl_color_format.full = rgb2rgb.RGB24toRGB16(out_rgb.r,out_rgb.g,out_rgb.b);
        lv_style_set_line_color(&style_line_cmyk, LV_STATE_DEFAULT, lvgl_color_format);
    }
}

static void slider_y_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        int sample = lv_slider_get_value(obj);
        cmyk_values_struct.y = sample > 100 ? 100 : sample;
        snprintf(buf, 4, "%u", cmyk_values_struct.y);
        strcat(buf,"%");
        lv_label_set_text(slider_label_y, buf);

        lv_color_t lvgl_color_format;
        cmyk_values_struct.y = sample;
        rgb out_rgb = cmykConverter.CMYKtoRGB(cmyk_values_struct,out_rgb);
        lvgl_color_format.full = rgb2rgb.RGB24toRGB16(out_rgb.r,out_rgb.g,out_rgb.b);
        lv_style_set_line_color(&style_line_cmyk, LV_STATE_DEFAULT, lvgl_color_format);
    }
}

static void slider_k_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        int sample = lv_slider_get_value(obj);
        cmyk_values_struct.k = sample > 100 ? 100 : sample;
        snprintf(buf, 4, "%u", cmyk_values_struct.k);
        strcat(buf,"%");
        lv_label_set_text(slider_label_k, buf);

        lv_color_t lvgl_color_format;
        cmyk_values_struct.k = sample;
        rgb out_rgb = cmykConverter.CMYKtoRGB(cmyk_values_struct,out_rgb);
        lvgl_color_format.full = rgb2rgb.RGB24toRGB16(out_rgb.r,out_rgb.g,out_rgb.b);
        lv_style_set_line_color(&style_line_cmyk, LV_STATE_DEFAULT, lvgl_color_format);
    }
}

/*cb do +/- que trata os textarea hsv*/
static void hsv_plus_minus_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED){
        const char * txt = lv_btnmatrix_get_active_btn_text(obj);

        if (strcmp(txt, LV_SYMBOL_MINUS) == 0){
            Serial.println("menos");
            if (txt_area_index == 0){
                hsv_values.h = hsv_values.h > 0 ? hsv_values.h-1 : hsv_values.h;
            }
            else if (txt_area_index  == 1){
                hsv_values.s = hsv_values.s > 0 ? hsv_values.s-1 :hsv_values.s;
            }
            else if (txt_area_index == 2){
                hsv_values.v = hsv_values.v > 0 ? hsv_values.v-1 : hsv_values.v;
            }
        }
        else if (strcmp(txt, LV_SYMBOL_PLUS) == 0){
            Serial.println("mais");
            if (txt_area_index == 0){
                hsv_values.h = hsv_values.h < 360 ? hsv_values.h+1 : hsv_values.h;
            }
            else if (txt_area_index  == 1){
                hsv_values.s = hsv_values.s < 100 ? hsv_values.s+1 : hsv_values.s;
            }
            else if (txt_area_index == 2){
                hsv_values.v = hsv_values.v < 100 ? hsv_values.v+1 : hsv_values.v;
            }
        }
        char buf[4];
        memset(buf,0,sizeof(buf));
        String(hsv_values.h).toCharArray(buf,sizeof(buf));
        lv_textarea_set_text(txt_areas[0],buf);

        memset(buf,0,sizeof(buf));
        String(hsv_values.s).toCharArray(buf,sizeof(buf));
        lv_textarea_set_text(txt_areas[1],buf);

        memset(buf,0,sizeof(buf));
        String(hsv_values.v).toCharArray(buf,sizeof(buf));
        lv_textarea_set_text(txt_areas[2],buf);

        colorToSample();

    }
}

//TODO: Posicionar +/- próximo do volumne; labels nos textarea; sample à direita

/*cb do txtarea hue. Se ele for selecionado, envia o evento
para que o btn_plus_minus faça o incremento do valor a partir
do hsv_color.h */
static void txtarea_hue_cb(lv_obj_t * obj, lv_event_t event){
    if (event == LV_EVENT_FOCUSED){
        txt_area_index = 0;
        Serial.println("foco no hue");
    }
    if (event == LV_EVENT_VALUE_CHANGED){
        colorToSample();
    }
} 

static void txtarea_sat_cb(lv_obj_t * obj, lv_event_t event){
    if (event == LV_EVENT_FOCUSED){
        txt_area_index = 1;
        Serial.println("foco no sat");
    }
    if (event == LV_EVENT_VALUE_CHANGED){
        colorToSample();
    }
} 

static void txtarea_val_cb(lv_obj_t * obj, lv_event_t event){
    if (event == LV_EVENT_FOCUSED){
        txt_area_index = 2;
        Serial.println("foco no val");
    }
    if (event == LV_EVENT_VALUE_CHANGED){
        colorToSample();
    }
} 

static void hsv_matrix_button_cb(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        const char * txt = lv_btnmatrix_get_active_btn_text(obj);

        if (strcmp(txt,"H") == 0){
            hsv_values.h = lv_cpicker_get_hue(cpicker);
            //lv_cpicker_set_hsv(cpicker, hsv_values);
            lv_cpicker_set_color_mode(cpicker,LV_CPICKER_COLOR_MODE_HUE);
            lv_cpicker_set_hue(cpicker,hsv_values.h);
        }
        else if ( strcmp(txt,"S") == 0){
            hsv_values.h = lv_cpicker_get_hue(cpicker);
            lv_cpicker_set_color_mode(cpicker,LV_CPICKER_COLOR_MODE_SATURATION);
            lv_cpicker_set_hue(cpicker,hsv_values.h);

        }
        else if (strcmp(txt,"V") == 0){
            hsv_values.h = lv_cpicker_get_hue(cpicker);
            lv_cpicker_set_hue(cpicker,hsv_values.h);
            //lv_cpicker_set_hsv(cpicker, hsv_values);
            lv_cpicker_set_color_mode(cpicker,LV_CPICKER_COLOR_MODE_VALUE);
        }
    }
}

//spinbox
static void lv_spinbox_increment_event_hsv_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox_ink_volume_hsv);
        spinbox_ink_volume_value = spinbox_ink_volume_value <500 ? spinbox_ink_volume_value+1 : spinbox_ink_volume_value;
        //int32_t value = lv_spinbox_get_value(spinbox)/10;
        lv_spinbox_set_value(spinbox_ink_volume_hsv,spinbox_ink_volume_value*10);
        Serial.println("spin value: ");
        Serial.println(spinbox_ink_volume_value);
    }
}

static void lv_spinbox_decrement_event_hsv_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox_ink_volume_hsv);
        //int32_t value = lv_spinbox_get_value(spinbox)/10;
        spinbox_ink_volume_value = spinbox_ink_volume_value >0 ? spinbox_ink_volume_value-1 : spinbox_ink_volume_value;
        lv_spinbox_set_value(spinbox_ink_volume_hsv,spinbox_ink_volume_value*10);
        Serial.println("spin value: ");
        Serial.println(spinbox_ink_volume_value);
    }
}

static void lv_spinbox_increment_event_cmyk_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox_ink_volume_cmyk);
        spinbox_ink_volume_value = spinbox_ink_volume_value <500 ? spinbox_ink_volume_value+1 : spinbox_ink_volume_value;
        //int32_t value = lv_spinbox_get_value(spinbox)/10;
        lv_spinbox_set_value(spinbox_ink_volume_cmyk,spinbox_ink_volume_value*10);
        Serial.println("spin value: ");
        Serial.println(spinbox_ink_volume_value);
    }
}

static void lv_spinbox_decrement_event_cmyk_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox_ink_volume_cmyk);
        //int32_t value = lv_spinbox_get_value(spinbox)/10;
        spinbox_ink_volume_value = spinbox_ink_volume_value >0 ? spinbox_ink_volume_value-1 : spinbox_ink_volume_value;
        lv_spinbox_set_value(spinbox_ink_volume_cmyk,spinbox_ink_volume_value*10);
        Serial.println("spin value: ");
        Serial.println(spinbox_ink_volume_value);
    }
}


void cpicker_cb(lv_obj_t * obj, lv_event_t event){
    if ( LV_EVENT_CLICKED == event){
        hsv_values.h = lv_cpicker_get_hue(cpicker);
        lv_cpicker_set_hue(cpicker,hsv_values.h);
        Serial.println(hsv_values.h);
        Serial.print("hue: ");
        Serial.println(hsv_values.h);

        hsv_values.s = lv_cpicker_get_saturation(cpicker);
        lv_cpicker_set_saturation(cpicker,hsv_values.s);

        Serial.print("Saturation: ");
        Serial.println(hsv_values.s);

        hsv_values.v = lv_cpicker_get_value(cpicker);
        lv_cpicker_set_value(cpicker,hsv_values.v);

        Serial.print("Value: ");
        Serial.println(hsv_values.v);
    }
}

//callback do switch button do HSV
static void event_handler_hsv_switch(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        //printf("State: %s\n", lv_switch_get_state(obj) ? "On" : "Off");
        if (lv_switch_get_state(obj)){
            lv_obj_set_hidden(cpicker,true);
            lv_obj_set_hidden(hsv_matrix_button,true);

           //alimenta txtarea com os valores da wheel
            for (uint8_t i=0;i<3;i++){
                lv_obj_set_hidden(txt_areas[i],false);
            }

            char buf[4];
            memset(buf,0,sizeof(buf));
            String(hsv_values.h).toCharArray(buf,sizeof(buf));
            lv_textarea_set_text(txt_areas[0],buf);

            memset(buf,0,sizeof(buf));
            String(hsv_values.s).toCharArray(buf,sizeof(buf));
            lv_textarea_set_text(txt_areas[1],buf);

            memset(buf,0,sizeof(buf));
            String(hsv_values.v).toCharArray(buf,sizeof(buf));
            lv_textarea_set_text(txt_areas[2],buf);

            hsv_plus_minus(*color_hsv);
            lv_obj_set_hidden(btn_plus_minus_hsv,false);

            lv_obj_set_hidden(line1,false);
            //TODO: HSV para RGB888 e depois RGB565
            //lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, LV_COLOR_BLUE);

            colorToSample();
        }
        else{
            /*
            voltar os valores do txtarea do hsv_plus_minus
            para HSV da wheel, porque pode ter sido modificado
            e estar conferindo na roda de cores agora
            */
            lv_obj_set_hidden(cpicker,false);     
            lv_obj_set_hidden(hsv_matrix_button,false);

            lv_cpicker_set_hue(cpicker,hsv_values.h);
            lv_cpicker_set_saturation(cpicker,hsv_values.s);
            lv_cpicker_set_value(cpicker,hsv_values.v);
  
            for (uint8_t i=0;i<3;i++){
                lv_obj_set_hidden(txt_areas[i],true);
            }
            lv_obj_set_hidden(btn_plus_minus_hsv,true);
            lv_obj_set_hidden(line1,true);
            
        }
    }
}

//btn start
static void btn_start_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
        printf("Clicked\n");
        Serial.println(hsv_values.h);
        Serial.println(hsv_values.s);
        Serial.println(hsv_values.v);

        Serial.println(cmyk_values_struct.c);
        Serial.println(cmyk_values_struct.m);
        Serial.println(cmyk_values_struct.y);
        Serial.println(cmyk_values_struct.k);

    }
    else if(event == LV_EVENT_VALUE_CHANGED) {
        printf("Toggled\n");
    }
}

void colorToSample(){
            rgb out_rgb;
            hsv in_hsv;
            in_hsv.h = hsv_values.h;
            in_hsv.s = hsv_values.s;
            in_hsv.v = hsv_values.v;
            out_rgb  =  hsvConverter.HSVtoRGB(in_hsv,out_rgb);
            lv_color_t lvgl_color_format;
            
            lvgl_color_format.full = rgb2rgb.RGB24toRGB16(out_rgb.r,out_rgb.g,out_rgb.b);
            lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, lvgl_color_format);
}

void colorToSampleLineCMYK(){
            rgb out_rgb;

            out_rgb  =  cmykConverter.CMYKtoRGB(cmyk_values_struct,out_rgb);
            lv_color_t lvgl_color_format;
            
            lvgl_color_format.full = rgb2rgb.RGB24toRGB16(out_rgb.r,out_rgb.g,out_rgb.b);
            lv_style_set_line_color(&style_line_cmyk, LV_STATE_DEFAULT, lvgl_color_format);
}

void sliders_cmyk(lv_obj_t target){

    lv_obj_t * slider_cmyk_c = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_cmyk_c, 130);
    lv_obj_align(slider_cmyk_c, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);
    lv_obj_set_event_cb(slider_cmyk_c, slider_c_cb);
    lv_slider_set_range(slider_cmyk_c, 0, 110);

    slider_label_c = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_c, "0%");
    lv_obj_set_auto_realign(slider_label_c, true);
    lv_obj_align(slider_label_c, slider_cmyk_c, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t * slider_cmyk_m = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_cmyk_m, 130);
    lv_obj_align(slider_cmyk_m, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 50);
    lv_obj_set_event_cb(slider_cmyk_m, slider_m_cb);
    lv_slider_set_range(slider_cmyk_m, 0, 110);

    slider_label_m = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_m, "0%");
    lv_obj_set_auto_realign(slider_label_m, true);
    lv_obj_align(slider_label_m, slider_cmyk_m, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t * slider_cmyk_y = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_cmyk_y, 130);
    lv_obj_align(slider_cmyk_y, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 90);
    lv_obj_set_event_cb(slider_cmyk_y, slider_y_cb);
    lv_slider_set_range(slider_cmyk_y, 0, 110);

    slider_label_y = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_y, "0%");
    lv_obj_set_auto_realign(slider_label_y, true);
    lv_obj_align(slider_label_y, slider_cmyk_y, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    lv_obj_t * slider_cmyk_k = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_cmyk_k, 130);
    lv_obj_align(slider_cmyk_k, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 130);
    lv_obj_set_event_cb(slider_cmyk_k, slider_k_cb);
    lv_slider_set_range(slider_cmyk_k, 0, 110);

    slider_label_k = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_k, "0%");
    lv_obj_set_auto_realign(slider_label_k, true);
    lv_obj_align(slider_label_k, slider_cmyk_k, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
      
}

void hsvSample(){
    static lv_point_t line_points[] = { {0, 60}, {80, 60}};
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, LV_STATE_DEFAULT, 60);
    lv_style_set_line_color(&style_line, LV_STATE_DEFAULT, LV_COLOR_BLUE);
    lv_style_set_line_rounded(&style_line, LV_STATE_DEFAULT, false);

    /*Create a line and apply the new style*/
    
    line1 = lv_line_create(color_hsv, NULL);
    lv_line_set_points(line1, line_points, 2);     /*Set the points*/
    lv_obj_add_style(line1, LV_LINE_PART_MAIN, &style_line);     /*Set the points*/
    lv_obj_align(line1, NULL, LV_ALIGN_IN_TOP_LEFT, 116, 60);
}

void cmykSample(){
    static lv_point_t line_points[] = { {0, 0}, {0, 130}};
    lv_style_init(&style_line_cmyk);
    lv_style_set_line_width(&style_line_cmyk, LV_STATE_DEFAULT, 5);
    lv_style_set_line_color(&style_line_cmyk, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_line_rounded(&style_line_cmyk, LV_STATE_DEFAULT, false);

    /*Create a line and apply the new style*/
    
    line_cmyk = lv_line_create(color_cmyk, NULL);
    lv_line_set_points(line_cmyk, line_points, 2);     /*Set the points*/
    lv_obj_add_style(line_cmyk, LV_LINE_PART_MAIN, &style_line_cmyk);     /*Set the points*/
    lv_obj_align(line_cmyk, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 10);
}

void hsv_plus_minus(lv_obj_t target){
    btn_plus_minus_hsv = lv_btnmatrix_create(&target, NULL);
    lv_btnmatrix_set_map(btn_plus_minus_hsv, hsv_plus_minus_btns);
    //lv_btnmatrix_set_btn_width(btn_plus_minus_hsv, 10, 2);
    //lv_btnmatrix_set_btn_ctrl(btnm1, 10, LV_BTNMATRIX_CTRL_CHECKABLE);
    //lv_btnmatrix_set_btn_ctrl(btnm1, 11, LV_BTNMATRIX_CTRL_CHECK_STATE);
    lv_obj_set_height(btn_plus_minus_hsv,40);
    lv_obj_set_width(btn_plus_minus_hsv,100);

    lv_obj_align(btn_plus_minus_hsv, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 90);
    lv_obj_set_event_cb(btn_plus_minus_hsv, hsv_plus_minus_cb);
}

void start_button(lv_obj_t target){
    btn_start_mixer = lv_btn_create(&target,NULL);
    lv_obj_set_height(btn_start_mixer,35);
    lv_obj_set_width(btn_start_mixer,70);

    lv_obj_set_style_local_radius(btn_start_mixer,LV_BTN_PART_MAIN,LV_STATE_DEFAULT,8);

    lv_obj_set_event_cb(btn_start_mixer,btn_start_cb);
    lv_obj_align(btn_start_mixer, NULL, LV_ALIGN_OUT_BOTTOM_RIGHT, -12, -47);
    lv_obj_t * label = lv_label_create(btn_start_mixer, NULL);
    lv_label_set_text(label, "Start");

}

void hsv_cpicker_choice(lv_obj_t  dst){
    hsv_matrix_button = lv_btnmatrix_create(&dst, NULL);
    lv_btnmatrix_set_map(hsv_matrix_button, hsv_btns);

    lv_obj_align(hsv_matrix_button, &dst, LV_ALIGN_IN_TOP_LEFT, 0, 54);

    lv_obj_set_height(hsv_matrix_button,100);
    lv_obj_set_width(hsv_matrix_button,40);
    //lv_btn_set_checkable(btn, true)
     //set_ctrl_map(hsv_matrix_button, ctrl_map);
    //LV_BTNMATRIX_CTRL_CHECKABLE 
    lv_btnmatrix_set_one_check(hsv_matrix_button, true);
    lv_btnmatrix_set_focused_btn(hsv_matrix_button,0);
    lv_btnmatrix_set_btn_ctrl_all(hsv_matrix_button, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_obj_set_event_cb(hsv_matrix_button, hsv_matrix_button_cb);

}

void PickerSelector(lv_obj_t dst,lv_cpicker_color_mode_t new_mode){

    cpicker = lv_cpicker_create(&dst, NULL);

    //lv_cpicker_set_type(cpicker, LV_CPICKER_TYPE_RECT);
    lv_cpicker_set_color_mode(cpicker, new_mode);

    lv_obj_set_style_local_scale_width(cpicker,LV_CPICKER_PART_MAIN,LV_STATE_DEFAULT,15);
    lv_obj_set_style_local_scale_width(cpicker,LV_CPICKER_PART_KNOB,LV_STATE_DEFAULT,30);
    
    lv_obj_set_size(cpicker, 100, 100);
    lv_obj_set_event_cb(cpicker,cpicker_cb);
    lv_obj_align(cpicker, NULL, LV_ALIGN_CENTER, 0, -5);
}

void spinbox_ink_volumeHSV(lv_obj_t target){
    spinbox_ink_volume_hsv = lv_spinbox_create(&target, NULL);
    lv_spinbox_set_range(spinbox_ink_volume_hsv, +0, 500); //como step nao tá dando 1, dividir por 10 no cb
    lv_spinbox_set_digit_format(spinbox_ink_volume_hsv, 4, 3);
    lv_spinbox_set_step(spinbox_ink_volume_hsv,10);
    lv_spinbox_step_prev(spinbox_ink_volume_hsv);
    lv_obj_set_width(spinbox_ink_volume_hsv, 120);
    lv_obj_align(spinbox_ink_volume_hsv, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 5, -13);
    lv_spinbox_set_padding_left(spinbox_ink_volume_hsv,3);
    lv_obj_set_style_local_margin_left(spinbox_ink_volume_hsv, LV_SPINBOX_PART_BG, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_margin_right(spinbox_ink_volume_hsv, LV_SPINBOX_PART_BG, LV_STATE_DEFAULT, 0);

    lv_coord_t h = lv_obj_get_height(spinbox_ink_volume_hsv);
    lv_obj_t * btn = lv_btn_create(&target, NULL);
    lv_obj_set_size(btn, h, h);
    lv_obj_align(btn, spinbox_ink_volume_hsv, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    lv_theme_apply(btn, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(btn, lv_spinbox_increment_event_hsv_cb);

    btn = lv_btn_create(&target, btn);
    lv_obj_align(btn, spinbox_ink_volume_hsv, LV_ALIGN_IN_BOTTOM_LEFT, -5, 0);
    lv_obj_set_event_cb(btn, lv_spinbox_decrement_event_hsv_cb);
    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_MINUS);
}

void spinbox_ink_volumeCMYK(lv_obj_t target){
    spinbox_ink_volume_cmyk = lv_spinbox_create(&target, NULL);
    lv_spinbox_set_range(spinbox_ink_volume_cmyk, +0, 500); //como step nao tá dando 1, dividir por 10 no cb
    lv_spinbox_set_digit_format(spinbox_ink_volume_cmyk, 4, 3);
    lv_spinbox_set_step(spinbox_ink_volume_cmyk,10);
    lv_spinbox_step_prev(spinbox_ink_volume_cmyk);
    lv_obj_set_width(spinbox_ink_volume_cmyk, 120);
    lv_obj_align(spinbox_ink_volume_cmyk, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 5, -13);
    lv_spinbox_set_padding_left(spinbox_ink_volume_cmyk,3);
    lv_obj_set_style_local_margin_left(spinbox_ink_volume_cmyk, LV_SPINBOX_PART_BG, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_margin_right(spinbox_ink_volume_cmyk, LV_SPINBOX_PART_BG, LV_STATE_DEFAULT, 0);

    lv_coord_t h = lv_obj_get_height(spinbox_ink_volume_cmyk);
    lv_obj_t * btn = lv_btn_create(&target, NULL);
    lv_obj_set_size(btn, h, h);
    lv_obj_align(btn, spinbox_ink_volume_cmyk, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    lv_theme_apply(btn, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(btn, lv_spinbox_increment_event_cmyk_cb);

    btn = lv_btn_create(&target, btn);
    lv_obj_align(btn, spinbox_ink_volume_cmyk, LV_ALIGN_IN_BOTTOM_LEFT, -5, 0);
    lv_obj_set_event_cb(btn, lv_spinbox_decrement_event_cmyk_cb);
    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_MINUS);
}

//textarea HSV
/*
enviar sinal do id selecionado, cada qual com seu cb
*/
void type_values_hsv(lv_obj_t target){
    txt_areas[0] = lv_textarea_create(&target, NULL);
    lv_obj_set_size(txt_areas[0], 60, 30);
    lv_obj_align(txt_areas[0], NULL, LV_ALIGN_IN_TOP_LEFT, 0, 50);
    lv_textarea_set_text(txt_areas[0], "0");    /*Set an initial text*/
    lv_obj_set_event_cb(txt_areas[0], txtarea_hue_cb);

    txt_areas[1] = lv_textarea_create(&target, NULL);
    lv_obj_set_size(txt_areas[1], 60, 30);
    lv_obj_align(txt_areas[1], NULL, LV_ALIGN_IN_TOP_LEFT, 70, 50);
    lv_textarea_set_text(txt_areas[1], "255");
    lv_obj_set_event_cb(txt_areas[1], txtarea_sat_cb);

    txt_areas[2] = lv_textarea_create(&target, NULL);
    lv_obj_set_size(txt_areas[2], 60, 30);
    lv_obj_align(txt_areas[2], NULL, LV_ALIGN_IN_TOP_LEFT, 140, 50);
    lv_textarea_set_text(txt_areas[2], "0");
    lv_obj_set_event_cb(txt_areas[2], txtarea_val_cb);
}

void tabs(){
    /* CRIA UM OBJETO TABVIEW */
    lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), NULL); //TABVIEW PRINCIPAL

    /* tabs principais */
    lv_obj_t *tab_color = lv_tabview_add_tab(tabview, "Color");
    lv_obj_t *tab_setup = lv_tabview_add_tab(tabview, "Setup");
    lv_obj_t *tab_about = lv_tabview_add_tab(tabview, "About");

    lv_obj_t *tabview_color;
    tabview_color =  lv_tabview_create(tab_color, NULL); //TABVIEW COLORS NA ABA COLORS

    lv_obj_align(tabview_color, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_local_pad_bottom(tabview,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,4);
    lv_obj_set_style_local_pad_bottom(tabview_color,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,4);

    lv_obj_set_style_local_pad_top(tabview,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,4);
    lv_obj_set_style_local_pad_top(tabview_color,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,1);

    //tabs secundárias
    color_cmyk = lv_tabview_add_tab(tabview_color, "CMYK");
    color_rgb  = lv_tabview_add_tab(tabview_color, "RGB");
    color_hsl  = lv_tabview_add_tab(tabview_color, "HSL");
    color_hsv  = lv_tabview_add_tab(tabview_color, "HSV");

    lv_tabview_set_btns_pos(tabview_color, LV_TABVIEW_TAB_POS_BOTTOM);


    /* UMA ANIMAÇÃO SIMPLES (OPCIOINAL)*/
    lv_tabview_set_anim_time(tabview, 1000);

    //--------------------------TAB 1: COLORS-------------------------------------------
    

    //--------------------------TAB 2: SETUP-------------------------------------------
    lv_obj_t *l2 = lv_label_create(tab_setup, NULL);
    lv_label_set_text(l2, "Aba setup");
    lv_obj_align(l2, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    //--------------------------TAB 3: ABOUT-------------------------------------------
    lv_obj_t *l3 = lv_label_create(tab_about, NULL);
    lv_label_set_text(l3, "Aba About");
    lv_obj_align(l3, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    //==============TAB HSV ===============
    lv_obj_t *switch_hsv = lv_switch_create(color_hsv, NULL);
    lv_obj_align(switch_hsv, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    lv_obj_set_event_cb(switch_hsv, event_handler_hsv_switch);

    lv_obj_t  *label_wheel = lv_label_create(color_hsv, NULL);
    lv_label_set_text(label_wheel, "Wheel");
    lv_obj_align(label_wheel, NULL, LV_ALIGN_IN_TOP_LEFT, 15, 10);

    lv_obj_t  *label_sliders = lv_label_create(color_hsv, NULL);
    lv_label_set_text(label_sliders, "Input");
    lv_obj_align(label_sliders, NULL, LV_ALIGN_IN_TOP_RIGHT, -25, 10);

    // cpicker_ghost = color_hsv;
    PickerSelector(*color_hsv,LV_CPICKER_COLOR_MODE_HUE);

    hsv_cpicker_choice(*color_hsv);
    start_button(*color_hsv);
    spinbox_ink_volumeHSV(*color_hsv);

    //=========    SLIDER HSV ==================
    type_values_hsv(*color_hsv);
    for (uint8_t i=0;i<3;i++){
        lv_obj_set_hidden(txt_areas[i],true);
    }
    
    hsvSample();
    lv_obj_set_hidden(line1,true);

    //CMYK
    sliders_cmyk(*color_cmyk);
    start_button(*color_cmyk);
    spinbox_ink_volumeCMYK(*color_cmyk);
    cmykSample();

    //---------------------------- ULTIMA CAMADA -----------------------------------
    /* ESSA CAMADA VAI NA SCREEN PRINCIPAL, "FLUTUANDO" SOBRE A TABVIEW. 
    AQUI TEMOS UM LABEL E O BOTÃO DE LOGOFF. */
    
    //lv_obj_t *bb = lv_label_create(lv_scr_act(), NULL);
    //lv_label_set_text(bb, "Do bit Ao Byte - LVGL");
    //lv_obj_align(bb, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);

    lv_obj_t * btn_logoff = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_align(btn_logoff,NULL,LV_ALIGN_IN_BOTTOM_RIGHT,-5,8);
    lv_obj_set_width(btn_logoff,30);
    lv_obj_set_height(btn_logoff,30);
    //lv_obj_set_event_cb(btn_logoff, event_logoff);

    lv_obj_t *icon_logoff = lv_label_create(btn_logoff, NULL);
    lv_label_set_text(icon_logoff, LV_SYMBOL_POWER);

}

//-------------------------------------------------------------------------------//
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p){
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors(&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

bool my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data){
    if ((millis()-time_to_next) > 10){
        if (tft.getTouch(&t_x, &t_y)){
            data->state = LV_INDEV_STATE_PR ;
            data->point.x = t_x;
            data->point.y = t_y;
            time_to_next = millis();
            Serial.println("ok");
        }
        else{
            data->state = LV_INDEV_STATE_REL;
            data->point.x = 0;
            data->point.y = 0;
        }
    }
    
    return false; /*No buffering now so no more data read*/
}

void setup() {
    hsv_values.h = 0;
    hsv_values.s = 100;
    hsv_values.v = 100;

    cmyk_values_struct.c = 0;
    cmyk_values_struct.m = 0;
    cmyk_values_struct.y = 0;
    cmyk_values_struct.k = 0;

    Serial.begin(115200);

    if (!pcfSmart.startI2C(21,22)){
        Serial.println("Not started. Check pin and address.");
        while (true);
    }

    lv_init();

    uint16_t calData[5] = { 313, 3260, 491, 3315, 6 };

    tft.begin(); /* TFT init */
    tft.setRotation(0); /* Landscape orientation */


    tft.setTouch(calData);

    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 240;
    disp_drv.ver_res = 320;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER; //pointer para mouse e touch
    indev_drv.read_cb = my_input_read;
    lv_indev_drv_register(&indev_drv);


    //Tutorial LVGL - 02
    tabs();

    rgb out_rgb;
    out_rgb.r = 220;
    out_rgb.g = 180;
    out_rgb.b = 90;

    hsv out_hsv;
    out_hsv.h = 284;
    out_hsv.s = 46;
    out_hsv.v = 60;

    rgb in_rgb;
    in_rgb.r = 200;
    in_rgb.g = 100;
    in_rgb.b = 190;

    hsv in_hsv;
    in_hsv.h = 192.94;
    in_hsv.s = 100;
    in_hsv.v = 255;

    delay(2000);

    Serial.println("RGB");
    out_rgb =  hsvConverter.HSVtoRGB(in_hsv,out_rgb);
    Serial.println(out_rgb.r);
    Serial.println(out_rgb.g);
    Serial.println(out_rgb.b);

    Serial.println("HVS");
    out_hsv = hsvConverter.RGBtoHSV(in_rgb, out_hsv);
    Serial.println(out_hsv.h);
    Serial.println(out_hsv.s);
    Serial.println(out_hsv.v);

    //Serial.println(teste1.g->);
    //hsvConverter
    uint16_t teste1 = rgb2rgb.RGB24toRGB16(0x80,0x80,0x00);
    Serial.println(teste1,HEX);

    rgb teste3 = rgb2rgb.RGB16toRGB24(0x8400);
    Serial.println(teste3.r);
    Serial.println(teste3.g);
    Serial.println(teste3.b);
}

void loop() {
    lv_task_handler();
    delay(5);
}