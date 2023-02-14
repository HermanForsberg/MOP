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

void init_app(void)
{
	* ((unsigned long *) 0x40021000) = 0x5555;

}

typedef struct
{
	char x,y;
} POINT, *PPOINT;

typedef struct
{
	POINT p0;
	POINT p1;
} LINE, *PLINE;

int draw_line(PLINE l)
{
	int steep;
	int deltax;
	int deltay;
	int error;
	int y;
	int ystep;
	if(abs(*l.p1.y - *l.p0.y) > abs(*l.p1.x - *l.p0.x))
	{
		steep = 1;
	}else{
		steep = 0;
	}
	if (steep == 1)
	{
		swap(l->p0.x, l.p0.y);
		swap(l.p1.x, l.p1.y);
	}
	if(l.p0.x > l.p1.x)
	{
		swap(l.p0.x, l.p1.x);
		swap(l.p0.y, l.p1.y);
	}
	deltax = l.p1.x - l.p0.x;
	deltay = abs(l.p1.y - l.p0.y);
	error = 0;
	y = l.p0.y;
	if (l.p0.y < l.p1.y )
	{
		ystep = 1;
	}else{
		ystep = -1;
	}
	for(int x; x < (l.p0.x - l.p1.x); x++)
	{
		if (steep == 1)
		{
			graphic_pixel_set(y, x);
		}else{
			graphic_pixel_set(x, y);
		}
		error = error + deltay
		if ((2*error) >= deltax){
			y = y + ystep;
			error = error- deltax
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

int swap(int i1, int i2)
{
	int tmp;
	tmp = i1;
	i1 = i2;
	i2 = tmp;
	return i1, i2;
}

LINE lines

void main(void)
{
	graphic_initalize();
	graphic_clear_screen();
	while(1)
	{
		for (int i = 0; i < sizeof(lines)/sizeof(LINE); i++)
		{
			draw_line(&lines[i]);
			delay_milli(500);
		}
		graphic_clear_screen();
	}
}

