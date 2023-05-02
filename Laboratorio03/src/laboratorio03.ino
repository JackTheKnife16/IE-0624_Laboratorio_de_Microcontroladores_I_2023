/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#define ILI9341_BLACK       0x0000  ///<   0,   0,   0
#define ILI9341_NAVY        0x000F  ///<   0,   0, 123
#define ILI9341_DARKGREEN   0x03E0  ///<   0, 125,   0
#define ILI9341_DARKCYAN    0x03EF  ///<   0, 125, 123
#define ILI9341_MAROON      0x7800  ///< 123,   0,   0
#define ILI9341_PURPLE      0x780F  ///< 123,   0, 123
#define ILI9341_OLIVE       0x7BE0  ///< 123, 125,   0
#define ILI9341_LIGHTGREY   0xC618  ///< 198, 195, 198
#define ILI9341_DARKGREY    0x7BEF  ///< 123, 125, 123
#define ILI9341_BLUE        0x001F  ///<   0,   0, 255
#define ILI9341_GREEN       0x07E0  ///<   0, 255,   0
#define ILI9341_CYAN        0x07FF  ///<   0, 255, 255
#define ILI9341_RED         0xF800  ///< 255,   0,   0
#define ILI9341_MAGENTA     0xF81F  ///< 255,   0, 255
#define ILI9341_YELLOW      0xFFE0  ///< 255, 255,   0
#define ILI9341_WHITE       0xFFFF  ///< 255, 255, 255
#define ILI9341_ORANGE      0xFD20  ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define ILI9341_PINK        0xFC18  ///< 255, 130, 198

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_CLK 13
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8

#define TOTAL_SOURCE 4

int voltage_source[4] = {0,0,0,0};
int voltage_buffer[4] = {0,0,0,0};
int voltage_colors[4] = {ILI9341_YELLOW, ILI9341_DARKGREEN, ILI9341_CYAN, ILI9341_WHITE};
char* v_str[4] = {"V1: ", "V2: ", "V3: ", "V4: "};
char* c_type[2] = {"DC", "AC"};
int current_type;
int current_buffer = 0;
int voltages_position = 4;

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// If using the breakout, change pins as desired
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);


void update_source_val(){
  for(int k = 0; k < TOTAL_SOURCE; k++){
    // se guardan los resultados cuantizados de las entradas analogicas 0, 1, 2, 3
    // en el array voltage_source
    voltage_source[k] = analogRead(k);
  }

}

void setear_pantalla(){
  tft.setCursor(50, 1);
  tft.fillRect(0, 1 * 8, tft.width(), 8, ILI9341_BLACK); 

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(90, 1 * 30); 
  tft.println("IE-0624");

  tft.setTextSize(1);
  tft.setCursor(50, 2 * 30); 
  tft.println("VOLTIMETRO DE 4 CANALES");

  // este codigo hace el recuadro blanco y escribe los voltajes en colores
  tft.setTextSize(3);
  tft.drawRoundRect(15, voltages_position * 30 - 15, tft.width()-30, 160, 10, ILI9341_WHITE);
  for(int k = 0; k < 4; k++){
    tft.setCursor(20, (voltages_position + k) * 30); // 8 píxeles de altura por línea
    tft.setTextSize(3);
    tft.setTextColor(voltage_colors[k]);
    tft.print(v_str[k]);
    if(voltage_source[k] != voltage_buffer[k]){
      voltage_buffer[k] = voltage_source[k];
      tft.fillRect(90, (voltages_position + k) * 30, 120, 30, ILI9341_BLACK); // Borrar la línea anterior   
    }
    tft.print(voltage_buffer[k]);
    tft.println("V");
  }

  // este codigo es para indicar si estamos en AC o en DC
  tft.setTextSize(2);
  tft.setCursor(195, 245);
  tft.setTextColor(ILI9341_WHITE);
  if(current_type != current_buffer){
    current_buffer = current_type;
    tft.fillRect(195, 245, 20, 16, ILI9341_BLACK);
  }
  tft.print(c_type[current_buffer]);


}

void setup() {
  // se declara el pin digital 7 como una entrada
  pinMode(7, INPUT);
  // Inicializar el objeto de pantalla TFTLCD
  tft.begin();

}

void loop() {
  // se guarda el valor del pin digital 7 en la variable current_type
  current_type = digitalRead(7);
  // se guardan el valor cuantizado de los pines analogicos 0, 1, 2 y 3
  update_source_val();
  // setea los datos que se mostraran en la pantalla
  setear_pantalla();
  
  //delay(100); // Esperar antes de actualizar de nuevo
}