/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include "NuMicro.h"

#include "misc_config.h"
#include "cmd_78M6610_PSD.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
power_sample meter_sample;

/*_____ D E F I N I T I O N S ______________________________________________*/

// #define ENABLE_RAW_DATA

#define FIFO_THRESHOLD 		(4)
#define RX_BUFFER_SIZE 		(64)
#define RX_TIMEOUT_CNT 		(60) //40~255

enum
{
    eUART_RX_Received_Data_Finish = 0,
    eUART_RX_Received_Data_NOT_Finish
};
volatile uint8_t g_au8UART_RX_Buffer[RX_BUFFER_SIZE] = {0}; // UART Rx received data Buffer (RAM)
volatile uint8_t g_bUART_RX_Received_Data_State = eUART_RX_Received_Data_NOT_Finish;
volatile uint8_t g_u8UART_RDA_Trigger_Cnt = 0; // UART RDA interrupt trigger times counter
volatile uint8_t g_u8UART_RXTO_Trigger_Cnt = 0; // UART RXTO interrupt trigger times counter

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

//default configuration is 9600 baud, 8-bit, no-parity, 1 stop-bit, no flow control.
//Accum : 100ms

void meter_read_power(void)
{
	//start listening
	meter_sample.valid = FALSE;	
	UART_EnableInt(METER_UART_PORT, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
}

void meter_output_log(void)
{
	#if 1
	printf("\r\n\r\n");//printf("\r\n\r\n>>>>>>log:start<<<<<<\r\n");
	printf("ChipTemp 	= %0.4f\r\n" ,meter_sample.Chip_Temperature);
	printf("Frequency 	= %0.4f\r\n" ,meter_sample.Frequency);    
	printf("ExtTemp 	= %0.4f\r\n" ,meter_sample.ExtTemp);       
	printf("Vavg 		= %0.4f\r\n" ,meter_sample.Vavg);         
	printf("Iavg 		= %0.4f\r\n" ,meter_sample.Iavg);         
	printf("Vrms 		= %0.4f\r\n" ,meter_sample.Vrms);         
	printf("Irms 		= %0.4f\r\n" ,meter_sample.Irms);         
	printf("Power 		= %0.4f\r\n" ,meter_sample.Power);        
	printf("VAR 		= %0.4f\r\n" ,meter_sample.VAR);        	
	printf("VFUN 		= %0.4f\r\n" ,meter_sample.VFUN);         
	printf("IFUN 		= %0.4f\r\n" ,meter_sample.IFUN);           
	printf("PFUN 		= %0.4f\r\n" ,meter_sample.PFUN);           
	printf("QFUN 		= %0.4f\r\n" ,meter_sample.QFUN);           
	printf("HARM 		= %0.4f\r\n" ,meter_sample.HARM); 
	#endif
}

uint8_t meter_process_sample(uint8_t *buffer)
{
	//process 30 byte data packet buffer
	uint8_t checksum = 0;
	uint8_t bytes[3] = {0};
	int32_t tmp[14] = {0};
	uint8_t i = 0;
	
	//1.) check for header and length
	if(buffer[0] != METER_PACKET_HEADER || buffer[1] != METER_PACKET_LENGTH)
	{
		printf("bad header:0x%2X or length:0x%2X\r\n" , buffer[0] , buffer[1]);
		return FALSE;
	}
	
	//2.) compute checksum
	for(i = 0; i < (METER_PACKET_LENGTH - 1); i++)
	{
		checksum += buffer[i];
	}
	checksum = (~checksum)+1;
	if(checksum != buffer[METER_PACKET_LENGTH - 1])
	{
		printf("bad checksum:0x%2X\r\n" , buffer[METER_PACKET_LENGTH - 1]);
		return FALSE;
	}

	#if 1          
	// dump_buffer_hex( (uint8_t *) buffer , METER_PACKET_LENGTH);
	for(i = 0; i < 14; i++)
	{
		bytes[0] = buffer[3*i+2];
		bytes[1] = buffer[3*i+3];
		bytes[2] = buffer[3*i+4];		
		tmp[i] = bytes[0] | bytes[1]<<8 | bytes[2]<<16;
		if((bytes[2] & 0x80) == 0x80)
		{
		  tmp[i] |= (0xFFUL << 24); //sign extend top byte
		}
		// printf("tmp[%d] = %4d\r\n" ,i,tmp[i]);
	}
	//4.) Populate the power struct
	meter_sample.Chip_Temperature 	= (float) (tmp[0]/1000.0);
	meter_sample.Frequency 			= (float) (tmp[1]/1000.0);   
	meter_sample.ExtTemp 			= (float) (tmp[2]/1000.0);         
	meter_sample.Vavg 				= (float) (tmp[3]/1000.0);           
	meter_sample.Iavg 				= (float) (tmp[4]*7.77e-6);           
	meter_sample.Vrms 				= (float) (tmp[5]/1000.0); // /1000.0;          
	meter_sample.Irms 				= (float) (tmp[6]*7.77e-6); // *7.77e-6;          
	meter_sample.Power 				= (float) (tmp[7]/200.0); // /200.0;         
	meter_sample.VAR 				= (float) (tmp[8]/1000.0);          	
	meter_sample.VFUN 				= (float) (tmp[9]/1000.0);           
	meter_sample.IFUN 				= (float) (tmp[10]);            
	meter_sample.PFUN 				= (float) (tmp[11]);            
	meter_sample.QFUN 				= (float) (tmp[12]);            
	meter_sample.HARM 				= (float) (tmp[13]);  

	#else
	//3.) Parse raw data into values
	//    Data is 3 byte signed LSB
	for(i = 0; i < 9; i++)
	{
		bytes[0] = buffer[3*i+2];
		bytes[1] = buffer[3*i+3];
		bytes[2] = buffer[3*i+4];
		tmp[i] = bytes[0] | bytes[1]<<8 | bytes[2]<<16;
		if((bytes[2]&0x80) == 0x80)
		{
		  tmp[i] |= 0xFF << 24; //sign extend top byte
		}
	}
	//4.) Populate the power struct
	meter_sample.vrms = tmp[2];
	meter_sample.irms = tmp[3];
	meter_sample.watts= tmp[4];
	meter_sample.pavg = tmp[5];
	meter_sample.pf = tmp[6];
	meter_sample.freq   = tmp[7];
	meter_sample.kwh  = tmp[8];
	#endif
	
	
	//5.) Set the valid flag
	meter_sample.valid = TRUE; 
	//all done
	return TRUE;
};

void meter_uart_process(void)
{
	uint8_t tmp;
	static uint8_t buf[METER_PACKET_LENGTH+1]; //30 byte packet
	static uint8_t buf_idx=0;
	tmp = UART_READ(METER_UART_PORT);
	// printf("0x%2X\r\n" , tmp);
	
	switch(buf_idx)
	{
		case 0: //search for sync byte
			if(tmp == METER_PACKET_HEADER)
			{
				//found sync byte, start capturing the packet
				buf[buf_idx++] = tmp;
			}
			break;
		case METER_PACKET_LENGTH: //sample is full, read checksum and return data
			if(meter_process_sample(buf))
			{				
				// dump_buffer_hex( (uint8_t *) buf , (int) sizeof(buf));
				//success, stop listening to the UART					
				UART_DisableInt(METER_UART_PORT, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
				//reset the index
				buf_idx = 0;
			} 
			else 
			{ //failure, look for the next packet
				buf_idx = 0;
			}
			break;
		case 1: //make sure the packet length is valid
			if(tmp != METER_PACKET_LENGTH)
			{
				buf_idx = 0;	
				break;			
			} //else, add to buf			
		default: //reading packet
			buf[buf_idx++] = tmp;
			// break;
	}	
}


void meter_loop_process(void)
{
	#if defined (ENABLE_RAW_DATA)

	/* Wait to receive UART data */
	// while(UART_RX_IDLE(METER_UART_PORT));

	// /* Start to received UART data */
	// g_bUART_RX_Received_Data_State = eUART_RX_Received_Data_NOT_Finish;        
	// /* Wait for receiving UART message finished */
	// while(g_bUART_RX_Received_Data_State != eUART_RX_Received_Data_Finish); 

	if (g_bUART_RX_Received_Data_State == eUART_RX_Received_Data_Finish)
	{
		// printf("\nMETER_UART_PORT Rx Received Data : %s\n",g_au8UART_RX_Buffer);
		dump_buffer_hex( (uint8_t *) g_au8UART_RX_Buffer , (int) sizeof(g_au8UART_RX_Buffer));
		// printf("METER_UART_PORT Rx RDA (Fifofull) interrupt times : %d\n",g_u8UART_RDA_Trigger_Cnt);
		// printf("METER_UART_PORT Rx RXTO (Timeout) interrupt times : %d\n",g_u8UART_RXTO_Trigger_Cnt);
	    UART_WAIT_TX_EMPTY(UART0);

		/* Reset UART interrupt parameter */
		g_bUART_RX_Received_Data_State = eUART_RX_Received_Data_NOT_Finish;
		reset_buffer( (void *) g_au8UART_RX_Buffer , 0x00 ,  (unsigned int) sizeof(g_au8UART_RX_Buffer));
		UART_EnableInt(METER_UART_PORT, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
		g_u8UART_RDA_Trigger_Cnt = 0; // UART RDA interrupt times
		g_u8UART_RXTO_Trigger_Cnt = 0; // UART RXTO interrupt times
	}


	#else
	if (meter_sample.valid)	// data ready
	{
		meter_output_log();
	}
	meter_read_power();
	#endif
}

void meter_send_packet(uint8_t on)
{
	const uint8_t len = 10;
	uint8_t packet[len] = {0};
	uint8_t Register_Address_MSB = 0;
	uint8_t Register_Address_LSB = 0;
	uint8_t checksum = 0;
	uint8_t i = 0;
	
	packet[0] = METER_SYNC_HEADER;
	packet[1] = len;
	packet[2] = METER_REG_ADDR_SELECT;
	packet[3] = Register_Address_LSB;
	packet[4] = Register_Address_MSB;
	packet[5] = METER_WRITE_COMMAND;

	if (on)
	{
		packet[6] = 0x01;
	}
	else
	{
		packet[6] = 0x00;
	}
	packet[7] = 0x00;
	packet[8] = METER_PACKET_HEADER;

	//2.) compute checksum
	for(i = 0; i < (len-1); i++)
	{
		checksum += packet[i];
	}
	checksum = (~checksum)+1;
	// printf("checksum = 0x%2X,len : %2d\r\n",checksum,len);

	packet[9] = checksum;

	UART1_TX_Send(len,packet);
}


void UART13_IRQHandler(void)
{
	#if defined (ENABLE_RAW_DATA)
    uint8_t i;
    static uint16_t u16UART_RX_Buffer_Index = 0;

	if ((UART_GET_INT_FLAG(METER_UART_PORT,UART_INTSTS_RDAINT_Msk)))
	{
        /* UART receive data available flag */
        
        /* Record RDA interrupt trigger times */
        g_u8UART_RDA_Trigger_Cnt++;
        
        /* Move the data from Rx FIFO to sw buffer (RAM). */
        /* Every time leave 1 byte data in FIFO for Rx timeout */
        for(i = 0 ; i < (FIFO_THRESHOLD - 1) ; i++)
        {
            g_au8UART_RX_Buffer[u16UART_RX_Buffer_Index] = UART_READ(METER_UART_PORT);
            u16UART_RX_Buffer_Index ++;

            if (u16UART_RX_Buffer_Index >= RX_BUFFER_SIZE) 
                u16UART_RX_Buffer_Index = 0;
        }	
	}
    else if(UART_GET_INT_FLAG(METER_UART_PORT, UART_INTSTS_RXTOINT_Msk)) 
    {
        /* When Rx timeout flag is set to 1, it means there is no data needs to be transmitted. */

        /* Record Timeout times */
        g_u8UART_RXTO_Trigger_Cnt++;

        /* Move the last data from Rx FIFO to sw buffer. */
        while(UART_GET_RX_EMPTY(METER_UART_PORT) == 0)
        {
            g_au8UART_RX_Buffer[u16UART_RX_Buffer_Index] = UART_READ(METER_UART_PORT);
            u16UART_RX_Buffer_Index ++;
        }

        /* Clear UART RX parameter */
        UART_DISABLE_INT(METER_UART_PORT, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
        u16UART_RX_Buffer_Index = 0;
        g_bUART_RX_Received_Data_State = eUART_RX_Received_Data_Finish;
    }

	#else
    if(UART_GET_INT_FLAG(METER_UART_PORT, UART_INTSTS_RDAINT_Msk | UART_INTSTS_RXTOINT_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(METER_UART_PORT) == 0)
        {
			meter_uart_process();
        }
    }
	#endif

    if(METER_UART_PORT->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
    {
        UART_ClearIntFlag(METER_UART_PORT, (UART_INTSTS_RLSINT_Msk| UART_INTSTS_BUFERRINT_Msk));
    }	
}

void UART1_TX_Send(uint32_t len,uint8_t *ptr)
{
    uint32_t i = 0;

    /* UART send response to master */    
    for (i = 0; i < len; i++) 
	{        
		#if 1
		/* Wait for TX not full */
		while (UART_GET_TX_FULL(METER_UART_PORT));

		/* UART send data */
		UART_WRITE(METER_UART_PORT, ptr[i]);
		#else
		UART_WRITE(METER_UART_PORT, *ptr++);
		UART_WAIT_TX_EMPTY(METER_UART_PORT);
		#endif
    }
}

void UART1_Init(void)
{
    SYS_ResetModule(UART1_RST);

    /* Configure METER_UART_PORT and set METER_UART_PORT baud rate */
    UART_Open(METER_UART_PORT, 9600);

    UART_EnableInt(METER_UART_PORT, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk);
    NVIC_EnableIRQ(UART13_IRQn);

    METER_UART_PORT->FIFO = ((METER_UART_PORT->FIFO & (~UART_FIFO_RFITL_Msk)) | UART_FIFO_RFITL_4BYTES);
	UART_SetTimeoutCnt(METER_UART_PORT, RX_TIMEOUT_CNT);

    meter_send_packet(1);
}


