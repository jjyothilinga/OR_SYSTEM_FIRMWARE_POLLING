/*********************************************************************
 *
 *  Main Application Entry Point and TCP/IP Stack Demo
 *  Module for Microchip TCP/IP Stack
 *   -Demonstrates how to call and use the Microchip TCP/IP stack
 *   -Reference: Microchip TCP/IP Stack Help (TCPIP Stack Help.chm)1150

 *
 *********************************************************************
 * FileName:        MainDemo.c
 * Dependencies:    TCPIP.h
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.11b or higher
 *                  Microchip C30 v3.24 or higher
 *                  Microchip C18 v3.36 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2010 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *      ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *      used in conjunction with a Microchip ethernet controller for
 *      the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 * File Description:
 * Change History:
 * Rev   Description
 * ----  -----------------------------------------
 * 1.0   Initial release
 * V5.36 ---- STACK_USE_MPFS support has been removed 
 ********************************************************************/
/*
 * This macro uniquely defines this file as the main entry point.
 * There should only be one such definition in the entire project,
 * and this file must define the AppConfig variable as described below.
 */
#define THIS_IS_STACK_APPLICATION

// Include all headers for any enabled TCPIP Stack functions
#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
#include "TCPIP Stack/ZeroconfLinkLocal.h"
#endif
#if defined(STACK_USE_ZEROCONF_MDNS_SD)
#include "TCPIP Stack/ZeroconfMulticastDNS.h"
#endif

// Include functions specific to this stack application
#include "MainDemo.h"
#include "linearkeypad.h"

// Used for Wi-Fi assertions
#define WF_MODULE_NUMBER   WF_MODULE_MAIN_DEMO

// Declare AppConfig structure and some other supporting stack variables
APP_CONFIG AppConfig;
static unsigned short wOriginalAppConfigChecksum;    // Checksum of the ROM defaults for AppConfig
BYTE AN0String[8];


BYTE keyinfo;

// Use UART2 instead of UART1 for stdout (printf functions).  Explorer 16 
// serial port hardware is on PIC UART2 module.
#if defined(EXPLORER_16) || defined(PIC24FJ256DA210_DEV_BOARD)
    int __C30_UART = 2;
#endif


// Private helper functions.
// These may or may not be present in all applications.
static void InitAppConfig(void);
static void InitializeBoard(void);
static void ProcessIO(void);
#if defined(WF_CS_TRIS)
    void WF_Connect(void);
    #if !defined(MRF24WG)
    extern BOOL gRFModuleVer1209orLater;
    #endif
    
    #if defined(DERIVE_KEY_FROM_PASSPHRASE_IN_HOST)
    tPassphraseReady g_WpsPassphrase;
    #endif    /* defined(DERIVE_KEY_FROM_PASSPHRASE_IN_HOST) */
#endif





#if defined(WF_CS_TRIS)
// Global variables
UINT8 ConnectionProfileID;
#endif

//
// Main application entry point.
//
#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{
    static DWORD currentTick = 0;
	static DWORD bsdTick = 0;
	static DWORD keypadTick = 0;
    static DWORD dwLastIP = 0;
	UINT8 data = 0;

    // Initialize application specific hardware
    InitializeBoard();

    // Initialize stack-related hardware components that may be 
    // required by the UART configuration routines
    TickInit();
    #if defined(STACK_USE_MPFS2)
    MPFSInit();
    #endif

    // Initialize Stack and application related NV variables into AppConfig.
    InitAppConfig();

    // Initialize core stack layers (MAC, ARP, TCP, UDP) and
    // application modules (HTTP, SNMP, etc.)
    StackInit();

    // Initialize any application-specific modules or functions/
    // For this demo application, this only includes the
    // UART 2 TCP Bridge
    #if defined(STACK_USE_UART2TCP_BRIDGE)
    UART2TCPBridgeInit();
    #endif

    #if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
    ZeroconfLLInitialize();
    #endif

