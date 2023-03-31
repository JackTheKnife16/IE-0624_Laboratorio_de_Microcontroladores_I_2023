#include <pic14/pic12f675.h>

//char vector_resultados[16];
char head = 0; // apunta al ultimo resultado
char ball_counter = 0;

    
short int time = 1;
char flag_eje = 0;
char counter = 0;
char num = 0b11111111; 
char bandera = 0;
char random = 0;


//To compile:
//sdcc -mpic14 -p16f675 blink.c
 
//To program the chip using picp:
//Assuming /dev/ttyUSB0 is the serial port.
 
//Erase the chip:
//picp /dev/ttyUSB0 16f887 -ef
 
//Write the program:
//picp /dev/ttyUSB0 16f887 -wp blink.hex
 
//Write the configuration words (optional):
//picp /dev/ttyUSB0 16f887 -wc 0x2ff4 0x3fff
 
//Doing it all at once: erasing, programming, and reading back config words:
//picp /dev/ttyUSB0 16f887 -ef -wp blink.hex -rc
 
//To program the chip using pk2cmd:
//pk2cmd -M -PPIC16f887 -Fblink.hex

// estados
#define IDLE 0
#define FIRST 1
#define NORMAL 2
#define LAST 3
#define BLINK 4

void calcular_random(char* random);
void delay (unsigned int tiempo);
void generador_pulso(unsigned int tiempo);
void reset_counter(unsigned int tiempo);
void setear_bit2bit(char* flag_eje, char* num, char* counter, unsigned int time);
char obtener_codigo(char* random);

char current_state = IDLE;



void main(void) {

    TRISIO = 0b00001000; //Poner todos los pines como salidas
	GPIO = 0x00; //Poner pines en bajo
    


    // inicializar los valores del vector en 100
 
    //Loop forever
    while ( 1 ) {

        calcular_random(&random);
        if(bandera == 0 && GP3 == 1){
            bandera = 1;
            flag_eje = 0;
            num = obtener_codigo(&random);
        }
        if(GP3 == 0){
            bandera = 0;
        }
        
        setear_bit2bit(&flag_eje, &num, &counter, time);
        
    }
 
}

char obtener_codigo(char* random){
    char num;
    char dig1 = (*random) % 10;
    char dig2 = (*random) / 10;
    num = dig2 << 4;
    num |= dig1;
    return num;
}


// esta funcion es un simple contador que aumenta su valor cada vez que
// es llamada, va de 0 a 99.
void calcular_random(char* random){
    if(*random == 99){
        *random = 0;
    }
    else {
        (*random)++;
    }
}

// esta funcion es la encargada de la comunicacion serial con el circuito externo
// maneja los pines GP0 que comunica el dato, el pin GP1 que genera el pulso
// y el pin GP2 que resetea el counter externo como medida de seguridad.
void setear_bit2bit(char* flag_eje, char* num, char* counter, unsigned int time){
    if(*flag_eje == 0){
        GP0 = *num & 1;
        *num = *num >> 1;
    
        if(*counter == 0){
            reset_counter(time);
        }
        generador_pulso(time);
        
        (*counter)++;
        if(*counter == 8){
            *counter = 0;
            *flag_eje = 1;
        }
    }
}

// funcion que agrega un delay de tiempo segun lo que se desee, es a puro ojo
void delay(unsigned int tiempo)
{
	unsigned int i;
	unsigned int j;

	for(i=0;i<tiempo;i++)
	  for(j=0;j<200;j++);
}

// esta funcion genera los pulsos para el contador que maneja el clk de los flip flops de datos
// y el clk del contador que decide hacia que flip flop se dirige el dato
void generador_pulso(unsigned int tiempo){
    delay(tiempo);
    GP1 = 1;
    delay(tiempo);
    GP1 = 0;
}

// esta funcion es una medida de seguridad para resetear el contador siempre que se terminan de
// enviar los datos seriales
void reset_counter(unsigned int tiempo){
    delay(tiempo);
    GP2 = 1;
    delay(tiempo);
    GP2 = 0;
}

