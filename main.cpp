/*
Author:  Djames Suhanko <djames.suhanko@gmail.com>
Website: https://www.dobitaobyte.com.br
Youtube: youtube.com/dobitaobytebrasil
~/.platformio/packages/framework-arduinoespressif32/tools/partitions/default.csv
You are free to use this code, just preserve author reference.
*/

#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <EasyPCF8574.h>
#include <WiFi.h>
#include <math.h>
#include "EasyColor.h"
#include "fileHandler.h"

#define DISPLAY_WIDTH  240
#define DISPLAY_HEIGHT 320

#define HUE_ANGLE      360
#define PCF_ADDR       0x27

#define SOCK_FILE         "/socket.ini"
#define WIFI_FILE         "/wifi.ini"
#define PASSWD_LOGIN_FILE "/loginP.ini"
#define ENABLE_LOGIN_FILE "/loginE.ini"

#define SSID   "colorMixer"
#define PASSWD "dobitaobyte"

#define DEFAULT_LOGIN_PASSWD "123456"


EasyPCF8574 pcfSmart(PCF_ADDR,0xFF);
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
void loginScreen(); //TODO: fazer a tela de login. Código em SmartControl
void dashboard();
void tabs();

void hsvSample();
void colorToSample();

void cmykSample();
void colorToSampleLineCMYK();

void rgbSample();
void colorToSampleLineCMYK();

void list_of_patterns();

void list_of_files_roller_files();

//setup socket
bool change_socket_to();
bool can_start_socket();
bool change_login_to();

//setup wifi
bool change_wifi_to();
bool can_start_wifi();

void hsv_plus_minus(lv_obj_t target);

void start_button(lv_obj_t target);

void spinbox_ink_volumeRGB(lv_obj_t target);

void mtx_load_color(lv_obj_t  target);

void type_values_hsv(lv_obj_t target);

void sliders_cmyk(lv_obj_t target);

static void hsv_matrix_button_cb(lv_obj_t * obj, lv_event_t event);
static void load_matrix_button_cb(lv_obj_t * obj, lv_event_t event);
static void load_matrix_button_roller_files_cb(lv_obj_t * obj, lv_event_t event);

static void event_handler_hsv_switch(lv_obj_t * obj, lv_event_t event); //callback do switch do hsv
static void btn_start_cb(lv_obj_t * obj, lv_event_t event); //callback botão start

//setup
static void event_handler_socket_switch(lv_obj_t * obj, lv_event_t event);
static void event_handler_login_switch(lv_obj_t * obj, lv_event_t event);

//HSV
static void txtarea_hue_cb(lv_obj_t * obj, lv_event_t event); 
static void txtarea_sat_cb(lv_obj_t * obj, lv_event_t event);
static void txtarea_val_cb(lv_obj_t * obj, lv_event_t event);

//CMYK
static void slider_c_cb(lv_obj_t * obj, lv_event_t event);
static void slider_m_cb(lv_obj_t * obj, lv_event_t event);
static void slider_y_cb(lv_obj_t * obj, lv_event_t event);
static void slider_k_cb(lv_obj_t * obj, lv_event_t event);

//RGB
static void slider_red_cb(lv_obj_t * obj, lv_event_t event);
static void slider_green_cb(lv_obj_t * obj, lv_event_t event);
static void slider_blue_cb(lv_obj_t * obj, lv_event_t event);

static void hsv_plus_minus_cb(lv_obj_t * obj, lv_event_t event);

void cpicker_cb(lv_obj_t * obj, lv_event_t event);

void exclude_file_cb(lv_obj_t * obj, lv_event_t event);

//----------------- SYSTEM ---------------------
//info
void infoWiFi(lv_obj_t target);
void infoSock(lv_obj_t target);

//setup
void setupCredentials(lv_obj_t target);
void setupCalibratePump(lv_obj_t target);
void setupEnableSock(lv_obj_t target); //switch button - salva em arquivo
void setupChangeLogin(lv_obj_t target);
void setupEnableLogin(lv_obj_t target); //switch button - salva em arquivo
void setupDisableWiFi(lv_obj_t target);
void setupWiFiMode(lv_obj_t target);

//-------------------- tasks ------------------------
void pump(void *pvParameters);
void fromPicker(void *pvParameters);
//////////////////////tasks//////////////////////////

//Mudando de aba, realimenta o spinbox
static void tab_feeding_spinbox_cb(lv_obj_t *obj, lv_event_t event);

lv_obj_t * hsv_matrix_button;
lv_obj_t * load_color_mtx_btn;
lv_obj_t * load_files_mtx_btn;

lv_obj_t * slider_label_sat;
lv_obj_t * slider_label_hue;
lv_obj_t * slider_label_val;

lv_obj_t * slider_label_red;
lv_obj_t * slider_label_green;
lv_obj_t * slider_label_blue;

lv_obj_t * slider_cmyk_c;
lv_obj_t * slider_cmyk_m;
lv_obj_t * slider_cmyk_y;
lv_obj_t * slider_cmyk_k;

lv_obj_t * btn_start_mixer;
lv_obj_t * spinbox_ink_volume_hsv;
lv_obj_t * spinbox_ink_volume_cmyk;
lv_obj_t * spinbox_ink_volume_rgb;
lv_obj_t * spinbox_ink_volume_load;
lv_obj_t * cpicker;

lv_obj_t *color_cmyk;
lv_obj_t *color_rgb;
lv_obj_t *color_load;
lv_obj_t *color_hsv;

lv_obj_t *txt_areas[3];
lv_obj_t * btn_plus_minus_hsv;

lv_obj_t * line1;
lv_obj_t *line_cmyk;
lv_obj_t *line_rgb;
lv_obj_t *line_load;

lv_obj_t * slider_label_c;
lv_obj_t * slider_label_m;
lv_obj_t * slider_label_y;
lv_obj_t * slider_label_k;

lv_obj_t * roller_patterns;
lv_obj_t * roller_files;

lv_obj_t *tabview;

lv_obj_t * msgBoxDel;

//system
lv_obj_t *tab_info;
lv_obj_t *tab_setup;

//login
lv_obj_t *labelB;

static lv_style_t style_line;
static lv_style_t style_line_cmyk;
static lv_style_t style_line_rgb;
static lv_style_t style_line_load;

static const char * hsv_btns[]  = {"H", "\n","S", "\n","V",""};
static const char * load_btns[] = {LV_SYMBOL_UP,
                                 LV_SYMBOL_DOWN,
                                 LV_SYMBOL_FILE,
                                  LV_SYMBOL_CUT,
                                  LV_SYMBOL_DOWNLOAD,
                                            ""};

static const char * load_btns_files[] = {LV_SYMBOL_UP,
                                           "\n",
                                 LV_SYMBOL_DOWN,
                                           "\n",
                                  LV_SYMBOL_CUT,
                                            ""};
static const char * hsv_plus_minus_btns[] = {LV_SYMBOL_MINUS,LV_SYMBOL_PLUS,""};

//========================VALORES IMPORTANTES PARA EXECUCAO=================================
//struct dos valores HSV
lv_color_hsv_t hsv_values;
//struct dos valores CMYK - guarda os valores para a execução
cmyk cmyk_values_struct;
//valor rgb nos sliders do rgb
rgb rgb_from_sliders;

uint32_t spinbox_ink_volume_value  = 0; //volume de tinta
//===============================================================

bool initSock           = true;
bool list_created       = false;
bool list_files_created = true;

uint8_t txt_area_index  = 0;
uint8_t roller_pos      = 0;

uint8_t txt_info_dist   = 0;
char full_msg[20]; //arquivo a excluir

//-=-=-=-=-=-=-=-=-=-=-=-=-=- REQUISITOS FUNCIONAIS -=-=-=-=-=-=-=-=-=-=-=-=-=-=-
boolean pump_is_running     = false;
boolean is_mode_ap          = true;
boolean unused              = true; // TODO: eliminar, nao é util mais

SemaphoreHandle_t myMutex   = NULL;

TaskHandle_t task_zero      = NULL;
TaskHandle_t task_one       = NULL;
TaskHandle_t task_two       = NULL;
TaskHandle_t task_three     = NULL;

