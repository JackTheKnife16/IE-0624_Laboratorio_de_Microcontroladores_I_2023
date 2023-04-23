#include <avr/io.h>
#include <avr/interrupt.h>

#define OFF 0
#define ON 1
// Define el estado del bloque ON/PAUSE
#define AMBOS_DESACTIVADOS 0
#define ON_ACTIVADO 1
#define PAUSE_ACTIVADO 2
// define los parametros para las matrices
#define ROWS 3
#define COLS 4

#define BAJA 0
#define MEDIA 1
#define ALTA 2
#define SUMINISTRO 0
#define LAVAR 1
#define ENJUAGAR 2
#define CENTRIFUGAR 3


volatile uint8_t contador = 0;
volatile uint8_t value;
volatile uint8_t valueK;
volatile uint8_t on_pause_led = OFF;
volatile uint8_t estado_on_pause = AMBOS_DESACTIVADOS;
volatile uint16_t pause_time;

volatile uint8_t carga;
volatile uint8_t state = SUMINISTRO;

// matrices de tiempos e intervalos
volatile uint8_t matriz_tiempos[ROWS][COLS] = {{1,3,2,3},{2,7,4,6},{3,10,5,9}};
volatile uint8_t limite_inferior[ROWS][COLS];
volatile uint8_t limite_superior[ROWS][COLS];
volatile uint8_t tiempos_iniciales[ROWS] = {0,0,0};


// prototipos de funciones
void apagar_leds();
void seteando_matrices();
void encender_led_carga();
void encender_led_on_pause();
void refrescar_displays();
void interrupcion_modo_carga_buttons(int habilitar);
void interrupcion_on_pause_button(int habilitar);
void interrupcion_cuenta_regresiva(int habilitar);
void finite_state_machine();

// Función de inicialización del hardware
void init(void)
{

    // Configura Timer/Counter1 para contar eventos internos en modo CTC
    TCCR1B |= (1 << WGM12);
    // Configura el valor de comparación del timer para 15624 ciclos de reloj
    OCR1A = 15624;
    // Configura el prescaler del timer para dividir el reloj del sistema por 1024
    TCCR1B |= (1 << CS12) | (1 << CS10);

    //////////////////////////////////////////////////////////////
    // Configuración del Timer0
    TCCR0A |= (1 << WGM01);  // Modo de temporizador CTC
    TCCR0B |= (1 << CS01) | (1 << CS00);  // Preescaler de 64
    OCR0A = 49;

    // Configuración de la interrupción
    TIMSK |= (1 << OCIE0A);  // Habilitar interrupción en comparación con A



    ///////////////////////////////////////////////////////////////
    // Configura todos los pines del puerto B como salidas, estos son los
    // que controlan todos los leds y los displays de 7 segmentos
    DDRB = 0xff;

    // Configurar el pin de interrupción INT0 (PD2) como entrada
    DDRD &= ~(1 << PD2);
    
    ////////////// ESTAS SON LAS SALIDAS QUE MANEJAN LOS LEDS DE: SECUENCIA DE LAVADO //////////////
    DDRD |= (1 << PD6); // ETIQUETA: SELECTOR
    // configurar pines PD4 y PD5 como salidas
    DDRD |= (1 << PD4); // ETIQUETA: ENJUAGAR
    DDRD |= (1 << PD5); // ETIQUETA: CENTRIFUGAR
    // configurar pines PA1 y PA0 como salidas
    DDRA |= (1 << PA0); // ETIQUETA: LAVAR
    DDRA |= (1 << PA1); // ETIQUETA: SUMINISTRO

    
    // activar pull down externa
    //PORTD &= ~(1 << PD2);
    // Configurar la interrupción INT0 para que se active en flanco de subida
    MCUCR |= (1 << ISC01);
    MCUCR |= (1 << ISC00);
  
    // Habilitar la interrupcion de los PCINT17..11
    interrupcion_modo_carga_buttons(1);

    // esto habilita los pines PD0 PD1 PD3 como pines de interrupcion
    PCMSK2 |= (1 << PCINT11); // BAJA
    PCMSK2 |= (1 << PCINT12); // MEDIA
    PCMSK2 |= (1 << PCINT14); // ALTA
  
    // Habilitar las interrupciones globales
    sei();
}

