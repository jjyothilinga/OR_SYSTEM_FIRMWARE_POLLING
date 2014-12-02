#ifndef CONFIG_H
#define CONFIG_H

/*
*------------------------------------------------------------------------------
* config.h
*
*/






#define DEVICE_ADDRESS			(0xFA)

//#define __FACTORY_CONFIGURATION__



//TIMER Configuration
#define TIMESTAMP_DURATION 		(200)


/*
*------------------------------------------------------------------------------
* USART Configuration
*------------------------------------------------------------------------------
*/
#define ACTIVE_USARTS		1
#define UART1
//#define UART2


enum
{
	RX_PACKET_SIZE = 64,
	TX_PACKET_SIZE = 64
};

#define 	RECEIVER_MAX_PAKET_SIZE		(RX_PACKET_SIZE)	
#define 	TRANSMITTER_MAX_PAKET_SIZE	(TX_PACKET_SIZE)

/*
*------------------------------------------------------------------------------
* LCD Configuration
*------------------------------------------------------------------------------
*/
//#define TEST_LCD


//TOWER LAMP Configuration
//#define TOWER_LAMP_TEST


//KEYPAD Configuration
#define KEYPAD_BUFFER_LENGTH 	(20)
//#define KEYPAD_TEST

//COM module configuration
#define __BCC_XOR__
#define __RESPONSE_ENABLED__
//#define __UART_TEST__
//#define __LOOP_BACK__
#define BROADCAST_ADDRESS		0xFA
#define CMD_SOP	0xAA
#define CMD_EOP 0xBB
#define RESP_SOP	0xCC
#define RESP_EOP	0xDD



//TCPIP CONFIGURATIONS
//#define TCPIP_DATA_ON_UART


// Enable for external eeprom access
// Comment out this line if internal eeprom access required
#define EEP_EXTERRNAL


/*
*------------------------------------------------------------------------------
* Public Data Types
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Variables (extern)
*------------------------------------------------------------------------------
*/

/*
*------------------------------------------------------------------------------
* Public Constants (extern)
*------------------------------------------------------------------------------
*/


/*
*------------------------------------------------------------------------------
* Public Function Prototypes (extern)
*------------------------------------------------------------------------------
*/

#endif
/*
*  End of config.h
*/



