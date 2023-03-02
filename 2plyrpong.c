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

//defines===============================================================
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


//Macro för port D (vi skriver PORT_D då vi använder adressen)
#define PORT_D 0x40020C00

//Gör en macro för pekarn som pekar på värdet i registerna
#define GPIO_D_MODER ((volatile unsigned int *)PORT_D)

//Mikrokontrollerns GPIO_D_PUPDR register 
#define GPIO_D_PUPDR ((volatile unsigned int *)PORT_D + 0x0C)

//Mikrokontrollerns speed GPIO_D_OSPEEDR register 
#define GPIO_D_OSPEEDR ((volatile unsigned int *)PORT_D +0x08)

//Mikrokontrollerns input register GPIO_ODR denna är kolomnen för key 
#define GPIO_D_ODR_HIGH ((volatile unsigned char *)PORT_D +0x15) 

#define GPIO_D_ODR_LOW ((volatile unsigned char *)PORT_D +0x14) 

//Mikrokontrollerns input register GPIO_IDR denna är raden för key 
#define GPIO_D_IDR_HIGH ((volatile unsigned char *)PORT_D +0x11) 

#define GPIO_D_OTYPER ((volatile unsigned short *)(PORT_D + 0x4))

//DELAY===============================================================


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


int kbdGetCol_l(void){
	unsigned char cl; 
	cl = *GPIO_D_IDR_HIGH; //rad värdet placeras i c 
	if(cl & 0x1) return 1;
	return 0; 
}


int kbdGetCol_r(void){
	unsigned char cl; 
	cl = *GPIO_D_IDR_HIGH; //rad värdet placeras i c 
	if(cl & 0x4) return 3;
	return 0; 
}


// Hjälp rution för att sätta output registret till den kolumn vi tittar på. Om ett värde ges i GPIO_IDR så vet vi vilken rad och kolumn.
void kdbActivate(unsigned int row) { //hjälp rutin (MULTIPLEX SAKER FATTAR EJ)
	switch(row){
		case 1: *GPIO_D_ODR_HIGH = 0x10; 
			break; 
		case 2: *GPIO_D_ODR_HIGH = 0x20; 
			break;
		case 3: *GPIO_D_ODR_HIGH = 0x40; 
			break;
		case 4: *GPIO_D_ODR_HIGH = 0x80; 
			break;
		default: *GPIO_D_ODR_HIGH = 0x0;
			break;
	}
}


char keyb_r(void){
	char keys[] = {1,2,3,0xA,4,5,6,0xB,7,8,9,0x39,0xE,0,0xF6,0xD}; 
	int row;
	int col;
	for(row = 1; row <= 4; row++){ //väljer en rad att se på
		kdbActivate(row);			//Ger ström till den raden vi har valt 
		col = kbdGetCol_r();
		if(col == 3){
			return (row*3 + (col-3));
		}
	}
	kdbActivate(0);
	return 0xFF;
}

char keyb_l(void){
	char keys[] = {1,2,3,0xA,4,5,6,0xB,7,8,9,0x39,0xE,0,0xF6,0xD}; 
	int row;
	int col;
	for(row = 1; row <= 4; row++){ //väljer en rad att se på
		kdbActivate(row);			//Ger ström till den raden vi har valt 
		col = kbdGetCol_l();
		if(col == 1){
			return (row*3 + (col-3));
		}
	}
	kdbActivate(0);
	return 0xFF;
}