// REFERIDO: al timer0, en modo comparacion
// TIPO: timer
// DESCRIPCION: se encarga de hacer la actualizacion y el
// refrescamiento de los dos  displays de 7 segmentos
ISR(TIMER0_COMPA_vect) {
    refrescar_displays();
    if(contador == 0){
        apagar_leds();
        // cuando el contador llega a 0 se desactiva la interrupcion.
        interrupcion_cuenta_regresiva(OFF);
    }
}

// REFERIDO: al timer1, en modo comparacion
// TIPO: timer
// DESCRIPCION: se encarga de hacer la cuenta regresiva, se ejecuta
// cada segundo y empieza la cuenta regresiva 
ISR(TIMER1_COMPA_vect) {
    if( contador != 0 ){
        contador--;
        finite_state_machine();
    }
}


// REFERIDO: al MODO CARGA BUTTONS,
// TIPO: activacion por pulsador o boton.
// DESCRIPCION: setea tanto el valor de los leds de 7 segmentos como los leds de MODO CARGA
// esto nos muestra si el modo es Baja carga, Media carga o Alta carga.
ISR(PCINT2_vect) {
    // se activa la interrupcion del ON/PAUSE BUTTON
    interrupcion_on_pause_button(ON);
    encender_led_carga();
}


// REFERIDO: al ON/PAUSE BUTTON
// TIPO: activacion por pulsador o boton.
// DESCRIPCION: se dispara cuando es presionado el boton ON/PAUSE
ISR(INT0_vect)
{
    // Desactiva la interrupcion de MODO CARGA BUTTONS
    interrupcion_modo_carga_buttons(OFF);
    // pone en cero el contador del timer1 para que cuente el primer
    // segundo de manera correcta
    if(estado_on_pause == AMBOS_DESACTIVADOS){
        TCNT1 = 0;
        // enciende el led correspondiente a suministro de agua
        PORTA |= (1 << PA1);
        state = SUMINISTRO;
        estado_on_pause = ON_ACTIVADO;
    }
    // si esta con ON_ACTIVADO, al presionarse el pulsador guardara la cuenta que lleva
    // en el momento una variable y su nuevo estado sera de PAUSE
    else if(estado_on_pause == ON_ACTIVADO){
        pause_time = TCNT1;
        estado_on_pause = PAUSE_ACTIVADO;
    }
    // si esta con PAUSE_ACTIVADO cuando se presione el pulsador retormara la cuenta donde
    // se habia quedado y el estado cambiara a ON_ACTIVADO
    else{
        TCNT1 = pause_time;
        estado_on_pause = ON_ACTIVADO;
    }
    encender_led_on_pause();
}


void encender_led_on_pause(){
    // ESTADO DE PAUSE
    // se activa si el pinB0 esta en alto
    if(PINB & (1 << PB0)){
        // enciende el led indicador de PAUSE
        PORTB |= (1 << PB1);
        // apaga el led indicador de ON
        PORTB &= ~(1 << PB0);
        // se desactiva la interrupcion de 1 segundo cuando estamos en pausa
        interrupcion_cuenta_regresiva(OFF);
    }
    // ESTADO DE ON
    // se activa si el pinB0 esta en bajo
    else{
        // enciende el led indicador de ON
        PORTB |= (1 << PB0);
        // apaga el led indicador de PAUSE
        PORTB &= ~(1 << PB1);
        // se activa la interrupcion de 1 segundo cuando estamos en on
        on_pause_led = ON;
        interrupcion_cuenta_regresiva(ON);
    }
}


void refrescar_displays(){
    PORTD ^= (1 << PD6);
    if(PIND & (1 << PD6)) {
        // value es el valor de las unidades que se mostraran en el
        // display de 7 segmentos de la derecha
        if(on_pause_led == ON){
            value = contador / 10;
        }
        else{
            value = 0;
        }
        PORTB = (PORTB & 0x0F) | (value << 4);
    }
    else{
        // value es el valor de las decenas que se mostraran en el
        // display de 7 segmentos de la izquierda
        if(on_pause_led == ON){
            value = contador % 10;
        }
        else{
            value = 0;
        }
        PORTB = (PORTB & 0x0F) | (value << 4);
    }
}

