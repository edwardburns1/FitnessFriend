#include <TFT_eSPI.h>
#include <EEPROM.h>

// Background Memory
#include "background.h"
#include "house.h"
#include "street.h"
#include "dock.h"

// Cat Animation Memory

#include "idle1.h"
#include "idle2.h"
#include "walk1.h"
#include "sleep1.h"
#include "eating.h"
#include "lick.h"
#include "poop.h"
#include "scratching.h"
#include "BongoCat.h"
#include "look_right.h"
#include "pacman.h"

// Game asset Memory
#include "bowl.h"
#include "scratcher.h"
#include "fish1.h"
#include "battery.h"
#include "Panel.h"
#include "bongo.h"


// Classes
#include "Animation.h"
#include "StatusBar.h"
#include <Button2.h>

// BEGIN BAT
#include <esp_adc_cal.h>

#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34
int vref = 1100;



#define BUTTON_1            35
#define BUTTON_2            0

Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);


#define SCALE 4
#define NUM_ANIMATIONS 11
int FRAME_DELAY = 100;

int maxSpriteDim();



TFT_eSPI    tft = TFT_eSPI();  

TFT_eSprite cat = TFT_eSprite(&tft);  
TFT_eSprite background = TFT_eSprite(&tft);
TFT_eSprite fish = TFT_eSprite(&tft);
TFT_eSprite cat_bowl = TFT_eSprite(&tft);
TFT_eSprite cat_scratcher = TFT_eSprite(&tft);
TFT_eSprite battery = TFT_eSprite(&tft);
TFT_eSprite textSprite = TFT_eSprite(&tft);

TFT_eSprite panelSprite = TFT_eSprite(&tft);
TFT_eSprite panelTextSprite = TFT_eSprite(&tft);

TFT_eSprite exitSprite = TFT_eSprite(&tft);
TFT_eSprite exitTextSprite = TFT_eSprite(&tft);

TFT_eSprite bongoSprite = TFT_eSprite(&tft);




AnimationBase* AnimationList[NUM_ANIMATIONS] = {
  new Animation<20, 20, 8>(idle1),
  new Animation<20, 20, 8>(idle2),
  new Animation<21, 20, 8>(walk1),
  new Animation<24, 10, 8>(sleep1),
  new Animation<22, 20, 8>(eating),
  new Animation<26, 10, 4>(lick),
  new Animation<20, 15, 4>(poop),
  new Animation<16, 18, 8>(scratching),
  new Animation<11, 13, 2>(bongoing),
  new Animation<20, 14, 12>(look_right),
  new Animation<15, 13, 4>(pacman)
};

Animation<25, 13, 5> * BatteryAnimation = new Animation<25, 13, 5>(battery_img);

enum AnimationState {STATE_IDLE1, STATE_IDLE2, STATE_WALK, STATE_SLEEP, STATE_EAT, STATE_LICK, STATE_POOP, STATE_SCRATCHING, STATE_BONGO, STATE_RIGHT, STATE_PAC};

#define NUM_BARS 5

uint32_t BarColors[NUM_BARS] = {TFT_RED, TFT_GREEN, TFT_YELLOW, TFT_ORANGE, TFT_WHITE};

StatusBar* StatusBars[NUM_BARS];

StatusBar* swoleBar = new StatusBar(140, 200, TFT_RED);

void makeStatusBars(int length, int gap){
  int x = 8;
  for(int i = 0; i < NUM_BARS; i++){
    StatusBars[i] = new StatusBar(x, x + length, BarColors[i]);
    x += (length + gap);
  }

}

#define BAR_MAX 100
#define BAR_MIN 0;
// Bar State
uint8_t hunger = 0;
uint8_t thirst = 0;
uint8_t energy = 0;
uint8_t fun = 0;
uint8_t love = 0;

uint8_t swole = 0;
uint8_t coins = 0;


#define HUNGER_MEM 0
#define THIRST_MEM 1
#define ENERGY_MEM 2
#define FUN_MEM    3
#define LOVE_MEM   4
#define FIRST_BOOT_MEM 5
const int EEPROM_EN = 1;
int FIRST_BOOT = 1;
#define EEPROM_SIZE 6



int BUFFER_W = AnimationList[0]->getWidth() * SCALE; 
int BUFFER_H = AnimationList[0]->getHeight() * SCALE;

