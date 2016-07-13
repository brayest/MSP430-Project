 /* msp_serial_com.c
 *
 *  Created on: Feb 18, 2016
 *      Author: Hector Pérez, Brayan Lemus
 */

#include "string.h"
#include "msp430g2553.h"
#include "msp_serial_com.h"
#define CMD_NUMBER 6
#define TXLED BIT0 					// Led rojo para transmision
#define TXD BIT2					// pin de transmisión por hardware
#define RXD BIT1					// pin de recepción por hardware
#define COUNT_LED BIT6				// Led verde para conteos
#define COUNT_PIN BIT4				// P1.4 para entrada de conteos

/* Variables globales */

char tx_str_buffer[ TX_BUF_SIZE ];
unsigned int tx_str_len;
unsigned int tx_i;
unsigned int TX_STR = 0;
volatile unsigned int TX_TRS = 0;


char rx_str_buffer[ RX_BUF_SIZE ];
unsigned int rx_i = 0;
volatile unsigned int RX_TRS = 0;

unsigned int OUTPUT_DISABLE = 0;
unsigned int SILENT_MODE = 2;
unsigned int NUM_FORMAT = 0;
extern const unsigned int CMD_NMR;
extern const command COMMAND_LIST[];


/* TX interrupt */
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
	if ( TX_STR ){
		if (TX_TRS==0){
			TX_TRS = 1;
			tx_i = 0;
			UCA0TXBUF = tx_str_buffer[tx_i++];		// TX next character
		} else if (tx_i < tx_str_len ){ 	// TX over?
				UCA0TXBUF = tx_str_buffer[tx_i++];
		} else {
				UC0IE &= ~UCA0TXIE; 		// Disable USCI_A0 TX interrupt
				TX_TRS = 0;
				TX_STR = 0;
		}
	} else UC0IE &= ~UCA0TXIE;
}



#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    if ( rx_i == 0 ){
    	rx_str_buffer[0] = '\x0';
    }
    if ( rx_i > 30 ){
		tx_str_buffer[0] = '\x07';
		tx_str_buffer[1] = '\r';
		tx_str_buffer[2] = '\n';
		tx_str_len = 3;
    	UCA0RXBUF;
    	rx_i = 0;
    } else if ( ( UCA0RXBUF > 0x1F ) && ( UCA0RXBUF < 0x61 ) ) {
		tx_str_buffer[0] = UCA0RXBUF;
		tx_str_len = 1;
    	rx_str_buffer[rx_i++] = UCA0RXBUF;
    } else if ( ( UCA0RXBUF > 0x60 ) && ( UCA0RXBUF < 0x7B ) ){
		tx_str_buffer[0] = UCA0RXBUF - 0x20;
		tx_str_len = 1;
    	rx_str_buffer[rx_i++] = UCA0RXBUF - 0x20;
    } else if ( ( UCA0RXBUF > 0x7A ) && ( UCA0RXBUF < 0x7F ) ){
		tx_str_buffer[0] = UCA0RXBUF;
		tx_str_len = 1;
    	rx_str_buffer[rx_i++] = UCA0RXBUF;
    } else if ( UCA0RXBUF == 0x8 ) {
    	if (rx_i > 0 ){
    		tx_str_buffer[0]='\x08';
    		tx_str_buffer[1]=' ';
    		tx_str_buffer[2]='\x08';
    		tx_str_len = 3;
    		rx_str_buffer[--rx_i] = 0;
    	}
    } else if ( UCA0RXBUF == 0xD ) {
		tx_str_buffer[0] = '\n';
		tx_str_buffer[1] = '\r';
		tx_str_len = 2;
    	rx_str_buffer[rx_i] = '\x0';
    	RX_TRS = 1;
    	rx_i = 0;
    }
    if( !SILENT_MODE ){
        	if( tx_str_len > 1 ){
        		TX_STR = 1;
        		UC0IE |= UCA0TXIE;			// Enable USCI_A0 TX interrupt
        	} else UCA0TXBUF = tx_str_buffer[ 0 ];
        }
}


void PrintStr(const char *String){
	if (String != NULL && ( SILENT_MODE < 2 ) ) {
		while (*String != '\0') {
			/* Wait for the transmit buffer to be ready */
            while (TX_TRS || !(IFG2 & UCA0TXIFG));
            /* Transmit data */
            UCA0TXBUF = *String;
            /* If there is a line-feed, add a carriage return */
            if (*String == '\n') {
            	/* Wait for the transmit buffer to be ready */
            	while (TX_TRS || !(IFG2 & UCA0TXIFG));
            	UCA0TXBUF = '\r';
            }
            String++;
		}
	}
}


void Serial_PrintStr(const char *String){
	if (String != NULL && ( SILENT_MODE < 2 ) ) {
		while (*String != '\0') {
            while (TX_TRS || !(IFG2 & UCA0TXIFG));
            UCA0TXBUF = *String;
            if (*String == '\n') {
            	while (TX_TRS || !(IFG2 & UCA0TXIFG));
            	UCA0TXBUF = '\r';
            }
            String++;
		}
	}
}

