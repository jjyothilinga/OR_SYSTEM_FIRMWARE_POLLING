#include "app.h"
#include "config.h"


/*
*------------------------------------------------------------------------------
* Structures
*------------------------------------------------------------------------------
*/



typedef struct _EVENT_LOG
{
	UINT8 writeIndex;
	UINT8 readIndex;
	UINT8 logBuffer[MAX_ENTRIES][MAX_LENGTH_OF_ENTRY];
	UINT8 StatusBuf[MAX_ENTRIES * 2];
	UINT8 StatBufIndex;
	
}EVENT_LOG;


/*
*------------------------------------------------------------------------------
* Variables
*------------------------------------------------------------------------------
*/

#pragma idata LOG_DATA
EVENT_LOG log = {0};
#pragma idata


/*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

void logWrite( far UINT8 **data, UINT8 length );
UINT8 logRead( far UINT8 **logBuff );

/*
*------------------------------------------------------------------------------
* void APP-init(void)
*------------------------------------------------------------------------------
*/

void APP_init(void)
{
	UINT8 i;
	//Modbus
	UINT16 sbaudrate = 9600, saddress = 0x0A;


	for(i = 0; i < MAX_ENTRIES * 2; i++)
		log.StatusBuf[i] = 0XFF;

}


/*
*------------------------------------------------------------------------------
* void APP_task(void)
*------------------------------------------------------------------------------
*/

void APP_task(void)
{
	
	if(RB1hasData == 1)
	{
		ENTER_CRITICAL_SECTION();  //turn OFF all interrupts

		app.portb1_data[0] = PORTB1_CODE;
		app.portb1_data[1] = 0X01;

		logWrite( &app.portb1_data, 2 );    // Write status of PORTB1

		RB0hasData = 0; 

		EXIT_CRITICAL_SECTION();  //turn OFF all interrupts
	}

	else if( RB0hasData == 1 )
    {
		ENTER_CRITICAL_SECTION();  //turn OFF all interrupts

		app.portb0_data[0] = PORTB0_CODE;
		app.portb0_data[1] = 0X01;

		logWrite( &app.portb0_data, 2 );    // Write status of PORTB1

	    RB0hasData = 0; 

		EXIT_CRITICAL_SECTION();  //turn OFF all interrupts
 	}


}


/*---------------------------------------------------------------------------------------------------------------
*	void updateLog(void)
*----------------------------------------------------------------------------------------------------------------
*/
void logWrite( far UINT8 **data, UINT8 length )
{
	UINT8 i, j = 0;
	UINT8 temp = 0;
	UINT8 tempData = 0;
 
	length = length * 2;   //length multiplied by number of bytes

	
	log.StatBufIndex = **data;
	(*data)++;
	tempData = **data;

	if(log.StatusBuf[log.StatBufIndex+1] != tempData)
	{
		(*data)--;
		//Store data in the buffer
		for( i = 0, j =0; i < length; i++ )
		{
			temp = **data ;
	
	
			log.logBuffer[log.writeIndex][j++] = (temp / 10) + '0';
			temp = **data ;
			log.logBuffer[log.writeIndex][j++] = (temp % 10) + '0';
			
			(*data)++;
		}
	
		log.logBuffer[log.writeIndex][j] = '\0'; //store null character to indicate end of the packet

		log.writeIndex++;
		if( log.writeIndex >= MAX_ENTRIES)
			log.writeIndex = 0;

		//store previous state
		log.StatusBuf[log.StatBufIndex+1] = tempData;
	}

}

/*---------------------------------------------------------------------------------------------------------------
*	UINT8 logRead(void)
*----------------------------------------------------------------------------------------------------------------
*/
UINT8 logRead( far UINT8** logBuff )
{
	UINT8 length = 0;

	if( log.writeIndex == log.readIndex )
	{
		*logBuff = 0;
	}

/*	if((log.logBuffer[log.readIndex][0] == '\0') || (log.writeIndex == log.readIndex))
		return *logBuff = 0; */
	else
	{
		*logBuff = log.logBuffer[log.readIndex];
		length = strlen( log.logBuffer[log.readIndex] );
		//length = 4;
		
		log.readIndex++;
		if( log.readIndex >= MAX_ENTRIES)
			log.readIndex = 0;	
	}

	return length;
			
}

