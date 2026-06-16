#include <WiFi.h>
#include <HTTPClient.h>
#include "lvgl_port.h"

// ========== CONFIGURACIÓN WiFi ==========
const char *ssid = "MEGACABLE-2.4G-9CAA";
const char *password = "bSYmEa7NHa";

// ========== DIRECCIONES DE LOS SLAVES ==========
const char* slave1_ip = "http://192.168.100.27";
const char* slave2_ip = "http://192.168.100.28";

// ========== CONFIGURACIÓN DE UI ==========
// Colores
#define COLOR_BG         0xFFFFFF  // Blanco
#define COLOR_HEADER_BG  0x1A237E  // Azul oscuro
#define COLOR_HEADER_TEXT 0xFFFFFF // Blanco
#define COLOR_BTN_OFF    0x757575  // Gris
#define COLOR_BTN_ON     0x4CAF50  // Verde
#define COLOR_BTN_TEXT   0xFFFFFF  // Blanco
#define COLOR_WIFI_OK    0x4CAF50  // Verde
#define COLOR_WIFI_ERROR 0xF44336  // Rojo
#define COLOR_WIFI_WAIT  0xFF9800  // Naranja
#define COLOR_TEMP_TEXT  0x333333  // Gris oscuro
#define COLOR_HUM_TEXT   0x333333  // Gris oscuro
#define COLOR_SIDEBAR_BG 0x008000 
#define COLOR_ICON       0x000000 // Negro

// Dimensiones (botones más grandes)
#define BTN_WIDTH  260      // Aumentado de 200 a 260
#define BTN_HEIGHT 90       // Aumentado de 70 a 90
#define HEADER_HEIGHT 60
#define BTN_SPACING 25      // Aumentado de 20 a 25
#define BTN_ROW_SPACING 25  // Aumentado de 15 a 25
#define SIDEBAR_WIDTH 70    // Aumentado de 60 a 70

// Posiciones de los botones (ajustadas por la barra lateral)
// Fila 1 (superior)
#define BTN1_ROW1_X (-BTN_WIDTH - BTN_SPACING + (SIDEBAR_WIDTH/2))
#define BTN2_ROW1_X (0 + (SIDEBAR_WIDTH/2))
#define BTN3_ROW1_X (BTN_WIDTH + BTN_SPACING + (SIDEBAR_WIDTH/2))
// Fila 2 (inferior) - posiciones ajustadas
#define BTN1_ROW2_X (-BTN_WIDTH - BTN_SPACING + (SIDEBAR_WIDTH/2))
#define BTN2_ROW2_X (0 + (SIDEBAR_WIDTH/2))
#define BTN3_ROW2_X (BTN_WIDTH + BTN_SPACING + (SIDEBAR_WIDTH/2))

// Fuentes (ligeramente más grandes)
#define FONT_TITLE    &lv_font_montserrat_32  // Aumentado de 26 a 32
#define FONT_BUTTON   &lv_font_montserrat_18  // Aumentado de 16 a 18
#define FONT_WIFI     &lv_font_montserrat_16
#define FONT_SENSOR   &lv_font_montserrat_24  // Aumentado de 20 a 24
#define FONT_ICON     &lv_font_montserrat_14  // Aumentado de 12 a 14
#define FONT_MAMAWEBO &lv_font_montserrat_40

// Textos
#define TITLE_TEXT    "CONTROL DOMOTICO"
#define WIFI_CONNECTING "WiFi: Conectando..."
#define WIFI_OK_TEXT  "WiFi: Conectado"
#define WIFI_ERROR_TEXT "WiFi: Error"

// Textos de los botones SLAVE 1
#define BTN1_S1_TEXT_ON   "PUERTA ENTRADA 1: ON"
#define BTN1_S1_TEXT_OFF  "PUERTA ENTRADA 1: OFF"
#define BTN2_S1_TEXT_ON   "TRITURADOR: ON"
#define BTN2_S1_TEXT_OFF  "TRITURADOR: OFF"
#define BTN3_S1_TEXT_ON   "LUZ PATIO 1: ON"
#define BTN3_S1_TEXT_OFF  "LUZ PATIO 1: OFF"

