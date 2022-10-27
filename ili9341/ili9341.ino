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
// DEFINICIÓN DE PANTALLA ILI9341
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
// COFIGURACIÓN DE ALIENS
#define NUM_ALIEN_COLUMNS 7
#define NUM_ALIEN_ROWS 3
#define Y_START_POS 0
#define X_START_OFFSET 10 // POSICION DE SEPARACIÓN ENTRE BORDE DE PANTALLA Y ALIEN
#define SPACE_BETWEEN_ALIEN_COLUMNS 15
#define LARGEST_ALIEN_WIDTH 24
#define ALIEN_HEIGHT 16
#define SPACE_BETWEEN_ROWS 26
#define INVADERS_DROP_BY 4 // PIXELES QUE LOS INVADERS SE MUEVEN HACIA ABAJO
#define INVADERS_SPEED 12  // VELOCIDAD DE MOVIMIENTO ENTRE MÁS ALTO VALOR MÁS LENTO
#define ACTIVE 0
// DEFINICIÓN DE PINES PARA ILI9341
#define LCD_RST PD_0
#define LCD_CS PD_1
#define LCD_RS PD_2
#define LCD_WR PD_3
#define LCD_RD PE_1
int DPINS[] = {PB_0, PB_1, PB_2, PB_3, PB_4, PB_5, PB_6, PB_7};  


//***************************************************************************************************************************************
// Variables & arrays (INVADERS)
//***************************************************************************************************************************************
int acrossDisp = 0;
int downDisp = 0;
int acrossMov = 0;
int downMov = 0;
int AcrossRMP = NUM_ALIEN_COLUMNS - 1;
int DownRMP = 0;
int Largest = 0;
int RightPos;
int AcrossLMP = 0;
int DownLMP = 0;
int Smallest = SCREEN_WIDTH*2;

int previousMillis = 0;
int previousMillis2 = 0;
int previousMillis3 = 0;
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
int moveByXAmount1 = 0;
int moveByXAmount2 = 0;
int moveByXAmount3 = 0;
int moveByYAmount = 0;
signed char InvadersMoveCounter = 0;
int activeFrame = 1;
int frameCount = 0;
bool lowered = false;
int moveInvaderTime = 3;
int moveInvaderTime2 = 5;
int moveInvader = 0;
//***************************************************************************************************************************************
// Functions Prototypes
//***************************************************************************************************************************************
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
void UpdateInvaders(void);
void initInvaders(int Y_initial);
void invaderControl(void);
int RightMostPos(void);
int LeftMostPos(void);
void posControl(void);
void animationUpdate(void);

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  // Frecuencia de reloj (utiliza TivaWare)
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  Serial.begin(9600);
  // Configuración del puerto (utiliza TivaWare)
  GPIOPadConfigSet(GPIO_PORTB_BASE, 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
  Serial.println("Inicio");
  
  // Inicialización de la pantalla
  LCD_Init();
  LCD_Clear(0x0000);
  // Referencia para colores RGB565: http://www.rinkydinkelectronics.com/calc_rgb565.php
  initInvaders(Y_START_POS);
  pinMode(6, OUTPUT);
 
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************

void loop() {

  // PRESIÓN DE BOTONES
  //invaderControl();
  animationUpdate();
  UpdateInvaders();
    
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
  unsigned long currentMillis = millis();
  unsigned long currentMillis3 = millis();
  if ((currentMillis - previousMillis) > 1000 & (currentMillis - previousMillis) < 2005){
    moveByXAmount1 = 1;
    moveByXAmount2 = 1;
    moveByXAmount3 = 1;
    digitalWrite(6, HIGH);
  }
  else if((currentMillis3 - previousMillis3) >= 2000 & (currentMillis3 - previousMillis3) < 20000){
    moveByXAmount1 = -1;
    moveByXAmount2 = -1;
    moveByXAmount3 = -1;
    digitalWrite(6, LOW);    
    previousMillis = currentMillis;
    previousMillis3 = currentMillis3;
  }
}

void animationUpdate(void){
  unsigned long currentMillis2 = millis();
  if ((currentMillis2 - previousMillis2) > 1000){
    activeFrame = !activeFrame;
    previousMillis2 = currentMillis2;
  }
}

void invaderControl(void){
  if ((InvadersMoveCounter--)<0){
    lowered = false;
    if ((RightMostPos() + AlienXMoveAmount >= SCREEN_WIDTH) | (LeftMostPos() + AlienXMoveAmount < 0)){
      AlienXMoveAmount = -AlienXMoveAmount;
      lowered = true;
    }
    if (acrossMov < NUM_ALIEN_COLUMNS){
      if (downMov < 3){
        if (Alien[acrossMov][downMov].Ord.Status == ACTIVE){
          if (lowered == false){
            Alien[acrossMov][downMov].Ord.X = Alien[acrossMov][downMov].Ord.X + AlienXMoveAmount;
            digitalWrite(6, LOW);
          }
          else {
            Alien[acrossMov][downMov].Ord.Y += INVADERS_DROP_BY;
            
          }
          if (acrossMov == (NUM_ALIEN_COLUMNS-1)){
            downMov++;
            acrossMov = -1;
          }
        }       
        acrossMov++;
      }     
      InvadersMoveCounter = INVADERS_SPEED;
      digitalWrite(6, HIGH);
      //activeFrame = !activeFrame;
    }/*
    if (acrossMov == (NUM_ALIEN_COLUMNS-1) & downMov == (NUM_ALIEN_ROWS-1)){
      downMov = 0;
      acrossMov = 0;
    }*/
  }
  
}

int RightMostPos(void){
  if (AcrossRMP>=0){
    if (DownRMP < NUM_ALIEN_ROWS){
      if (Alien[AcrossRMP][DownRMP].Ord.Status == ACTIVE){
        RightPos = Alien[AcrossRMP][DownRMP].Ord.X + AlienWidth[DownRMP];
        if (RightPos > Largest){Largest = RightPos;}
      }
      DownRMP++;
    }
    if (Largest > 0){
      return Largest;
    }
    AcrossRMP--;
  }
  else if (AcrossRMP < 0){
    AcrossRMP = NUM_ALIEN_COLUMNS-1;
    DownRMP = 0;
    Largest = 0;
    RightPos = 0;
  }
  //return 0;
}

int LeftMostPos(void){
  if (AcrossLMP < NUM_ALIEN_COLUMNS){
    if (DownLMP < 3){
      if (Alien[AcrossLMP][DownLMP].Ord.Status == ACTIVE){
        if (Alien[AcrossLMP][DownLMP].Ord.X < Smallest){
          Smallest = Alien[AcrossLMP][DownLMP].Ord.X;
        }
      }
      DownLMP++;
    }
    if (Smallest < SCREEN_WIDTH*2){
      return Smallest;
    }
    AcrossLMP++;
  }
  else if (AcrossLMP == (NUM_ALIEN_COLUMNS-1)){
    AcrossLMP = 0;
    DownLMP = 0;
    Smallest = SCREEN_WIDTH*2;
  }
  //return 0;
}

//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
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
