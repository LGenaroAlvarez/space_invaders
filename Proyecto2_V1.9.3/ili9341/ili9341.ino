// PROYECTO 2 === PROYECTO 2 === PROYECTO 2 === PROYECTO 2
/*Codigo del proyecto 2 funcional. Version 1.9.3
 *Ultima actualizacion realizada el 27/10/2022 a las 11:27pm.
 *Se implemento la colision entre misiles y aliens. Tanto los misiles como los aliens desaparecen al colisionar.
 */
//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo 8 bits
 * Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 * Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 * Con ayuda de: José Guerra
 * IE3027: Electrónica Digital 2 - 2019
 */
//***************************************************************************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "TM4C123GH6PM.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "bitmaps.h"
#include "font.h"
#include "lcd_registers.h"
// DEFINICIÓN DE PINES PARA ILI9341
#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};  
// DEFINICIÓN DE CONSTANTES
#define STATUS 0
#define MISSILE_SPEED2 1 // A menor valor mayor velocidad
#define MISSILE_SPEED1 1
#define MISSILE_H 20    // Altura de misil (y)
#define MISSILE_W 3     // Ancho de misil (x)
#define ACTIVE 0        // Estado de activación de disparo1
#define DESTROYED 1     // Estado de desactivación de disparo1
#define ACTIVE2 0       // Estado de activación de disparo2
#define DESTROYED2 1    // Estado de desactivación de disparo2
#define SHIP_H 24       // Altura de naves (y)
#define SHIP_W 20       // Ancho de naves (x)
#define SHIP_SPEED 1    // Velocidad general de las naves
// DEFINICIÓN DE PANTALLA ILI9341
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
// COFIGURACIÓN DE ALIENS
#define NUM_ALIEN_COLUMNS 7
#define NUM_ALIEN_ROWS 3
#define Y_START_POS 0
#define X_START_OFFSET 44 // POSICION DE SEPARACIÓN ENTRE BORDE DE PANTALLA Y ALIEN
#define SPACE_BETWEEN_ALIEN_COLUMNS 10 //15
#define LARGEST_ALIEN_WIDTH 24
#define ALIEN_HEIGHT 16
#define SPACE_BETWEEN_ROWS 26
#define INVADERS_DROP_BY 4 // PIXELES QUE LOS INVADERS SE MUEVEN HACIA ABAJO
#define INVADERS_SPEED 12  // VELOCIDAD DE MOVIMIENTO ENTRE MÁS ALTO VALOR MÁS LENTO

// DEFINICIÓN DE VARIABLES
int missile_status = 1; // Bandera de activación de disparo
int missile_end = 0;    // Bandera de salida del disparo de la pantalla
int missile_status2 = 1; // Bandera de activación de disparo
int missile_end2 = 0;    // Bandera de salida del disparo de la pantalla
int missile1_pos_X = 0; // Posición X del misil1
int missile1_pos_Y = 0; // Posición Y del misil1
int missile2_pos_X = 0; // Posición X del misil2
int missile2_pos_Y = 0; // Posición Y del misil2
int ship1_pos_X = 80;   // Posición X de nave1
int ship1_pos_Y = 240;  // Posición Y de nave1
int ship2_pos_X = 240;  // Posición X de nave2
int ship2_pos_Y = 240;  // Posición Y de nave2
int x = 80;             // Posición inicial en X de nave1
int x1 = 240;           // Posición inicial en X de nave2
int estado1 = 0;        // Estado nave1 con bot1 (Derecha)
int estado2 = 0;        // Estado nave2 con bot2 (Derecha)
int estado3 = 0;        // Estado de 
int estado4 = 0;
int estado5 = 0;
int estado6 = 0;
int y_line = 26;
int y_line2 = 26;
float intervalo1 = 1.5;
float intervalo2 = 1.5;
long previousMillis = 0;
long previousMillis2 = 0;   
long previousMillis3 = 0;
long previousMillis4 = 0;
long previousMillis5 = 0;
long previousMillis6 = 0;
long previousMillis7 = 0;
long previousMillis8 = 0;
long previousMillis9 = 0;
long previousMillis10 = 0;
long previousMillis11 = 0;
long previousMillis12 = 0;
long previousMillis13 = 0;
long previousMillis14 = 0;
//***************************************************************************************************************************************
// Variables & arrays (INVADERS)
//***************************************************************************************************************************************
int acrossDisp = 0;
int downDisp = 0;
int acrossMov = 0;
int downMov = 0;
int AcrossRMP = NUM_ALIEN_COLUMNS - 1;
int DownRMP = 0;
int acrossHit = 0;
int downHit = 0;
int Largest = 0;
int RightPos;
int AcrossLMP = 0;
int DownLMP = 0;
int Smallest = SCREEN_WIDTH*2;