int maxCatDim = maxSpriteDim();
uint8_t * CatBuffer = new uint8_t[maxCatDim];

#define FISH_SCALE 2
uint8_t * FishBuffer = new uint8_t[20 * 20 * FISH_SCALE * FISH_SCALE];

#define BOWL_SCALE 1
#define BOWL_W 28
#define BOWL_H 15
uint8_t * BowlBuffer = new uint8_t[BOWL_W * BOWL_H * BOWL_SCALE * BOWL_SCALE];


#define SCRATCH_SCALE 4
#define SCRATCH_H 26
#define SCRATCH_W 12
uint8_t * ScratchBuffer = new uint8_t[SCRATCH_H * SCRATCH_SCALE * SCRATCH_W * SCRATCH_SCALE];


#define POS_FISH 80
#define POS_BOWL 200
#define POS_SCRATCHER 180
uint8_t test = 100;

#define AI_TIMEOUT 30 * 1000
int AI_MODE = 0;
unsigned long LAST_CLICK = 0;

void setup() {
  Serial.begin(9600);
  delay(50);
  EEPROM.begin(EEPROM_SIZE);
  

  // Initialise the TFT registers
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  cat.setColorDepth(8);
  fish.setColorDepth(8);
  cat_bowl.setColorDepth(8);
  cat_scratcher.setColorDepth(8);
  battery.setColorDepth(8);
  

  background.setColorDepth(8);
  background.createSprite(320, 170);

  fish.createSprite(20* FISH_SCALE, 20 * FISH_SCALE);
  upscale(fish1, 20, 20, FishBuffer, 20*FISH_SCALE, 20*FISH_SCALE, FISH_SCALE, 1);

  fish.pushImage(0, 0, 20*FISH_SCALE, 20*FISH_SCALE, (uint16_t *) FishBuffer, 8);

  cat_bowl.createSprite(BOWL_W * BOWL_SCALE, BOWL_H * BOWL_SCALE);
  upscale(bowl, BOWL_W, BOWL_H, BowlBuffer, BOWL_W * BOWL_SCALE, BOWL_H * BOWL_SCALE, BOWL_SCALE, 1);
  cat_bowl.pushImage(0, 0, BOWL_W * BOWL_SCALE, BOWL_H * BOWL_SCALE, (uint16_t *) BowlBuffer, 8);

  cat_scratcher.createSprite(SCRATCH_W * SCRATCH_SCALE, SCRATCH_H * SCRATCH_SCALE);
  upscale(scratcher, SCRATCH_W, SCRATCH_H, ScratchBuffer, SCRATCH_W * SCRATCH_SCALE, SCRATCH_H * SCRATCH_SCALE, SCRATCH_SCALE, 1);
  cat_scratcher.pushImage(0, 0, SCRATCH_W * SCRATCH_SCALE, SCRATCH_H * SCRATCH_SCALE, (uint16_t *) ScratchBuffer, 8);

  battery.createSprite(25, 13);

  textSprite.setColorDepth(8);
  textSprite.createSprite(240,135);
  textSprite.setTextColor(TFT_WHITE, TFT_BLACK);

  panelSprite.setColorDepth(8);
  panelSprite.createSprite(60, 20);
  panelSprite.pushImage(0, 0, 60, 20,(uint16_t *) panel, 8);

  panelTextSprite.setColorDepth(8);
  panelTextSprite.createSprite(60, 20);
  panelTextSprite.setTextColor(TFT_WHITE, TFT_BLACK);

  exitSprite.setColorDepth(8);
  exitSprite.createSprite(60, 20);
  exitSprite.pushImage(0, 0, 60, 20,(uint16_t *) panel, 8);

  exitTextSprite.setColorDepth(8);
  exitTextSprite.createSprite(60, 20);
  exitTextSprite.setTextColor(TFT_WHITE, TFT_BLACK);

  bongoSprite.setColorDepth(8);
  bongoSprite.createSprite(60, 20);
  bongoSprite.pushImage(0, 0, 40, 13, (uint16_t *) bongo, 8);
  
  button_init();
  makeStatusBars(25, 18);
  StatusBars[0]->updateState(test);
  calibrateADC();

  // zeroEEPROM(); // Use this to reset 
  FIRST_BOOT = readFirstBoot();

  if(!FIRST_BOOT){
    loadStats();
    statDecline(hunger);
    statDecline(thirst);
    statDecline(fun);
    if(love > 5){
      love -= 5;
    }
  }
  swoleBar->setWidth(10);


  bootSequence();
  delay(3000);

}