void Serial_GetRxBuff(char *String){
	strcpy(String, rx_str_buffer);
}

void Exec_Commands(void){
	char Rcv_Cmd[ RX_BUF_SIZE ];
	unsigned int CMD = 0;
	char *token;
	Serial_GetRxBuff(Rcv_Cmd);
	token = strtok(Rcv_Cmd," ");
	unsigned int i = CMD_NMR;
	while(i--){
		if ( !strcmp( token, COMMAND_LIST[ i ].cmd_name ) ){
		token = strtok( NULL, " ");
		COMMAND_LIST[ i ].cmd_func(token);
		CMD = 1;
		}
	}
	if( !CMD ) Serial_PrintStr( "> Invalid command\n" );
	RX_TRS = 0;
}

void Show_help(char *arg){
	unsigned int i = CMD_NUMBER;
	Serial_PrintStr( "\nCommands avaible:\n" );
	while( i-- ){
		Serial_PrintStr( "\n" );
		Serial_PrintStr( COMMAND_LIST[ i ].cmd_help );
	}
	Serial_PrintStr( "\n\n" );
}

void Serial_PrintStrOSM(const char *String){
	if (String != NULL ) {
		while (*String != '\0') {
            while (TX_TRS || !(IFG2 & UCA0TXIFG));
            UCA0TXBUF = *String;
            if (*String == '\n') {
            	while (TX_TRS || !(IFG2 & UCA0TXIFG));
            	UCA0TXBUF = '\r';
            }
            String++;
		}
	}
}

void Serial_PrintHex(const unsigned int val){
	char HexStr[ 5 ];
	unsigned int tmp;
	tmp = (val & 0xF000) >> 12;
	HexStr[ 0 ] = ( tmp < 0x000A ? tmp + 0x0030 : tmp + 0x0037 );
	tmp = (val & 0x0F00) >> 8;
	HexStr[ 1 ] = ( tmp < 0x000A ? tmp + 0x0030 : tmp + 0x0037 );
	tmp = (val & 0x00F0) >> 4;
	HexStr[ 2 ] = ( tmp < 0x000A ? tmp + 0x0030 : tmp + 0x0037 );
	tmp = val & 0x000F;
	HexStr[ 3 ] = ( tmp < 0x000A ? tmp + 0x0030 : tmp + 0x0037 );
	HexStr[ 4 ] = '\0';
	Serial_PrintStrOSM( HexStr );
}

void Serial_PrintDec(unsigned int val){
	char DecStr[ 6 ];
	unsigned int i = 10000;
	unsigned int j = 0;
	unsigned int tmp;
	while( i ){
		tmp = val / i;
		if( tmp || j ) DecStr[ j++ ] = tmp + 48;
		val = val - tmp * i;
		i/=10;
	}
	if (j) DecStr[j] = '\0';
	else {
			DecStr[0] = '0';
			DecStr[1] = '\0';
	}
	Serial_PrintStrOSM( DecStr );
	Serial_PrintStrOSM("\n");
}

void Serial_PrintInt(const unsigned int Value){
	if ( NUM_FORMAT ) Serial_PrintDec( Value );
	else Serial_PrintHex( Value );
}

unsigned int Serial_DecStrToInt(const char *String){
	if( strlen(String) < 6 ){
		unsigned int val = 0;
		unsigned int i = 1;
		unsigned int j = strlen(String);
		unsigned int tmp;
		while( j ){
			tmp =  String[ --j ] - 48 ;
			val = val + tmp * i;
			i*=10;
		}
		return val;
	} return 0;
}

unsigned int Serial_HexStrToInt(const char *String){
	if( strlen(String) < 5 ){
		unsigned int val = 0;
		unsigned int i = 0;
		unsigned int j = strlen(String);
		unsigned int tmp;
		while( j ){
			tmp =  String[ --j ];
			if( ( tmp > 47 )&& (tmp < 58) ){
				val += ( tmp - 48 ) << i;
			} else if ( ( tmp > 64 ) && ( tmp < 71 ) ){
				val += ( tmp - 55 ) << i;
			} else return 0;
			i+=4;
		}
		return val;
	} return 0;
}

unsigned int Serial_StrToInt(const char *String){
	if ( NUM_FORMAT ) return Serial_DecStrToInt( String );
	else return Serial_HexStrToInt( String );
}


void Output_select(char *arg){
	unsigned int CMD_OK = 0;
	switch( arg[0] ){
	case '0':
		OUTPUT_DISABLE = 1;
		CMD_OK = 1;
		break;
	case '1':
		OUTPUT_DISABLE = 0;
		CMD_OK = 1;
		break;
	}
	if ( CMD_OK )
		PrintStr( "> OK\n" );
	else
		PrintStr( "> Invalid argument\n" );
}