struct GameObjectStruct{
  signed int X;
  signed int Y;
  unsigned char Status;
};
struct AlienStruct{
  GameObjectStruct Ord;
};
AlienStruct Alien[NUM_ALIEN_COLUMNS][NUM_ALIEN_ROWS]; // ARREGLO DE ALIENS EN LA PANTALLA
byte AlienWidth[]={16, 22, 24};

char AlienXMoveAmount=2;
int moveByXAmount = 0;
int moveByXAmount1 = 1;
int moveByXAmount2 = 1;
int moveByXAmount3 = 1;
int moveByYAmount = 0;
uint8_t InvadersMoveCounter = 1;
int activeFrame = 1;
int frameCount = 0;
bool lowered = false;
int moveInvaderTime = 3;
int moveInvaderTime2 = 5;
int moveInvader = 0;
//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
void Timer0IntHandler();
void LCD_Init(void);
void LCD_CMD(uint8_t cmd);
void LCD_DATA(uint8_t data);
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
void LCD_Clear(unsigned int c);
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c);
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c);
void LCD_Print(String text, int x, int y, int fontSize, int color, int background);
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]);
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset);
void shipInit(void);
void moveRControlAll(void);
void moveRControl1(void);
void moveRControl2(void);
void moveRLControl(void);
void moveLRControl(void);
void moveLControlAll(void);
void moveLControl1(void);
void moveLControl2(void);
void movePlayers(void);
void fireMissiles(void);  
void UpdateInvaders(void);
void initInvaders(int Y_initial);
void posControl(void);
void animationUpdate(void);
void missileAlienCollision(void);
bool collisionCheck(int missile_x, int missile_y, int mi_W, int mi_H, GameObjectStruct Obj, unsigned char width, unsigned char height);

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  // Frecuencia de reloj (utiliza TivaWare)
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  // Configuración del puerto (utiliza TivaWare)
  MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");

  float period_0 = 0.5; // 200ms
  MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, MAP_SysCtlClockGet()*period_0);
  TimerIntRegister(TIMER0_BASE, TIMER_A, &Timer0IntHandler);
  MAP_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  MAP_TimerEnable(TIMER0_BASE, TIMER_A);
  
  // Inicialización de la pantalla
  LCD_Init();
  LCD_Clear(0x0000);
  // Referencia para colores RGB565: http://www.rinkydinkelectronics.com/calc_rgb565.php
  shipInit();
  initInvaders(Y_START_POS);
  pinMode (6, OUTPUT);
  pinMode (11, INPUT_PULLUP);
  pinMode (13, INPUT_PULLUP);
  pinMode (12, INPUT_PULLUP);
  pinMode (8, INPUT_PULLUP);
  pinMode (9, INPUT_PULLUP);
  pinMode (10, INPUT_PULLUP);
 
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
//void loop() {
void loop() {
  estado1 = digitalRead(11);
  estado2 = digitalRead(13);
  estado3 = digitalRead(12);
  estado4 = digitalRead(8);
  estado5 = digitalRead(9);
  estado6 = digitalRead(10);
  // PRESIÓN DE BOTONES
  //animationUpdate();
  UpdateInvaders();
  movePlayers();
  // CONTROL DE DISPARO DE MISIL
  fireMissiles();
  missileControl();
  missileAlienCollision();
}


//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void Timer0IntHandler() {
  MAP_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  animationUpdate();
  InvadersMoveCounter = !InvadersMoveCounter;
}

void shipInit(void){
  ship1_pos_Y = 235-SHIP_H;
  ship2_pos_Y = 235-SHIP_H;
  LCD_Bitmap(ship1_pos_X, ship1_pos_Y, SHIP_W, SHIP_H, J1_nave);
  LCD_Bitmap(ship2_pos_X, ship2_pos_Y, SHIP_W, SHIP_H, J2_nave);
}

