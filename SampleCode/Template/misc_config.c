/*_____ I N C L U D E S ____________________________________________________*/
#include "misc_config.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

struct flag_8bit flag_MISC_CTL;
#define FLAG_MISC_ERROR                                 (flag_MISC_CTL.bit0)
#define FLAG_MISC_REVERSE1                 				(flag_MISC_CTL.bit1)
#define FLAG_MISC_REVERSE2                 				(flag_MISC_CTL.bit2)
#define FLAG_MISC_REVERSE3                              (flag_MISC_CTL.bit3)
#define FLAG_MISC_REVERSE4                              (flag_MISC_CTL.bit4)
#define FLAG_MISC_REVERSE5                              (flag_MISC_CTL.bit5)
#define FLAG_MISC_REVERSE6                              (flag_MISC_CTL.bit6)
#define FLAG_MISC_REVERSE7                              (flag_MISC_CTL.bit7)

/*_____ D E F I N I T I O N S ______________________________________________*/

#if defined (ENABLE_TICK_EVENT)
typedef void (*sys_pvTimeFunPtr)(void);   /* function pointer */
typedef struct timeEvent_t
{
    unsigned char             active;
    unsigned int       initTick;
    unsigned int       curTick;
    sys_pvTimeFunPtr    funPtr;
} TimeEvent_T;

#define TICKEVENTCOUNT                                 (8)                   
volatile  TimeEvent_T tTimerEvent[TICKEVENTCOUNT];
volatile unsigned char _sys_uTimerEventCount = 0;             /* Speed up interrupt reponse time if no callback function */
#endif

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

void compare_buffer(unsigned char *src, unsigned char *des, int nBytes)
{
    unsigned short  i = 0;	
	
    #if 1
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            dbg_printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			FLAG_MISC_ERROR = 1;//set_flag(flag_error , ENABLE);
        }
    }

	if (!FLAG_MISC_ERROR)//(!is_flag_set(flag_error))
	{
    	dbg_printf("%s finish \r\n" , __FUNCTION__);	
		FLAG_MISC_ERROR = 0;//set_flag(flag_error , DISABLE);
	}
    #else
    if (memcmp(src, des, nBytes))
    {
        dbg_printf("\nMismatch!! - %d\n", nBytes);
        for (i = 0; i < nBytes; i++)
            dbg_printf("0x%02x    0x%02x\n", src[i], des[i]);
        return -1;
    }
    #endif

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    unsigned char *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (unsigned char *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    unsigned char *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (unsigned char *)dest;
    pu8Src  = (unsigned char *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(unsigned char *pucBuff, int nBytes)
{
    unsigned short  i = 0;
    
    dbg_printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        dbg_printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            dbg_printf("\r\n");
        }            
    }
    dbg_printf("\r\n\r\n");
}

void dump_buffer_hex(unsigned char *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        dbg_printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            dbg_printf("%02X ", pucBuff[nIdx + i]);
        dbg_printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                dbg_printf("%c", pucBuff[nIdx + i]);
            else
                dbg_printf(".");
            nBytes--;
        }
        nIdx += 16;
        dbg_printf("\n");
    }
    dbg_printf("\n");
}

#if defined (ENABLE_TICK_EVENT)
void TickCallback_processB(void)
{
    dbg_printf("%s test \r\n" , __FUNCTION__);
}

void TickCallback_processA(void)
{
    dbg_printf("%s test \r\n" , __FUNCTION__);
}

void TickClearTickEvent(unsigned char u8TimeEventID)
{
    if (u8TimeEventID > TICKEVENTCOUNT)
        return;

    if (tTimerEvent[u8TimeEventID].active == TRUE)
    {
        tTimerEvent[u8TimeEventID].active = FALSE;
        _sys_uTimerEventCount--;
    }
}

signed char TickSetTickEvent(unsigned int uTimeTick, void *pvFun)
{
    int  i;
    int u8TimeEventID = 0;

    for (i = 0; i < TICKEVENTCOUNT; i++)
    {
        if (tTimerEvent[i].active == FALSE)
        {
            tTimerEvent[i].active = TRUE;
            tTimerEvent[i].initTick = uTimeTick;
            tTimerEvent[i].curTick = uTimeTick;
            tTimerEvent[i].funPtr = (sys_pvTimeFunPtr)pvFun;
            u8TimeEventID = i;
            _sys_uTimerEventCount += 1;
            break;
        }
    }

    if (i == TICKEVENTCOUNT)
    {
        return -1;    /* -1 means invalid channel */
    }
    else
    {
        return u8TimeEventID;    /* Event ID start from 0*/
    }
}

void TickCheckTickEvent(void)
{
    unsigned char i = 0;

    if (_sys_uTimerEventCount)
    {
        for (i = 0; i < TICKEVENTCOUNT; i++)
        {
            if (tTimerEvent[i].active)
            {
                tTimerEvent[i].curTick--;

                if (tTimerEvent[i].curTick == 0)
                {
                    (*tTimerEvent[i].funPtr)();
                    tTimerEvent[i].curTick = tTimerEvent[i].initTick;
                }
            }
        }
    }
}

void TickInitTickEvent(void)
{
    unsigned char i = 0;

    _sys_uTimerEventCount = 0;

    /* Remove all callback function */
    for (i = 0; i < TICKEVENTCOUNT; i++)
        TickClearTickEvent(i);

    _sys_uTimerEventCount = 0;
}
#endif 