// Textos de los botones SLAVE 2
#define BTN2_S2_TEXT_ON   "LUZ COCINA 2: ON"
#define BTN2_S2_TEXT_OFF  "LUZ COCINA 2: OFF"
#define BTN1_S2_TEXT_ON   "VENTILADOR: ON"
#define BTN1_S2_TEXT_OFF  "VENTILADOR: OFF"
#define BTN3_S2_TEXT_ON   "LAVADORA: ON"
#define BTN3_S2_TEXT_OFF  "LAVADORA: OFF"

// Textos de sensores (estáticos)
#define TEMP_TEXT     "Temperatura: 30.5 C"
#define HUM_TEXT      "Humedad: 45%"

// Pines en los slaves
// Slave 1
#define RELE1_S1_PIN   5
#define RELE2_S1_PIN   15
#define RELE3_S1_PIN   16
// Slave 2
#define RELE1_S2_PIN   14
#define RELE2_S2_PIN   27
#define RELE3_S2_PIN   26

// Timeout para HTTP
#define HTTP_TIMEOUT 1000

// ========== VARIABLES GLOBALES ==========
// Botones Slave 1 (fila superior)
static lv_obj_t *btn1_s1;
static lv_obj_t *lbl1_s1;
static lv_obj_t *btn2_s1;
static lv_obj_t *lbl2_s1;
static lv_obj_t *btn3_s1;
static lv_obj_t *lbl3_s1;

// Botones Slave 2 (fila inferior)
static lv_obj_t *btn1_s2;
static lv_obj_t *lbl1_s2;
static lv_obj_t *btn2_s2;
static lv_obj_t *lbl2_s2;
static lv_obj_t *btn3_s2;
static lv_obj_t *lbl3_s2;

static lv_obj_t *wifi_label;
static lv_obj_t *temp_label;
static lv_obj_t *hum_label;
static lv_obj_t *sidebar;

// Estados Slave 1
static bool estado_s1_rele1 = false;
static bool estado_s1_rele2 = false;
static bool estado_s1_rele3 = false;

// Estados Slave 2
static bool estado_s2_rele1 = false;
static bool estado_s2_rele2 = false;
static bool estado_s2_rele3 = false;

static bool wifi_connected = false;
static esp_lcd_touch_handle_t tp_handle = NULL;

// ========== FUNCIÓN PARA CONTROLAR RELE ==========
void controlar_rele(const char* slave_ip, int pin, bool estado) {
  if (!wifi_connected) {
    return;
  }
  
  HTTPClient http;
  String url = String(slave_ip) + "/rele/" + String(pin) + "/" + (estado ? "on" : "off");
  
  http.begin(url);
  http.setTimeout(HTTP_TIMEOUT);
  
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String response = http.getString();
  }
  
  http.end();
  delay(10);
}

