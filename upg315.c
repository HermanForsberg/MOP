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

//GRAPHIC drivrutiner============================================

__attribute__((naked))
void graphic_initalize(void)
{
	__asm volatile (".HWORD 0xDFF0\n");
	__asm volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_clear_screen(void)
{
	__asm volatile (".HWORD 0xDFF1\n");
	__asm volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_set(int x, int y)
{
	__asm volatile (".HWORD 0xDFF2\n");
	__asm volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_clear(int x, int y)
{
	__asm volatile (".HWORD 0xDFF3\n");
	__asm volatile (" BX LR\n");
}

//DELAY===============================================================

#define STK_CTRL ((volatile unsigned int *)(0xE000E010))  
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))  
#define STK_VAL ((volatile unsigned int *)(0xE000E018))  
#define Bargraph ((volatile unsigned int *)(0x40021014))
#define GPIO_MODER ((volatile unsigned int *)(0x40021000))

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

//Structs ------------------------------------------------------
typedef struct
{
	char x,y;
} POINT, *PPOINT;

#define MAX_POINTS 30
typedef struct
{
	int numpoints;
	int sizex;
	int sizey;
	POINT px[MAX_POINTS];
} GEOMETRY, *PGEOMETRY;

typedef struct
{
	PGEOMETRY geo;
	int dirx,diry;
	int posx,posy;
	void (* draw) (struct tObj *);
	void (* clear) (struct tObj *);
	void (* move) (struct tObj *);
	void (* set_speed) (struct tObj *,int, int);
} OBJECT, *POBJECT;

//FUNKTIONER -------------------------------------------------------------------
void draw_ballobject(POBJECT o)
{
	PGEOMETRY gmy = o->geo;
	POINT *px = gmy->px;
	int numpoints = gmy->numpoints;
	int x = o->posx;
	int y = o->posy;
	for(int i = 0; i < numpoints; i++)
	{
		graphic_pixel_set(x, y);
		px = px + sizeof(POINT);
	}
}

void clear_ballobject(POBJECT o)
{
	PGEOMETRY gmy = o->geo;
	POINT *px = gmy->px;
	int numpoints = gmy->numpoints;
	int x = o->posx;
	int y = o->posy;
	for(int i = 0; i < numpoints; i++)
	{
		graphic_pixel_clear(x, y);
		px = px + sizeof(POINT);
	}
}

void move_ballobject(POBJECT o)
{
	
}

void set_ballobject_speed(POBJECT o, int speedx, int speedy)
{
	
}

//MAIN ----------------------------------------------------

void main(void)
{
}