//	ADCON2 = 0X0F;

	ADCON0 = 0X00;	// Disable A/D
	ADCON1 = 0X0F;	// Set all pin as GPIO
	ADCON0bits.ADON = 0;
	ADCON1bits.PCFG3 = 1;
	ADCON1bits.PCFG2 = 1;
	ADCON1bits.PCFG1 = 1;
	ADCON1bits.PCFG0 = 1;

	EnableInterrupts( );
    // Now that all items are initialized, begin the co-operative
    // multitasking loop.  This infinite loop will continuously 
    // execute all stack-related tasks, as well as your own
    // application's functions.  Custom functions should be added
    // at the end of this loop.
    // Note that this is a "co-operative mult-tasking" mechanism
    // where every task performs its tasks (whether all in one shot
    // or part of it) and returns so that other tasks can do their
    // job.
    // If a task needs very lgong time to do its job, it must be broken
    // down into smaller pieces so that other tasks can have CPU time.
    while(1)
    {
        // Blink LED0 (right most one) every second.
        if(TickGet() - currentTick >= TICK_SECOND/2ul)
        {
            currentTick = TickGet();
	        LED0_IO ^= 1;
//			TxData = TRUE;
        }

		 if(TickGet() - keypadTick >= TICK_SECOND/10ul)
        {
            keypadTick = currentTick ;
			LinearKeyPad_scan();

        }

        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();
        


        // This tasks invokes each of the core stack application tasks
        StackApplications();

        // Process application specific tasks here.
        // For this demo app, this will include the Generic TCP 
        // client and servers, and the SNMP, Ping, and SNMP Trap
        // demos.  Following that, we will process any IO from
        // the inputs on the board itself.
        // Any custom modules or processing you need to do should
        // go here.
        #if defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE)
        GenericTCPClient();
        #endif
        
        #if defined(STACK_USE_GENERIC_TCP_SERVER_EXAMPLE)
        GenericTCPServer();
        #endif
        
        #if defined(STACK_USE_SMTP_CLIENT)
        SMTPDemo();
        #endif
        
        #if defined(STACK_USE_ICMP_CLIENT)
        PingDemo();
        #endif

		#if defined(STACK_USE_TFTP_CLIENT) && defined(WF_CS_TRIS)	
		TFTPGetUploadStatus();
		#endif
        
        #if defined(STACK_USE_SNMP_SERVER) && !defined(SNMP_TRAP_DISABLED)
        //User should use one of the following SNMP demo
        // This routine demonstrates V1 or V2 trap formats with one variable binding.
        SNMPTrapDemo();
        
        #if defined(SNMP_STACK_USE_V2_TRAP) || defined(SNMP_V1_V2_TRAP_WITH_SNMPV3)
        //This routine provides V2 format notifications with multiple (3) variable bindings
        //User should modify this routine to send v2 trap format notifications with the required varbinds.
        //SNMPV2TrapDemo();
        #endif 
        if(gSendTrapFlag)
            SNMPSendTrap();
        #endif
        
        #if defined(STACK_USE_BERKELEY_API)
       // BerkeleyTCPClientDemo();
		if(TickGet() - bsdTick >= TICK_SECOND/80ul)
        {
			bsdTick = currentTick;
			BerkeleyTCPServerDemo();

        }
     	
          
        //BerkeleyUDPClientDemo();
        #endif

        // If the local IP address has changed (ex: due to DHCP lease change)
        // write the new IP address to the LCD display, UART, and Announce 
        // service
        if(dwLastIP != AppConfig.MyIPAddr.Val)
        {
            dwLastIP = AppConfig.MyIPAddr.Val;
            
            #if defined(STACK_USE_UART)
                putrsUART((ROM char*)"\r\nNew IP Address: ");
            #endif

//            DisplayIPValue(AppConfig.MyIPAddr);

            #if defined(STACK_USE_UART)
                putrsUART((ROM char*)"\r\n");
            #endif


            #if defined(STACK_USE_ANNOUNCE)
                AnnounceIP();
            #endif

            #if defined(STACK_USE_ZEROCONF_MDNS_SD)
                mDNSFillHostRecord();
            #endif

			#ifdef WIFI_NET_TEST	
			#ifdef STACK_USE_TFTP_CLIENT
			if(AppConfig.Flags.bIsDHCPEnabled && DHCPIsBound(0)) {	
				static UINT8  tftpInitDone = 0;
				static BYTE dummy_file[] = "TFTP test dummy contents";
				static ROM BYTE file_name[] = "dontcare";
				static ROM BYTE host_name[] = "tftp" WIFI_NET_TEST_DOMAIN;	
				if (!tftpInitDone) {
					TFTPUploadRAMFileToHost(host_name, file_name, dummy_file, sizeof(dummy_file));
					tftpInitDone = 1;
				}
			}
			#endif
			#endif
			
        }    
        #if defined(DERIVE_KEY_FROM_PASSPHRASE_IN_HOST) && defined (MRF24WG)
            if (g_WpsPassphrase.valid) {
                WF_ConvPassphrase2Key(g_WpsPassphrase.passphrase.keyLen, g_WpsPassphrase.passphrase.key,
                g_WpsPassphrase.passphrase.ssidLen, g_WpsPassphrase.passphrase.ssid);
                WF_SetPSK(g_WpsPassphrase.passphrase.key);
                g_WpsPassphrase.valid = FALSE;
            }
        #endif    /* defined(DERIVE_KEY_FROM_PASSPHRASE_IN_HOST) */
		#if defined(STACK_USE_AUTOUPDATE_HTTPSERVER) && defined(WF_CS_TRIS) && defined(MRF24WG)
		{
			static DWORD t_UpdateImage=0;
			extern UINT8 Flag_ImageUpdate_running;
			if(Flag_ImageUpdate_running == 1)
			{
				UINT8 buf_command[4];
				if( (TickGet() - t_UpdateImage) >= TICK_SECOND * 120ul)
				{
					putsUART((char *)"Update Firmware timeout, begin to restore..\r\n");
					buf_command[0]=UPDATE_CMD_ERASE0; //Erase bank0
					buf_command[1]=UPDATE_SERITY_KEY_1; 
					buf_command[2]=UPDATE_SERITY_KEY_2; 
					buf_command[3]=UPDATE_SERITY_KEY_3; 
					SendSetParamMsg(PARAM_FLASH_update, buf_command, 4);
					buf_command[0]=UPDATE_CMD_CPY_1TO0; //Copy bank1 to bank0
					buf_command[1]=UPDATE_SERITY_KEY_1; 
					buf_command[2]=UPDATE_SERITY_KEY_2; 
					buf_command[3]=UPDATE_SERITY_KEY_3; 
					SendSetParamMsg(PARAM_FLASH_update, buf_command, 4);
					putsUART((char *)"restore Done\r\n");
					Flag_ImageUpdate_running = 0;
				}
					
			}
			else
			{
				t_UpdateImage = TickGet();
			}
		}
		#endif
	}
}