void apagar_leds(){
    // apaga el led indicador de ON
    PORTB &= ~(1 << PB0);
    // apaga el led indicador de PAUSE
    PORTB &= ~(1 << PB1);
    // estas dos instrucciones apagan el
    // led de MODO CARGA que este encendido
    PORTB &= ~(1 << PB2);
    PORTB &= ~(1 << PB3);

    // leds de secuencia de lavado
    PORTA &= ~(1 << PA0);            
    PORTA &= ~(1 << PA1);  
    PORTD &= ~(1 << PD4);      
    PORTD &= ~(1 << PD5);
    // se habilita la interrupcion de MODO CARGA BUTTONS
    interrupcion_modo_carga_buttons(ON);
    on_pause_led = OFF;
    estado_on_pause = AMBOS_DESACTIVADOS;
}

void interrupcion_modo_carga_buttons(int habilitar){
    if( habilitar == ON ){
        // habilita la interrupcion
        GIMSK |= (1 << PCIE2); 
    }
    else{
        // deshabilita la interrupcion
        GIMSK &= ~(1 << PCIE2);
    }
}

void interrupcion_on_pause_button(int habilitar){
    if( habilitar == ON ){
        // habilita la interrupcion
        GIMSK |= (1 << INT0); 
    }
    else{
        // deshabilita la interrupcion
        GIMSK &= ~(1 << INT0);
    }
}

void interrupcion_cuenta_regresiva(int habilitar){
    if( habilitar == ON ){
        // habilita la interrupcion
        TIMSK |= (1 << OCIE1A);
    }
    else{
        // deshabilita la interrupcion
        TIMSK &= ~(1 << OCIE1A);
    }
}

void encender_led_carga(){
    // baja carga
    if( PIND & (1 << PD0) ) {
        PORTB |= (1 << PB2);
        PORTB &= ~(1 << PB3);
        // setea el contador dependiendo de la carga
        contador = tiempos_iniciales[0];
        carga = BAJA;
    }
    // media carga
    else if (PIND & (1 << PD1) ){
        PORTB &= ~(1 << PB2);
        PORTB |= (1 << PB3);
        // setea el contador dependiendo de la carga
        contador = tiempos_iniciales[1];
        carga = MEDIA;
    }
    // alta carga
    else if (PIND & (1 << PD3) ){
        PORTB |= (1 << PB2);
        PORTB |= (1 << PB3);
        // setea el contador dependiendo de la carga
        contador = tiempos_iniciales[2];
        carga = ALTA;
    }
}


void seteando_matrices(){
    // esto setea los tiempos maximos de cada una de los modos de
    // carga, sera el valor maximo de la cuenta regresiva
    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLS; j++){
            tiempos_iniciales[i] += matriz_tiempos[i][j];
        }
    }
    // Crea las matrices para los limites inferiores y superiores
    // de los conteos, esto nos dice cuando cambiar entre un estado
    // y otro en la FSM
    for(int i = 0; i < ROWS; i++){
        valueK = tiempos_iniciales[i];
        for(int j = 0; j < COLS; j++){
            valueK -= matriz_tiempos[i][j];
            limite_inferior[i][j] = valueK;
            limite_superior[i][j] = valueK + matriz_tiempos[i][j];
        }
    }
}
void finite_state_machine(){
    // MAQUINA DE ESTADOS FINITA
    switch (state) {
    case SUMINISTRO:
        if(contador <= limite_superior[carga][SUMINISTRO] && contador > limite_inferior[carga][SUMINISTRO]){
            state = SUMINISTRO;
            PORTA |= (1 << PA1);
        }
        else{
            PORTA |= (1 << PA0);
            PORTA &= ~(1 << PA1);
            state = LAVAR;
        }
        break;
    case LAVAR:
        if(contador <= limite_superior[carga][LAVAR] && contador > limite_inferior[carga][LAVAR]){
            state = LAVAR;
        }
        else{
            PORTA &= ~(1 << PA0);
            PORTD |= (1 << PD4);
            state = ENJUAGAR;
        }
        break;
    case ENJUAGAR:
        if(contador <= limite_superior[carga][ENJUAGAR] && contador > limite_inferior[carga][ENJUAGAR]){
            state = ENJUAGAR;
        }
        else{
            PORTD &= ~(1 << PD4);
            PORTD |= (1 << PD5);
            state = CENTRIFUGAR;
        }
        break;
    case CENTRIFUGAR:
        state = CENTRIFUGAR;
        break;
    }
}



int main(void)
{
  // Inicializar el hardware
  init();
  seteando_matrices();
  PORTB = 0x00;

  while (1)
  {
    // El programa principal no hace nada
  }
  
  return 0;
}