void movePlayers(void){
  if(estado1 == LOW & estado2 == HIGH){
    moveRControl1();
  }
  if(estado1 == HIGH & estado2 == LOW){
    moveRControl2();
  }
  if(estado1 == LOW & estado2 == LOW){
    moveRControlAll();
  }
  if(estado1 == LOW & estado4 == LOW){
    moveRLControl();
  }
  if(estado3 == LOW & estado2 == LOW){
    moveLRControl();
  }
  if(estado3 == LOW & estado4 == LOW){
    moveLControlAll();
  }
  if(estado3 == LOW){
    moveLControl1();
  }
  if(estado4 == LOW){
    moveLControl2();
  }
}
void fireMissiles(void){
  if(estado5 == LOW & estado6 == HIGH){
    if(missile_status == DESTROYED){
      missile1_pos_X = ship1_pos_X + (SHIP_W/2);
      missile1_pos_Y = ship1_pos_Y - MISSILE_H;
      missile_status = ACTIVE;
    }
    else{
      missile_status = missile_status;
    }    
  }
  if(estado6 == LOW & estado5 == HIGH){
    if(missile_status2 == DESTROYED2){
      missile2_pos_X = ship2_pos_X + (SHIP_W/2);
      missile2_pos_Y = ship2_pos_Y - MISSILE_H;
      missile_status2 = ACTIVE2;
    }
    else{
      missile_status2 = missile_status2;
    }    
  }
  if(estado5 == LOW & estado6 == LOW){
    if(missile_status == DESTROYED & missile_status2 == DESTROYED2){
      missile1_pos_X = ship1_pos_X + (SHIP_W/2);
      missile1_pos_Y = ship1_pos_Y - MISSILE_H;
      
      missile2_pos_X = ship2_pos_X + (SHIP_W/2);
      missile2_pos_Y = ship2_pos_Y - MISSILE_H;
      missile_status = ACTIVE;
      missile_status2 = ACTIVE2;
    }
    if(missile_status == ACTIVE & missile_status2 == DESTROYED2){
      missile2_pos_X = ship2_pos_X + (SHIP_W/2);
      missile2_pos_Y = ship2_pos_Y - MISSILE_H;
      missile_status2 = ACTIVE2;
    }
    if(missile_status == DESTROYED & missile_status2 == ACTIVE2){
      missile1_pos_X = ship1_pos_X + (SHIP_W/2);
      missile1_pos_Y = ship1_pos_Y - MISSILE_H;
      missile_status = ACTIVE;
    }
  }
}
void missileControl(void){
  if (missile_status == ACTIVE){
    unsigned long currentMillis = millis();
     if(currentMillis - previousMillis > intervalo1) {
      missile1_pos_Y-=MISSILE_SPEED1;
      
      if(missile1_pos_Y >= 0){
        LCD_Bitmap(missile1_pos_X, missile1_pos_Y, MISSILE_W, MISSILE_H, disparo1);
        H_line(missile1_pos_X, missile1_pos_Y + MISSILE_H+1, MISSILE_W, 0x0000);
      }
      if (missile1_pos_Y <= 0){
        y_line--;
        if (y_line >= 0){
          H_line(missile1_pos_X, y_line, MISSILE_W, 0x0000);        
        }
        if(y_line == 0){
          missile_status = DESTROYED;
          y_line = 26;
        }
      }
      previousMillis = currentMillis;  
    }
  }
  if (missile_status2 == ACTIVE2){
    unsigned long currentMillis2 = millis();
    if(currentMillis2 - previousMillis2 > intervalo2) {
      missile2_pos_Y-=MISSILE_SPEED2;
      if (missile2_pos_Y >= 0){
        LCD_Bitmap(missile2_pos_X, missile2_pos_Y, MISSILE_W, MISSILE_H, disparo1);
        H_line(missile2_pos_X, missile2_pos_Y + MISSILE_H+1, MISSILE_W, 0x0000);
      }
      if (missile2_pos_Y <= 0){
        y_line2--;
        if (y_line2 >= 0){
          H_line(missile2_pos_X, y_line2, MISSILE_W, 0x0000);
        }
        if (y_line2 == 0){
          missile_status2 = DESTROYED2;
          y_line2 = 26;
        }
      }
      previousMillis2 = currentMillis2;
    } 
  }
}