/****************************************************************************
  Function:
    static void InitializeBoard(void)

  Description:
    This routine initializes the hardware.  It is a generic initialization
    routine for many of the Microchip development boards, using definitions
    in HardwareProfile.h to determine specific initialization.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/
static void InitializeBoard(void)
{    
    // LEDs
    LED0_TRIS = 0;
//    LED1_TRIS = 0;
    LED2_TRIS = 0;
    LED3_TRIS = 0;
    LED4_TRIS = 0;
    LED5_TRIS = 0;
//	LED6_TRIS = 0;
//	LED7_TRIS = 0;
    LED_PUT(0x00);

#if defined(__18CXX)
    // Enable 4x/5x/96MHz PLL on PIC18F87J10, PIC18F97J60, PIC18F87J50, etc.
    OSCTUNE = 0x40;

    // Set up analog features of PORTA

    ADCON0 = 0x00;        // Disable A/D
    ADCON1 = 0x0F;        // Set all pin as digital I/O



    // Enable internal PORTB pull-ups
   // INTCON2bits.RBPU = 0;

    // Configure USART
    TXSTA = 0x20;
    RCSTA = 0x90;

    // See if we can use the high baud rate setting
    #if ((GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1) <= 255
        SPBRG = (GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1;
        TXSTAbits.BRGH = 1;
    #else    // Use the low baud rate setting
        SPBRG = (GetPeripheralClock()+8*BAUD_RATE)/BAUD_RATE/16 - 1;
    #endif


    // Enable Interrupts
//    RCONbits.IPEN = 1;        // Enable interrupt priorities
//    INTCONbits.GIEH = 1;
//    INTCONbits.GIEL = 1;

 
// Deassert all chip select lines so there isn't any problem with 
// initialization order.  Ex: When ENC28J60 is on SPI2 with Explorer 16, 
// MAX3232 ROUT2 pin will drive RF12/U2CTS ENC28J60 CS line asserted, 
// preventing proper 25LC256 EEPROM operation.
#if defined(ENC_CS_TRIS)
    ENC_CS_IO = 1;
    ENC_CS_TRIS = 0;
#endif
#if defined(ENC100_CS_TRIS)
    ENC100_CS_IO = (ENC100_INTERFACE_MODE == 0);
    ENC100_CS_TRIS = 0;
#endif
#if defined(EEPROM_CS_TRIS)
    EEPROM_CS_IO = 1;
    EEPROM_CS_TRIS = 0;
#endif
#if defined(SPIRAM_CS_TRIS)
    SPIRAM_CS_IO = 1;
    SPIRAM_CS_TRIS = 0;
#endif
#if defined(SPIFLASH_CS_TRIS)
    SPIFLASH_CS_IO = 1;
    SPIFLASH_CS_TRIS = 0;
#endif
#if defined(WF_CS_TRIS)
    WF_CS_IO = 1;
    WF_CS_TRIS = 0;
#endif



	// Initialize linear keypad 
	LinearKeyPad_init();

	/*--------------------------------------------------------

		----------------------------------------- 
		|  RE1	|  RB0	|  RB2	|  RA2	|  3.3V	|
		----------------------------------------- 
		|  RE0  |  RB1	|  RB3	|  RA3	|  GND	|
		----------------------------------------- 
	--------------------------------------------------------*/
	// Set the direction for Keypad Port
	TRISAbits.TRISA3 = 1; // #define KEY_PIN_0 PORTAbits.RA3
	TRISAbits.TRISA2 = 1; // #define KEY_PIN_1 PORTAbits.RA2
	TRISBbits.TRISB3 = 1; // #define KEY_PIN_2 PORTBbits.RB3
	TRISBbits.TRISB2 = 1; // #define KEY_PIN_3 PORTBbits.RB2
	TRISBbits.TRISB1 = 1; // #define KEY_PIN_4 PORTBbits.RB1
	TRISBbits.TRISB0 = 1; // #define KEY_PIN_5 PORTBbits.RB0
	TRISEbits.TRISE0 = 1; // #define KEY_PIN_6 PORTEbits.RE0
	TRISEbits.TRISE1 = 1; // #define KEY_PIN_7 PORTEbits.RE1



	ADCON1 = 0x0F; // all digital
}

