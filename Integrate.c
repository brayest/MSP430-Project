/*
 * Controlador General via USB
 * Created on: June 1, 2016
 * Auth: Héctor López, Brayan Lemus
 *
 *
 */
#include <msp430g2553.h>  //Header file para msp430g2553
#include <msp_serial_com.h> // Controlador Serial.
#include <string.h> // Libreria para manipulación de caracteres.

#ifndef TIMER0_A1_VECTOR   // Cambios de nomencaltura en vectores de interrupción
#define TIMER0_A1_VECTOR TIMERA1_VECTOR
#define TIMER0_A0_VECTOR TIMERA0_VECTOR
#endif

#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)   // Temperature Sensor Calibration-30 C
                                                      //See device datasheet for TLV table memory mapping
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)   // Temperature Sensor Calibration-85 C

/* Definiciones */
#define RD_LED BIT0	 	// Renombrado leds
#define GR_LED BIT6
#define CMD_NUMBER 6 		// Numero de compandos a interpretar
#define TXLED BIT0 					// Led rojo para transmision
#define TXD BIT2					// pin de transmisión por hardware
#define RXD BIT1					// pin de recepción por hardware
#define COUNT_LED BIT6				// Led verde para conteos
#define COUNT_PIN BIT4				// P1.4 para entrada de conteos



void FaultRoutine(void);
void ConfigWDT(void);
void ConfigClocks_1MHz(void);
void ConfigClocks_16MHz(void);
void ConfigPINs(void);
void ConfigTimerA2(void);
void PrintMeasure(void);
void Serial_PrintStrOSM(const char *String);
void Serial_PrintDec(unsigned int val);
void Serial_PrintInt(const unsigned int Value);
void ConfigUART_9600_1MHz(void);
void ConfigUART_9600_16MHz(void);
void ConfigUART_115200_16MHz(void);
void CLOCK(char *);
void ADC_CONVER(char *);
void Silent_Mode_Select(char *);
void EnviarConteos(void);
void COUNT_FUNC(char *);
void Serial_PrintStr(const char *String);
void Serial_PrintDec(unsigned int val);
void ConfigADC(void);
unsigned int Serial_DecStrToInt(const char *String);
unsigned int Serial_HexStrToInt(const char *String);


/* Variables globales */
unsigned int GRL_STATUS = 0;
unsigned int BLK_STATUS = 0;
unsigned int CNT_STATUS = 0;
unsigned int ADC_STATUS = 0;
extern unsigned int SILENT_MODE;
const int D_DELAY = 16000;		// Defaul delay.
unsigned int DELAY = 0; 	//Custom Delay
unsigned int COUNT = 0;  			// mide el numero de interrupts en pin de conteo
unsigned int TX_BUFFER;				// almacena el valor a transmitir
unsigned int M_SEGUNDOS = 0;		// mide el número de milisegundos
unsigned int SAMPLES = 1;
unsigned int DELIMIT = 1;
unsigned int TX_BUFFER;				// almacena el valor a transmitir
unsigned int MEASUREMENT = 0;


unsigned int ADC_OFF = 0;
unsigned int ADC = 0;


volatile long MEASURE = 0;
int limit = 0;
int PULSE = 1;           			// indica que un pulso fue capturado
int TRANS = 0;


/* Commands table */
const command COMMAND_LIST[ CMD_NUMBER ] = {
		{"AD", ADC_CONVER, "AD n limit \t n={0|1}; ADC Function with sps(samples per second) until limit(seconds)"},
		{"CK", CLOCK, "CK n frec \t n={0|1}; Clock Function with Frecuency (Hz) max: 100Hz until limit(seconds)"},
		{"OT", Output_select, "OT n\t \t n={0|1}; Hide (0) or show (1) the text when writing"},
		{"CT", COUNT_FUNC, "CT n limit \t n={0|1}; Count until limit"},
		{"SM", Silent_Mode_Select, "SM n\t \t Silent mode select:\
				\t \n\t \t n=0\tDisable. Keyboard feedback and all messages are show.\
				\t \n\t \t n=1\tNo keyboard feedback. Commands responses are show until \\n is sent.\
				\t \n\t \t n=2\tOnly prints numerical values (PrintHex, Serial_PrintDec) and strings\n\t\t from Serial_PrintStrOSM func."},
		{"HL", Show_help, "HL\t \t Show commands help"}
};
const unsigned int CMD_NMR = CMD_NUMBER;