void statDecline(uint8_t &stat){
  int div = random(50) + 25;
  if(stat < div){
    stat = 0;
  }
  else {
    stat -= div;
  }

}
const int MAX_FRAMES = 8;
int frame = 0;

int STATE = STATE_SLEEP;

unsigned long lastFrameTime = 0;
unsigned long currTime;

int pos_x;

int buttonDir;
enum BUTTON_ID {LEFT, RIGHT};

#define numBackgrounds 3
enum BACKGROUND {Field, Dock, House, Gym, Shop};
int back_offset[numBackgrounds] = {0, -8, 2};

int currBackground = Field;

unsigned long lastBatCheck = 0;
#define BAT_PERIOD 1000 * 1 //* 60 // Once A minute

float voltage = 0;

enum CONTEXT {START, GAME, MENU, DEEPSLEEP, FITNESS_MENU, GYM, SHOP};
int currContext = GAME;

int textTicks = 0;
int textNum = 0;

char textString[20];
enum ActionState {NONE, EATING, DRINKING, SCRATCHING, SLEEPING, WALKING, POOPING, BONGOING};

int repeat = 0;

bool catShow = false;
void loop() {
  currTime = millis();

  
  if(currTime - lastFrameTime >= FRAME_DELAY){
    lastFrameTime = currTime;
  
    if(currTime - LAST_CLICK > AI_TIMEOUT){
      AI_MODE = 1;
      catAI(currTime);

    }
    else {
      AI_MODE = 0;
    }
    manageState();

    Serial.println(pos_x);
    manageBackground();
    updateCatStats();

    frame = frame % AnimationList[STATE]->getNumFrames();



    BUFFER_W = AnimationList[STATE]->getWidth() * SCALE;
    BUFFER_H = AnimationList[STATE]->getHeight() * SCALE;

    
    upscale(AnimationList[STATE]->getFrame(frame), AnimationList[STATE]->getWidth(), AnimationList[STATE]->getHeight(), CatBuffer, BUFFER_W, BUFFER_H, SCALE, buttonDir);


    if(catShow){
      Serial.println("Visible");
      cat.createSprite(BUFFER_W, BUFFER_H);
      cat.pushImage(0, 0, BUFFER_W, BUFFER_H, (uint16_t *) CatBuffer, 8);
      
      cat.pushToSprite(&background,pos_x,((20 - AnimationList[STATE]->getHeight()) + (10) + back_offset[currBackground])* SCALE, TFT_BLACK);

      updateBarValues();
      drawStatusBars();
    }

    if(currTime - lastBatCheck >= BAT_PERIOD){
      lastBatCheck = currTime;
      voltage = readVoltage();
    }
    displayBattery(voltage);
    // Serial.println(voltage);

    checkContext();
    manageLove();
    
    if(textTicks){
      textSprite.fillSprite(TFT_BLACK);
      textSprite.drawString(String(textString), 0, 0, 4);

      textSprite.pushToSprite(&background, 40 - ((textNum - 11) * 6), 35, TFT_BLACK);
      textTicks -= 1;
    }

    background.pushSprite(0,0);

    if(!repeat){
     frame = (frame + 1);
    }

  }
  btn1.loop();
  btn2.loop();

}
bool BongosUnlocked = 0;

unsigned long lastAI = 0;
unsigned long AIPeriod = 2000;

#define ACTION_THRESH 30
#define ACTION_WIDTH 30

#define SCRATCH_L 100
#define SCRATCH_R 200

#define SCRATCH_TP 135

#define MILK_L 100
#define MILK_R 180
#define MILK_TP 135

#define FISH_L 80
#define FISH_R 120
#define FISH_TP 80

#define BONGO_L 110
#define BONGO_R 160
#define BONGO_TP 138


