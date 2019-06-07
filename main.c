#include <xc.h>
#include <p18f4550.h>
#include <adc.h>
#include <portb.h>
#include <delays.h>
#include <stdio.h>

#pragma config PLLDIV = 5
#pragma config USBDIV = 2
#pragma config VREGEN = ON
#pragma config FOSC = HSPLL_HS
#pragma config WDT = OFF
#pragma config PBADEN = OFF
#pragma config LVP = OFF
#pragma config WRTB = ON

#define LCD_RS PORTCbits.RC6 	  // Pino do PIC ligado no RS do LCD
#define LCD_EN PORTCbits.RC7 	  // Pino do PIC ligado no Enable do LCD
#define LCD_TRIS_RS TRISCbits.RC6 // Direção da Porta Porta do PIC ligado no RS do LCD
#define LCD_TRIS_EN TRISCbits.RC7 // Direção da Porta Porta do PIC ligado no EN do LCD
#define LCD_DB PORTD 		  // Porta do PIC ligado nos pinos DB0..DB7 do LCD
#define LCD_TRIS TRISD 		  // Direção da Porta do PIC ligado nos pinos DB0..DB7 do LCD

void lcd_init();            // Protótipo da função lcd_init
void lcd_cmd(char cmd);     // Protótipo da função lcd_cmd
void lcd_data(char data);   // Protótipo da função lcd_data
void lcd_number(int number);// Protótipo da função lcd_number
void lcd_print(const char* s);
void lcd_clear(void);
void lcd_set_cursor(char line, char column);



void main(void)
{
    float x, y, tensao;
    // Configura Portas
	TRISB = 0xff;
	TRISE = 0x00;
	TRISC = 0x00;

	// Configura pull up porta B
	OpenPORTB(PORTB_PULLUPS_ON);

	OpenADC(ADC_FOSC_16 // Fosc=20MHz Fad=20MHz/16 Tad = 0,8us
	&ADC_RIGHT_JUST // Resultado justificado a direita
	&ADC_4_TAD, // Configuração do tempo automático (4*Tad=3,2us)
	ADC_INT_OFF // Interrupção desabilitada
	&ADC_REF_VDD_VSS // Vref+ = Vcc (5V) e Vref- = Vss
	&ADC_CH4, // Seleciona canal 0
	ADC_5ANA); // Habilita entrada analógica AN0, digital AN1 a AN15

	PORTE = 0x00; // Apaga Leds
	PORTC = 0x00; // Apaga Leds

	lcd_init(); // Inicializa LCD
        lcd_cmd(0xc2); // Posiciona LCD na primeira linha
                
        
	while(1)
	{
		SetChanADC (ADC_CH4); //Seleciona canal canal 0
		ConvertADC(); //Inicia conversão
		while(BusyADC()); //Aguarda fim da conversão
		
                float tensao = ReadADC()*5.0/1023.0;  //Lê a tensão do sensor
                lcd_cmd(0x82); //posiciona o cursor de display
                char tensao_ [17]; //buffer
                sprintf(tensao_,"Tensao:%3dmV", (int)(tensao*100.0)); 
                lcd_print(tensao_);   //imprime a tensão

                x = tensao;
                if (x<0.16)
                    x = 0.16;
                if (x>1.3)
                    x = 1.3; //limite de exibição de temp.

               
                lcd_cmd(0xca);
                y = 94.857*x*x*x*x - 344.42*x*x*x + 482.6*x*x - 344.9*x + 153.63; //interpreta o sinal de tensão, exibindo-o em graus Celsius.
                lcd_cmd(0xc2); // Posiciona LCD na primeira linha.
                char temp [17]; //buffer.
                sprintf(temp,"Temp Ar:%2d%cC", (int) y, 0xdf); //imprime a temperatura.
                lcd_print(temp);


		lcd_cmd(0xcf);
                lcd_data(255);
                lcd_cmd(0x8f);
                lcd_data(255);
                lcd_cmd(0xc0);
                lcd_data(255);
                lcd_cmd(0x80);
                lcd_data(255);//enfeites de layout

                lcd_clear;
                
		Delay10KTCYx(255);
		PORTCbits.RC2 = !PORTCbits.RC2; //led piscando para indicar o funcionamento
	}
}

void lcd_number(int number)
{
	lcd_data((unsigned char) ((number/1000)%10)+ '0');
	lcd_data((unsigned char) ((number/100)%10) + '0');
	lcd_data((unsigned char) ((number/10)%10)  + '0');
	lcd_data((unsigned char) (number%10)       + '0');
}

void lcd_init()
{
	LCD_TRIS = 0x00; 		// Configura porta do LCD de saída
	LCD_TRIS_EN = 0; 		// Configura pino EN como saída
	LCD_TRIS_RS = 0; 		// Configura pino RS como saída
	LCD_EN = 0; 			// EN = 0
	LCD_RS = 0; 			// RS = 0
	lcd_cmd(0b00111000);	// Modo 8 bits, 7x5 e 2 linhas
	lcd_cmd(0b00010110); 	// Cursor para a Direita
	lcd_cmd(0b00001100); 	// Liga display e desiga cursor
	lcd_cmd(0b00000001); 	// Limpa o display
	Delay1KTCYx(30);
}

void lcd_cmd(char cmd)
{
	LCD_RS = 0; 	// Comando
	LCD_DB = cmd;
	LCD_EN = 1;
	Delay1KTCYx(3);
	LCD_EN = 0;
}

void lcd_data(char data)
{
	LCD_RS = 1; 	// Dado
	LCD_DB = data;
	LCD_EN = 1;
	Delay1KTCYx(3);
	LCD_EN = 0;
}
void lcd_print(const char* s)
{
	for (; *s; s++)
		lcd_data(*s);
}

void lcd_clear(void)
{
	lcd_cmd(0b00000001); 	// Limpa o display
	Delay1KTCYx(30);
};

void lcd_set_cursor(char line, char column)
{
	line = (line == 0) ? 0x80 : 0xC0;
	column &= 0x0F;
	lcd_cmd(line | column);
}