void main(void)
{
    ConfigWDT();
    ConfigClocks_16MHz();
    ConfigPINs();
    ConfigUART_115200_16MHz();
    //ConfigUART_9600_16MHz();
    ConfigTimerA2();
    ConfigADC();

    __bis_SR_register(GIE); // Enter LPM0 w/ int until Byte RXed
//    _bis_SR_register(LPM3_bits + GIE); // Low power mode.. CPU, MCLOCK, SMCLOCK, apagados
	// solo el A clock encedido, el que maneja el clock central

    while (1)
    {
    	if( RX_TRS ) Exec_Commands( );
    	if( MEASUREMENT ) PrintMeasure();
    }
}


void ConfigADC(void)
{
	ADC10CTL1 = INCH_3 + ADC10DIV_7 + ADC10SSEL_3;
}


void ConfigUART_9600_1MHz(void)
{
    UCA0CTL1 |= UCSSEL_2; 			// SMCLK
	UCA0BR0 = 0x68; 				// 1MHz 9600
	UCA0BR1 = 0x00; 				// 1MHz 9600
    UCA0MCTL = UCBRS0;
    UCA0CTL1 &= ~UCSWRST; 			// inicializa USCI, /* USCI Software Reset */
    UC0IE |= UCA0RXIE;				// Enable USCI_A0 RX interrupt
}

void ConfigUART_9600_16MHz(void){
	/* UART setup */
	    UCA0CTL1 |= UCSSEL_2; 			// SMCLK
		UCA0BR0 = 0x82; 				// 16MHz 9600
		UCA0BR1 = 0x06; 				// 16MHz 9600
	    UCA0MCTL = UCBRS1 + UCBRS2; 	// Modulation UCBRSx = 6
	    UCA0CTL1 &= ~UCSWRST; 			// Enable USCI
	    UC0IE |= UCA0RXIE; 				// Enable USCI_A0 RX interrupt
}

void ConfigUART_115200_16MHz(void){
	/* UART setup */
	    UCA0CTL1 |= UCSSEL_2; 			// SMCLK
		UCA0BR0 = 0x8A; 				// 16MHz 115200
		UCA0BR1 = 0x00; 				// 16MHz 115200
	    UCA0MCTL = UCBRS0 + UCBRS1 + UCBRS2; // Modulation UCBRSx = 7
	    UCA0CTL1 &= ~UCSWRST; 			// Enable USCI
	    UC0IE |= UCA0RXIE; 				// Enable USCI_A0 RX interrupt
}

void ConfigTimerA2(void)        // Configuracion de Timer
{
    TA1CCR0 = D_DELAY;   // DELAY
    TA1CTL = TASSEL_2 + MC_1 + ID_0;  // Timer controlado por SMCLK/1  + up to CCR0
    TA0CCR0 = D_DELAY;   // DELAY
    TA0CTL = TASSEL_2 + MC_1 + ID_0; // + ID_3;  // Timer controlado por SMCLK/1  + up to CCR0
}

void ConfigWDT(void)
 {
	 WDTCTL = WDTPW + WDTHOLD;  // Stop watchdog timer
 }

void ConfigClocks_1MHz(void)  // COnfiguración del clock
{
	if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)
		FaultRoutine();  // If calibration data is erased
                                                //  run FaultRoutine()
	BCSCTL1 = CALBC1_1MHZ;  //Set range
    DCOCTL = CALDCO_1MHZ;   // Set DCO step + modulation
}

void ConfigClocks_16MHz(void)  // COnfiguración del clock
{
	if (CALBC1_16MHZ ==0xFF || CALDCO_16MHZ == 0xFF)
		FaultRoutine();  // If calibration data is erased
                                                //  run FaultRoutine()
	BCSCTL1 = CALBC1_16MHZ;  //Set range
    DCOCTL = CALDCO_16MHZ;   // Set DCO step + modulation
}

void FaultRoutine(void)  // En caso de rupturas de configuración
{
	P1OUT = BIT0 + BIT6;
	while(1);
}

void ConfigPINs(void) // Configuración de I/O pins
{
/* Inicializacion de salidas */
	P1DIR |= 0xFF;               	// all es salida en P1
	P1OUT &= 0x00;               	// Todas las salidas P1 en low
    P2DIR |= 0xFF; 					// all es salida en P2
    P2OUT &= 0x00; 					// Todas las salidas P2 en low

/* Interrupts en COUNT_PIN cuando el nivel pase alto a bajo */
   	P1DIR &= ~COUNT_PIN;			// Pin de conteo como entrada
   	P1IE |= COUNT_PIN;				// interrupt en pin de entrada
   	P1IES |= COUNT_PIN;				// configura interrupt en cambios
   	P1IFG &= ~COUNT_PIN;			// limpia el flag en el pin de entrada
   	P1DIR &= ~BIT3;					// P1.3 INPUT


/* Inicializacion de transmisión serial e interrupt de recepcion */
    P1SEL |= RXD + TXD ; 			// P1.1 = RXD, P1.2=TXD
    P1SEL2 |= RXD + TXD ; 			// P1.1 = RXD, P1.2=TXD
}