struct pump_t {
    uint8_t pcf_value       = 255;                                      // estados dos pinos
    uint8_t pumps_bits[4]   = {7,6,5,4};                                // apenas para ordenar da esquerda para direita logicamente  
    TaskHandle_t handles[4] = {task_zero,task_one,task_two,task_three}; //manipuladores das tasks
    uint8_t running         = 0;                                        //cada task incrementa e decrementa. 0 é parado.
    unsigned long  times[4] = {0,0,0,0};                                
} pump_params;

float one_ml                = 2141; //o volume está em spinbox_ink_volume_value

char *labels[4]             = {"cyan","magent","yellow","black"};
char asterisc[6]            = {' '};

uint8_t pwd_pos             = 0;

bool has_logged_in          = false;

bool was_logoff             = false; //depois de logar 1x, nao precisa mais iniciar os infos após tabs
//socket server para o color picker =======================================
WiFiServer server(1234); 

//callbacks


//logoff
static void event_logoff(lv_obj_t * obj, lv_event_t event){
    if(event == LV_INDEV_STATE_PR) {
        ESP.restart();
    }
}

//passwd
static void event_handler_passwd(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        const char * txt = lv_btnmatrix_get_active_btn_text(obj);
        //CONDICAO 1: senha bate!
        if (strcmp(txt,">") == 0 && asterisc[5] != ' '){
            if (strcmp(asterisc,"123456") == 0){
                Serial.println("login ok");  
                pwd_pos = 0;
                memset(asterisc,' ',sizeof(asterisc));  
            }
            memset(asterisc,' ',sizeof(asterisc));
            lv_obj_del(obj);
            if (can_start_wifi()){
                WiFi.softAP(SSID,PASSWD); //TODO: validar o modo
            }
            
            if (can_start_socket() && can_start_wifi()){
                server.begin();
                xTaskCreatePinnedToCore(fromPicker,"fromPicker",10000,NULL,0,NULL,0);
            }
            lv_obj_del(obj);
            tabs();
            delay(2000);
            infoWiFi(*tab_info);
            infoSock(*tab_info);
            
        }
        //CONDICAO 2: senha nao bate
        else if (strcmp(txt,">") == 0 && asterisc[5] != DEFAULT_LOGIN_PASSWD[5]){
            memset(asterisc,' ',sizeof(asterisc));
            Serial.println("You should not pass");
            pwd_pos = 0;
        }
        //CONDICAO 3: digitou mais do que deveria
        else if (strcmp(txt,">") != 0 && asterisc[5] != ' ' && txt[0] != 'x'){
            memset(asterisc,' ',sizeof(asterisc));
            pwd_pos = 0;
            Serial.println("You should not pass");
        }
        //CONDICAO 4: ainda nao completou a senha, entao concatena (se nao for correcao)
        else if (asterisc[5] == ' ' && txt[0] != 'x'){
            asterisc[pwd_pos] = txt[0];
            pwd_pos += 1;
        }
        //CONDICAO 5: corrigindo senha
        else if (txt[0] == 'x'){
            pwd_pos = pwd_pos > 0 ? pwd_pos-1 : 0; //apaga só até chegar em 0
            asterisc[pwd_pos] = ' ';
            
        }
        Serial.println(asterisc);
        lv_label_set_text_fmt(labelB,"%s\0",asterisc);
    }
}

void reboot_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        if (strcmp(lv_msgbox_get_active_btn_text(obj),"Yes") == 0){
            ESP.restart();
        }
        else{
            lv_obj_del(obj);
        }
    }
}
void exclude_file_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        if (strcmp(lv_msgbox_get_active_btn_text(obj),"Yes") == 0){
            deleteFile(SPIFFS,full_msg);
            lv_obj_del(msgBoxDel);
            lv_obj_del(roller_patterns);
            list_of_patterns();
        }
        else if (strcmp(lv_msgbox_get_active_btn_text(obj),"Cancel") == 0)
        lv_obj_del(msgBoxDel);
    }
}

void exclude_file_roller_files_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        if (strcmp(lv_msgbox_get_active_btn_text(obj),"Yes") == 0){
            Serial.println(full_msg);
            Serial.println("^^^^^^");
            deleteFile(SPIFFS,full_msg);
            lv_obj_del(msgBoxDel);
            lv_obj_del(roller_files);
            list_of_files_roller_files();
        }
        else if (strcmp(lv_msgbox_get_active_btn_text(obj),"Cancel") == 0)
        lv_obj_del(msgBoxDel);
    }
}
/*static void tab_feeding_spinbox_cb(lv_obj_t *obj, lv_event_t event){
    lv_spinbox_set_value(spinbox_ink_volume_hsv,spinbox_ink_volume_value*10);
    lv_spinbox_set_value(spinbox_ink_volume_cmyk,spinbox_ink_volume_value*10);
    lv_spinbox_set_value(spinbox_ink_volume_rgb,spinbox_ink_volume_value*10);
    Serial.println("tab changed");
    
}*/

//RGB
static void slider_red_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        int sample = lv_slider_get_value(obj);

        rgb_from_sliders.r = sample > 255 ? 255 : sample;
        snprintf(buf, 4, "%u", rgb_from_sliders.r);
        lv_label_set_text(slider_label_red, buf);

        lv_color_t lvgl_color_format;
        cmyk_values_struct = cmykConverter.RGBtoCMYK(rgb_from_sliders,cmyk_values_struct);
        lvgl_color_format.full = rgb2rgb.RGB24toRGB16(rgb_from_sliders.r,rgb_from_sliders.g,rgb_from_sliders.b);
        lv_style_set_line_color(&style_line_rgb, LV_STATE_DEFAULT, lvgl_color_format);

        Serial.println("sliders RGB - R");
        Serial.println(rgb_from_sliders.r);
        Serial.println(rgb_from_sliders.g);
        Serial.println(rgb_from_sliders.b);
    }
}

static void slider_green_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        int sample = lv_slider_get_value(obj);

        rgb_from_sliders.g = sample > 255 ? 255 : sample;
        snprintf(buf, 4, "%u", rgb_from_sliders.g);
        lv_label_set_text(slider_label_green, buf);

        lv_color_t lvgl_color_format;
        cmyk_values_struct = cmykConverter.RGBtoCMYK(rgb_from_sliders,cmyk_values_struct);
        lvgl_color_format.full = rgb2rgb.RGB24toRGB16(rgb_from_sliders.r,rgb_from_sliders.g,rgb_from_sliders.b);
        lv_style_set_line_color(&style_line_rgb, LV_STATE_DEFAULT, lvgl_color_format);

        Serial.println("sliders RGB - G");
        Serial.println(rgb_from_sliders.r);
        Serial.println(rgb_from_sliders.g);
        Serial.println(rgb_from_sliders.b);
    }
}

static void slider_blue_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        int sample = lv_slider_get_value(obj);

        rgb_from_sliders.b = sample > 255 ? 255 : sample;
        snprintf(buf, 4, "%u", rgb_from_sliders.b);
        lv_label_set_text(slider_label_blue, buf);

        lv_color_t lvgl_color_format;
        cmyk_values_struct = cmykConverter.RGBtoCMYK(rgb_from_sliders,cmyk_values_struct);
        lvgl_color_format.full = rgb2rgb.RGB24toRGB16(rgb_from_sliders.r,rgb_from_sliders.g,rgb_from_sliders.b);
        lv_style_set_line_color(&style_line_rgb, LV_STATE_DEFAULT, lvgl_color_format);

        Serial.println("sliders RGB - B");
        Serial.println(rgb_from_sliders.r);
        Serial.println(rgb_from_sliders.g);
        Serial.println(rgb_from_sliders.b);
    }
}

//CMYK
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