// ========== FUNCIÓN DE LECTURA DEL TOUCH ==========
void touch_driver_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    if (tp_handle == NULL) {
        data->state = LV_INDEV_STATE_REL;
        return;
    }
    
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;
    
    esp_lcd_touch_read_data(tp_handle);
    bool touched = esp_lcd_touch_get_coordinates(tp_handle, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);
    
    if (touched && touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

// ========== ACTUALIZAR APARIENCIA DE LOS BOTONES ==========
// Slave 1
void actualizar_boton1_s1() {
  if (estado_s1_rele1) {
    lv_label_set_text(lbl1_s1, BTN1_S1_TEXT_ON);
    lv_obj_set_style_bg_color(btn1_s1, lv_color_hex(COLOR_BTN_ON), 0);
  } else {
    lv_label_set_text(lbl1_s1, BTN1_S1_TEXT_OFF);
    lv_obj_set_style_bg_color(btn1_s1, lv_color_hex(COLOR_BTN_OFF), 0);
  }
}

void actualizar_boton2_s1() {
  if (estado_s1_rele2) {
    lv_label_set_text(lbl2_s1, BTN2_S1_TEXT_ON);
    lv_obj_set_style_bg_color(btn2_s1, lv_color_hex(COLOR_BTN_ON), 0);
  } else {
    lv_label_set_text(lbl2_s1, BTN2_S1_TEXT_OFF);
    lv_obj_set_style_bg_color(btn2_s1, lv_color_hex(COLOR_BTN_OFF), 0);
  }
}

void actualizar_boton3_s1() {
  if (estado_s1_rele3) {
    lv_label_set_text(lbl3_s1, BTN3_S1_TEXT_ON);
    lv_obj_set_style_bg_color(btn3_s1, lv_color_hex(COLOR_BTN_ON), 0);
  } else {
    lv_label_set_text(lbl3_s1, BTN3_S1_TEXT_OFF);
    lv_obj_set_style_bg_color(btn3_s1, lv_color_hex(COLOR_BTN_OFF), 0);
  }
}

// Slave 2
void actualizar_boton1_s2() {
  if (estado_s2_rele1) {
    lv_label_set_text(lbl1_s2, BTN1_S2_TEXT_ON);
    lv_obj_set_style_bg_color(btn1_s2, lv_color_hex(COLOR_BTN_ON), 0);
  } else {
    lv_label_set_text(lbl1_s2, BTN1_S2_TEXT_OFF);
    lv_obj_set_style_bg_color(btn1_s2, lv_color_hex(COLOR_BTN_OFF), 0);
  }
}

void actualizar_boton2_s2() {
  if (estado_s2_rele2) {
    lv_label_set_text(lbl2_s2, BTN2_S2_TEXT_ON);
    lv_obj_set_style_bg_color(btn2_s2, lv_color_hex(COLOR_BTN_ON), 0);
  } else {
    lv_label_set_text(lbl2_s2, BTN2_S2_TEXT_OFF);
    lv_obj_set_style_bg_color(btn2_s2, lv_color_hex(COLOR_BTN_OFF), 0);
  }
}

void actualizar_boton3_s2() {
  if (estado_s2_rele3) {
    lv_label_set_text(lbl3_s2, BTN3_S2_TEXT_ON);
    lv_obj_set_style_bg_color(btn3_s2, lv_color_hex(COLOR_BTN_ON), 0);
  } else {
    lv_label_set_text(lbl3_s2, BTN3_S2_TEXT_OFF);
    lv_obj_set_style_bg_color(btn3_s2, lv_color_hex(COLOR_BTN_OFF), 0);
  }
}

// ========== ACTUALIZAR ESTADO WIFI ==========
void actualizar_label_wifi() {
  if (wifi_connected) {
    lv_label_set_text_fmt(wifi_label, "%s %s", WIFI_OK_TEXT, WiFi.localIP().toString().c_str());
    lv_obj_set_style_text_color(wifi_label, lv_color_hex(COLOR_WIFI_OK), 0);
  } else {
    lv_label_set_text(wifi_label, WIFI_CONNECTING);
    lv_obj_set_style_text_color(wifi_label, lv_color_hex(COLOR_WIFI_WAIT), 0);
  }
}

// ========== EVENTOS DE LOS BOTONES ==========
// Slave 1
static void btn1_s1_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    estado_s1_rele1 = !estado_s1_rele1;
    controlar_rele(slave1_ip, RELE1_S1_PIN, estado_s1_rele1);
    actualizar_boton1_s1();
  }
}

static void btn2_s1_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    estado_s1_rele2 = !estado_s1_rele2;
    controlar_rele(slave1_ip, RELE2_S1_PIN, estado_s1_rele2);
    actualizar_boton2_s1();
  }
}

static void btn3_s1_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    estado_s1_rele3 = !estado_s1_rele3;
    controlar_rele(slave1_ip, RELE3_S1_PIN, estado_s1_rele3);
    actualizar_boton3_s1();
  }
}

// Slave 2
static void btn1_s2_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    estado_s2_rele1 = !estado_s2_rele1;
    controlar_rele(slave2_ip, RELE1_S2_PIN, estado_s2_rele1);
    actualizar_boton1_s2();
  }
}

static void btn2_s2_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    estado_s2_rele2 = !estado_s2_rele2;
    controlar_rele(slave2_ip, RELE2_S2_PIN, estado_s2_rele2);
    actualizar_boton2_s2();
  }
}

