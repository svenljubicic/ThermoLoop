#include <OneWire.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>

MCUFRIEND_kbv tft;

#include <DallasTemperature.h>

#include <TouchScreen.h>
#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define DS18S20_Pin 20
#define DS18B20_ID 0x28

#define OUT_3 39
#define OUT_7 41
#define OUT_5 43
#define OUT_1 45
#define OUT_4 47
#define OUT_6 49
#define OUT_2 51
#define OUT_8 53

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

//kalibracija
const int XP = 6, XM = A2, YP = A1, YM = 7;  
const int TS_LEFT = 100, TS_RT = 890, TS_TOP = 900, TS_BOT = 210;


TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button down_btn, up_btn;

OneWire ds(DS18S20_Pin);
DallasTemperature sensors(&ds);
const int screenArea_y = 320;
const int screenArea_x = 480;
const int screenAreaIn_y = 280;
const int screenAreaMenu_y = (screenArea_y - screenAreaIn_y);
int screenDivided_x, screenDivided_y;
int tiles = 1;
int margin_x = 5, margin_y = 5;
bool redraw = true;
bool menu = false;
float temp;
float upper_limit_1;
float lower_limit_1;
float upper_limit_2;
float lower_limit_2;
float upper_limit_3;
float lower_limit_3;
float upper_limit_4;
float lower_limit_4;
float upper_limit_5;
float lower_limit_5;

bool err = false;
int active_tile = 2;




uint8_t sensor_1[8] = { 0x28, 0x02, 0x00, 0x07, 0x43, 0x3A, 0x01, 0xB7 };
uint8_t sensor_2[8] = { 0x28, 0xFF, 0x9B, 0x7F, 0x31, 0x18, 0x01, 0x0C };



int sensorsTable[5][3] = { { RED, upper_limit_1, lower_limit_1 }, { BLUE, upper_limit_2, lower_limit_2 } };


int pixel_x, pixel_y;  //Touch_getXY() updates global vars
bool Touch_getXY(void) {
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);  //restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);  //because TFT control pins
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed) {
    pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());  //.kbv makes sense to me
    pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
  }
  return pressed;
}



void setCursor(int16_t x, int16_t y);
void setTextColor(uint16_t color);
void setTextColor(uint16_t color, uint16_t backgroundcolor);
void setTextSize(uint8_t size);
void setTextWrap(boolean w);


void setup(void) {
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3) ID = 0x9486;  // write-only shield
  sensors.begin();
  sensors.setResolution(sensor_1, 12);
  sensors.setResolution(sensor_2, 12);
  tft.begin(ID);
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  pinMode(OUT_1, OUTPUT);
  pinMode(OUT_2, OUTPUT);
  pinMode(OUT_3, OUTPUT);
  pinMode(OUT_4, OUTPUT);
  pinMode(OUT_5, OUTPUT);
  pinMode(OUT_6, OUTPUT);
  pinMode(OUT_7, OUTPUT);
  pinMode(OUT_8, OUTPUT);

  digitalWrite(OUT_1, LOW);
  digitalWrite(OUT_2, LOW);
  digitalWrite(OUT_3, LOW);
  digitalWrite(OUT_4, LOW);
  digitalWrite(OUT_5, LOW);
  digitalWrite(OUT_6, LOW);
  digitalWrite(OUT_7, LOW);
  digitalWrite(OUT_8, LOW);

  down_btn.initButton(&tft, 40, 180, 60, 60, RED, RED, WHITE, "-", 4);
  up_btn.initButton(&tft, 440, 180, 60, 60, RED, RED, WHITE, "+", 4);



  up_btn.drawButton(false);
  down_btn.drawButton(false);
  mainLoop();
}

void loop(void) {
  
  bool down = Touch_getXY();
  down_btn.press(down && down_btn.contains(pixel_x, pixel_y));
  up_btn.press(down && up_btn.contains(pixel_x, pixel_y));


  if (down_btn.justReleased()) {
    down_btn.drawButton();

    if (active_tile > 1) {
      active_tile -= 1;
      redraw = true;
      mainLoop();
    }
  }

  if (up_btn.justReleased()) {
    up_btn.drawButton();
    if (active_tile < 2) {
      active_tile += 1;
      redraw = true;
      mainLoop();
    }
  }
  
}


float checkTemperature(DeviceAddress deviceAddress) {
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C) {

    return -127;
  }
  return tempC;
}


void printTemperature(DeviceAddress deviceAddress, int panel_no) {
  tft.setCursor((screenDivided_x / 2 - 150), 180);
  tft.setTextColor(WHITE, BLACK);
  tft.setTextWrap(false);
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C) {
    if (err == true) {
      return;
    }
    drawPanel(panel_no);
    tft.setTextSize(5);
    tft.setCursor((screenDivided_x / 2 - 130), 190);
    tft.print("ODSPOJEN");
    err = true;
  } else {
    tft.setTextSize(10);
    tft.print(tempC);
    err = false;
  }
}

void drawPanel(int tileLocation) {
  tileLocation = tileLocation - 1;
  int color = sensorsTable[tileLocation][0];
  float upper_limit = sensorsTable[tileLocation][1];
  float lower_limit = sensorsTable[tileLocation][2];
  screenDivided_x = screenArea_x;
  screenDivided_y = screenAreaIn_y;
  tft.fillRect(margin_x, (margin_y + 30), (screenDivided_x - 2 * margin_x), (margin_y + 60), color);
  tft.fillCircle((240), (margin_y + 60), 50, BLACK);

  if (digitalRead(OUT_6) == HIGH) {
    tft.fillRoundRect(40, 107, 120, 30, 2, GREEN);
    tft.setCursor(55, 115);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("HLADNJAK");
  } else if (digitalRead(OUT_6) == LOW) {
    tft.fillRoundRect(40, 107, 120, 30, 2, RED);
    tft.setCursor(55, 115);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("HLADNJAK");
  }
  if (digitalRead(OUT_7) == HIGH) {
    tft.fillRoundRect(180, 107, 120, 30, 2, GREEN);
    tft.setCursor(210, 115);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("PUMPA");
  } else if (digitalRead(OUT_7) == LOW) {
    tft.fillRoundRect(180, 107, 120, 30, 2, RED);
    tft.setCursor(210, 115);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("PUMPA");
  }
  if (digitalRead(OUT_8) == HIGH) {
    tft.fillRoundRect(320, 107, 120, 30, 2, GREEN);
    tft.setCursor(345, 115);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("VENTIL");
  } else if (digitalRead(OUT_8) == LOW) {
    tft.fillRoundRect(320, 107, 120, 30, 2, RED);
    tft.setCursor(345, 115);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print("VENTIL");
  }

  tft.setCursor(35, 50 + margin_y);
  tft.setTextSize(4);
  tft.setTextColor(WHITE);
  tft.print(upper_limit);
  tft.setCursor(360, 50 + margin_y);
  tft.print(lower_limit);
  redraw = false;
  tft.fillRect(200, 260, 60, 60, BLACK);
  tft.setCursor(230, 280);
  tft.setTextSize(4);
  tft.setTextColor(WHITE);
  tft.print(tileLocation + 1);
}

void mainLoop() {

  sensors.requestTemperatures();

  if (redraw == true) {
    drawPanel(active_tile);
    redraw = false;
  }

  switch (active_tile) {
    case 1:
      printTemperature(sensor_1, 1);
      break;
    case 2:
      printTemperature(sensor_2, 2);
      break;
  }
}
