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

void delay_milli(unsigned int ms)
{
	while(ms>0){
		int i = 50;
		while(i>0){
			delay_250ns();
			i = i-1;
		
		}
	ms--;
	}
}

void init_app(void)
{
	* ((unsigned long *) 0x40021000) = 0x55555555;
}

void ascii_ctrl_bit_set( char x )
{ /* x: bitmask bits are 1 to set */
char c;
c = *GPIO_E_ODRLOW;
*GPIO_E_ODRLOW = B_SELECT | x | c;
}

void ascii_ctrl_bit_clear( char x )
{ /* x: bitmask bits are 1 to clear */
char c;
c = *GPIO_E_ODRLOW;
c = c & ~x;
*GPIO_E_ODRLOW = B_SELECT | c;
}


unsigned char ascii_read_controller( void )
{
*GPIO_E_MODER = 0x00005555;
char c;
ascii_ctrl_bit_set( B_E );
delay_250ns();
delay_250ns();
c = *GPIO_E_IDRHIGH;
ascii_ctrl_bit_clear( B_E );
*GPIO_E_MODER = 0x55555555;
return c;
}

unsigned char ascii_read_data(void)
{
char c;
ascii_ctrl_bit_set( B_RW );
ascii_ctrl_bit_set( B_RS );
c = ascii_read_controller();
return c;
}

unsigned char ascii_read_status( void )
{
char c;
ascii_ctrl_bit_set( B_RW );
ascii_ctrl_bit_clear( B_RS );
c = ascii_read_controller();
return c;
}

char ascii_write_controller(unsigned char cmd)
{
delay_milli(1);
ascii_ctrl_bit_set( B_E );
*GPIO_E_ODRHIGH = cmd;
delay_250ns();
ascii_ctrl_bit_clear(B_E);
delay_250ns();
}

void ascii_write_cmd(unsigned char cmd)
{
ascii_ctrl_bit_clear( B_RS );
ascii_ctrl_bit_clear( B_RW );
ascii_write_controller(cmd);

}

void ascii_write_data(unsigned char data)
{
ascii_ctrl_bit_set( B_RS );
ascii_ctrl_bit_clear( B_RW );
ascii_write_controller(data);
}

void ascii_init(void)
{
delay_milli(1);
ascii_write_cmd(56);
delay_milli(2);
ascii_write_cmd(0xe);
delay_milli(2);
clear_display();
delay_milli(2);
ascii_write_cmd(6);
delay_milli(2);

}

void clear_display()
{
while((ascii_read_status() & 0x80)== 0x80 );
delay_micro(8);
ascii_write_cmd(1);
delay_milli(2);
}


void ascii_gotoxy(int x, int y)
{
int adress = x-1;
if (y == 2){
	adress = adress + 0x40;
}
ascii_write_cmd(0x80 | adress);
	
}

void ascii_write_char(unsigned char c)
{
while((ascii_read_status() & 0x80)== 0x80 );
delay_micro(8);
ascii_write_data(c);
delay_milli(2);
}

int main(void)
{
char *s;
char test1[] = "Alfanumerisk";
char test2[] = "Display - test";

init_app();
ascii_init();
ascii_gotoxy(1, 1);
s = test1;
while(*s){
	ascii_write_char(*s++);
}
ascii_gotoxy(1, 2);
s = test2;
while(*s){
	ascii_write_char(*s++);
}
return 0;
}
