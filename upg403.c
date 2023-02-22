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


#define B_E 0x40 /* Enable-signal */
#define B_SELECT 4 /* Välj ASCII-display */
#define B_RW 2 /* 0=Write, 1=Read */
#define B_RS 1 /* 0=Control, 1=Data */


#define SBC_VTOR ((volatile unsigned int*) (0xE000ED08))

#define GPIO_E_MODER ((volatile unsigned int *)(0x40021000))
#define GPIO_E_IDRLOW ((volatile unsigned char *)(0x40021010))
#define GPIO_E_IDRHIGH ((volatile unsigned char *)(0x40021011))
#define GPIO_E_ODRLOW ((volatile unsigned char *)(0x40021014))
#define GPIO_E_ODRHIGH ((volatile unsigned char *)(0x40021015))


#define GPIO_D_MODER ((volatile unsigned int *)(0x40020C00))
#define GPIO_D_IDRLOW ((volatile unsigned char *)(0x40020C10))
#define GPIO_D_IDRHIGH ((volatile unsigned char *)(0x40020C11))
#define GPIO_D_ODRLOW ((volatile unsigned char *)(0x40020C14))
#define GPIO_D_ODRHIGH ((volatile unsigned char *)(0x40020C15))

static int systick_flag;

//DELAY==============================================================================


void delay_1mikro(void)
{
	systick_flag = 0;
	*STK_CTRL = 0;
	*STK_LOAD = ( 168 - 1 );
	*STK_VAL = 0;
	*STK_CTRL = 7;
}

unsigned int delay_count;
void delay(unsigned int count)
{
	if (count == 0) return;
	delay_count = count;
	delay_1mikro();
}


//HANDLER==============================================================================
void systick_irq_handler(void)
{
	*STK_CTRL = 0;
	delay_count -= 1;
	if(delay_count > 0)
	{
		delay_1mikro();
	}else{
		systick_flag = 1;
	}
}


//INIT==============================================================================

void init_handler_systick(void)
{
	*((void (**)(void)) 0x2001C03C) = systick_irq_handler;
	systick_flag = 0;
}



#define DELAY_COUNT 1000

void init_app(void)
{
	*SBC_VTOR = 0x2001C000;
	*GPIO_D_MODER = 0x00005555;
	init_handler_systick();
}

void main(void)
{
	init_app();
	*GPIO_D_ODRLOW = 0;
	delay(DELAY_COUNT);
	*GPIO_D_ODRLOW = 0xFF;
	while(1)
	{
		if(systick_flag)
		{
			break;
		}
	}
	*GPIO_D_ODRLOW = 0;
}