static void hsv_matrix_button_cb(lv_obj_t * obj, lv_event_t event){
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
static void load_matrix_button_roller_files_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
        const char * txt = lv_btnmatrix_get_active_btn_text(obj);
        //PRA CIMA (é down mesmo)
        if (strcmp(txt,LV_SYMBOL_DOWN) == 0){
            uint16_t item = lv_roller_get_selected(roller_files);

            char all_items[300];
            memset(all_items,0,sizeof(all_items));
            strcpy(all_items,lv_roller_get_options(roller_files));

            uint8_t len_items = lv_roller_get_option_cnt(roller_files);

            roller_pos = roller_pos == len_items-1 ? 0 : item+1; 
            lv_roller_set_selected(roller_files,(uint16_t) roller_pos,LV_ANIM_ON);    
        }
        //PRA BAIXO (é up mesmo)
        else if ( strcmp(txt,LV_SYMBOL_UP) == 0){
            uint16_t item = lv_roller_get_selected(roller_files);
            Serial.print("ITEM: ");
            Serial.println(item);
            
            char all_items[300];
            memset(all_items,0,sizeof(all_items));
            strcpy(all_items,lv_roller_get_options(roller_files));

            //uint8_t len_items = lv_roller_get_option_cnt(roller_patterns);

            roller_pos = roller_pos > 0 ? roller_pos-1 : roller_pos; 
            lv_roller_set_selected(roller_files,(uint16_t) roller_pos,LV_ANIM_ON);
        }
        else if (strcmp(txt,LV_SYMBOL_CUT) == 0){
            char target[20];
            //char full_msg[4];
            memset(full_msg,0,sizeof(full_msg));
            full_msg[0] = '/';
            memset(target,0,sizeof(target));
            lv_roller_get_selected_str(roller_files,target,20);
            
            if (target[0] == ' '){
                target[0] = '/';
                strcpy(full_msg, target);
            }
            else{
                strcat(full_msg,target);
            }
            Serial.println("full_msg e target");
            Serial.println(full_msg);
            Serial.println(target);

            //deleteFile(SPIFFS,full_msg);
            //lv_obj_del(roller_patterns);
            //list_of_patterns();

            //confirma exclusao
            static const char * btns[] ={"Yes", "Cancel", ""};

            msgBoxDel = lv_msgbox_create(lv_scr_act(), NULL);
            String really = "Deseja realmente excluir ";
            really = really + target + "?";
            lv_msgbox_set_text(msgBoxDel, really.c_str());
            lv_msgbox_add_btns(msgBoxDel, btns);
            lv_obj_set_width(msgBoxDel, 200);
            lv_obj_set_event_cb(msgBoxDel, exclude_file_roller_files_cb);
            lv_obj_align(msgBoxDel, NULL, LV_ALIGN_CENTER, 0, 0);
            roller_pos = 0;

        }
    }

}

static void load_matrix_button_cb(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_CLICKED) {
        char buf[20];
        const char * txt = lv_btnmatrix_get_active_btn_text(obj);
        //PRA CIMA (é down mesmo)
        if (strcmp(txt,LV_SYMBOL_DOWN) == 0){
            uint16_t item = lv_roller_get_selected(roller_patterns);

            char all_items[300];
            memset(all_items,0,sizeof(all_items));
            strcpy(all_items,lv_roller_get_options(roller_patterns));

            uint8_t len_items = lv_roller_get_option_cnt(roller_patterns);

            roller_pos = roller_pos == len_items-1 ? 0 : item+1; 
            lv_roller_set_selected(roller_patterns,(uint16_t) roller_pos,LV_ANIM_ON);    
        }
        //PRA BAIXO (é up mesmo)
        else if ( strcmp(txt,LV_SYMBOL_UP) == 0){
            uint16_t item = lv_roller_get_selected(roller_patterns);
            Serial.print("ITEM: ");
            Serial.println(item);
            
            char all_items[300];
            memset(all_items,0,sizeof(all_items));
            strcpy(all_items,lv_roller_get_options(roller_patterns));

            //uint8_t len_items = lv_roller_get_option_cnt(roller_patterns);

            roller_pos = roller_pos > 0 ? roller_pos-1 : roller_pos; 
            lv_roller_set_selected(roller_patterns,(uint16_t) roller_pos,LV_ANIM_ON);
        }
        //SALVAR
        else if (strcmp(txt,LV_SYMBOL_FILE) == 0){
            //salvar em arquivo
            String CMYK_txt =  String(cmyk_values_struct.c) + "|" +
                              String(cmyk_values_struct.m) + "|" +
                              String(cmyk_values_struct.y) + "|" +
                              String(cmyk_values_struct.k);

            if (list_created){
                memset(buf,0,sizeof(buf));
                //lv_roller_get_selected_str(roller_patterns,buf,sizeof(buf));
                uint16_t num_of_items = lv_roller_get_option_cnt(roller_patterns)+10;
                //selecionar o ultimo (o primeiro esta nulo) e manter o numero para adicionar. TODO: fazer padding left de 2.
                //lv_roller_set_selected(roller_patterns,num_of_items-2,LV_ANIM_ON);

                if (num_of_items < 20){ //era pra ser menor que 4, mas tem outros arquivos agora. TODO: tratar 
                    //TODO: aumentar patterns salvos?
                    String opts = lv_roller_get_options(roller_patterns);
                    
                    for (uint8_t i=1;i<num_of_items;i++){
                        //strchr retorna o ponteiro para a primeira ocorrencia em c_str().
                        if (strrchr(opts.c_str(), String(i).c_str()[0]) == NULL){
                            sprintf(buf, "/%03d", i);
                            Serial.print("SALVANDO: ");
                            Serial.println(buf);
                            writeFile(SPIFFS,buf,CMYK_txt.c_str());
                            lv_obj_del(roller_patterns);
                            list_of_patterns();
                            break;
                        }
                    }
                    roller_pos = 0;
                }
                else{
                    lv_obj_t * mbox1 = lv_msgbox_create(lv_scr_act(), NULL);
                    lv_msgbox_set_text(mbox1, "Maximo de itens preenchido");
                    lv_obj_set_width(mbox1, 180);
                    lv_msgbox_start_auto_close(mbox1, 2000);
                    lv_obj_align(mbox1, NULL, LV_ALIGN_CENTER, 0, 0);
                }
            }
        }
        else if (strcmp(txt,LV_SYMBOL_CUT) == 0){
            char target[20];
            //char full_msg[4];
            memset(full_msg,0,sizeof(full_msg));
            full_msg[0] = '/';
            memset(target,0,4);
            lv_roller_get_selected_str(roller_patterns,target,4);
            
            strcat(full_msg,target);

            //deleteFile(SPIFFS,full_msg);
            //lv_obj_del(roller_patterns);
            //list_of_patterns();

            //confirma exclusao
            static const char * btns[] ={"Yes", "Cancel", ""};

            msgBoxDel = lv_msgbox_create(lv_scr_act(), NULL);
            String really = "Deseja realmente excluir ";
            really = really + target + "?";
            lv_msgbox_set_text(msgBoxDel, really.c_str());
            lv_msgbox_add_btns(msgBoxDel, btns);
            lv_obj_set_width(msgBoxDel, 200);
            lv_obj_set_event_cb(msgBoxDel, exclude_file_cb);
            lv_obj_align(msgBoxDel, NULL, LV_ALIGN_CENTER, 0, 0);

            roller_pos = 0;

        }
        else if (strcmp(txt,LV_SYMBOL_DOWNLOAD) == 0){
            memset(buf,0,sizeof(buf));
            char target[20];
            lv_roller_get_selected_str(roller_patterns,target,4);
            buf[0] = '/';
            strcat(buf,target);
            String values = readFile(SPIFFS,buf);

            Serial.println(values);
            uint8_t i = 0;
            int from_c = values.indexOf("|");
            cmyk_values_struct.c = values.substring(0,from_c).toInt();
            if (cmyk_values_struct.c > 100) cmyk_values_struct.c = 100;

            int from_m = 0;
            from_m = values.indexOf('|',from_c+1);
            cmyk_values_struct.m = values.substring(from_c+1,from_m).toInt();
            if (cmyk_values_struct.m > 100) cmyk_values_struct.m = 100;

            int from_y = 0;
            from_y = values.indexOf('|',from_m+1);
            cmyk_values_struct.y = values.substring(from_m+1,from_y).toInt();
            if (cmyk_values_struct.y > 100) cmyk_values_struct.y = 100;

            int from_k = values.lastIndexOf("|");
            cmyk_values_struct.k = values.substring(from_k+1,values.length()).toInt();
            if (cmyk_values_struct.k > 100) cmyk_values_struct.k = 100;

            //TODO: cmyk passando de 100%. acertar o 100 quando > 100

            Serial.println("-=-=-=-=");
            Serial.println(cmyk_values_struct.c);
            lv_slider_set_value(slider_cmyk_c,cmyk_values_struct.c, LV_ANIM_OFF);
            lv_label_set_text(slider_label_c,String(cmyk_values_struct.c).c_str());
            Serial.println(cmyk_values_struct.m);
            lv_slider_set_value(slider_cmyk_m,cmyk_values_struct.m, LV_ANIM_OFF);
            lv_label_set_text(slider_label_m,String(cmyk_values_struct.m).c_str());
            Serial.println(cmyk_values_struct.y);
            lv_slider_set_value(slider_cmyk_y,cmyk_values_struct.y, LV_ANIM_OFF);
            lv_label_set_text(slider_label_y,String(cmyk_values_struct.y).c_str());
            Serial.println(cmyk_values_struct.k);
            lv_slider_set_value(slider_cmyk_k,cmyk_values_struct.k, LV_ANIM_OFF);
            lv_label_set_text(slider_label_k,String(cmyk_values_struct.k).c_str());

            Serial.println("-=-=-=-=");

            //ALIMENTAR A LINHA COM A COR CARREGADA

            lv_color_t lvgl_color_format;
            rgb rgb_result;
            rgb_result = cmykConverter.CMYKtoRGB(cmyk_values_struct,rgb_result);
            lvgl_color_format.full = rgb2rgb.RGB24toRGB16(rgb_result.r,rgb_result.g,rgb_result.b);
            lv_style_set_line_color(&style_line_load, LV_STATE_DEFAULT, lvgl_color_format);
            lv_style_set_line_color(&style_line_cmyk, LV_STATE_DEFAULT, lvgl_color_format);

            lv_obj_t * msgBox = lv_msgbox_create(lv_scr_act(), NULL);
                    lv_msgbox_set_text(msgBox, "Carregado. Veja a amostra a direita\n ou abra o menu CMYK");
                    lv_obj_set_width(msgBox, 180);
                    lv_msgbox_start_auto_close(msgBox, 2000);
                    lv_obj_align(msgBox, NULL, LV_ALIGN_CENTER, 0, 0);
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

static void lv_spinbox_increment_event_rgb_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox_ink_volume_rgb);
        spinbox_ink_volume_value = spinbox_ink_volume_value <500 ? spinbox_ink_volume_value+1 : spinbox_ink_volume_value;
        //int32_t value = lv_spinbox_get_value(spinbox)/10;
        lv_spinbox_set_value(spinbox_ink_volume_rgb,spinbox_ink_volume_value*10);
        Serial.println("spin value: ");
        Serial.println(spinbox_ink_volume_value);
    }
}

