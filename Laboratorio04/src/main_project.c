#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "clock.h"
#include "console.h"
#include "sdram.h"
#include "lcd-spi.h"
#include "gfx.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/adc.h>


#include "spi-mems.h"

#define WHO_AM_I 0x0f


static void gpio_setup(void)
{
    
	/* Enable GPIOG clock. */
	rcc_periph_clock_enable(RCC_GPIOG);
	rcc_periph_clock_enable(RCC_GPIOD);

	// ESTE LED INDICARA CUANDO ESTE HABILITADA LA TRANSMISION
    // DE DATOS SERIAL A LA COMPUTADORA.
	gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT,
			GPIO_PUPD_NONE, GPIO13);

    // ESTE LED INDICARA CUANDO LA TENSION DE LA BATERIA
    // ES MENOR O IGUAL A 7V.
	gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT,
			GPIO_PUPD_NONE, GPIO14);

	/* Enable the GPIO ports whose pins we are using */
	rcc_periph_clock_enable(RCC_GPIOF | RCC_GPIOC);

	gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN,
			GPIO7 | GPIO8 | GPIO9);
	gpio_set_af(GPIOF, GPIO_AF5, GPIO7 | GPIO8 | GPIO9);
	gpio_set_output_options(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ,
				GPIO7 | GPIO9);

    // PA0 CONFIGURADO COMO ENTRADA ANALOGICA PARA MEDIR LOS VALORES DE
    // LA BATERIA.
    gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
    
    // PD14 CONFIGURADO COMO ENTRADA DIGITAL PARA INDICAR CUANDO SE DEBE
    // TRANSMITIR DE MANERA SERIAL LOS DATOS.
    gpio_mode_setup(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO14);

	/* Chip select line */
	gpio_set(GPIOC, GPIO1);
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);

	rcc_periph_clock_enable(RCC_SPI5);

    ///////////////////////////////////////////////////////////////////
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_USART1);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);

    usart_set_baudrate(USART1, 115200);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

    usart_enable(USART1);
    ////////////////////////////////////////////////////////////////////

    
	//spi initialization;
	spi_set_master_mode(SPI1);
	spi_set_baudrate_prescaler(SPI1, SPI_CR1_BR_FPCLK_DIV_64);
	spi_set_clock_polarity_0(SPI1);
	spi_set_clock_phase_0(SPI1);
	spi_set_full_duplex_mode(SPI1);
	spi_set_unidirectional_mode(SPI1); /* bidirectional but in 3-wire */

	spi_enable_software_slave_management(SPI1);
	spi_send_msb_first(SPI1);
	spi_set_nss_high(SPI1);
	spi_enable(SPI1);
}


/*
 * This is our example, the heavy lifing is actually in lcd-spi.c but
 * this drives that code.
 */
int main(void)
{
    char EjeX[20];
    char EjeY[20];
    char EjeZ[20];

    int serial;
    int16_t vecs[3];
    uint32_t cr_tmp;

    clock_setup();
    console_setup(115200);
    sdram_init();
    lcd_spi_init();
    gpio_setup();
    
    gfx_init(lcd_draw_pixel, 240, 320);
    gfx_setTextColor(LCD_YELLOW, LCD_BLACK);
    gfx_setTextSize(1);

    clock_setup();
    console_setup(115200);

    cr_tmp = SPI_CR1_BAUDRATE_FPCLK_DIV_8 |
             SPI_CR1_MSTR |
             SPI_CR1_SPE |
             SPI_CR1_CPHA |
             SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE;

    put_status("\nBefore init: ");
    SPI_CR2(SPI5) |= SPI_CR2_SSOE;
    SPI_CR1(SPI5) = cr_tmp;
    put_status("After init: ");

    write_reg(0x20, 0x0f);  /* Normal mode */
    write_reg(0x21, 0x00);  /* standard filters */
    write_reg(0x23, 0xb0);  /* 250 dps */
    
    // Configuración ADC
    rcc_periph_clock_enable(RCC_ADC1);
    adc_power_off(ADC1);
    adc_disable_scan_mode(ADC1);
    adc_set_single_conversion_mode(ADC1);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC);
    adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
    adc_power_on(ADC1);
    //adc_reset_calibration(ADC1);
    //adc_calibrate(ADC1);
    
    while (1) {
        // Leer el valor analógico del canal 1 (PA0)
        adc_start_conversion_regular(ADC1);
        while (!adc_eoc(ADC1));
        uint16_t adc_value = adc_read_regular(ADC1);
        
        float value = (float) 127/61490 * (float) adc_value;
        int value2 = 100 * value;
        if(value2 <= 700) {
            gpio_set(GPIOG, GPIO14);
        }
        else{
            gpio_clear(GPIOG, GPIO14);
        }
        int porcentaje = value2 / 9;
        int value3 = value2 / 100;
        value2 = value2 % 100;
        if(value2 > 50) porcentaje++;
                            
        char parte_entera[20];
        char parte_decimal[20];
        char resultado[30] = "";
        char porcentaje_str[20];
        
        sprintf(parte_entera, "%d", value3);
        sprintf(parte_decimal, "%d", value2);
        sprintf(porcentaje_str, "%d", porcentaje);
        strcat(resultado, parte_entera);
        strcat(resultado, ".");
        strcat(resultado, parte_decimal);
        
        read_xyz(vecs);
        
        sprintf(EjeX, "%d", vecs[0]); 
        sprintf(EjeY, "%d", vecs[1]); 
        sprintf(EjeZ, "%d", vecs[2]);
        
        gfx_fillScreen(LCD_BLACK);
        
        gfx_setCursor(0, 66);
        gfx_puts("Tension de Bateria: ");
        gfx_puts(resultado);
        gfx_puts("V \n");
        gfx_puts("Tension de Bateria: ");
        gfx_puts(porcentaje_str);
        gfx_puts("\% \n");
        gfx_setCursor(0, 120);
        gfx_puts("Informacion de los Ejes:\n\n");
        gfx_puts("Eje X: ");
        gfx_puts(EjeX);
        gfx_puts("\n");
        gfx_puts("Eje Y: ");
        gfx_puts(EjeY);
        gfx_puts("\n");
        gfx_puts("Eje Z: ");
        gfx_puts(EjeZ);
        gfx_puts("\n");
        gfx_puts("\n");
        gfx_setCursor(0, 240);
        gfx_puts("Comunicacion Serial: ");

        // PIN A0 ES EL PIN QUE INDICA SI HAY O NO COMUNICACION SERIAL
        // SI ESTA EN ALTO ENCIENDE EL LED E INICIA LA COMUNICACION SERIAL
        // SI ESTA EN BAJO APAGA EL LED Y CIERRA LA COMUNICACION SERIAL
        serial = gpio_get(GPIOD, GPIO14);
        //serial = 1;
        if(serial){
            gfx_puts("ON\n");
            gpio_toggle(GPIOG, GPIO13);


            // crea el mensaje que 
            char serial_message[150] = "";
            sprintf(serial_message, "%s,%s,%s,%s,%s\n", resultado, EjeX, EjeY, EjeZ, porcentaje_str);
        
            for (int i = 0; serial_message[i] != '\0'; i++) {
                usart_send_blocking(USART1, serial_message[i]);
            }
        }
        else{
            gfx_puts("OFF\n");
            gpio_clear(GPIOG, GPIO13);
        }
        
        lcd_show_frame();

        msleep(500);
    }
}
