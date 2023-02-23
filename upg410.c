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
#define EXTI_PR ((volatile unsigned int *) (0x40013C14))

#define NVIC_ISER0 ((volatile unsigned int *) (0xE000E100))

#define NVIC_EXTI2_IRQ_BPOS (1<<8)
#define NVIC_EXTI1_IRQ_BPOS (1<<7)
#define NVIC_EXTI0_IRQ_BPOS (1<<6)

#define EXTI2_IRQ_BPOS (1<<2)
#define EXTI1_IRQ_BPOS (1<<1)
#define EXTI0_IRQ_BPOS (1<<0)


static int EXTI3_flag;

unsigned int count;

//DELAY=================================================================================
void delay_mikro(unsigned int u)
{
	for(unsigned int i = 0; i<u; i++)
	{
		*STK_CTRL = 0;
		*STK_LOAD = ( 168 - 1 );
		*STK_VAL = 0;
		*STK_CTRL = 7;
	}	
}


//HANDLERS==============================================================================

void EXTI0_irq_handler(void)
{
	if((*EXTI_PR & 1) == 1)
	{
		*EXTI_PR |= 1;
		if((*GPIO_E_IDRLOW & 1) == 1)
		{
			*GPIO_E_ODRLOW |= 16;
			*GPIO_E_ODRLOW &= ~16;
			count += 1;
		}
	}
}

void EXTI1_irq_handler(void)
{
	if((*EXTI_PR & 2) == 2)
	{
		*EXTI_PR |= 2;
		if((*GPIO_E_IDRLOW & 2) == 2)
		{
			*GPIO_E_ODRLOW |= 32;
			*GPIO_E_ODRLOW &= ~32;
			count = 0;
		}
	}
}

void EXTI2_irq_handler(void)
{
	if((*EXTI_PR & 4) == 4)
	{
		*EXTI_PR |= 4;
		if((*GPIO_E_IDRLOW & 4) == 4)
		{
			*GPIO_E_ODRLOW |= 64;
			*GPIO_E_ODRLOW &= ~64;
			if(count < 0xFF)
			{
				count = 0xFF;
				*GPIO_D_ODRLOW = count;
			}else{
				count = 0x00;
				*GPIO_D_ODRLOW = count;
			}
		}
	}
}

//INIT==============================================================================

void init_handler_EXTI(void)
{
	*((void (**)(void)) 0x2001C058) = EXTI0_irq_handler;
	*((void (**)(void)) 0x2001C05C) = EXTI1_irq_handler;
	*((void (**)(void)) 0x2001C060) = EXTI2_irq_handler;
	EXTI3_flag = 0;
}


void init_app(void)
{
	*SBC_VTOR = 0x2001C000;
	*GPIO_D_MODER = 0x00005555;
	*GPIO_E_MODER = 0x00005500;
	
	*SYSCFG_EXTICR1 &= ~0x0000;
	*SYSCFG_EXTICR1 |= 0x444;
	*EXTI_IMR |= EXTI2_IRQ_BPOS | EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS;
	*EXTI_RTSR |= EXTI2_IRQ_BPOS | EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS;
	*EXTI_FTSR &= ~EXTI2_IRQ_BPOS | EXTI1_IRQ_BPOS | EXTI0_IRQ_BPOS;
	
	init_handler_EXTI();
	
	*NVIC_ISER0 |= NVIC_EXTI2_IRQ_BPOS | NVIC_EXTI1_IRQ_BPOS | NVIC_EXTI0_IRQ_BPOS;

	
}

void main(void)
{
	init_app();
	while(1)
	{
		*GPIO_D_ODRLOW = count;
		*GPIO_D_ODRLOW = count;
	}
}