static void lv_spinbox_decrement_event_rgb_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox_ink_volume_rgb);
        //int32_t value = lv_spinbox_get_value(spinbox)/10;
        spinbox_ink_volume_value = spinbox_ink_volume_value >0 ? spinbox_ink_volume_value-1 : spinbox_ink_volume_value;
        lv_spinbox_set_value(spinbox_ink_volume_rgb,spinbox_ink_volume_value*10);
        Serial.println("spin value: ");
        Serial.println(spinbox_ink_volume_value);
    }
}

static void lv_spinbox_increment_event_load_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox_ink_volume_load);
        spinbox_ink_volume_value = spinbox_ink_volume_value <500 ? spinbox_ink_volume_value+1 : spinbox_ink_volume_value;
        //int32_t value = lv_spinbox_get_value(spinbox)/10;
        lv_spinbox_set_value(spinbox_ink_volume_load,spinbox_ink_volume_value*10);
        Serial.println("spin value: ");
        Serial.println(spinbox_ink_volume_value);
    }
}

static void lv_spinbox_decrement_event_load_cb(lv_obj_t * btn, lv_event_t e)
{
    if(e == LV_EVENT_SHORT_CLICKED || e == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox_ink_volume_load);
        //int32_t value = lv_spinbox_get_value(spinbox)/10;
        spinbox_ink_volume_value = spinbox_ink_volume_value >0 ? spinbox_ink_volume_value-1 : spinbox_ink_volume_value;
        lv_spinbox_set_value(spinbox_ink_volume_load,spinbox_ink_volume_value*10);
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

//callback do switch do wifi
static void event_handler_wifi_switch(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        unused = change_wifi_to();
        static const char * btns[] ={"Yes", "No", ""};
        msgBoxDel = lv_msgbox_create(lv_scr_act(), NULL);
        lv_msgbox_set_text(msgBoxDel, "Reiniciar sistema agora?");
        lv_msgbox_add_btns(msgBoxDel, btns);
        lv_obj_set_width(msgBoxDel, 200);
        lv_obj_set_event_cb(msgBoxDel, reboot_cb);
        lv_obj_align(msgBoxDel, NULL, LV_ALIGN_CENTER, 0, 0);
    }
}

static void event_handler_login_switch(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        unused = change_login_to();
        static const char * btns[] ={"Yes", "No", ""};
        msgBoxDel = lv_msgbox_create(lv_scr_act(), NULL);
        lv_msgbox_set_text(msgBoxDel, "Reiniciar sistema agora?");
        lv_msgbox_add_btns(msgBoxDel, btns);
        lv_obj_set_width(msgBoxDel, 200);
        lv_obj_set_event_cb(msgBoxDel, reboot_cb);
        lv_obj_align(msgBoxDel, NULL, LV_ALIGN_CENTER, 0, 0);
    }
}