void catAI(unsigned long currTime){

  if(STATE == STATE_POOP){
    return;
  }
  if(currTime - lastAI < AIPeriod){
    if(atWall() && STATE == STATE_WALK){
     STATE = STATE_IDLE1;
    }
    return;
  }
  lastAI = currTime;
  int results[NUM_ANIMATIONS];
  int weights[NUM_ANIMATIONS] = {10, 10, 20, 5, 5, 3, 0, 5, 0, 8, 0};
  int max_idx;
  int max_val = 0;
  int seed;
  int val;

  switch(checkAction()){
    case EATING:
      weights[STATE_EAT] += 15;
      break;
    case SCRATCHING:
     weights[STATE_SCRATCHING] += 15;
      break;
    case DRINKING:
      weights[STATE_LICK] += 15;
      break;
    case BONGOING:
      if(BongosUnlocked){
        weights[STATE_BONGO] += 20;
      }
      break;
    default:
      break;
  }
  for(int i = 0; i < NUM_ANIMATIONS; i++){
    seed = random(10);
    val = seed * weights[i] ;
    if(val > max_val){
      max_val = val;
      max_idx = i;
    }
  }
  buttonDir = random(2);

  int TP = checkAction();
  AIPeriod = random(10) * 1000;
  switch(max_idx){
    case STATE_BONGO:
      if(TP == BONGOING){
        STATE = STATE_BONGO;
        pos_x = BONGO_TP;
      }
      break;
    case STATE_SCRATCHING:
      if(TP == SCRATCHING){
        STATE = STATE_SCRATCHING;
        pos_x = SCRATCH_TP;
        buttonDir = RIGHT;
      }
      break;
    case STATE_LICK:
      if(TP == DRINKING){
          pos_x = MILK_TP;
          STATE = STATE_LICK;
          buttonDir = RIGHT;
      }
      AIPeriod = 2 * 1000;
    break;
    case STATE_EAT:
      if(TP == EATING){
        pos_x = FISH_TP;
        buttonDir = LEFT;
      }
      AIPeriod = 2 * 1000;

      break;
    case STATE_WALK:
      if(atWall()){
        switch(currBackground){
          case Dock:
            buttonDir = RIGHT;
            break;
          case House:
            buttonDir = LEFT;
            break;
          default:
            break;
        }
      }
      break;
    case STATE_SLEEP:
      AIPeriod *= 6;

    case STATE_RIGHT:
      AIPeriod = 2000;
    default:
      break;
  }
  STATE = max_idx;
  frame = 0;
  
}
void bootSequence(){
  tft.fillScreen(TFT_PINK);
  textSprite.fillSprite(TFT_BLACK);
  textSprite.drawString(String("gattOS"), 80, 50, 4);
  textSprite.pushSprite(0, 0, TFT_BLACK);
}


const int BONGO_THRESH = 25;
const int PAC_MAN_THRESH = 50;
const int STINKY_THRESH = 75;
const int YUKI_LOVE_THRESH = 95;

bool PacManUnlocked = false;
bool Yuki = false;
bool StinkyUnlocked = false;

void manageLove(){
  if(love >= YUKI_LOVE_THRESH && !Yuki){
    Yuki = true;
    PacManUnlocked = 1;
    BongosUnlocked = 1;
    StinkyUnlocked = 1;
    strcpy(textString, "Gatto Loves You!");
    textTicks = 50;
    textNum = 16;

  }
  else if(love >= STINKY_THRESH && !StinkyUnlocked){
    PacManUnlocked = 1;
    BongosUnlocked = 1;
    StinkyUnlocked = 1;
    strcpy(textString, "Stinky Mode!");
    textTicks = 50;
    textNum = 12;
  }
  else if(love >= PAC_MAN_THRESH && !PacManUnlocked){
    PacManUnlocked = 1;
    BongosUnlocked = 1;
    strcpy(textString, "Double Click!");
    textTicks = 80;
    textNum = 13;
  }
  else if(love >= BONGO_THRESH && !BongosUnlocked){
    BongosUnlocked = 1;
    strcpy(textString, "Bongos Unlocked!");
    textTicks = 50;
    textNum = 15;
  }

  if(love < YUKI_LOVE_THRESH){
    Yuki = false;
  }
  if(love < STINKY_THRESH){
    StinkyUnlocked = 0;
  }
  if(love < PAC_MAN_THRESH){
    PacManUnlocked = false;
  }
  if(love < BONGO_THRESH ){
    BongosUnlocked = 0;
  }

}
void storeStats(){
  if(!EEPROM_EN){
    return;
  }
  EEPROM.write(HUNGER_MEM, hunger);
  EEPROM.write(THIRST_MEM, thirst);
  EEPROM.write(ENERGY_MEM, energy);
  EEPROM.write(FUN_MEM, fun);
  EEPROM.write(LOVE_MEM, love);
  EEPROM.write(FIRST_BOOT_MEM, 0);

  EEPROM.commit();
}