void moveLControl2(void){
  unsigned long currentMillis6 = millis();
  if (currentMillis6 - previousMillis6 > SHIP_SPEED){
    if(x1 == 160){
      x1 = 160;
    }
    else{
      x1--;
    }
    ship2_pos_X = x1;
    ship2_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship2_pos_X,ship2_pos_Y,20,24,J2_nave);
    V_line(x1 + SHIP_W, ship2_pos_Y, SHIP_H, 0x0000);
    previousMillis6 = currentMillis6;
  }  
}
void moveLControl1(void){
  unsigned long currentMillis7 = millis();
  if (currentMillis7 - previousMillis7 > SHIP_SPEED){
    if(x == 0){
      x = 0;
    }
    else{
      x--;
    }
    ship1_pos_X = x;
    ship1_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship1_pos_X,ship1_pos_Y,20,24,J1_nave);
    V_line(x + SHIP_W, ship1_pos_Y, SHIP_H, 0x0000);
    previousMillis7 = currentMillis7;
  }
}
void moveLControlAll(void){
  unsigned long currentMillis8 = millis();
  if (currentMillis8 - previousMillis8 > SHIP_SPEED){
    if(x == 0){
      x = 0;
    }
    else{
      x--;
    }
    ship1_pos_X = x;
    ship1_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship1_pos_X,ship1_pos_Y,20,24,J1_nave);
    V_line(x + SHIP_W, ship1_pos_Y, SHIP_H, 0x0000);
    
    if(x1 == 160){
      x1 = 160;
    }
    else{
      x1--;
    }
    ship2_pos_X = x1;
    ship2_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship2_pos_X,ship2_pos_Y,20,24,J2_nave);
    V_line(x1 + SHIP_W, ship2_pos_Y, SHIP_H, 0x0000);
    previousMillis8 = currentMillis8;
  }
}
void moveLRControl(void){
  unsigned long currentMillis9 = millis();
  if (currentMillis9 - previousMillis9 > SHIP_SPEED){
    if(x == 0){
      x = 0;
    }
    else{
      x--;
    }
    ship1_pos_X = x;
    ship1_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship1_pos_X,ship1_pos_Y,20,24,J1_nave);
    V_line(x + SHIP_W, ship1_pos_Y, SHIP_H, 0x0000);
    
    if(x1 == 320-SHIP_W){
      x1 = 320-SHIP_W;
    }
    else{
      x1++;
    }
    ship2_pos_X = x1;
    ship2_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship2_pos_X,ship2_pos_Y,20,24,J2_nave);
    V_line(x1 - 1, ship2_pos_Y, SHIP_H, 0x0000);
    previousMillis9 = currentMillis9;
  }
}
void moveRLControl(void){
  unsigned long currentMillis10 = millis();
  if (currentMillis10 - previousMillis10 > SHIP_SPEED){
    if(x == 160-SHIP_W){
      x = 160-SHIP_W;
    }
    else{
      x++;
    }
    ship1_pos_X = x;
    ship1_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship1_pos_X,ship1_pos_Y,20,24,J1_nave);
    V_line(x - 1, ship1_pos_Y, SHIP_H, 0x0000);
    
    if(x1 == 160){
      x1 = 160;
    }
    else{
      x1--;
    }
    ship2_pos_X = x1;
    ship2_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship2_pos_X,ship2_pos_Y,20,24,J2_nave);
    V_line(x1 + SHIP_W, ship2_pos_Y, SHIP_H, 0x0000);
    previousMillis10 = currentMillis10;
  }
}
void moveRControlAll(void){
  unsigned long currentMillis11 = millis();
  if (currentMillis11 - previousMillis11 > SHIP_SPEED){
    if(x == 160-SHIP_W){
      x = 160-SHIP_W;
    }
    else{
      x++;
    }
    ship1_pos_X = x;
    ship1_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship1_pos_X,ship1_pos_Y,20,24,J1_nave);
    V_line(x - 1, ship1_pos_Y, SHIP_H, 0x0000);
    
    if(x1 == 320-SHIP_W){
      x1 = 320-SHIP_W;
    }
    else{
      x1++;
    }
    ship2_pos_X = x1;
    ship2_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship2_pos_X,ship2_pos_Y,20,24,J2_nave);
    V_line(x1 - 1, ship2_pos_Y, SHIP_H, 0x0000);
    previousMillis11 = currentMillis11;
  }
}
void moveRControl1(void){
  unsigned long currentMillis12 = millis();
  if (currentMillis12 - previousMillis12 > SHIP_SPEED){
    if(x == 160-SHIP_W){
      x = 160-SHIP_W;
    }
    else{
      x++;
    }
    ship1_pos_X = x;
    ship1_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship1_pos_X,ship1_pos_Y,20,24,J1_nave);
    V_line(x - 1, ship1_pos_Y, SHIP_H, 0x0000);
    previousMillis12 = currentMillis12;
  }
}
void moveRControl2(void){
  unsigned long currentMillis13 = millis();
  if (currentMillis13 - previousMillis13 > SHIP_SPEED){
    if(x1 == 320-SHIP_W){
      x1 = 320-SHIP_W;
    }
    else{
      x1++;
    }
    ship2_pos_X = x1;
    ship2_pos_Y = 235-SHIP_H;
    LCD_Bitmap(ship2_pos_X,ship2_pos_Y,20,24,J2_nave);
    V_line(x1 - 1, ship2_pos_Y, SHIP_H, 0x0000);
    previousMillis13 = currentMillis13;
  }
}

