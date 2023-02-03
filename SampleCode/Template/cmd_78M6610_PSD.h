/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include "NuMicro.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/

#define METER_UART_PORT								(UART1)

/*
	7.1.4  Auto-Reported Data 

	Parameter/Register 		Number of Bytes 		Description 
	Packet Header(0xAE)  	1  						Start of Data Packet 
	Packet Length(42)  		1  						Number of Bytes in the Packet 
	Chip Temperature  		3  						Chip Temperature 
	Frequency  				3  						Line Frequency  
	ExtTemp  				3  						ATEMP1 Reading 
	Vavg  					3  						Avg Voltage 
	Iavg  					3  						Avg Current  
	Vrms  					3  						RMS Voltage  
	Irms  					3  						RMS Current  
	Power  					3  						Active Power  
	VAR  					3  						Reactive Power  
	VFUND  					3  						Fundamental RMS Voltage  
	IFUND  					3  						Fundamental RMS Current  
	PFUND  					3  						Fundamental Active Power  
	QFUND  					3  						Fundamental Reactive Power  
	HARM  					3  						Fundamental Harmonic Measured  
	CHKSUM  				1  						Check Sum 

	data : 3 * 14 = 42 
	header , length , checksum = 3

*/

#define	METER_PACKET_HEADER							(0xAE) 
// #define METER_PACKET_LENGTH							(42)		//30
#define METER_PACKET_LENGTH							(45)		//0x2D


#define	METER_SYNC_HEADER							(0xAA) 
#define	METER_REG_ADDR_SELECT						(0xA3) 
#define	METER_WRITE_COMMAND							(0xD3) 


typedef struct power_sample_struct 
{
	uint8_t valid;          			//struct valid flag
	uint8_t Packet_Header;          	//Start of Data Packet 
	uint8_t Packet_Length;          	//Number of Bytes in the Packet  
	
	float Chip_Temperature;           	//Chip Temperature
	float Frequency;           			//Line Frequency
	float ExtTemp;           			//ATEMP1 Reading	
	float Vavg;           				//Avg Voltage 	
	float Iavg;           				//Avg Current  	
	float Vrms;           				//RMS Voltage  	
	float Irms;           				//RMS Current  	
	float Power;           				//Active Power  	
	float VAR;           				//Reactive Power  	
	float VFUN;           				//Fundamental RMS Voltage  		
    float IFUN;                   		//Fundamental RMS Current  
    float PFUN;                   		//Fundamental Active Power  
    float QFUN;                   		//Fundamental Reactive Power  
    float HARM;                   		//Fundamental Harmonic Measured

	uint8_t CHKSUM;          			//Check Sum
	
	// int32_t vrms;           //RMS voltage
	// int32_t irms;           //RMS current
	// int32_t watts;          //watts
	// int32_t pavg;           //Average power (30s window)
	// int32_t freq;           //Line frequency
	// int32_t pf;             //Power factor
	// int32_t kwh;            //kWh since turn on
} power_sample;



/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

void meter_uart_process(void);
void meter_loop_process(void);

void meter_send_packet(uint8_t on);

void UART1_TX_Send(uint32_t len,uint8_t *ptr);
void UART1_Init(void);
