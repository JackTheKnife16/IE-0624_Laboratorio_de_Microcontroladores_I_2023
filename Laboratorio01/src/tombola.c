#include <pic14/pic12f675.h>
 
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
 
void delay (unsigned int tiempo);
void generador_pulso(unsigned int tiempo);
void reset_counter(unsigned int tiempo);
void reset_prueba(unsigned int* tiempo);

/* void enviar_datos(unsigned int* flag_, */
/*                   unsigned int* num_, */
/*                   unsigned int* counter_, */
/*                   unsigned int time_); */
 
void main(void)
{

    TRISIO = 0b00000000; //Poner todos los pines como salidas
	GPIO = 0x00; //Poner pines en bajo
 
    unsigned int time = 500;
    unsigned int flag_eje = 0;
    unsigned int counter = 0;
    unsigned int num = 0b10000111;
 
    //Loop forever
    while ( 1 ) {
        
        //enviar_datos(&flag_eje, &num, &counter, time);

        if(flag_eje == 0){
            GP0 = num & 1;
            num = num >> 1;
            reset_prueba(&time);

            if(counter == 0){
                reset_counter(time);
            }
            generador_pulso(time);
        
            counter++;
            if(counter == 8){
                counter = 0;
                flag_eje = 1;
            }
        }
    }
 
}



void delay(unsigned int tiempo)
{
	unsigned int i;
	unsigned int j;

	for(i=0;i<tiempo;i++)
	  for(j=0;j<500;j++);
}


void generador_pulso(unsigned int tiempo){
    delay(tiempo);
    GP1 = 1;
    delay(tiempo);
    GP1 = 0;
}

void reset_counter(unsigned int tiempo){
    delay(tiempo);
    GP2 = 1;
    delay(tiempo);
    GP2 = 0;
}

void reset_prueba(unsigned int* tiempo){
    unsigned int tie = *tiempo;
    delay(tie);
    GP5 = 1;
    delay(tie);
    GP5 = 0;
    
}

/* void enviar_datos(unsigned int* flag_, */
/*                   unsigned int* num_, */
/*                   unsigned int* counter_, */
/*                   unsigned int time_){ */
    
/*     if(*flag_ == 0){ */
/*         GP0 = *num_ & 1; */
/*         *num_ = *num_ >> 1; */

/*         if(*counter_ == 0){ */
/*             reset_counter(time_); */
/*         } */
/*         generador_pulso(time_); */
        
/*         *counter_++; */
/*         if(*counter_ == 8){ */
/*             *counter_ = 0; */
/*             *flag_ = 1; */
/*         } */
/*     } */
/* } */