void PrintMeasure(void){
	Serial_PrintInt( ADC );
	Serial_PrintStrOSM("\n");
	MEASUREMENT = 0;
}


void Silent_Mode_Select(char *arg){
	unsigned int CMD_OK = 0;
	switch( arg[0] ){
	case '0':
		SILENT_MODE = 0;
		CMD_OK = 1;
		break;
	case '1':
		SILENT_MODE = 1;
		CMD_OK = 1;
		break;
	case '2':
		SILENT_MODE = 2;
		CMD_OK = 1;
		break;
	}
	if ( CMD_OK )
		Serial_PrintStr( "> OK\n" );
	else
		Serial_PrintStr( "> Invalid argument\n" );
}

void CLOCK(char *arg)
{
	unsigned int CMD_OK = 0;
	char *VAR;
	char *VAR2;
	unsigned int FREC = 0;
	switch( arg[0])
	{
	case '0':
		if (BLK_STATUS)
		{
			CNT_STATUS = 0;
			TA0CCTL0 &= ~CCIE;
			ConfigTimerA2();
			CMD_OK = 1;
			P1OUT = 0;
			BLK_STATUS = 0;
			GRL_STATUS = 0;
			SAMPLES = 0;
			CNT_STATUS = 0;
			ADC_OFF = 0;

		}else CMD_OK=2;
		break;
	case '1':
		if (!BLK_STATUS)
		{
			VAR2 = strtok( NULL, " ");
			limit = Serial_DecStrToInt( VAR2 );
			VAR = strtok( NULL, " ");
			FREC = Serial_DecStrToInt( VAR );
			if ( FREC )
			{
				Serial_PrintStr("\n");
				//TA0CTL = TASSEL_2 + MC_1 + ID_0;
				TA0CCTL0 |= CCIE;
			    DELAY =  FREC;
			    /* 1000  En interrupción T = 1s   Periodo   1 ->  1HZ  probar max 1KHz*/
				ADC_OFF = 1;
				CMD_OK = 1;
				SAMPLES = 0;
				CNT_STATUS = 0;
				BLK_STATUS = 1;
				GRL_STATUS = 0;
			}else CMD_OK = 0;
		}else CMD_OK = 2;
		break;
	}
	switch ( CMD_OK ){
	case 0:
		Serial_PrintStr( "> Invalid argument\n" );
		break;
	case 1:
		Serial_PrintStr( "> OK\n" );
		break;
	case 2:
		Serial_PrintStr( "> No change\n" );
		break;
	}
}

void COUNT_FUNC(char *arg)
{
	unsigned int CMD_OK = 0;
	char *CONTEO;
	switch( arg[0])
	{
	case '0':
		if (CNT_STATUS)
		{
			CNT_STATUS = 0;
			TA1CCTL0 &= ~CCIE;
			CMD_OK = 1;
			P1OUT = 0;
			BLK_STATUS = 0;
			GRL_STATUS = 0;
			CNT_STATUS = 0;

		}else CMD_OK=2;
		break;
	case '1':
		if (!CNT_STATUS)
		{
			CONTEO = strtok( NULL, " ");
			limit = Serial_DecStrToInt( CONTEO );
			if ( limit )
			{
				Serial_PrintStr("\n");
				TA1CCTL0 |= CCIE;
				CMD_OK = 1;
				CNT_STATUS = 1;
				BLK_STATUS = 0;
				GRL_STATUS = 0;
			}else CMD_OK = 0;
		}else CMD_OK = 2;
		break;
	}
	switch ( CMD_OK ){
	case 0:
		Serial_PrintStr( "> Invalid argument\n" );
		break;
	case 1:
		Serial_PrintStr( "> OK\n" );
		break;
	case 2:
		Serial_PrintStr( "> No change\n" );
		break;
	}
}

