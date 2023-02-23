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

#define SYSCFG_EXTICR1 ((volatile unsigned int *) (0x40013808))

#define EXTI_IMR ((volatile unsigned int *) (0x40013C00))
#define EXTI_RTSR ((volatile unsigned int *) (0x40013C08))
#define EXTI_FTSR ((volatile unsigned int *) (0x40013C0C))

#define NVIC_ISER0 ((volatile unsigned int *) (0xE000E100))

static int EXTI3_flag;

unsigned int count;

//HANDLER==============================================================================
void EXTI3_irq_handler(void)
{
	count += 1;
	
}


//INIT==============================================================================

void init_handler_systick(void)
{
	*((void (**)(void)) 0x2001C064) = EXTI3_irq_handler;
	EXTI3_flag = 0;
}


void init_app(void)
{
	*SBC_VTOR = 0x2001C000;
	*GPIO_D_MODER = 0x00005555;
	
	*SYSCFG_EXTICR1 &= ~0xF000;
	*SYSCFG_EXTICR1 |= 0x4000;
	*EXTI_IMR |= 8;
	*EXTI_RTSR |= 8;
	*EXTI_FTSR &= ~8;
	init_handler_systick();
	
	*NVIC_ISER0 |= (1<<9);
	
}

void main(void)
{
	init_app();
	while(1)
	{
		*GPIO_D_ODRLOW = count;
	}
}