/*********************************************************************
 * Function:        void InitAppConfig(void)
 *
 * PreCondition:    MPFSInit() is already called.
 *
 * Input:           None
 *
 * Output:          Write/Read non-volatile config variables.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
// MAC Address Serialization using a MPLAB PM3 Programmer and 
// Serialized Quick Turn Programming (SQTP). 
// The advantage of using SQTP for programming the MAC Address is it
// allows you to auto-increment the MAC address without recompiling 
// the code for each unit.  To use SQTP, the MAC address must be fixed
// at a specific location in program memory.  Uncomment these two pragmas
// that locate the MAC address at 0x1FFF0.  Syntax below is for MPLAB C 
// Compiler for PIC18 MCUs. Syntax will vary for other compilers.
//#pragma romdata MACROM=0x1FFF0
static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata

static void InitAppConfig(void)
{
#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
    unsigned char vNeedToSaveDefaults = 0;
#endif
    
    while(1)
    {
        // Start out zeroing all AppConfig bytes to ensure all fields are 
        // deterministic for checksum generation
        memset((void*)&AppConfig, 0x00, sizeof(AppConfig));
        
        AppConfig.Flags.bIsDHCPEnabled = TRUE;
        AppConfig.Flags.bInConfigMode = TRUE;
        memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
//        {
//            _prog_addressT MACAddressAddress;
//            MACAddressAddress.next = 0x157F8;
//            _memcpy_p2d24((char*)&AppConfig.MyMACAddr, MACAddressAddress, sizeof(AppConfig.MyMACAddr));
//        }
        AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
        AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
        AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
        AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
        AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
        AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
        AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;
    
    
        // SNMP Community String configuration
        #if defined(STACK_USE_SNMP_SERVER)
        {
            BYTE i;
            static ROM char * ROM cReadCommunities[] = SNMP_READ_COMMUNITIES;
            static ROM char * ROM cWriteCommunities[] = SNMP_WRITE_COMMUNITIES;
            ROM char * strCommunity;
            
            for(i = 0; i < SNMP_MAX_COMMUNITY_SUPPORT; i++)
            {
                // Get a pointer to the next community string
                strCommunity = cReadCommunities[i];
                if(i >= sizeof(cReadCommunities)/sizeof(cReadCommunities[0]))
                    strCommunity = "";
    
                // Ensure we don't buffer overflow.  If your code gets stuck here, 
                // it means your SNMP_COMMUNITY_MAX_LEN definition in TCPIPConfig.h 
                // is either too small or one of your community string lengths 
                // (SNMP_READ_COMMUNITIES) are too large.  Fix either.
                if(strlenpgm(strCommunity) >= sizeof(AppConfig.readCommunity[0]))
                    while(1);
                
                // Copy string into AppConfig
                strcpypgm2ram((char*)AppConfig.readCommunity[i], strCommunity);
    
                // Get a pointer to the next community string
                strCommunity = cWriteCommunities[i];
                if(i >= sizeof(cWriteCommunities)/sizeof(cWriteCommunities[0]))
                    strCommunity = "";
    
                // Ensure we don't buffer overflow.  If your code gets stuck here, 
                // it means your SNMP_COMMUNITY_MAX_LEN definition in TCPIPConfig.h 
                // is either too small or one of your community string lengths 
                // (SNMP_WRITE_COMMUNITIES) are too large.  Fix either.
                if(strlenpgm(strCommunity) >= sizeof(AppConfig.writeCommunity[0]))
                    while(1);
    
                // Copy string into AppConfig
                strcpypgm2ram((char*)AppConfig.writeCommunity[i], strCommunity);
            }
        }
        #endif
    
        // Load the default NetBIOS Host Name
        memcpypgm2ram(AppConfig.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
        FormatNetBIOSName(AppConfig.NetBIOSName);
    
        #if defined(WF_CS_TRIS)
            // Load the default SSID Name
            WF_ASSERT(sizeof(MY_DEFAULT_SSID_NAME)-1 <= sizeof(AppConfig.MySSID));
            memcpypgm2ram(AppConfig.MySSID, (ROM void*)MY_DEFAULT_SSID_NAME, sizeof(MY_DEFAULT_SSID_NAME));
            AppConfig.SsidLength = sizeof(MY_DEFAULT_SSID_NAME) - 1;
    
            AppConfig.SecurityMode = MY_DEFAULT_WIFI_SECURITY_MODE;
            
            #if (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_OPEN)
                memset(AppConfig.SecurityKey, 0x00, sizeof(AppConfig.SecurityKey));
                AppConfig.SecurityKeyLength = 0;
    
            #elif MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WEP_40
                AppConfig.WepKeyIndex  = MY_DEFAULT_WEP_KEY_INDEX;
                memcpypgm2ram(AppConfig.SecurityKey, (ROM void*)MY_DEFAULT_WEP_KEYS_40, sizeof(MY_DEFAULT_WEP_KEYS_40) - 1);
                AppConfig.SecurityKeyLength = sizeof(MY_DEFAULT_WEP_KEYS_40) - 1;
    
            #elif MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WEP_104
                AppConfig.WepKeyIndex  = MY_DEFAULT_WEP_KEY_INDEX;
                memcpypgm2ram(AppConfig.SecurityKey, (ROM void*)MY_DEFAULT_WEP_KEYS_104, sizeof(MY_DEFAULT_WEP_KEYS_104) - 1);
                AppConfig.SecurityKeyLength = sizeof(MY_DEFAULT_WEP_KEYS_104) - 1;
    
            #elif (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_WITH_KEY)       || \
                  (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA2_WITH_KEY)      || \
                  (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_AUTO_WITH_KEY)
                memcpypgm2ram(AppConfig.SecurityKey, (ROM void*)MY_DEFAULT_PSK, sizeof(MY_DEFAULT_PSK) - 1);
                AppConfig.SecurityKeyLength = sizeof(MY_DEFAULT_PSK) - 1;
    
            #elif (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_WITH_PASS_PHRASE)     || \
                  (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA2_WITH_PASS_PHRASE)    || \
                  (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE)
                memcpypgm2ram(AppConfig.SecurityKey, (ROM void*)MY_DEFAULT_PSK_PHRASE, sizeof(MY_DEFAULT_PSK_PHRASE) - 1);
                AppConfig.SecurityKeyLength = sizeof(MY_DEFAULT_PSK_PHRASE) - 1;
            #elif (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPS_PUSH_BUTTON)
                memset(AppConfig.SecurityKey, 0x00, sizeof(AppConfig.SecurityKey));
                AppConfig.SecurityKeyLength = 0;
            #elif (MY_DEFAULT_WIFI_SECURITY_MODE == WF_SECURITY_WPS_PIN)
                memcpypgm2ram(AppConfig.SecurityKey, (ROM void*)MY_DEFAULT_WPS_PIN, sizeof(MY_DEFAULT_WPS_PIN) - 1);
                AppConfig.SecurityKeyLength = sizeof(MY_DEFAULT_WPS_PIN) - 1;
            #else 
                #error "No security defined"
            #endif /* MY_DEFAULT_WIFI_SECURITY_MODE */
    
        #endif

        // Compute the checksum of the AppConfig defaults as loaded from ROM
        wOriginalAppConfigChecksum = CalcIPChecksum((BYTE*)&AppConfig, sizeof(AppConfig));

        #if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
        {
            NVM_VALIDATION_STRUCT NVMValidationStruct;

            // Check to see if we have a flag set indicating that we need to 
            // save the ROM default AppConfig values.
            if(vNeedToSaveDefaults)
                SaveAppConfig(&AppConfig);
        
            // Read the NVMValidation record and AppConfig struct out of EEPROM/Flash
            #if defined(EEPROM_CS_TRIS)
            {
                XEEReadArray(0x0000, (BYTE*)&NVMValidationStruct, sizeof(NVMValidationStruct));
                XEEReadArray(sizeof(NVMValidationStruct), (BYTE*)&AppConfig, sizeof(AppConfig));
            }
            #elif defined(SPIFLASH_CS_TRIS)
            {
                SPIFlashReadArray(0x0000, (BYTE*)&NVMValidationStruct, sizeof(NVMValidationStruct));
                SPIFlashReadArray(sizeof(NVMValidationStruct), (BYTE*)&AppConfig, sizeof(AppConfig));
            }
            #endif
    
            // Check EEPROM/Flash validitity.  If it isn't valid, set a flag so 
            // that we will save the ROM default values on the next loop 
            // iteration.
            if((NVMValidationStruct.wConfigurationLength != sizeof(AppConfig)) ||
               (NVMValidationStruct.wOriginalChecksum != wOriginalAppConfigChecksum) ||
               (NVMValidationStruct.wCurrentChecksum != CalcIPChecksum((BYTE*)&AppConfig, sizeof(AppConfig))))
            {
                // Check to ensure that the vNeedToSaveDefaults flag is zero, 
                // indicating that this is the first iteration through the do 
                // loop.  If we have already saved the defaults once and the 
                // EEPROM/Flash still doesn't pass the validity check, then it 
                // means we aren't successfully reading or writing to the 
                // EEPROM/Flash.  This means you have a hardware error and/or 
                // SPI configuration error.
                if(vNeedToSaveDefaults)
                {
                    while(1);
                }
                
                // Set flag and restart loop to load ROM defaults and save them
                vNeedToSaveDefaults = 1;
                continue;
            }
            
            // If we get down here, it means the EEPROM/Flash has valid contents 
            // and either matches the ROM defaults or previously matched and 
            // was run-time reconfigured by the user.  In this case, we shall 
            // use the contents loaded from EEPROM/Flash.
            break;
        }
        #endif
        break;
    }
}

