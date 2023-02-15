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
#define GPIO_MODER ((volatile unsigned int *)(0x40021000))

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

//structs -------------------------------------------------->>>>

typedef struct
{
	char x,y;
} POINT, *PPOINT;

typedef struct
{
	POINT p0;
	POINT p1;
} LINE, *PLINE;

typedef struct polygonpoint
{
	char x, y;
	struct polygonpoint *next;
} POLYPOINT, *PPOLYPOINT;

//drawline -----------------------------------------------------------------------------------------

int draw_line(PLINE l)
{
	int steep;
	int deltax;
	int deltay;
	int error;
	int y;
	int x;
	int ystep;
	int x0 = l->p0.x;
	int x1 = l->p1.x;
	int y0 = l->p0.y;
	int y1 = l->p1.y;
	if(abs(y1 - y0) > abs(x1 - x0))
	{
		steep = 1;
	}else{
		steep = 0;
	}
	if (steep == 1)
	{
		swap(&x0, &y0);
		swap(&x1, &y1);
	}
	if(x0 > x1)
	{
		swap(&x0, &x1);
		swap(&y0, &y1);
	}
	deltax = x1 - x0;
	deltay = abs(y1 - y0);
	error = 0;
	y = y0;
	if (y0 < y1)
	{
		ystep = 1;
	}else{
		ystep = -1;
	}
	for(int x = x0; x <= x1; x++)
	{
		if (steep == 1)
		{
			graphic_pixel_set(y, x);
		}else{
			graphic_pixel_set(x, y);
		}
		error = error + deltay;
		if ((2*error) >= deltax){
			y = y + ystep;
			error = error - deltax;
		}
	}
}

int abs(int i)
{
	if(i<0){
		i = (-i);
	}
return i;
}

void swap(int *i1, int *i2)
{
	int tmp;
	tmp = *i1;
	*i1 = *i2;
	*i2 = tmp;
}


//drawPOLY -----------------------------------------------------------------------------------------

int draw_polygon(PPOLYPOINT p)
{
	char p0_x;
	char p0_y;
	char p1_x;
	char p1_y;
	p0_x = p->x;
	p0_y = p->y;
	PPOLYPOINT ptr = p->next;
	while(ptr != 0)
	{
		p1_x = ptr->x;
		p1_y = ptr->y;
		LINE ln = {p0_x,p0_y, p1_x,p1_y};
		draw_line(&ln);
		p0_x = p1_x;
		p0_y = p1_y;
		ptr = ptr->next;
		if(p0_x <= 0){
			return 0;
		}
		if(p0_y <= 0){
			return 0;
		}
		if(p0_x >= 128){
			return 0;
		}
		if(p0_y >= 64){
		return 0;
		}
	}
	return 1;
}

//MAIN ------------------------------------------------------------------

POLYPOINT pg8 = {20,20, 0};
POLYPOINT pg7 = {20,55, &pg8};
POLYPOINT pg6 = {70,60, &pg7};
POLYPOINT pg5 = {80,35, &pg6};
POLYPOINT pg4 = {100,25, &pg5};
POLYPOINT pg3 = {90,10, &pg4};
POLYPOINT pg2 = {40,10, &pg3};
POLYPOINT pg1 = {20,20, &pg2};

void main(void)
{
	graphic_initalize();
	graphic_clear_screen();
	while(1)
	{
		draw_polygon(&pg1);
		delay_milli(50);
		graphic_clear_screen();
	}
}