//callback do switch do socket
static void event_handler_socket_switch(lv_obj_t * obj, lv_event_t event){
    if(event == LV_EVENT_VALUE_CHANGED) {
        unused = change_socket_to();
        static const char * btns[] ={"Yes", "No", ""};
        msgBoxDel = lv_msgbox_create(lv_scr_act(), NULL);
        lv_msgbox_set_text(msgBoxDel, "Reiniciar sistema agora?");
        lv_msgbox_add_btns(msgBoxDel, btns);
        lv_obj_set_width(msgBoxDel, 200);
        lv_obj_set_event_cb(msgBoxDel, reboot_cb);
        lv_obj_align(msgBoxDel, NULL, LV_ALIGN_CENTER, 0, 0);
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

            char buf[20];
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
    //TODO: checar se o volume em ml é 0, então não inicia
    
    if(event == LV_EVENT_CLICKED) {
        if (spinbox_ink_volume_value > 0){
            printf("botao start\n");

            for (int j=0; j<4;j++){
                xTaskCreatePinnedToCore(pump,labels[j],10000,(void*) j,0,&pump_params.handles[j],0);
            }
        }
        else if(event == LV_EVENT_VALUE_CHANGED) {
            printf("Toggled\n");
        }
        else{
            lv_obj_t * msgBoxVolumeZero = lv_msgbox_create(lv_scr_act(), NULL);
            lv_msgbox_set_text(msgBoxVolumeZero, "Defina um volume para extracao (ML)");
            lv_obj_set_width(msgBoxVolumeZero, 180);
            lv_msgbox_start_auto_close(msgBoxVolumeZero, 3000);
            lv_obj_align(msgBoxVolumeZero, NULL, LV_ALIGN_CENTER, 0, 0);
        }
    }
}


//=================== FIM CALLBACKS ========================
static void list_cb(lv_obj_t * obj, lv_event_t event){
    if (event == LV_EVENT_CLICKED){
        lv_obj_t * label_target = lv_list_get_btn_label(obj);
        String txt_label = lv_label_get_text(label_target);
        
        Serial.println(txt_label);
    }
}

void list_of_files_roller_files(){
    String result;
    result = getFilenames(SPIFFS,"/",1);
    result.replace("/","");
    result.replace("|","\n");

    Serial.print("RESULT: ");
    Serial.println(result);
    Serial.println("====");

    char items_to_list[300];
    memset(items_to_list,0,sizeof(items_to_list));
    result.toCharArray(items_to_list,sizeof(items_to_list));
    items_to_list[0] = ' ';


    //Create a roller
    roller_files = lv_roller_create(tab_info, NULL);
    lv_roller_set_options(roller_files, items_to_list, LV_ROLLER_MODE_INIFINITE);

    lv_roller_set_fix_width(roller_files,150);
    lv_roller_set_visible_row_count(roller_files, 3);
    lv_obj_align(roller_files, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, -16);

    list_files_created = true;
    //TODO: callback
/*
    char *it;
    it = strtok(items_to_list,"|");
    
    while (it != NULL){
        Serial.println(it);
        list_ev = lv_list_add_btn(target, NULL, it);
        lv_obj_set_event_cb(list_ev, list_cb);
        it = strtok(NULL,"|");     
    }*/
}

void list_of_patterns(){
    String result;
    result = getFilenames(SPIFFS,"/",1);
     Serial.println(result);
    Serial.println("^^^^^");
    result.replace("/","");
    result.replace("|","\n");

    char items_to_list[300];
    memset(items_to_list,0,sizeof(items_to_list));
    result.toCharArray(items_to_list,sizeof(items_to_list));
    items_to_list[0] = ' ';
   

     //Ordenando a lista    
    char ordered_list[300];
    memset(ordered_list,0,sizeof(ordered_list));

    uint8_t max_items = 10;

    char file_pattern[5];
    memset(file_pattern,0,sizeof(file_pattern));
    uint8_t k = 0;
    String content = result;
    for (uint8_t i=1;i<max_items;i++){
        int pos = content.indexOf(String(i).c_str());
        if (pos > -1){
            for (uint8_t j=pos-2;j<pos+1;j++){
                file_pattern[k] = result[j];
                k++;
            }
            file_pattern[3] = '\n'; 
            strcat(ordered_list,file_pattern);       
            k=0;
            memset(file_pattern,0,sizeof(file_pattern));
        }
    }
    Serial.print("ordered_list: ");
    Serial.print(ordered_list);

    //Create a roller
    roller_patterns = lv_roller_create(color_load, NULL);
    lv_roller_set_options(roller_patterns, ordered_list, LV_ROLLER_MODE_INIFINITE); //TODO: reload

    lv_roller_set_fix_width(roller_patterns,174);
    lv_roller_set_visible_row_count(roller_patterns, 3);
    lv_obj_align(roller_patterns, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    list_created = true;
    //TODO: callback
/*
    char *it;
    it = strtok(items_to_list,"|");
    
    while (it != NULL){
        Serial.println(it);
        list_ev = lv_list_add_btn(target, NULL, it);
        lv_obj_set_event_cb(list_ev, list_cb);
        it = strtok(NULL,"|");     
    }*/
}

bool can_start_wifi(){
    String st = readFile(SPIFFS, WIFI_FILE);
    Serial.print("Conteudo do wifi.ini: ");
    Serial.println(st);
    if ( st == ""){
        return true;
    }
    else if (st == "disable"){
        //esta desativado
        return false;
    }
    else{
        //deve ser enable. outra condicao cai aqui
        return true;
    }
}

bool can_start_socket(){
    String st = readFile(SPIFFS, SOCK_FILE);
    Serial.print("Conteudo do socket.ini");
    Serial.println(st);
    if ( st == ""){
        return true;
    }
    else if (st == "disable"){
        //esta desativado
        return false;
    }
    else{
        //deve ser enable. outra condicao cai aqui
        return true;
    }
}

bool can_start_login(){ //TODO: chamar no loginScreen
    String st = readFile(SPIFFS, ENABLE_LOGIN_FILE);
    Serial.print("Conteudo do loginE.ini");
    Serial.println(st);
    if ( st == ""){
        return true;
    }
    else if (st == "disable"){
        //esta desativado
        return false;
    }
    else{
        //deve ser enable. outra condicao cai aqui
        return true;
    }
}

bool change_wifi_to(){
    String st = readFile(SPIFFS, WIFI_FILE);
    if ( st == ""){
        //nao existe, entao o padrao é enable. desativar
        writeFile(SPIFFS, WIFI_FILE, "disable");
        return false;
    }
    else if (st == "disable"){
        //esta desativado, entao ativa
        deleteFile(SPIFFS, WIFI_FILE);
        writeFile(SPIFFS, WIFI_FILE, "enable");
        return true;
    }
    else{
        //deve ser enable, entao desativa. outra condicao cai aqui
        deleteFile(SPIFFS, WIFI_FILE);
        writeFile(SPIFFS, WIFI_FILE, "disable");
        return false;
    }
}

bool change_login_to(){
    String st = readFile(SPIFFS, ENABLE_LOGIN_FILE);
    if ( st == ""){
        //nao existe, entao o padrao é enable. desativar
        writeFile(SPIFFS, ENABLE_LOGIN_FILE, "disable");
        return false;
    }
    else if (st == "disable"){
        //esta desativado, entao ativa
        deleteFile(SPIFFS, ENABLE_LOGIN_FILE);
        writeFile(SPIFFS, ENABLE_LOGIN_FILE, "enable");
        return true;
    }
    else{
        //deve ser enable, entao desativa. outra condicao cai aqui
        deleteFile(SPIFFS, ENABLE_LOGIN_FILE);
        writeFile(SPIFFS, ENABLE_LOGIN_FILE, "disable");
        return false;
    }
}

bool change_socket_to(){
    String st = readFile(SPIFFS, SOCK_FILE);
    if ( st == ""){
        //nao existe, entao o padrao é enable. desativar
        writeFile(SPIFFS, SOCK_FILE, "disable");
        return false;
    }
    else if (st == "disable"){
        //esta desativado, entao ativa
        deleteFile(SPIFFS, SOCK_FILE);
        writeFile(SPIFFS, SOCK_FILE, "enable");
        return true;
    }
    else{
        //deve ser enable, entao desativa. outra condicao cai aqui
        deleteFile(SPIFFS, SOCK_FILE);
        writeFile(SPIFFS, SOCK_FILE, "disable");
        return false;
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

void colorToSampleLineRGB(){
    lv_color_t lvgl_color_format;
            
    lvgl_color_format.full = rgb2rgb.RGB24toRGB16(rgb_from_sliders.r,rgb_from_sliders.g,rgb_from_sliders.b);
    lv_style_set_line_color(&style_line_rgb, LV_STATE_DEFAULT, lvgl_color_format);
}

void sliders_cmyk(lv_obj_t target){

    slider_cmyk_c = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_cmyk_c, 130);
    lv_obj_align(slider_cmyk_c, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);
    lv_obj_set_event_cb(slider_cmyk_c, slider_c_cb);
    lv_slider_set_range(slider_cmyk_c, 0, 110);

    slider_label_c = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_c, "0%");
    lv_obj_set_auto_realign(slider_label_c, true);
    lv_obj_align(slider_label_c, slider_cmyk_c, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    slider_cmyk_m = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_cmyk_m, 130);
    lv_obj_align(slider_cmyk_m, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 50);
    lv_obj_set_event_cb(slider_cmyk_m, slider_m_cb);
    lv_slider_set_range(slider_cmyk_m, 0, 110);

    slider_label_m = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_m, "0%");
    lv_obj_set_auto_realign(slider_label_m, true);
    lv_obj_align(slider_label_m, slider_cmyk_m, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    slider_cmyk_y = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_cmyk_y, 130);
    lv_obj_align(slider_cmyk_y, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 90);
    lv_obj_set_event_cb(slider_cmyk_y, slider_y_cb);
    lv_slider_set_range(slider_cmyk_y, 0, 110);

    slider_label_y = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_y, "0%");
    lv_obj_set_auto_realign(slider_label_y, true);
    lv_obj_align(slider_label_y, slider_cmyk_y, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    slider_cmyk_k = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_cmyk_k, 130);
    lv_obj_align(slider_cmyk_k, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 130);
    lv_obj_set_event_cb(slider_cmyk_k, slider_k_cb);
    lv_slider_set_range(slider_cmyk_k, 0, 110);

    slider_label_k = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_k, "0%");
    lv_obj_set_auto_realign(slider_label_k, true);
    lv_obj_align(slider_label_k, slider_cmyk_k, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
      
}