#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
void SaveAppConfig(const APP_CONFIG *ptrAppConfig)
{
    NVM_VALIDATION_STRUCT NVMValidationStruct;

    // Ensure adequate space has been reserved in non-volatile storage to 
    // store the entire AppConfig structure.  If you get stuck in this while(1) 
    // trap, it means you have a design time misconfiguration in TCPIPConfig.h.
    // You must increase MPFS_RESERVE_BLOCK to allocate more space.
    #if defined(STACK_USE_MPFS2)
        if(sizeof(NVMValidationStruct) + sizeof(AppConfig) > MPFS_RESERVE_BLOCK)
            while(1);
    #endif

    // Get proper values for the validation structure indicating that we can use 
    // these EEPROM/Flash contents on future boot ups
    NVMValidationStruct.wOriginalChecksum = wOriginalAppConfigChecksum;
    NVMValidationStruct.wCurrentChecksum = CalcIPChecksum((BYTE*)ptrAppConfig, sizeof(APP_CONFIG));
    NVMValidationStruct.wConfigurationLength = sizeof(APP_CONFIG);

    // Write the validation struct and current AppConfig contents to EEPROM/Flash
    #if defined(EEPROM_CS_TRIS)
        XEEBeginWrite(0x0000);
        XEEWriteArray((BYTE*)&NVMValidationStruct, sizeof(NVMValidationStruct));
        XEEWriteArray((BYTE*)ptrAppConfig, sizeof(APP_CONFIG));
    #else
        SPIFlashBeginWrite(0x0000);
        SPIFlashWriteArray((BYTE*)&NVMValidationStruct, sizeof(NVMValidationStruct));
        SPIFlashWriteArray((BYTE*)ptrAppConfig, sizeof(APP_CONFIG));
    #endif
}
#endif
