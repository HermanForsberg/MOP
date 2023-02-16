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

//Tangetbordsrutin----------------------------------------------------------------------------------------------------
//Macro för port D (vi skriver PORT_D då vi använder adressen)
#define PORT_D 0x40020C00

//Gör en macro för pekarn som pekar på värdet i registerna
#define GPIO_MODER ((volatile unsigned int *)PORT_D)

//Mikrokontrollerns GPIO_PUPDR register 
#define GPIO_PUPDR ((volatile unsigned int *)PORT_D + 0x0C)

//Mikrokontrollerns speed GPIO_OSPEEDR register 
#define GPIO_OSPEEDR ((volatile unsigned int *)PORT_D +0x08)

//Mikrokontrollerns input register GPIO_ODR denna är kolomnen för key 
#define GPIO_ODR_HIGH ((volatile unsigned char *)PORT_D +0x15) 

//Mikrokontrollerns input register GPIO_IDR denna är raden för key 
#define GPIO_IDR_HIGH ((volatile unsigned char *)PORT_D +0x11) 

#define GPIO_ODR_LOW ((volatile unsigned char *)PORT_D +0x14) 

#define GPIO_OTYPER ((volatile unsigned short *)(PORT_D + 0x4))




void app_init(void){
	/* accessing the values using the pointers */
*((unsigned long *) 0x40023830) = 0x18; // starta klockor port D och E
*GPIO_MODER = 0x55005555; //Gör D 8-15 till en inport och 0-7 till utport  
*GPIO_PUPDR = 0x00AA0000; // sätter 10 (pull-down ger 1 då kretsen är sluten) på varje 8-15 pin floating på 0-7
*GPIO_OSPEEDR = 0x55555555;  // port D medium speed	
*GPIO_ODR_HIGH = 0;
*GPIO_ODR_LOW = 0;
*GPIO_OTYPER = 0x0000000;
}


int kbdGetCol(void){
	unsigned char c; 
	c = *GPIO_IDR_HIGH; //rad värdet placeras i c 
	if(c & 0x8) return 4;
	if(c & 0x4) return 3;
	if(c & 0x2) return 2;
	if(c & 0x1) return 1;
	return 0; 
		
}



// Hjälp rution för att sätta output registret till den kolumn vi tittar på. Om ett värde ges i GPIO_IDR så vet vi vilken rad och kolumn.
void kdbActivate(unsigned int row) { //hjälp rutin (MULTIPLEX SAKER FATTAR EJ)
	switch(row){
		case 1: *GPIO_ODR_HIGH = 0x10; 
			break; 
		case 2: *GPIO_ODR_HIGH = 0x20; 
			break;
		case 3: *GPIO_ODR_HIGH = 0x40; 
			break;
		case 4: *GPIO_ODR_HIGH = 0x80; 
			break;
		default: *GPIO_ODR_HIGH = 0x0;
			break;
	}
}


char keyb(void){
	//char keys[] = {1,2,3,0xA,4,5,6,0xB,7,8,9,0x39,0xE,0,0xF6,0xD}; 
	int row;
	int col;
	for(row = 1; row <= 4; row++){ //väljer en rad att se på
		kdbActivate(row);			//Ger ström till den raden vi har valt 
		col = kbdGetCol();
		if( col = 3){
			if(row = 1) return 3;
			if(row = 2) return 6;
			if(row = 3) return 9;
		}
	}
	kdbActivate(0);
	return 0xFF;
}