void zeroEEPROM(){
  EEPROM.write(HUNGER_MEM, 0);
  EEPROM.write(THIRST_MEM, 0);
  EEPROM.write(ENERGY_MEM, 0);
  EEPROM.write(FUN_MEM, 0);
  EEPROM.write(LOVE_MEM, 0);

  EEPROM.commit();
}

void loadStats(){
  if(EEPROM_EN){
    hunger = EEPROM.read(HUNGER_MEM);
    thirst = EEPROM.read(THIRST_MEM);
    energy = EEPROM.read(ENERGY_MEM);
    fun = EEPROM.read(FUN_MEM);
    love = EEPROM.read(LOVE_MEM);
  }

}

uint8_t readFirstBoot(){
  if(!EEPROM_EN){
    return 1;
  }
  return EEPROM.read(FIRST_BOOT_MEM);
}

void setFirstBoot(){
  if(!EEPROM_EN){
    return;
  }
  EEPROM.write(FIRST_BOOT_MEM, 255);
}

void displayBattery(float voltage){
  int i;
  if(voltage > 4.2){
    i = 4;
  }
  else if (voltage > 4.0){
    i = 3;
  }
  else if (voltage > 3.75){
    i = 2;
  }
  else if (voltage > 3.5){
    i = 1;
  }
  else {
    i = 0;
  }

  battery.pushImage(0, 0, 25, 13, (uint16_t*) BatteryAnimation->getFrame(i), 8);
  battery.pushToSprite(&background, 215, 0);
}
void calibrateADC(){
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);    //Check type of calibration value used to characterize ADC
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
      Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
      vref = adc_chars.vref;
  } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
      Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
  } else {
      Serial.println("Default Vref: 1100mV");
  }
}

float readVoltage(){
  uint16_t v = analogRead(ADC_PIN);
  float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);

  return battery_voltage;
}

void updateBarValues(){
    StatusBars[0]->updateState(love);
    StatusBars[1]->updateState(fun);
    StatusBars[2]->updateState(energy);
    StatusBars[3]->updateState(hunger);
    StatusBars[4]->updateState(thirst);
}

void drawStatusBars(){
  for(int i = 0; i < NUM_BARS; i++){
    background.drawWideLine(StatusBars[i]->getStartPos() - 1,  StatusBars[i]->getPosY(), StatusBars[i]->getEndPos() + 1,
      StatusBars[i]->getPosY(),  StatusBars[i]->getWidth() + 2, TFT_BLACK, TFT_BLACK);

    background.drawWideLine(StatusBars[i]->getStartPos(),  StatusBars[i]->getPosY(), StatusBars[i]->getDisplayPos(), 
      StatusBars[i]->getPosY(), StatusBars[i]->getWidth(), StatusBars[i]->getColor(), StatusBars[i]->getColor());
  }
}

void drawSingleBar(StatusBar* bar){
    background.drawWideLine(bar->getStartPos() - 1,  bar->getPosY(), bar->getEndPos() + 1,
      bar->getPosY(),  bar->getWidth() + 2, TFT_BLACK, TFT_BLACK);

    background.drawWideLine(bar->getStartPos(),  bar->getPosY(), bar->getDisplayPos(), 
      bar->getPosY(), bar->getWidth(), bar->getColor(), bar->getColor());
}


enum BOUNDS {NORMAL, OUT_LEFT, OUT_RIGHT};

#define LEFT_ENTER -80
#define RIGHT_ENTER 230


int checkBounds(){
  if(pos_x < -100){
    return OUT_LEFT;
  }
  else if(pos_x > 250){
    return OUT_RIGHT;
  }
  return NORMAL;
}