void sliders_rgb(lv_obj_t target){
    //RED
    lv_obj_t * slider_rgb_r = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_rgb_r, 130);
    lv_obj_align(slider_rgb_r, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 50);
    lv_obj_set_event_cb(slider_rgb_r, slider_red_cb);
    lv_slider_set_range(slider_rgb_r, 0, 270);

    slider_label_red = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_red, "0");
    lv_obj_set_auto_realign(slider_label_red, true);
    lv_obj_align(slider_label_red, slider_rgb_r, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    //GREEN
    lv_obj_t * slider_rgb_g = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_rgb_g, 130);
    lv_obj_align(slider_rgb_g, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 90);
    lv_obj_set_event_cb(slider_rgb_g, slider_green_cb);
    lv_slider_set_range(slider_rgb_g, 0, 270);

    slider_label_green = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_green, "0");
    lv_obj_set_auto_realign(slider_label_green, true);
    lv_obj_align(slider_label_green, slider_rgb_g, LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    //BLUE
    lv_obj_t * slider_rgb_b = lv_slider_create(&target, NULL);
    lv_obj_set_width(slider_rgb_b, 130);
    lv_obj_align(slider_rgb_b, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 130);
    lv_obj_set_event_cb(slider_rgb_b, slider_blue_cb);
    lv_slider_set_range(slider_rgb_b, 0, 270);

    slider_label_blue = lv_label_create(&target, NULL);
    lv_label_set_text(slider_label_blue, "0");
    lv_obj_set_auto_realign(slider_label_blue, true);
    lv_obj_align(slider_label_blue, slider_rgb_b, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
      
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

void loadSample(){ //TODO: carregar a cor do loadSample e do cmykSample quando carregada uma amostra
    static lv_point_t line_points[] = { {0, 0}, {0, 95}};
    lv_style_init(&style_line_load);
    lv_style_set_line_width(&style_line_load, LV_STATE_DEFAULT, 5);
    lv_style_set_line_color(&style_line_load, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_line_rounded(&style_line_load, LV_STATE_DEFAULT, false);

    /*Create a line and apply the new style*/
    
    line_load = lv_line_create(color_load, NULL);
    lv_line_set_points(line_load, line_points, 2);     /*Set the points*/
    lv_obj_add_style(line_load, LV_LINE_PART_MAIN, &style_line_load);     /*Set the points*/
    lv_obj_align(line_load, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 10);
}

void rgbSample(){
    static lv_point_t line_points[] = { {0, 0}, {0, 130}};
    lv_style_init(&style_line_rgb);
    lv_style_set_line_width(&style_line_rgb, LV_STATE_DEFAULT, 5);
    lv_style_set_line_color(&style_line_rgb, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_line_rounded(&style_line_rgb, LV_STATE_DEFAULT, false);

    /*Create a line and apply the new style*/
    
    line_rgb = lv_line_create(color_rgb, NULL);
    lv_line_set_points(line_rgb, line_points, 2);     /*Set the points*/
    lv_obj_add_style(line_rgb, LV_LINE_PART_MAIN, &style_line_rgb);     /*Set the points*/
    lv_obj_align(line_rgb, NULL, LV_ALIGN_IN_TOP_RIGHT, -15, 10);
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

void mtx_load_color(lv_obj_t  target){
    load_color_mtx_btn = lv_btnmatrix_create(&target, NULL);
    lv_btnmatrix_set_map(load_color_mtx_btn, load_btns);

    lv_obj_align(load_color_mtx_btn, &target, LV_ALIGN_IN_BOTTOM_LEFT, 0, 38);

    lv_obj_set_height(load_color_mtx_btn,40);
    lv_obj_set_width(load_color_mtx_btn,204);

    lv_btnmatrix_set_one_check(load_color_mtx_btn, true);
    //lv_btnmatrix_set_focused_btn(load_color_mtx_btn,0);
    
    lv_btnmatrix_set_btn_ctrl_all(load_color_mtx_btn, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_obj_set_event_cb(load_color_mtx_btn, load_matrix_button_cb);
}

void mtx_load_files_btn(lv_obj_t  target){
    load_files_mtx_btn = lv_btnmatrix_create(&target, NULL);
    lv_btnmatrix_set_map(load_files_mtx_btn, load_btns_files);

    lv_obj_align(load_files_mtx_btn, &target, LV_ALIGN_IN_BOTTOM_RIGHT, 198, -6);

    lv_obj_set_height(load_files_mtx_btn,120);
    lv_obj_set_width(load_files_mtx_btn,50);

    lv_btnmatrix_set_one_check(load_files_mtx_btn, true);
    //lv_btnmatrix_set_focused_btn(load_color_mtx_btn,0);
    
    lv_btnmatrix_set_btn_ctrl_all(load_files_mtx_btn, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_obj_set_event_cb(load_files_mtx_btn, load_matrix_button_roller_files_cb);
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

//TODO: callbacks faltando
void spinbox_ink_volumeRGB(lv_obj_t target){
    spinbox_ink_volume_rgb = lv_spinbox_create(&target, NULL);
    lv_spinbox_set_range(spinbox_ink_volume_rgb, +0, 500);
    lv_spinbox_set_digit_format(spinbox_ink_volume_rgb, 4, 3);
    lv_spinbox_set_step(spinbox_ink_volume_rgb,10);
    lv_spinbox_step_prev(spinbox_ink_volume_rgb);
    lv_obj_set_width(spinbox_ink_volume_rgb, 120);
    lv_obj_align(spinbox_ink_volume_rgb, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 5, -13);
    lv_spinbox_set_padding_left(spinbox_ink_volume_rgb,3);
    lv_obj_set_style_local_margin_left(spinbox_ink_volume_rgb, LV_SPINBOX_PART_BG, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_margin_right(spinbox_ink_volume_rgb, LV_SPINBOX_PART_BG, LV_STATE_DEFAULT, 0);

    lv_coord_t h = lv_obj_get_height(spinbox_ink_volume_rgb);
    lv_obj_t * btn = lv_btn_create(&target, NULL);
    lv_obj_set_size(btn, h, h);
    lv_obj_align(btn, spinbox_ink_volume_rgb, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    lv_theme_apply(btn, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(btn, lv_spinbox_increment_event_rgb_cb);

    btn = lv_btn_create(&target, btn);
    lv_obj_align(btn, spinbox_ink_volume_rgb, LV_ALIGN_IN_BOTTOM_LEFT, -5, 0);
    lv_obj_set_event_cb(btn, lv_spinbox_decrement_event_rgb_cb);
    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_MINUS);
}
void spinbox_ink_volumeLoad(lv_obj_t target){
    spinbox_ink_volume_load = lv_spinbox_create(&target, NULL);
    lv_spinbox_set_range(spinbox_ink_volume_load, +0, 500);
    lv_spinbox_set_digit_format(spinbox_ink_volume_load, 4, 3);
    lv_spinbox_set_step(spinbox_ink_volume_load,10);
    lv_spinbox_step_prev(spinbox_ink_volume_load);
    lv_obj_set_width(spinbox_ink_volume_load, 120);
    lv_obj_align(spinbox_ink_volume_load, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 5, -13);
    lv_spinbox_set_padding_left(spinbox_ink_volume_load,3);
    lv_obj_set_style_local_margin_left(spinbox_ink_volume_load, LV_SPINBOX_PART_BG, LV_STATE_DEFAULT, 4);
    lv_obj_set_style_local_margin_right(spinbox_ink_volume_load, LV_SPINBOX_PART_BG, LV_STATE_DEFAULT, 0);

    lv_coord_t h = lv_obj_get_height(spinbox_ink_volume_load);
    lv_obj_t * btn = lv_btn_create(&target, NULL);
    lv_obj_set_size(btn, h, h);
    lv_obj_align(btn, spinbox_ink_volume_load, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    lv_theme_apply(btn, LV_THEME_SPINBOX_BTN);
    lv_obj_set_style_local_value_str(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);
    lv_obj_set_event_cb(btn, lv_spinbox_increment_event_load_cb);

    btn = lv_btn_create(&target, btn);
    lv_obj_align(btn, spinbox_ink_volume_load, LV_ALIGN_IN_BOTTOM_LEFT, -5, 0);
    lv_obj_set_event_cb(btn, lv_spinbox_decrement_event_load_cb);
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

//info
void infoWiFi(lv_obj_t target){
    IPAddress myIP;
    if (is_mode_ap){
        myIP = WiFi.softAPIP();
    }
    else{
        myIP = WiFi.localIP();
    }
    

    String ipaddr = "Address: " + String(myIP.toString().c_str());
    lv_obj_t *label_ip = lv_label_create(&target, NULL); 
    lv_label_set_text(label_ip, ipaddr.c_str());
    lv_obj_align(label_ip, NULL, LV_ALIGN_IN_TOP_LEFT, 0, txt_info_dist);

    String str_buf;
    //SSID - botao Change ao lado
    lv_obj_t *label_ssid = lv_label_create(&target, NULL);
    str_buf      = "SSID: " + String(SSID); 
    lv_label_set_text(label_ssid, str_buf.c_str());
    txt_info_dist += 20;
    lv_obj_align(label_ssid, NULL, LV_ALIGN_IN_TOP_LEFT, 0, txt_info_dist);

    //PASSWD - botao Change ao lado
    lv_obj_t *label_passwd = lv_label_create(&target, NULL);
    str_buf      = "Passwd: " + String(PASSWD); 
    lv_label_set_text(label_passwd, str_buf.c_str());
    txt_info_dist += 20;
    lv_obj_align(label_passwd, NULL, LV_ALIGN_IN_TOP_LEFT, 0, txt_info_dist);
    
    //MODE
    lv_obj_t *label_mode = lv_label_create(&target, NULL);
    if (is_mode_ap){
        str_buf = "Mode: Access Point"; 
    }
    else{
        str_buf      = "Mode: Station"; 
    }
    txt_info_dist += 20;
    lv_label_set_text(label_mode, str_buf.c_str());
    lv_obj_align(label_mode, NULL, LV_ALIGN_IN_TOP_LEFT, 0, txt_info_dist);
}

void infoSock(lv_obj_t target){
    txt_info_dist += 20;
    if(can_start_socket()){
        lv_obj_t *label_sock = lv_label_create(&target, NULL);
        lv_label_set_text(label_sock, "Listen to ColorPicker: YES");
        lv_obj_align(label_sock, NULL, LV_ALIGN_IN_TOP_LEFT, 0, txt_info_dist);
    }
    else{
        lv_obj_t *label_sock = lv_label_create(&target, NULL);
        lv_label_set_text(label_sock, "Listen to ColorPicker: NO");
        lv_obj_align(label_sock, NULL, LV_ALIGN_IN_TOP_LEFT, 0, txt_info_dist);
    }
}

//setup
void setupCredentials(lv_obj_t target){
    //escrever /credentials.ini
    //modificar inicializacao para 'if /credentials.ini exists'

}

void setupCalibratePump(lv_obj_t target){

}

void setupEnableSock(lv_obj_t target){
    //TODO: criar label esquerdo e direito
    //TODO: testar a mudança de estado sem acionar o callback
    //TODO: mudar label  Listen do info
    lv_obj_t *label_sock_title = lv_label_create(&target, NULL);
    lv_label_set_text(label_sock_title, "Socket listener");
    lv_obj_align(label_sock_title, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 0);

    lv_obj_t *label_sock_off = lv_label_create(&target, NULL);
    lv_label_set_text(label_sock_off, "OFF");
    lv_obj_align(label_sock_off, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 25);


    lv_obj_t *switch_socket = lv_switch_create(&target, NULL);
    lv_obj_align(switch_socket, NULL, LV_ALIGN_IN_TOP_LEFT, 40, 20);

    lv_obj_t *label_sock_on = lv_label_create(&target, NULL);
    lv_label_set_text(label_sock_on, "ON");
    lv_obj_align(label_sock_on, NULL, LV_ALIGN_IN_TOP_LEFT, 96, 25);

    if (can_start_socket()){
        lv_switch_toggle(switch_socket, LV_ANIM_OFF);
    }

    lv_obj_set_event_cb(switch_socket, event_handler_socket_switch);
}

void setupChangeLogin(lv_obj_t target){

}

void setupEnableLogin(lv_obj_t target){
    lv_obj_t *label_title = lv_label_create(&target, NULL);
    lv_label_set_text(label_title, "Enable login");
    lv_obj_align(label_title, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 110);

    lv_obj_t *label_login_off = lv_label_create(&target, NULL);
    lv_label_set_text(label_login_off, "OFF");
    lv_obj_align(label_login_off, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 135);


    lv_obj_t *switch_login = lv_switch_create(&target, NULL);
    lv_obj_align(switch_login, NULL, LV_ALIGN_IN_TOP_LEFT, 40, 130);

    lv_obj_t *label_login_on = lv_label_create(&target, NULL);
    lv_label_set_text(label_login_on, "ON");
    lv_obj_align(label_login_on, NULL, LV_ALIGN_IN_TOP_LEFT, 96, 135);

    if (can_start_login()){
        lv_switch_toggle(switch_login, LV_ANIM_OFF);
    }

    lv_obj_set_event_cb(switch_login, event_handler_login_switch);
}

void setupDisableWiFi(lv_obj_t target){
    //TODO: criar label esquerdo e direito
    //TODO: testar a mudança de estado sem acionar o callback
    //TODO: mudar label  Listen do info
    lv_obj_t *label_sock_title = lv_label_create(&target, NULL);
    lv_label_set_text(label_sock_title, "Enable WiFi");
    lv_obj_align(label_sock_title, NULL, LV_ALIGN_IN_TOP_LEFT, 2, 55);

    lv_obj_t *label_sock_off = lv_label_create(&target, NULL);
    lv_label_set_text(label_sock_off, "OFF");
    lv_obj_align(label_sock_off, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 80);


    lv_obj_t *switch_wifi = lv_switch_create(&target, NULL);
    lv_obj_align(switch_wifi, NULL, LV_ALIGN_IN_TOP_LEFT, 40, 75);

    lv_obj_t *label_sock_on = lv_label_create(&target, NULL);
    lv_label_set_text(label_sock_on, "ON");
    lv_obj_align(label_sock_on, NULL, LV_ALIGN_IN_TOP_LEFT, 96, 80);

    if (can_start_wifi()){
        lv_switch_toggle(switch_wifi, LV_ANIM_OFF);
    }

    lv_obj_set_event_cb(switch_wifi, event_handler_wifi_switch);
}

void setupWiFiMode(lv_obj_t target){

}

void loginScreen(){
    //cria a tela para receber os widgets
    lv_obj_t * page = lv_page_create(lv_scr_act(), NULL); //a página é criada na janela principal
    lv_obj_set_size(page, DISPLAY_WIDTH, DISPLAY_HEIGHT); // ocupa o tamanho total da tela
    lv_page_set_scrlbar_mode(page, LV_SCRLBAR_MODE_OFF); //desabilita o scrollbar
    lv_obj_align(page, NULL, LV_ALIGN_CENTER, 0, 0); //alinha ao centro
    //Cria um array com os labels para os botões. Como explicado no artigo anterior, LF cria nova linha e "" finaliza.
    static const char * keypad[] = {"A","B","C","\n",
                                    "D","E","F","\n",
                                    "1","2","3","\n",
                                    "4","5","6","\n",
                                    "7","8","9","\n",
                                    "x","0",">",""};
    lv_obj_t * btnm1 = lv_btnmatrix_create(page, NULL); //cria um objeto do tipo lv_obj_t com o widget de matriz de botões
    lv_btnmatrix_set_map(btnm1, keypad); //configura o array de botões                               
    lv_obj_align(btnm1, NULL, LV_ALIGN_IN_BOTTOM_MID, -8, -86); //alinha o array de botões na base
    lv_obj_set_width(btnm1,230); //uma leve "apertadinha" na largura para sobrar uns pixels na tela
    lv_obj_set_height(btnm1,226);
    lv_obj_set_event_cb(btnm1, event_handler_passwd); //define a função de callback
    //label que exibirá a senha. Veremos sobre os símbolos disponíveis também
    labelB = lv_label_create(page, NULL);
    lv_label_set_text(labelB, LV_SYMBOL_EYE_OPEN "   (senha visivel)"); //https://docs.lvgl.io/v7/en/html/overview/font.html
    lv_obj_align(labelB, NULL, LV_ALIGN_IN_TOP_MID, -5, 20); //alinhamento
    
}

void tabs(){
    /* CRIA UM OBJETO TABVIEW */
    tabview = lv_tabview_create(lv_scr_act(), NULL); //TABVIEW PRINCIPAL

    /* tabs principais */
    lv_obj_t *tab_color = lv_tabview_add_tab(tabview, "Color");
    lv_obj_t *tab_system = lv_tabview_add_tab(tabview, "System");
    lv_obj_t *tab_about = lv_tabview_add_tab(tabview, "About");

    lv_obj_t *tabview_color;
    tabview_color =  lv_tabview_create(tab_color, NULL); //TABVIEW COLORS NA ABA COLORS

    lv_obj_align(tabview_color, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_local_pad_bottom(tabview,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,4);
    lv_obj_set_style_local_pad_bottom(tabview_color,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,4);

    lv_obj_set_style_local_pad_top(tabview,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,4);
    lv_obj_set_style_local_pad_top(tabview_color,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,1);

    //tabs secundárias - Color
    color_cmyk = lv_tabview_add_tab(tabview_color, "CMYK");
    color_rgb  = lv_tabview_add_tab(tabview_color, "RGB");
    color_hsv  = lv_tabview_add_tab(tabview_color, "HSV");
    color_load = lv_tabview_add_tab(tabview_color, "Save");

    lv_tabview_set_btns_pos(tabview_color, LV_TABVIEW_TAB_POS_BOTTOM);

    //lv_obj_set_event_cb(tabview, tab_feeding_spinbox_cb);


    /* UMA ANIMAÇÃO SIMPLES (OPCIOINAL)*/
    lv_tabview_set_anim_time(tabview, 1000);

    //--------------------------TAB 1: COLORS-------------------------------------------
    

    //--------------------------TAB 2: SYSTEM-------------------------------------------
    //lv_obj_t *l2 = lv_label_create(tab_setup, NULL);
    //lv_label_set_text(l2, "Aba System");
    //lv_obj_align(l2, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);

    //tabs secundárias - System
    lv_obj_t *tabview_system;
    tabview_system =  lv_tabview_create(tab_system, NULL);

    lv_obj_align(tabview_system, NULL, LV_ALIGN_CENTER, 0, 0);

    tab_info   = lv_tabview_add_tab(tabview_system, "Info");
    tab_setup  = lv_tabview_add_tab(tabview_system, "Setup");

    lv_obj_set_style_local_pad_bottom(tabview,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,4);
    lv_obj_set_style_local_pad_bottom(tabview_system,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,4);

    lv_obj_set_style_local_pad_top(tabview,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,4);
    lv_obj_set_style_local_pad_top(tabview_system,LV_TABVIEW_PART_TAB_BTN,LV_STATE_DEFAULT,1);

    lv_tabview_set_btns_pos(tabview_system, LV_TABVIEW_TAB_POS_BOTTOM);

    list_of_files_roller_files();
    mtx_load_files_btn(*tab_info);

    setupEnableSock(*tab_setup);
    setupDisableWiFi(*tab_setup);
    setupEnableLogin(*tab_setup);


     /* UMA ANIMAÇÃO SIMPLES (OPCIOINAL)*/
    lv_tabview_set_anim_time(tabview, 1000);
    //--------------------------TAB 3: ABOUT-------------------------------------------
    lv_obj_t *l3 = lv_label_create(tab_about, NULL);
    String txtAbout = " Author: Djames Suhanko\n";;
    txtAbout = txtAbout + "www.dobitaobyte.com.br\n\n";
    txtAbout = txtAbout + "   Board: AFSmartControl\n";
    txtAbout = txtAbout + "      afeletronica.com.br\n\n";
    txtAbout = txtAbout + "        Display: ILI9341 2.4\n\n";
    txtAbout = txtAbout + "lv_arduino\n";
    txtAbout = txtAbout + "TFT_eSPI\n";
    txtAbout = txtAbout + "EasyColor\n";
    txtAbout = txtAbout + "EasyPCF8574\n";
    txtAbout = txtAbout +  "           ColorMixer 2.0";
    lv_label_set_text(l3, txtAbout.c_str());
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

    //RGB
    sliders_rgb(*color_rgb);
    start_button(*color_rgb);
    spinbox_ink_volumeRGB(*color_rgb);
    rgbSample();

    start_button(*color_load);
    spinbox_ink_volumeLoad(*color_load);
    mtx_load_color(*color_load);
    list_of_patterns();
    loadSample();

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
    lv_obj_set_event_cb(btn_logoff, event_logoff);

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
    vSemaphoreCreateBinary(myMutex);

    hsv_values.h = 0;
    hsv_values.s = 100;
    hsv_values.v = 100;

    cmyk_values_struct.c = 0;
    cmyk_values_struct.m = 0;
    cmyk_values_struct.y = 0;
    cmyk_values_struct.k = 0;

    Serial.begin(115200);

    //SPIFFS.format();
    if(!SPIFFS.begin(true)){
        Serial.println("SPIFFS Mount Failed");
        while (true);
    }

    //if (!pcfSmart.startI2C(21,22)){
    //    Serial.println("Not started. Check pin and address.");
     //   while (true);
    //}
    Wire.begin(21,22);
    Wire.beginTransmission(PCF_ADDR);
    Wire.write(0xFF);
    Wire.endTransmission();

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

    cmyk_values_struct.c = 0;
    cmyk_values_struct.m = 0;
    cmyk_values_struct.y = 0;
    cmyk_values_struct.k = 0;

    if (can_start_login()){
        loginScreen();
    }
    else{
        tabs();
        delay(2000);
        infoWiFi(*tab_info);
        infoSock(*tab_info);
    }
    
    /* Essa tarefa recebe os valores CMYK do picker e atribui à variável
    values[n]. Fazendo isso, automaticamente a interface será atualizada.
    O início da mistura só pode ser feito pelo botão iniciar para não ter
    risco de ataque pela rede.
    */
}

void loop() {
    lv_task_handler();
    delay(5);
}
//TODO: se ml = 0, o botao de start inicia o msgbox informando e nao executa
void pump(void *pvParameters){
   int color_bit = (int) pvParameters;

   vTaskDelay(pdMS_TO_TICKS(200)); //apenas para entrar em fila com as outras tasks

   xSemaphoreTake(myMutex,portMAX_DELAY);
   int value[4];
   memset(value,0,sizeof(value));
   value[0] = cmyk_values_struct.c;
   value[1] = cmyk_values_struct.m;
   value[2] = cmyk_values_struct.y;
   value[3] = cmyk_values_struct.k;

   if (value[color_bit] > 0){
        pump_params.running += 1; //a partir de agora nenhuma alteração é permitida até voltar a 0.
        pump_params.times[color_bit] = spinbox_ink_volume_value*(value[color_bit])*one_ml/100; //tempo de execução da bomba

        pump_params.pcf_value = pump_params.pcf_value&~(1<<pump_params.pumps_bits[color_bit]); //baixa o bit (liga com 0)

        Wire.beginTransmission(PCF_ADDR); //inicia comunicação i2c no endereço do PCF
        Wire.write(pump_params.pcf_value); //escreve o valor recém modificado
        Wire.endTransmission(); //finaliza a transmissão
        Serial.println(pump_params.times[color_bit]);
   }
   xSemaphoreGive(myMutex);

   vTaskDelay(pdMS_TO_TICKS(pump_params.times[color_bit])); //executa o delay conforme calculado
    
    xSemaphoreTake(myMutex,portMAX_DELAY);
    if (value[color_bit] > 0){
        pump_params.pcf_value = pump_params.pcf_value|(1<<pump_params.pumps_bits[color_bit]);

        Wire.beginTransmission(PCF_ADDR);
        Wire.write(pump_params.pcf_value); 
        Wire.endTransmission();

        pump_params.running -= 1;
        pump_params.times[color_bit] = 0;
    }
    xSemaphoreGive(myMutex);

   vTaskDelete(NULL); //finaliza a task e se exclui
}

void fromPicker(void *pvParameters){
    //Serial.println("Start listening...");

   /* Lógica invertida: o GND é o PCF, portanto o pino deve ser colocado em 0 para 
   acionar os relés (addr: 0x27).
   As tasks são tCyan, tMagent, tYellow e tBlack
   RELES:
    128    64     32     16
   [ C ]  [ M ]  [ Y ]  [ K ]
     7      6      5      4
   */
    uint8_t i = 0;
    uint8_t result[6];
    memset(result,0,sizeof(result));

    while (true){
        WiFiClient client = server.available();
        if (client){
            //Serial.print("RGBarray[0]: ");
            //Serial.println(RGBarray[2]);
            i = 0;
            while (client.connected()){
                //avalia se tem dados e controla o buffer
                if (client.available() && i<6){
                    result[i] = client.read();
                    //Serial.println(result[i]);
                    i = result[0] == 94 ? i+1 : 0;
                    ////Serial.println(result[0]);
                }
            }
            client.stop();
        }

        //TODO: checar se os memset estão verificando os tipos com sizeof - IMPORTANTE
        if (result[0] == 0x5e && pump_params.running == 0){
            int value[4];
            memset(value,0,sizeof(value));
            value[0] = cmyk_values_struct.c;
            value[1] = cmyk_values_struct.m;
            value[2] = cmyk_values_struct.y;
            value[3] = cmyk_values_struct.k;

            for (uint k=0;k<4;k++){
                value[k] = result[k+1]; //esse incremento é porque a msg começa na posição 1 (^CMYK$)
            }
            memset(result,0,sizeof(result));
        }    
    }
}