void init_app(void)
{
		/* accessing the values using the pointers */
	*((unsigned long *) 0x40023830) = 0x18; // starta klockor port D och E
	*GPIO_MODER = 0x55005555; //Gör D 8-15 till en inport och 0-7 till utport  
	*GPIO_PUPDR = 0x00AA0000; // sätter 10 (pull-down ger 1 då kretsen är sluten) på varje 8-15 pin floating på 0-7
	*GPIO_OSPEEDR = 0x55555555;  // port D medium speed	
	*GPIO_ODR_HIGH = 0;
	*GPIO_ODR_LOW = 0;
	*GPIO_OTYPER = 0x0000000;
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


//funktioner -------------------------------------------------------------------
void draw_object(POBJECT o)
{
	PGEOMETRY gmy = o->geo;
	POINT *px = gmy->px;
	int numpoints = gmy->numpoints;
	int x = o->posx;
	int y = o->posy;
	int drawx;
	int drawy;
	for(int i = 0; i < numpoints; i++)
	{
		POINT crp = px[i];
		drawx = x + crp.x;
		drawy = y + crp.y;
		graphic_pixel_set(drawx, drawy);
	}
}

void clear_object(POBJECT o)
{
	PGEOMETRY gmy = o->geo;
	POINT *px = gmy->px;
	int numpoints = gmy->numpoints;
	int x = o->posx;
	int y = o->posy;
	int drawx;
	int drawy;
	for(int i = 0; i < numpoints; i++)
	{
		POINT crp = px[i];
		drawx = x + crp.x;
		drawy = y + crp.y;
		graphic_pixel_clear(drawx, drawy);
	}
}

//BALL funktioner -------------------------------------------------------------------

void move_ballobject(POBJECT o)
{
	int psx = o->posx;
	int psy = o->posy;
	int drx = o->dirx;
	int dry = o->diry;
	clear_object(o);
	o->posx = psx + drx;
	o->posy = psy + dry;
	if(o->posx < 1){
		o->dirx = (- o->dirx);
	}
	
	if(o->posy < 1){
		o->diry = (- o->diry);
	}
	if(o->posy > (64-4)){
		o->diry = (- o->diry);
	}
	draw_object(o);
}

void set_ballobject_speed(POBJECT o, int speedx, int speedy)
{
	o->dirx = speedx;
	o->diry = speedy;
}


//PADDLE funktioner---------------------------------------------------------------------------------------------------

void move_paddleobject(POBJECT o)
{
	int psy = o->posy;
	int dry = o->diry;
	o->posy = psy + dry;
	
	clear_object(o);
	
	if(o->posy < 1){
		o->diry = 0;
		o->posy = 1;
	}
	if(o->posy > (64-9)){
		o->diry = 0;
		o->posy = 64-9;
	}
	draw_object(o);
}

void set_paddleobject_speed(POBJECT o, int speedy)
{
	o->diry = speedy;
}


//OBJECTS=====================================================================================================================

//BALL-----------------------------------
GEOMETRY ball_geometry = 
{
	12, 4, 4,
	{
		{0,1},{0,2},{1,0},{1,1},{1,2},{1,3},{2,0},{2,1},{2,2},{2,3},{3,1},{3,2}
	}
};

OBJECT ball = {
	&ball_geometry,
	4,1,
	64,32,
	draw_object,
	clear_object,
	move_ballobject,
	set_ballobject_speed,
};

//PADDLE--------------------------------
GEOMETRY paddle_geometry = 
{
	25, 5, 9,
	{
		{0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},{1,0},{1,8},{2,0},{2,3},{2,4},
		{2,5},{2,8},{3,0},{3,8},{4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8}
	}
};

OBJECT paddle = {
	&paddle_geometry,
	0,0,
	120,32,
	draw_object,
	clear_object,
	move_paddleobject,
	set_paddleobject_speed,
};


//MAIN ---------------------------------------------------------------------------------------------------------------

int main(void)
{
	char c;
	POBJECT p = &ball;
	POBJECT paddle = &paddle;
	init_app();
	graphic_initalize();
	graphic_clear_screen();
	while(1)
	{
		p->move(p);
		paddle->move(paddle);
		delay_milli(20);
		c=keyb();
		switch(c)
		{
			case 3: paddle->set_speed(paddle, 2); break;
			case 6: p->set_speed(p, -3, 0); break;
			case 9: paddle->set_speed(paddle, 2); break;
		}
	}
}