int manageBackground(){

  cat.deleteSprite();

  int bounds = checkBounds();
  switch(currBackground){
    case Field:
      if(bounds == OUT_LEFT){
        currBackground = Dock;
        pos_x = RIGHT_ENTER;
      }
      else if(bounds == OUT_RIGHT){
        currBackground = House;
        pos_x = LEFT_ENTER;
      }

      break;
    case Dock:
      if(bounds == OUT_RIGHT){
        currBackground = Field;
        pos_x = LEFT_ENTER;
      }
      break;

    case House:
      if(bounds == OUT_LEFT){
        currBackground = Field;
        pos_x = RIGHT_ENTER;
      }
    break;
  }

  if (currBackground < 3){
    background.pushImage(0, 0, 240, 135, (uint16_t *) back_image[currBackground], 8);
  }
  else {
    // TODO
    background.fillSprite(TFT_BLUE);
  }

  switch(currBackground){
    case Field:
      cat_scratcher.pushToSprite(&background, POS_SCRATCHER, 20, TFT_BLACK);
      break;
    case Dock:
      fish.pushToSprite(&background, POS_FISH, 60, TFT_BLACK);
      if(BongosUnlocked){
        bongoSprite.pushToSprite(&background, 140, 76, TFT_BLACK);
      }
      break;
    case House:
      cat_bowl.pushToSprite(&background, POS_BOWL, 115, TFT_BLACK);
      break;
    case Gym:
      swoleBar->updateState(50); // TODO: Put real swole value here.
      drawSingleBar(swoleBar);
      break;

    case Shop:
      swoleBar->updateState(100); // TODO: Put real swole value here.
      drawSingleBar(swoleBar);
      break;

    default:
      break;


  }
  return 0;
}

void checkContext(){
  Serial.println(currContext);
  switch(currContext){
    case GAME:
      catShow = true;
      break;
    case GYM:
      currBackground = Gym;
    case SHOP:
      catShow = false;
      currBackground = Shop;
      break;
    case MENU:
      // Save Panel
      panelSprite.pushImage(0, 0, 60, 20,(uint16_t *) panel, 8);
      exitSprite.pushImage(0, 0, 60, 20,(uint16_t *) panel, 8);

      panelTextSprite.fillSprite(TFT_BLACK);
      panelTextSprite.drawString(String("Save"), 0, 0, 2);
      panelTextSprite.pushToSprite(&panelSprite, 15, 3, TFT_BLACK);

      panelSprite.pushToSprite(&background, 5, 10, TFT_BLACK);

      // Exit Panel
      exitTextSprite.fillSprite(TFT_BLACK);
      exitTextSprite.drawString(String("Exit"), 0, 0, 2);
      exitTextSprite.pushToSprite(&exitSprite, 20, 3, TFT_BLACK);

      exitSprite.pushToSprite(&background, 5, 110, TFT_BLACK);


      break;
    case FITNESS_MENU:
      // Save Panel
      panelSprite.pushImage(0, 0, 60, 20,(uint16_t *) panel, 8);
      exitSprite.pushImage(0, 0, 60, 20,(uint16_t *) panel, 8);

      panelTextSprite.fillSprite(TFT_BLACK);
      panelTextSprite.drawString(String("Gym"), 0, 0, 2);
      panelTextSprite.pushToSprite(&panelSprite, 15, 3, TFT_BLACK);


      panelSprite.pushToSprite(&background, 5, 10, TFT_BLACK);

      // Exit Panel
      exitTextSprite.fillSprite(TFT_BLACK);
      exitTextSprite.drawString(String("Shop"), 0, 0, 2);
      exitTextSprite.pushToSprite(&exitSprite, 20, 3, TFT_BLACK);

      exitSprite.pushToSprite(&background, 5, 110, TFT_BLACK);
      break;
  }
}

int maxSpriteDim(){
  int max_dim = 0;
  int w;
  int h;
  for(int i = 0; i < NUM_ANIMATIONS; i++){
    w = AnimationList[i]->getWidth();
    h = AnimationList[i]->getHeight();
    if (w*h > max_dim){
      max_dim = w*h;
    }
  }
  return max_dim * SCALE * SCALE;
}

void upscale(const uint8_t *img, int img_w, int img_h, uint8_t * buffer, int buffer_w, int buffer_h, int n, int dir){
  
  int buffer_idx = 0;

  int img_step = (dir == LEFT) ? -img_w : img_w;
  int img_incr = (dir == LEFT) ? -1 : 1;
  int img_idx = (dir == LEFT) ? img_w - 1 : 0;


  for(int i = 0; i < buffer_h; i++){
    for(int j = 0; j < buffer_w; j+= n){

      for(int k = 0; k < n; k++){
        buffer[buffer_idx++] = img[img_idx];
      }
      img_idx += img_incr;
    }
    if((i + 1) % n){
      img_idx -= img_step;
    }
    else if (dir == LEFT) {
      img_idx += 2 * img_w;
    }
  }
}