static void btn3_s2_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    estado_s2_rele3 = !estado_s2_rele3;
    controlar_rele(slave2_ip, RELE3_S2_PIN, estado_s2_rele3);
    actualizar_boton3_s2();
  }
}

// ========== EVENTOS DE LA BARRA LATERAL ==========
static void icono_casa_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    Serial.println("Icono CASA presionado");
  }
}

static void icono_engrane_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    Serial.println("Icono ENGRANE presionado");
  }
}

static void icono_mas_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    Serial.println("Icono MAS presionado");
  }
}

// ========== CREAR BARRA LATERAL ==========
void crear_sidebar() {
  sidebar = lv_obj_create(lv_scr_act());
  lv_obj_set_size(sidebar, SIDEBAR_WIDTH, lv_obj_get_height(lv_scr_act()));
  lv_obj_align(sidebar, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_set_style_bg_color(sidebar, lv_color_hex(COLOR_SIDEBAR_BG), 0);
  lv_obj_set_style_border_width(sidebar, 0, 0);
  lv_obj_set_style_radius(sidebar, 0, 0);
  lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, 0);
  
  // Icono 1: Casa
  lv_obj_t *icono_casa = lv_btn_create(sidebar);
  lv_obj_set_size(icono_casa, 50, 50);
  lv_obj_align(icono_casa, LV_ALIGN_TOP_MID, 0, 30);
  lv_obj_add_event_cb(icono_casa, icono_casa_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_style_bg_color(icono_casa, lv_color_hex(COLOR_SIDEBAR_BG), 0);
  lv_obj_set_style_border_width(icono_casa, 0, 0);
  
  lv_obj_t *lbl_casa = lv_label_create(icono_casa);
  lv_label_set_text(lbl_casa, "CASA");
  lv_obj_center(lbl_casa);
  lv_obj_set_style_text_color(lbl_casa, lv_color_hex(COLOR_ICON), 0);
  lv_obj_set_style_text_font(lbl_casa, FONT_ICON, 0);
  
  // Icono 2: Engrane
  lv_obj_t *icono_engrane = lv_btn_create(sidebar);
  lv_obj_set_size(icono_engrane, 50, 50);
  lv_obj_align(icono_engrane, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_event_cb(icono_engrane, icono_engrane_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_style_bg_color(icono_engrane, lv_color_hex(COLOR_SIDEBAR_BG), 0);
  lv_obj_set_style_border_width(icono_engrane, 0, 0);
  
  lv_obj_t *lbl_engrane = lv_label_create(icono_engrane);
  lv_label_set_text(lbl_engrane, "MENU");
  lv_obj_center(lbl_engrane);
  lv_obj_set_style_text_color(lbl_engrane, lv_color_hex(COLOR_ICON), 0);
  lv_obj_set_style_text_font(lbl_engrane, FONT_ICON, 0);
  
  // Icono 3: Más
  lv_obj_t *icono_mas = lv_btn_create(sidebar);
  lv_obj_set_size(icono_mas, 50, 50);
  lv_obj_align(icono_mas, LV_ALIGN_BOTTOM_MID, 0, -30);
  lv_obj_add_event_cb(icono_mas, icono_mas_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_style_bg_color(icono_mas, lv_color_hex(COLOR_SIDEBAR_BG), 0);
  lv_obj_set_style_border_width(icono_mas, 0, 0);
  
  lv_obj_t *lbl_mas = lv_label_create(icono_mas);
  lv_label_set_text(lbl_mas, "+");
  lv_obj_center(lbl_mas);
  lv_obj_set_style_text_color(lbl_mas, lv_color_hex(COLOR_ICON), 0);
  lv_obj_set_style_text_font(lbl_mas, FONT_MAMAWEBO, 0);
}

// ========== CREAR CABECERA ==========
void crear_cabecera() {
  lv_obj_t *header = lv_obj_create(lv_scr_act());
  lv_obj_set_size(header, lv_obj_get_width(lv_scr_act()), HEADER_HEIGHT);
  lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_HEADER_BG), 0);
  lv_obj_set_style_border_width(header, 0, 0);
  lv_obj_set_style_radius(header, 0, 0);
  
  lv_obj_t *title = lv_label_create(header);
  lv_label_set_text(title, TITLE_TEXT);
  lv_obj_center(title);
  lv_obj_set_style_text_color(title, lv_color_hex(COLOR_HEADER_TEXT), 0);
  lv_obj_set_style_text_font(title, FONT_TITLE, 0);
}

// ========== CREAR BOTONES ==========
void crear_botones() {
  // ===== FILA 1 (Slave 1) =====
  btn1_s1 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn1_s1, BTN_WIDTH, BTN_HEIGHT);
  lv_obj_align(btn1_s1, LV_ALIGN_CENTER, BTN1_ROW1_X, -70);
  lv_obj_add_event_cb(btn1_s1, btn1_s1_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_style_radius(btn1_s1, 15, 0);
  lv_obj_set_style_bg_color(btn1_s1, lv_color_hex(COLOR_BTN_OFF), 0);
  
  lbl1_s1 = lv_label_create(btn1_s1);
  lv_label_set_text(lbl1_s1, BTN1_S1_TEXT_OFF);
  lv_obj_center(lbl1_s1);
  lv_obj_set_style_text_color(lbl1_s1, lv_color_hex(COLOR_BTN_TEXT), 0);
  lv_obj_set_style_text_font(lbl1_s1, FONT_BUTTON, 0);
  
  btn2_s1 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn2_s1, BTN_WIDTH, BTN_HEIGHT);
  lv_obj_align(btn2_s1, LV_ALIGN_CENTER, BTN2_ROW1_X, -70);
  lv_obj_add_event_cb(btn2_s1, btn2_s1_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_style_radius(btn2_s1, 15, 0);
  lv_obj_set_style_bg_color(btn2_s1, lv_color_hex(COLOR_BTN_OFF), 0);
  
  lbl2_s1 = lv_label_create(btn2_s1);
  lv_label_set_text(lbl2_s1, BTN2_S1_TEXT_OFF);
  lv_obj_center(lbl2_s1);
  lv_obj_set_style_text_color(lbl2_s1, lv_color_hex(COLOR_BTN_TEXT), 0);
  lv_obj_set_style_text_font(lbl2_s1, FONT_BUTTON, 0);
  
  btn3_s1 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn3_s1, BTN_WIDTH, BTN_HEIGHT);
  lv_obj_align(btn3_s1, LV_ALIGN_CENTER, BTN3_ROW1_X, -70);
  lv_obj_add_event_cb(btn3_s1, btn3_s1_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_style_radius(btn3_s1, 15, 0);
  lv_obj_set_style_bg_color(btn3_s1, lv_color_hex(COLOR_BTN_OFF), 0);
  
  lbl3_s1 = lv_label_create(btn3_s1);
  lv_label_set_text(lbl3_s1, BTN3_S1_TEXT_OFF);
  lv_obj_center(lbl3_s1);
  lv_obj_set_style_text_color(lbl3_s1, lv_color_hex(COLOR_BTN_TEXT), 0);
  lv_obj_set_style_text_font(lbl3_s1, FONT_BUTTON, 0);
  
  // ===== FILA 2 (Slave 2) =====
  btn1_s2 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn1_s2, BTN_WIDTH, BTN_HEIGHT);
  lv_obj_align(btn1_s2, LV_ALIGN_CENTER, BTN1_ROW2_X, 50);
  lv_obj_add_event_cb(btn1_s2, btn1_s2_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_style_radius(btn1_s2, 15, 0);
  lv_obj_set_style_bg_color(btn1_s2, lv_color_hex(COLOR_BTN_OFF), 0);
  
  lbl1_s2 = lv_label_create(btn1_s2);
  lv_label_set_text(lbl1_s2, BTN1_S2_TEXT_OFF);
  lv_obj_center(lbl1_s2);
  lv_obj_set_style_text_color(lbl1_s2, lv_color_hex(COLOR_BTN_TEXT), 0);
  lv_obj_set_style_text_font(lbl1_s2, FONT_BUTTON, 0);
  
  btn2_s2 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn2_s2, BTN_WIDTH, BTN_HEIGHT);
  lv_obj_align(btn2_s2, LV_ALIGN_CENTER, BTN2_ROW2_X, 50);
  lv_obj_add_event_cb(btn2_s2, btn2_s2_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_style_radius(btn2_s2, 15, 0);
  lv_obj_set_style_bg_color(btn2_s2, lv_color_hex(COLOR_BTN_OFF), 0);
  
  lbl2_s2 = lv_label_create(btn2_s2);
  lv_label_set_text(lbl2_s2, BTN2_S2_TEXT_OFF);
  lv_obj_center(lbl2_s2);
  lv_obj_set_style_text_color(lbl2_s2, lv_color_hex(COLOR_BTN_TEXT), 0);
  lv_obj_set_style_text_font(lbl2_s2, FONT_BUTTON, 0);
  
  btn3_s2 = lv_btn_create(lv_scr_act());
  lv_obj_set_size(btn3_s2, BTN_WIDTH, BTN_HEIGHT);
  lv_obj_align(btn3_s2, LV_ALIGN_CENTER, BTN3_ROW2_X, 50);
  lv_obj_add_event_cb(btn3_s2, btn3_s2_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_set_style_radius(btn3_s2, 15, 0);
  lv_obj_set_style_bg_color(btn3_s2, lv_color_hex(COLOR_BTN_OFF), 0);
  
  lbl3_s2 = lv_label_create(btn3_s2);
  lv_label_set_text(lbl3_s2, BTN3_S2_TEXT_OFF);
  lv_obj_center(lbl3_s2);
  lv_obj_set_style_text_color(lbl3_s2, lv_color_hex(COLOR_BTN_TEXT), 0);
  lv_obj_set_style_text_font(lbl3_s2, FONT_BUTTON, 0);
}

// ========== CREAR LABELS DE SENSORES ==========
void crear_labels_sensores() {
  temp_label = lv_label_create(lv_scr_act());
  lv_label_set_text(temp_label, TEMP_TEXT);
  lv_obj_set_style_text_color(temp_label, lv_color_hex(COLOR_TEMP_TEXT), 0);
  lv_obj_set_style_text_font(temp_label, FONT_SENSOR, 0);
  lv_obj_align(temp_label, LV_ALIGN_BOTTOM_MID, -200, -50);
  
  hum_label = lv_label_create(lv_scr_act());
  lv_label_set_text(hum_label, HUM_TEXT);
  lv_obj_set_style_text_color(hum_label, lv_color_hex(COLOR_HUM_TEXT), 0);
  lv_obj_set_style_text_font(hum_label, FONT_SENSOR, 0);
  lv_obj_align(hum_label, LV_ALIGN_BOTTOM_MID, 200, -50);
}

// ========== CREAR LABEL DE WIFI ==========
void crear_label_wifi() {
  wifi_label = lv_label_create(lv_scr_act());
  actualizar_label_wifi();
  lv_obj_align(wifi_label, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_obj_set_style_text_font(wifi_label, FONT_WIFI, 0);
}

// ========== CREAR INTERFAZ COMPLETA ==========
void crear_interfaz() {
  lv_obj_t *scr = lv_scr_act();
  
  lv_obj_clean(scr);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BG), 0);
  
  crear_sidebar();
  crear_cabecera();
  crear_botones();
  crear_labels_sensores();
  crear_label_wifi();
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // 1. Conectar WiFi
  WiFi.begin(ssid, password);
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    delay(500);
    intentos++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
  } else {
    wifi_connected = false;
  }
  
  // 2. Inicializar pantalla
  static esp_lcd_panel_handle_t panel_handle = NULL;
  
  tp_handle = touch_gt911_init();
  panel_handle = waveshare_esp32_s3_rgb_lcd_init();
  wavesahre_rgb_lcd_bl_on();
  
  lv_init();
  lvgl_port_init(panel_handle, NULL);
  
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touch_driver_read;
  lv_indev_drv_register(&indev_drv);
  
  // 3. Crear interfaz
  if (lvgl_port_lock(-1)) {
    crear_interfaz();
    lvgl_port_unlock();
  }
}

void loop() {
  delay(50);
}