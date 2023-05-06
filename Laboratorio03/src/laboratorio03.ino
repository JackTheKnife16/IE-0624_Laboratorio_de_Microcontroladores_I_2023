/***************************************************
  LABORATORIO 03 - FREDDY ZUNIGA CERDAS
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
#include <math.h>

// For the Adafruit shield, these are the default.
#define TFT_CLK 13
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8

#define TOTAL_SOURCE 4
#define CRECIENTE 1
#define DECRECIENTE 0
#define ESTACIONARIO 2

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
int voltage_source[4] = {0,0,0,0};
int voltage_buffer[4] = {0,0,0,0};
int voltage_colors[4] = {ILI9341_YELLOW, ILI9341_DARKGREEN, ILI9341_CYAN, ILI9341_WHITE};
char* v_str[4] = {"V1: ", "V2: ", "V3: ", "V4: "};
char* c_type[2] = {"DC", "AC"};
int current_type;
int current_buffer = -1;
int comunicacion_serial_habilitada = 0;
int voltages_position = 4;
uint8_t primera_lectura = 1;
uint16_t lectura_actual;
uint16_t lectura_anterior;
uint32_t tiempo_actual;
uint32_t tiempo_anterior;
uint8_t primer_crecimiento = 1;
uint8_t crecimiento_actual;
uint8_t crecimiento_anterior;
uint8_t pivote_listo = 0;
uint16_t pivote[4];
uint32_t tiempo_pivote[4];
uint8_t indice = 0; // indice 
uint8_t habilitar_pantalla = 0;
double suma = 0.0;
double rms_array[4];
int rms_anterior[4] = {0,0,0,0};
int rms_actual[4];
double dc_array[4];
uint16_t minimo[4];
uint16_t maximo[4];
uint32_t periodo[4];

////////////////////////////////////////////////////////////////////////////////

// resetea valores para empezar de nuevo con la obtencion del rms
void reiniciar_marcas(){
  suma = 0.0;
  primera_lectura = 1;
  primer_crecimiento = 1;
  pivote_listo = 0;
}


// calcula el rms de la senal alterna
void calcular_rms(uint8_t index){
  periodo[index] = tiempo_actual - tiempo_pivote[index];
  double rms = (double) 1 / periodo[index] * suma;
  rms = sqrt(rms);
  rms_actual[index] = rms * 100;
  rms_array[index] = (double) rms_actual[index] / 100; 
}

// convierte un valor cuantizado a una tension real
double convertir_tension_real(int cuantizado){
  double tension_completa = (double) 48 / 1023 * cuantizado - 24;
  return tension_completa;
}

// esto acumula la suma de rectangulos bajo la curva, como en calculo 1
void sumar_rectangulo(){
  double tension_completa = (double) 48 / 1023 * lectura_actual - 24;
  int delta_tiempo = tiempo_actual - tiempo_anterior;
  suma += (double) pow(tension_completa, 2) * delta_tiempo;
}

void obtener_minimo(int k){
  if(crecimiento_anterior == CRECIENTE){;}
  else{
    if(crecimiento_actual == CRECIENTE) {
      minimo[k] = lectura_anterior;
    }
  }
}

void obtener_maximo(int k){
  if(crecimiento_anterior == DECRECIENTE){;}
  else{
    if(crecimiento_actual == DECRECIENTE) {
      maximo[k] = lectura_anterior;
    }
  }
}

// obtiene la monotonia de la curva
void obtener_crecimiento(){
  if(lectura_actual > lectura_anterior){
    crecimiento_actual = CRECIENTE;
  }
  else{
    if(lectura_actual < lectura_anterior){
      crecimiento_actual = DECRECIENTE;
    }
    else{
      crecimiento_actual = ESTACIONARIO;
    }
  }
}

// algoritmo para obtener el RMS a partir de las senales de entrada
void procesando_analog(int k){
  if(primera_lectura == 1){
    primera_lectura = 0;
    lectura_actual = analogRead(k);
    tiempo_actual = micros();
    minimo[indice] = 2000;
    maximo[indice] = 0;
  }
  else{
    lectura_anterior = lectura_actual;
    tiempo_anterior = tiempo_actual;
    lectura_actual = analogRead(k);
    tiempo_actual = micros();
    if(primer_crecimiento == 1) {
      obtener_crecimiento();
      primer_crecimiento = 0;
    }
    else{
      crecimiento_anterior = crecimiento_actual;
      obtener_crecimiento();
      obtener_minimo(indice);
      obtener_maximo(indice);
      if(pivote_listo == 1) {
        if(crecimiento_actual == DECRECIENTE){
          if(crecimiento_anterior == DECRECIENTE){
            sumar_rectangulo();
          }
          else{
            calcular_rms(indice);
            reiniciar_marcas();
            if(indice == 3){
              habilitar_pantalla = 1; 
            }
            else{
              indice++;
            }
          }

        }
        else{
          sumar_rectangulo();
        }
      }
      else{
        if(crecimiento_actual == DECRECIENTE){
          if(crecimiento_anterior == DECRECIENTE){
            ;
          }
          else{
            pivote_listo = 1;
            pivote[indice] = lectura_anterior;
            tiempo_pivote[indice] = tiempo_anterior;
            sumar_rectangulo();
          }
        }
      }
    }
  }
}




// 
void update_source_val(){
  for(int k = 0; k < TOTAL_SOURCE; k++){
    voltage_buffer[k] = voltage_source[k];
    voltage_source[k] = analogRead(k);
    dc_array[k] = convertir_tension_real(voltage_source[k]);
  }
}


// se encarga de setear la pantalla lcd
void setear_pantalla(){
  // esto dibuja el encabezado 
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
    tft.setCursor(20, (voltages_position + k) * 30); 
    tft.setTextSize(3);
    tft.setTextColor(voltage_colors[k]);
    tft.print(v_str[k]);

    if(current_type){
      tft.fillRect(90, (voltages_position + k) * 30, 130, 30, ILI9341_BLACK); // Borrar la línea anterior  
      tft.print(rms_array[k]);
    }
    else{
      tft.fillRect(90, (voltages_position + k) * 30, 130, 30, ILI9341_BLACK); // Borrar la línea anterior   
      tft.print(dc_array[k]);
    }
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


  // este codigo pone el mensaje del final sobre el rango
  tft.setTextSize(1);
  tft.setCursor(30, 280); 
  tft.println("rango soportado: [-24V, 24V]");


}



// funcion encargada de enviar datos por el puerto serial
void enviar_datos(){
  if(digitalRead(2) == HIGH){
    if(current_type == 1){
      for(int k = 0; k < 4; k++){
        if(rms_anterior[k] != rms_actual[k]){
          Serial.print("AC,"); // tipo de corriente
          Serial.print(k+1); // numero de fuente
          Serial.print(",");
          Serial.print(rms_array[k]); // rms de la fuente
          Serial.print(",");
          Serial.print(micros()); // tiempo en us
          Serial.println("");
        }
      }
    }
    else{
      for(int k = 0; k < 4; k++){
        if(voltage_source[k] != voltage_buffer[k]){
          Serial.print("DC,"); // tipo de corriente
          Serial.print(k+1); // numero de fuente
          Serial.print(",");
          Serial.print(dc_array[k]); // rms de la fuente
          Serial.print(",");
          Serial.print(micros()); // tiempo en us
          Serial.println("");
        }
      }
    }
  }
}



void setup() {
  // PINES DE ENTRADA Y SALIDA DIGITALES
  pinMode(7, INPUT); // BOTON AC/DC
  pinMode(6, OUTPUT); // LED INDICADOR PARA FUENTE 3
  pinMode(5, OUTPUT); // LED INDICADOR PARA FUENTE 2
  pinMode(4, OUTPUT); // LED INDICADOR PARA FUENTE 1
  pinMode(3, OUTPUT); // LED INDICADOR PARA FUENTE 0
  pinMode(2, INPUT); // BOTON HABILITADOR SERIAL 
  // Inicializar el objeto de pantalla TFTLCD
  tft.begin();
}



void comunicacion_serial(){
  // Verifica si el pin de habilitación está en estado bajo
  if (digitalRead(2) == HIGH) {
    if (comunicacion_serial_habilitada == 0) {
      Serial.begin(9600); // Inicia la comunicación serial
      comunicacion_serial_habilitada = 1;
      for(int k = 0; k < 4; k++){
        voltage_source[k] = 0;
        rms_anterior[k] = 0;
      }
    }
  } else {
    if (comunicacion_serial_habilitada == 1) {
      Serial.end(); // Detiene la comunicación serial
      comunicacion_serial_habilitada = 0;
    }
  }
}

void alarma_dc(){
  for(int k = 0; k < 4; k++){
    if(voltage_source[k] > 939 || voltage_source[k] < 85){
      digitalWrite(k+3, HIGH); 
    }
    else{
      digitalWrite(k+3, LOW); 
    }
  }
}

void alarma_ac(){
  for(int k = 0; k < 4; k++){
    if(maximo[k] > 937 || minimo[k] < 85){
      digitalWrite(k+3, HIGH); 
    }
    else{
      digitalWrite(k+3, LOW); 
    }
  }
}

void loop() {
  current_type = digitalRead(7);
  comunicacion_serial();

  if(current_type == 1) {
    for(int k = 0; k < 4; k++){
      voltage_source[k] = 0;
    }
    if(habilitar_pantalla == 1){
      enviar_datos();
      alarma_ac();
      setear_pantalla();
      reiniciar_marcas();
      habilitar_pantalla = 0;
      indice = 0;
      for(int k = 0; k < 4; k++){
        rms_anterior[k] = rms_actual[k];
      }
    }
    else{
      procesando_analog(indice);
    } 
  }
  else{
    for(int k = 0; k < 4; k++){
      rms_anterior[k] = 0;
    }
    habilitar_pantalla = 0;
    update_source_val();
    enviar_datos();
    alarma_dc();
    setear_pantalla();
  }
}