bool atWall(){
  switch(currBackground){
    case Dock:
      if(pos_x <= 80 && buttonDir == LEFT){
        return true;
      }
      break;
    case House:
      if(pos_x >= 165 && buttonDir == RIGHT){
        return true;
      }
  }

  return false;
}

int currAction = NONE;

int eng_dec = 0;

int POOP_MAX = 5000;
int poop_val = 2500;

uint8_t love_val = 0;
void updateCatStats(){
  poop_val++; // hidden stat

  // Poop Checks
  if(poop_val >= POOP_MAX){
    frame = 0;
    STATE = STATE_POOP;
    poop_val = 0;
    return;

  }
  switch(currAction){
    case NONE:
      break;
    case EATING:
      if(hunger != 100){
        hunger += 1;
      }
      break;
    case DRINKING:
      if(thirst != 100){
        thirst += 1;
      }
      break;
    case SCRATCHING:
    case BONGOING:
      if(fun != 100){
       fun += 1;
      }
      break;

    // TODO: Remove this: instead use the fitness manager to update energy.
    case SLEEPING:
      if(energy != 100){
        energy += 1;
      }
      break;
    case WALKING:
      if(energy > 0 ){
        eng_dec++;
        if(eng_dec == 20){
          eng_dec = 0;
          energy -= 1;
        }
      }
      break;

    default:
      break;

  }

  int love_thresh = 50;
  int love_speed = 25;

  if(hunger >= love_thresh && thirst >= love_thresh && fun >= love_thresh && love < 100){
    love_val += 1;
    if(love_val % love_speed == 0){
      love += 1;
    }
  }
  decr_stat(hunger);
  decr_stat(thirst);
  decr_stat(fun);
  if(hunger < 5 && thirst < 5 && fun < 5){
    decr_love(love);
  }
}


void decr_stat(uint8_t &stat){
  long num = random(1000);
  if(stat > 0 && num >= 950){
    stat -= 1;
  }
}
void decr_love(uint8_t &stat){
  long num = random(1000);
  if(stat > 0 && num >= 980){
    stat -= 1;
  }
}


int checkAction(){
  switch(currBackground){
    case Dock:
      if(pos_x >= FISH_L && pos_x <= FISH_R){
        Serial.println("FISH");
        return EATING;
      }
      else if(pos_x >= BONGO_L && pos_x <= BONGO_R && BongosUnlocked){
        return BONGOING;
      }
      break;
    case Field:
      if(pos_x >= SCRATCH_L && pos_x <= SCRATCH_R){
        Serial.println("SCRATCH");
        return SCRATCHING;
      }
      break;

    case House:
      if(pos_x >= MILK_L && pos_x <= MILK_R){
        Serial.println("MILK");
        return DRINKING;
      }
      break;
  }
  return 0;
}
void manageState(){
  FRAME_DELAY = 100;
  repeat = 0;
  if(energy == 0){
    STATE = STATE_SLEEP;
  }
  switch(STATE){
    case STATE_IDLE1:
    case STATE_IDLE2:
      currAction = NONE;
      break;
    case STATE_SLEEP:
      currAction = SLEEPING;
      break;

    case STATE_WALK:
      currAction = WALKING;
      if(atWall()){
        return;
      }

      if(buttonDir == LEFT){
        pos_x -= 5;
      }
      else {
        pos_x += 5;

      }
      break;

    case STATE_EAT:
      break;
    case STATE_POOP:
      FRAME_DELAY = 500;
      switchAnimation();
      break;
    case STATE_BONGO:
      break;
    case STATE_RIGHT:
      if(AnimationList[STATE]->checkDone()){
        switchAnimation();
      }
      break;
    case STATE_PAC:
      FRAME_DELAY = 250;
      break;      
    default:
      break;
  }
}
void switchAnimation(){
  int last_frame = frame;
  frame = 0;
  switch (STATE){
    case STATE_IDLE1:
    case STATE_IDLE2:
      STATE = STATE_WALK;
      break;
    case STATE_WALK:
      STATE = STATE_IDLE1;
      break;
    case STATE_SLEEP:
      STATE = STATE_IDLE2;
      break;
    case STATE_EAT:
      STATE = STATE_IDLE1;
      break;
    case STATE_LICK:
      STATE = STATE_IDLE1;
      break;
    case STATE_POOP:
      frame = last_frame;
      if(AnimationList[STATE]->checkDone()){
        FRAME_DELAY = 100;
        STATE = STATE_IDLE1;
      }
      break;
    case STATE_SCRATCHING:
      STATE = STATE_IDLE1;
      break;
    
    case STATE_BONGO:
      STATE = STATE_IDLE2;
      break;

    case STATE_RIGHT:
      STATE = STATE_IDLE1;
      //IDLE WALK;
      break;
    case STATE_PAC:
      STATE = STATE_IDLE2;
      break;

    default:
      break;
    }
}
void ShortClickHandler(Button2 & b){
  
  LAST_CLICK = millis();
  buttonDir = b.getID();
  if(currContext == GAME){
    switchAnimation();
    
  }
  else if(currContext == MENU || currContext == FITNESS_MENU){
    currContext = GAME;
  }

}