void init_app(void)
{
		/* accessing the values using the pointers */
	*((unsigned long *) 0x40023830) = 0x18; // starta klockor port D och E
	*GPIO_D_MODER = 0x55005555; //Gör D 8-15 till en inport och 0-7 till utport  
	*GPIO_D_PUPDR = 0x00AA0000; // sätter 10 (pull-down ger 1 då kretsen är sluten) på varje 8-15 pin floating på 0-7
	*GPIO_D_OSPEEDR = 0x55555555;  // port D medium speed	
	*GPIO_D_ODR_HIGH = 0;
	*GPIO_D_ODR_LOW = 0;
	*GPIO_D_OTYPER = 0x0000000;
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
	char tempx;
	char tempy;
	for(int i = 0; i < numpoints; i++)
	{
		POINT crp = px[i];
		tempx = crp.x;
		tempy = crp.y;
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
	
	//if(o->posx < 1){
	//	o->dirx = (- o->dirx);
	//}
	
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
	clear_object(o);
	int psy = o->posy;
	int dry = o->diry;
	o->posy = psy + dry;
	
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

void set_paddleobject_speed(POBJECT o, int speedx, int speedy)
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
	64,30,
	draw_object,
	clear_object,
	move_ballobject,
	set_ballobject_speed,
};

//PADDLE--------------------------------
GEOMETRY paddle_geometry = 
{
	27, 5, 9,
	{
		{0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},{1,0},{1,8},{2,0},{2,3},{2,4},
		{2,5},{2,8},{3,0},{3,8},{4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8}
	}
};

OBJECT paddle_right = {
	&paddle_geometry,
	0,0,
	115,27,
	draw_object,
	clear_object,
	move_paddleobject,
	set_paddleobject_speed,
};

OBJECT paddle_left = {
	&paddle_geometry,
	0,0,
	8,27,
	draw_object,
	clear_object,
	move_paddleobject,
	set_paddleobject_speed,
};

//ASCII-display=========================================================================================================

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

//ASCII-write score funktion----------------------------------------------------------
void display_score(char p1, char p2)
{
	char *s;
	char score_p1[] = "Player 1 score: 0";
	char score_p2[] = "Player 2 score: 0";
	ascii_gotoxy(1, 1);
	s = score_p1;
	while(*s){
		ascii_write_char(*s++);
	}
	ascii_gotoxy(1, 2);
	s = score_p2;
	while(*s){
		ascii_write_char(*s++);
	}
}


//MAIN ---------------------------------------------------------------------------------------------------------------

int main(void)
{
	char c;	
	char s;
	unsigned char points_player1 = 0x30;
	unsigned char points_player2 = 0x30;
	POBJECT p = &ball;
	POBJECT pdl_r = &paddle_right;
	POBJECT pdl_l = &paddle_left;
	init_app();
	ascii_init();
	graphic_initalize();
	graphic_clear_screen();
	display_score(points_player1, points_player2);
	while(1)
	{
		p->move(p);
		pdl_r->move(pdl_r);
		pdl_l->move(pdl_l);
		delay_milli(10);
		int pdl_r_posx = pdl_r->posx;
		int pdl_r_posy = pdl_r->posy;
		
		int pdl_l_posx = pdl_l->posx;
		int pdl_l_posy = pdl_l->posy;
		
		int ball_posx = p->posx;
		int ball_posy = p->posy;
		
		
		//COLISON with RIGHT paddle
		if(pdl_r_posx < ball_posx+4 && ball_posx < pdl_r_posx+5)
		{
			if(pdl_r_posy < ball_posy+4 && ball_posy < pdl_r_posy+9)
			{
				p->set_speed(p, -p->dirx, p->diry);
				//p->move(p);
			}
		}
		
		//COLISON with LEFT paddle
		if(pdl_l_posx < ball_posx+4 && ball_posx < pdl_l_posx+5)
		{
			if(pdl_l_posy < ball_posy+4 && ball_posy < pdl_l_posy+9)
			{
				p->set_speed(p, -p->dirx, p->diry);
				//p->move(p);
			}
		}
		
		
		//ball out of bounce--------
		if(ball_posx < 0)
		{
			p->posx = 64; 
			p->posy = 32; 
			p->dirx = 4; 
			p->diry = 1;
			points_player2 += 1;
			ascii_gotoxy(17, 1);
			ascii_write_char(points_player2);
		}
		
		if(ball_posx+4 > 128)
		{
			p->posx = 64; 
			p->posy = 32; 
			p->dirx = 4; 
			p->diry = 1;
			points_player1 += 1;
			ascii_gotoxy(17, 2);
			ascii_write_char(points_player1);
		}
		
		if(points_player2 >= 0x35 || points_player1 >= 0x35)
		{
			break;
		}
		
		s=keyb_l();
		c=keyb_r();
		switch(c)
		{
			case 3: pdl_r->set_speed(pdl_r, 0, -2); break;
			case 9: pdl_r->set_speed(pdl_r, 0, 2); break;
			default: pdl_r->set_speed(pdl_r, 0, 0); break;
		}
		
		switch(s)
		{
			case 1: pdl_l->set_speed(pdl_l, 0, -2); break;
			case 7: pdl_l->set_speed(pdl_l, 0, 2); break;
			default: pdl_l->set_speed(pdl_l, 0, 0); break;
		}
	}
	graphic_clear_screen();
	char* ggwp;
	char gg[] = "game over!";
	ggwp = gg;
	ascii_gotoxy(1, 1);
	while(*ggwp){
		ascii_write_char(*ggwp++);
	}
}

