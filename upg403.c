/*
 * 	startup.c
 *
 */
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

#define STK_CTRL ((volatile unsigned int *)(0xE000E010))  
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))  
#define STK_VAL ((volatile unsigned int *)(0xE000E018))  
#define Bargraph ((volatile unsigned int *)(0x40021014))

#define B_E 0x40 /* Enable-signal */
#define B_SELECT 4 /* Välj ASCII-display */
#define B_RW 2 /* 0=Write, 1=Read */
#define B_RS 1 /* 0=Control, 1=Data */

#define GPIO_E_MODER ((volatile unsigned int *)(0x40021000))

#define SBC_VTOR (volatile unsigned int*) 0xE000ED08

#define GPIO_E_IDRLOW ((volatile unsigned char *)(0x40021010))
#define GPIO_E_IDRHIGH ((volatile unsigned char *)(0x40021011))

#define GPIO_E_ODRLOW ((volatile unsigned char *)(0x40021014))
#define GPIO_E_ODRHIGH ((volatile unsigned char *)(0x40021015))


void delay_250ns( void )
{
/* SystemCoreClock = 168000000 */
*STK_CTRL = 0;
*STK_LOAD = ( (168/4) -1 );
*STK_VAL = 0;
*STK_CTRL = 5;
while( (*STK_CTRL & 0x10000 )== 0 );
*STK_CTRL = 0;
}

void delay_micro(unsigned int us)
{
while( us > 0 )
{
delay_250ns();
delay_250ns();
delay_250ns();
delay_250ns();
us--;
}
}



#ifdef SIMULATOR
#define DELAY_COUNT 100
#else
#define DELAY_COUNT 1000000
#endif
void init_app(void)
{
	
}

void main(void)
{
	init_app();
	*GPIO_ODR_LOW = 0;
	delay(DELAY_COUNT);
	*GPIO_ODR_LOW = 0xFF;
	while(1)
	{
		
	}
}