void LongClickHandler(Button2 & b){

  int longButtonDir = b.getID();
  if(currContext == GAME){
    int TP = checkAction();

    frame = 0;

    if(longButtonDir == LEFT){
      if( TP== BONGOING){
        STATE = STATE_BONGO;
        currAction = BONGOING;
        pos_x = BONGO_TP;
        return;
      }

      STATE = STATE_SCRATCHING;
      if(TP == SCRATCHING){
        currAction = SCRATCHING;
        pos_x = SCRATCH_TP;
        buttonDir = RIGHT;
      }
      return;

    }
    else {

      switch(currBackground){
        case House:
          if(TP == DRINKING){
            currAction = DRINKING;
            pos_x = MILK_TP;
          }
          STATE = STATE_LICK;
          
          break;
        case Dock:
          if(TP == EATING){
            currAction = EATING;
            pos_x = FISH_TP;
          }
          STATE = STATE_EAT;
          break;
      }
    }
  }
  else if(currContext == MENU){
    currContext = GAME;
    if(longButtonDir == LEFT){
      strcpy(textString, "Game Saved!");
      textTicks = 20;
      textNum = 11;
      Serial.println("Saving!");
      storeStats();
      
      // ADD EEPROM
    }
    else{
      Serial.println("Exiting");
      gattOSleep();
    }
  }
  else if (currContext == FITNESS_MENU){
    if(longButtonDir = LEFT){
      currContext = GYM;
    }
    else {
      currContext = SHOP;
    }
  }

}

void TripleClickHandler(Button2 & b){
  // Save EEPROM
  currContext = MENU;
}

void FitnessTripleClickHandler(Button2 & b){
  if (currContext == GYM || currContext == SHOP){
    currContext = GAME;
    currBackground = Field;
  }
  else{
    currContext = FITNESS_MENU;
  }
}

void LeftDoubleClickHandler(Button2 & b){

  if(PacManUnlocked){
    frame = 0;
    FRAME_DELAY = 250;
    STATE = STATE_PAC;
  }

}

void RightDoubleClickHandler(Button2 & b){
  if(StinkyUnlocked){
    if(POOP_MAX > 1000){
      POOP_MAX = 100;
    }
    else {
      POOP_MAX = 5000;
    }
  }

}


void button_init(){
  btn1.setID(RIGHT);
  btn1.setLongClickTime(500);
  btn1.setClickHandler(ShortClickHandler);
  btn1.setLongClickHandler(LongClickHandler);
  btn1.setTripleClickHandler(TripleClickHandler);
  btn1.setDoubleClickHandler(RightDoubleClickHandler);

  btn2.setID(LEFT);
  btn2.setLongClickTime(500);
  btn2.setClickHandler(ShortClickHandler);
  btn2.setLongClickHandler(LongClickHandler);
  btn2.setTripleClickHandler(FitnessTripleClickHandler);
  btn2.setDoubleClickHandler(LeftDoubleClickHandler);



}

void gattOSleep(){
  storeStats();
  // tft.writecommand(TFT_DISPOFF);
  // tft.writecommand(TFT_SLPIN);

  // exit(0);
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
  // esp_sleep_enable_ext1_wakeup(35, ESP_EXT1_WAKEUP_ALL_LOW);
  esp_deep_sleep_start();
}
