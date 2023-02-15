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

typedef struct
{
	POINT p;
	int x, y;
}RECT, *PRECT;


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


//drawrect -----------------------------------------------------------------------------------------

int draw_rect(PRECT r)
{
	int start_x;
	int start_y;
	int end_x;
	int end_y;

	start_x = r->p.x;
	start_y = r->p.y;
	end_x = r->p.x + r->x;
	end_y = r->p.y;
	LINE line1 = {start_x, start_y, end_x, end_y};
	draw_line(&line1); 
	
	start_x = r->p.x + r->x;
	start_y = r->p.y;
	end_x = r->p.x + r->x;
	end_y = r->p.y + r->y;
	LINE line2 = {start_x, start_y, end_x, end_y};
	draw_line(&line2);
	
	start_x = r->p.x + r->x;
	start_y = r->p.y + r->y;
	end_x = r->p.x;
	end_y = r->p.y + r->y;
	LINE line3 = {start_x, start_y, end_x, end_y};
	draw_line(&line3);
	
	start_x = r->p.x;
	start_y = r->p.y + r->y;
	end_x = r->p.x;
	end_y = r->p.y;
	LINE line4 = {start_x, start_y, end_x, end_y};
	draw_line(&line4);
	
	//returnerar beroende på om det gick att måla upp eller ej
	start_x = r->p.x;
	start_y = r->p.y;
	end_x = r->p.x + r->x;
	end_y = r->p.y + r->y;
	
	if(start_x <= 0){
		return 0;
	}
	if(start_y <= 0){
		return 0;
	}
	if(end_x >= 128){
		return 0;
	}
	if(end_y >= 64){
		return 0;
	}
	return 1;
	
}




//MAIN ------------------------------------------------------------------

RECT rectangles[] = {
{10,10, 20,10},
{25,25, 10,20},
{40,30, 70,20},
{60,35, 10,10},
{70,10, 5,5}
};

void main(void)
{
	graphic_initalize();
	graphic_clear_screen();
	while(1)
	{
		for (int i = 0; i < sizeof(rectangles)/sizeof(RECT); i++)
		{
			draw_rect(&rectangles[i]);
			delay_milli(5);
		}
		graphic_clear_screen();
	}
}