void ADC_CONVER(char *arg){
	char *token;
	unsigned int SPS = 0;
	char *token2;
	unsigned int CMD_OK = 0;
	switch( arg[0] ){
	case '0':
		if (ADC_STATUS){
			TA0CCTL0 &= ~CCIE;
			P1OUT = 0;
			BLK_STATUS = 0;
			ADC_STATUS = 0;
			CNT_STATUS = 0;
			CMD_OK = 1;
		} else CMD_OK = 2;
		break;
	case '1':
		if (!ADC_STATUS){
			token = strtok( NULL, " ");
			limit = Serial_DecStrToInt( token );
			token2 = strtok( NULL, " ");
			SPS = Serial_DecStrToInt( token2 );
			DELIMIT = 1000/SPS;
			if ( !DELIMIT ) { DELIMIT = 10;}
			if( limit ){
				TA0CCTL0 |= CCIE;
				//TA0CCR0 = 11000;
				ADC_STATUS = 1;
				SAMPLES = 1;
				BLK_STATUS = 0;
				CNT_STATUS = 0;
				CMD_OK = 1;
			} else CMD_OK = 0;
		}  else CMD_OK = 2;
		break;
	}
	switch ( CMD_OK ){
	case 0:
		Serial_PrintStr( "> Invalid argument\n" );
		break;
	case 1:
		Serial_PrintStr( "> OK\n" );
		break;
	case 2:
		Serial_PrintStr( "> No change\n" );
		break;
	}
}

/* Interrupt del COUNT_PIN */
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
 	COUNT++;						// Incrementa el contador
   	P1IFG &= ~COUNT_PIN;			// limpia el interrupt en el pin de conteo
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
	M_SEGUNDOS++;
	if (ADC_STATUS == 0)
	{
		if(M_SEGUNDOS >= 1000/DELAY)
		{
			SAMPLES++;
			if (SAMPLES == DELAY)
			{
				limit--;
				SAMPLES = 0;

				if (limit < 1)
				{
					TA0CCTL0 &= ~CCIE;
					ADC_STATUS = 0;
					CNT_STATUS = 0;
					SAMPLES = 0;
					BLK_STATUS = 0;
					GRL_STATUS = 0;
				}
			}
			P1OUT ^= BIT0;
			M_SEGUNDOS = 0;
		}
	}
	if (ADC_STATUS == 1)
	{
		if ( M_SEGUNDOS == SAMPLES*DELIMIT) {
			SAMPLES++;
			ADC10CTL0 = SREF_0 + ADC10SHT_3 + REFON + ADC10ON;
			//_delay_cycles(5); // Wait for ADC Ref to settle
			ADC10CTL0 |= ENC + ADC10SC ; // Sampling and conversion start
			P1OUT |= BIT6;	// P1.6 on (green LED)
			while(ADC10CTL1 & BUSY);
			ADC= ADC10MEM;
			ADC10CTL0 &= ~ENC;	// Disable ADC conversion
			ADC10CTL0 &= ~(REFON + ADC10ON); // Ref and ADC10 off
			P1OUT &= ~BIT6; // green LED off
			MEASUREMENT = 1;
			//Serial_PrintInt(  M_SEGUNDOS );
			//Serial_PrintStrOSM("\n");
			//Serial_PrintInt(  M_SEGUNDOS );
			//Serial_PrintStrOSM("	");
			//Serial_PrintInt( SAMPLES );
			//Serial_PrintStrOSM("\n");
		}

		if ( M_SEGUNDOS >= 1000)
		{
			M_SEGUNDOS = 0;
			SAMPLES = 1;
			limit--;
			//Serial_PrintInt( limit );
			//Serial_PrintStrOSM("\n");
			if (limit < 1)
			{
				TA0CCTL0 &= ~CCIE;
				ADC_STATUS = 0;
				SAMPLES = 1;
				CNT_STATUS = 0;
				BLK_STATUS = 0;
				GRL_STATUS = 0;
			}
		}
	}
}


#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A1 (void)
{
	M_SEGUNDOS++;
//	TX_BUFFER++;
	if(M_SEGUNDOS >= 1000)
	{
		TX_BUFFER = COUNT;			// almacena de forma temporal el valor de cuenta
		COUNT = 0;					// reinicia el contador de pulsos
		P1OUT ^= COUNT_LED; 	// Toggle P1.0
		Serial_PrintDec(TX_BUFFER);
		M_SEGUNDOS = 0;				// reinicia el contador de milisegundos
		limit--;
		if (limit < 1)
		{
			TA1CCTL0 &= ~CCIE;
			CNT_STATUS = 0;
			BLK_STATUS = 0;
			GRL_STATUS = 0;
		}
	}
}