void initInvaders(int Y_initial){
  for (int across = 0; across < NUM_ALIEN_COLUMNS; across++){ 
    for (int down = 0; down < 3; down++){
      Alien[across][down].Ord.X = X_START_OFFSET + (across *(LARGEST_ALIEN_WIDTH + SPACE_BETWEEN_ALIEN_COLUMNS)) - down;
      Alien[across][down].Ord.Y = Y_initial + (down * SPACE_BETWEEN_ROWS);
      Alien[across][down].Ord.Status = ACTIVE;
    }
  }
}


void UpdateInvaders(void){
  if (acrossDisp < NUM_ALIEN_COLUMNS) {
    if (downDisp < NUM_ALIEN_ROWS){
      if (Alien[acrossDisp][downDisp].Ord.Status == ACTIVE){
        switch(downDisp){
          case 0:
            if (activeFrame  == 1){
              LCD_Bitmap(Alien[acrossDisp][downDisp].Ord.X+=moveByXAmount1, Alien[acrossDisp][downDisp].Ord.Y+=moveByYAmount, AlienWidth[downDisp], ALIEN_HEIGHT, invader3_frame1);
              if (moveByXAmount1 == 1){
                V_line(Alien[acrossDisp][downDisp].Ord.X-1, Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
              else if (moveByXAmount1 == -1){
                V_line(Alien[acrossDisp][downDisp].Ord.X+AlienWidth[downDisp], Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
            }
            else {
              LCD_Bitmap(Alien[acrossDisp][downDisp].Ord.X+=moveByXAmount1, Alien[acrossDisp][downDisp].Ord.Y+=moveByYAmount, AlienWidth[downDisp], ALIEN_HEIGHT, invader3_frame2);
              if (moveByXAmount1 == 1){
                V_line(Alien[acrossDisp][downDisp].Ord.X-1, Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
              else if (moveByXAmount1 == -1){
                V_line(Alien[acrossDisp][downDisp].Ord.X+AlienWidth[downDisp], Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
            }
            break;
          case 1:
            if (activeFrame == 1){
              LCD_Bitmap(Alien[acrossDisp][downDisp].Ord.X+=moveByXAmount2, Alien[acrossDisp][downDisp].Ord.Y+=moveByYAmount, AlienWidth[downDisp], ALIEN_HEIGHT, invader2_frame1);
              if (moveByXAmount2 == 1){
                V_line(Alien[acrossDisp][downDisp].Ord.X-1, Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
              else if (moveByXAmount2 == -1){
                V_line(Alien[acrossDisp][downDisp].Ord.X+AlienWidth[downDisp], Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
            }
            else {
              LCD_Bitmap(Alien[acrossDisp][downDisp].Ord.X+=moveByXAmount2, Alien[acrossDisp][downDisp].Ord.Y+=moveByYAmount, AlienWidth[downDisp], ALIEN_HEIGHT, invader2_frame2);
              if (moveByXAmount2 == 1){
                V_line(Alien[acrossDisp][downDisp].Ord.X-1, Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
              else if (moveByXAmount2 == -1){
                V_line(Alien[acrossDisp][downDisp].Ord.X+AlienWidth[downDisp], Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
            }
            break;
          default:
            if (activeFrame  == 1){
              LCD_Bitmap(Alien[acrossDisp][downDisp].Ord.X+=moveByXAmount3, Alien[acrossDisp][downDisp].Ord.Y+=moveByYAmount, AlienWidth[downDisp], ALIEN_HEIGHT, invader1_frame1);
              if (moveByXAmount3 == 1){
                V_line(Alien[acrossDisp][downDisp].Ord.X-1, Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
              else if (moveByXAmount3 == -1){
                V_line(Alien[acrossDisp][downDisp].Ord.X+AlienWidth[downDisp], Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
            }
            else {
              LCD_Bitmap(Alien[acrossDisp][downDisp].Ord.X+=moveByXAmount3, Alien[acrossDisp][downDisp].Ord.Y+=moveByYAmount, AlienWidth[downDisp], ALIEN_HEIGHT, invader1_frame2);
              if (moveByXAmount3 == 1){
                V_line(Alien[acrossDisp][downDisp].Ord.X-1, Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
              else if (moveByXAmount3 == -1){
                V_line(Alien[acrossDisp][downDisp].Ord.X+AlienWidth[downDisp], Alien[acrossDisp][downDisp].Ord.Y, ALIEN_HEIGHT, 0x0000);
              }
            }
        }
      }
      else {
        switch(downDisp){
          case 0:
            FillRect((Alien[acrossDisp][downDisp].Ord.X+=moveByXAmount1)-1, Alien[acrossDisp][downDisp].Ord.Y, AlienWidth[downDisp]+2, ALIEN_HEIGHT, 0x0000);
            break;
          case 1:
            FillRect((Alien[acrossDisp][downDisp].Ord.X+=moveByXAmount2)-1, Alien[acrossDisp][downDisp].Ord.Y, AlienWidth[downDisp]+2, ALIEN_HEIGHT, 0x0000);
            break;
          default:
            FillRect((Alien[acrossDisp][downDisp].Ord.X+=moveByXAmount3)-1, Alien[acrossDisp][downDisp].Ord.Y, AlienWidth[downDisp]+2, ALIEN_HEIGHT, 0x0000);
        }
      }
      if (acrossDisp == (NUM_ALIEN_COLUMNS-1)){
        downDisp++;
        acrossDisp = -1;
      }
    }
    acrossDisp++;
  }
  moveInvader++;
  if (acrossDisp == (NUM_ALIEN_COLUMNS-1) & downDisp == (NUM_ALIEN_ROWS-1)){
    acrossDisp = 0;
    downDisp = 0;   
    posControl();   
  }
}

void posControl(void){
  if (Alien[6][0].Ord.X == (SCREEN_WIDTH - AlienWidth[0] - 10)){
    moveByXAmount1 = -1;
    moveByXAmount2 = -1;
    moveByXAmount3 = -1;
    digitalWrite(6, LOW);
  }
  else if (Alien[0][0].Ord.X == (10)){
    moveByXAmount1 = 1;
    moveByXAmount2 = 1;
    moveByXAmount3 = 1;
    digitalWrite(6, HIGH);
  }
}

void animationUpdate(void){
  unsigned long currentMillis5 = millis();
  if ((currentMillis5 - previousMillis5) > 1000){
    activeFrame = !activeFrame;
    previousMillis5 = currentMillis5;
  }
}

void missileAlienCollision(void){
  if (acrossHit < NUM_ALIEN_COLUMNS){
    if (downHit < NUM_ALIEN_ROWS){
      if(Alien[acrossHit][downHit].Ord.Status == ACTIVE){
        if (missile_status == ACTIVE){
          if (collisionCheck(missile1_pos_X , missile1_pos_Y, MISSILE_W, MISSILE_H, Alien[acrossHit][downHit].Ord, AlienWidth[downHit], ALIEN_HEIGHT)){
            Alien[acrossHit][downHit].Ord.Status = DESTROYED;
            missile_status = DESTROYED;
            FillRect(missile1_pos_X, missile1_pos_Y, MISSILE_W+2, MISSILE_H+2, 0x0000);
            y_line = 26;
          }
        }
        if (missile_status2 == ACTIVE){
          if (collisionCheck(missile2_pos_X , missile2_pos_Y, MISSILE_W, MISSILE_H, Alien[acrossHit][downHit].Ord, AlienWidth[downHit], ALIEN_HEIGHT)){
            Alien[acrossHit][downHit].Ord.Status = DESTROYED;
            missile_status2 = DESTROYED;
            FillRect(missile2_pos_X, missile2_pos_Y, MISSILE_W+2, MISSILE_H+2, 0x0000);
            y_line2 = 26;
          }
        }
      }
      if (acrossHit == (NUM_ALIEN_COLUMNS-1)){
        downHit++;
        acrossHit = -1;
      }
    }
    acrossHit++;
  }
  if (acrossHit == (NUM_ALIEN_COLUMNS-1) & downHit == (NUM_ALIEN_ROWS-1)){
    acrossHit = 0;
    downHit = 0;   
  }
}

bool collisionCheck(int missile_x, int missile_y, int mi_W, int mi_H, GameObjectStruct Obj, unsigned char width, unsigned char height){
  return ((missile_x+mi_W>Obj.X)&(missile_x<Obj.X+width)&(missile_y+mi_H>Obj.Y)&(missile_y<Obj.Y+height));
}

void LCD_Init(void) {
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_RS, OUTPUT);
  pinMode(LCD_WR, OUTPUT);
  pinMode(LCD_RD, OUTPUT);
  for (uint8_t i = 0; i < 8; i++){
    pinMode(DPINS[i], OUTPUT);
  }
  //****************************************
  // Secuencia de Inicialización
  //****************************************
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, HIGH);
  digitalWrite(LCD_RD, HIGH);
  digitalWrite(LCD_RST, HIGH);
  delay(5);
  digitalWrite(LCD_RST, LOW);
  delay(20);
  digitalWrite(LCD_RST, HIGH);
  delay(150);
  digitalWrite(LCD_CS, LOW);
  //****************************************
  LCD_CMD(0xE9);  // SETPANELRELATED
  LCD_DATA(0x20);
  //****************************************
  LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
  delay(100);
  //****************************************
  LCD_CMD(0xD1);    // (SETVCOM)
  LCD_DATA(0x00);
  LCD_DATA(0x71);
  LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0xD0);   // (SETPOWER) 
  LCD_DATA(0x07);
  LCD_DATA(0x01);
  LCD_DATA(0x08);
  //****************************************
  LCD_CMD(0x36);  // (MEMORYACCESS)
  LCD_DATA(0x40|0x80|0x20|0x08); // LCD_DATA(0x19);
  //****************************************
  LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
  LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
  //****************************************
  LCD_CMD(0xC1);    // (POWERCONTROL2)
  LCD_DATA(0x10);
  LCD_DATA(0x10);
  LCD_DATA(0x02);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
  LCD_DATA(0x00);
  LCD_DATA(0x35);
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x02);
  //****************************************
  LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
  LCD_DATA(0x04); // 72Hz
  //****************************************
  LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
  LCD_DATA(0x01);
  LCD_DATA(0x44);
  //****************************************
  LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
  LCD_DATA(0x04);
  LCD_DATA(0x67);
  LCD_DATA(0x35);
  LCD_DATA(0x04);
  LCD_DATA(0x08);
  LCD_DATA(0x06);
  LCD_DATA(0x24);
  LCD_DATA(0x01);
  LCD_DATA(0x37);
  LCD_DATA(0x40);
  LCD_DATA(0x03);
  LCD_DATA(0x10);
  LCD_DATA(0x08);
  LCD_DATA(0x80);
  LCD_DATA(0x00);
  //****************************************
  LCD_CMD(0x2A); // Set_column_address 320px (CASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0x3F);
  //****************************************
  LCD_CMD(0x2B); // Set_page_address 480px (PASET)
  LCD_DATA(0x00);
  LCD_DATA(0x00);
  LCD_DATA(0x01);
  LCD_DATA(0xE0);
//  LCD_DATA(0x8F);
  LCD_CMD(0x29); //display on 
  LCD_CMD(0x2C); //display on

  LCD_CMD(ILI9341_INVOFF); //Invert Off
  delay(120);
  LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
  delay(120);
  LCD_CMD(ILI9341_DISPON);    //Display on
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
  digitalWrite(LCD_RS, LOW);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = cmd;
  digitalWrite(LCD_WR, HIGH);
}

//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_WR, LOW);
  GPIO_PORTB_DATA_R = data;
  digitalWrite(LCD_WR, HIGH);
}

//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  LCD_CMD(0x2a); // Set_column_address 4 parameters
  LCD_DATA(x1 >> 8);
  LCD_DATA(x1);   
  LCD_DATA(x2 >> 8);
  LCD_DATA(x2);   
  LCD_CMD(0x2b); // Set_page_address 4 parameters
  LCD_DATA(y1 >> 8);
  LCD_DATA(y1);   
  LCD_DATA(y2 >> 8);
  LCD_DATA(y2);   
  LCD_CMD(0x2c); // Write_memory_start
}

//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c){  
  unsigned int x, y;
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);   
  SetWindows(0, 0, 319, 239); // 479, 319);
  for (x = 0; x < 320; x++)
    for (y = 0; y < 240; y++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
    }
  digitalWrite(LCD_CS, HIGH);
} 

//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i, j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + x;
  SetWindows(x, y, l, y);
  j = l;// * 2;
  for (i = 0; i < l; i++) {
      LCD_DATA(c >> 8); 
      LCD_DATA(c); 
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//*************************************************************************************************************************************** 
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {  
  unsigned int i,j;
  LCD_CMD(0x02c); //write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW);
  l = l + y;
  SetWindows(x, y, x, l);
  j = l; //* 2;
  for (i = 1; i <= j; i++) {
    LCD_DATA(c >> 8); 
    LCD_DATA(c);
  }
  digitalWrite(LCD_CS, HIGH);  
}

//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  H_line(x  , y  , w, c);
  H_line(x  , y+h, w, c);
  V_line(x  , y  , h, c);
  V_line(x+w, y  , h, c);
}

//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+w;
  y2 = y+h;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = w*h*2-1;
  unsigned int i, j;
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      LCD_DATA(c >> 8);
      LCD_DATA(c);
      
      //LCD_DATA(bitmap[k]);    
      k = k - 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background) 
//***************************************************************************************************************************************
void LCD_Print(String text, int x, int y, int fontSize, int color, int background) {
  int fontXSize ;
  int fontYSize ;
  
  if(fontSize == 1){
    fontXSize = fontXSizeSmal ;
    fontYSize = fontYSizeSmal ;
  }
  if(fontSize == 2){
    fontXSize = fontXSizeBig ;
    fontYSize = fontYSizeBig ;
  }
  
  char charInput ;
  int cLength = text.length();
  Serial.println(cLength,DEC);
  int charDec ;
  int c ;
  int charHex ;
  char char_array[cLength+1];
  text.toCharArray(char_array, cLength+1) ;
  for (int i = 0; i < cLength ; i++) {
    charInput = char_array[i];
    Serial.println(char_array[i]);
    charDec = int(charInput);
    digitalWrite(LCD_CS, LOW);
    SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize );
    long charHex1 ;
    for ( int n = 0 ; n < fontYSize ; n++ ) {
      if (fontSize == 1){
        charHex1 = pgm_read_word_near(smallFont + ((charDec - 32) * fontYSize) + n);
      }
      if (fontSize == 2){
        charHex1 = pgm_read_word_near(bigFont + ((charDec - 32) * fontYSize) + n);
      }
      for (int t = 1; t < fontXSize + 1 ; t++) {
        if (( charHex1 & (1 << (fontXSize - t))) > 0 ) {
          c = color ;
        } else {
          c = background ;
        }
        LCD_DATA(c >> 8);
        LCD_DATA(c);
      }
    }
    digitalWrite(LCD_CS, HIGH);
  }
}

//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char bitmap[]){  
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 
  
  unsigned int x2, y2;
  x2 = x+width;
  y2 = y+height;
  SetWindows(x, y, x2-1, y2-1);
  unsigned int k = 0;
  unsigned int i, j;

  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      //LCD_DATA(bitmap[k]);    
      k = k + 2;
     } 
  }
  digitalWrite(LCD_CS, HIGH);
}

//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],int columns, int index, char flip, char offset){
  LCD_CMD(0x02c); // write_memory_start
  digitalWrite(LCD_RS, HIGH);
  digitalWrite(LCD_CS, LOW); 

  unsigned int x2, y2;
  x2 =   x+width;
  y2=    y+height;
  SetWindows(x, y, x2-1, y2-1);
  int k = 0;
  int ancho = ((width*columns));
  if(flip){
  for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width -1 - offset)*2;
      k = k+width*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k - 2;
     } 
  }
  }else{
     for (int j = 0; j < height; j++){
      k = (j*(ancho) + index*width + 1 + offset)*2;
     for (int i = 0; i < width; i++){
      LCD_DATA(bitmap[k]);
      LCD_DATA(bitmap[k+1]);
      k = k + 2;
     } 
  }
    
    
    }
  digitalWrite(LCD_CS, HIGH);
}
