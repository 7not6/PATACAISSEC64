#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include <c64/rasterirq.h>
#include <c64/sid.h>
#include <c64/vic.h>
#include <c64/sprites.h>
#include <c64/joystick.h>
#include <c64/memmap.h>
#include <c64/keyboard.h>
#include <c64/cia.h>
#include <conio.h>

#include <oscar.h>

#define NBLEVELS 40

#include "levels.h"

char customchar[] = {
	#embed  2048 lzo "./charset.bin"
};

const char digisound[] = {
  #embed "tutut.raw"
};

#define SHAPE(a) ((unsigned)&(Sprites[0]) +(a) *64)/64

#define charplot(x,y,a,b) Screen[(x)+40*(y)]=a;Color[(x)+40*(y)]= b;
#define cls(ch,cl) {memset(Screen,ch, 1000);memset(Color, cl, 1000);}

#define GRID_SIZE 6
#define DECY 50
#define DECX 24
#define xy2num( x, y) ((y)*NBC)+(x)
#define xdec 3
#define ydec 3

#define TEMPXY 4
#define CHARBAR 19
		
#define COLBACK 6//15
#define COLFRNT 0

#define TILE_EMPTY 18
#define TILE_FOND 18
#define TILEMULT 3
#define BLANK 32

#define STARTX 15
#define CENTRED 1

#define COLOR_FADE_LENGTH 16
#define VERTICAL_START 8
#define NUM_LINES 8
#define LINE_START 3    
#define LINE_END 36     
	
#define MAX_LINE_LENGTH 40
static char drawTextBuffer[MAX_LINE_LENGTH];

enum sprite_fig { CROSS, MOVEH, MOVEV, CAR1, CAR2, CAR3, CAR4, BGBLACK1, BGBLACK2, BLACK16X16};

char spdata[] = {	
// cross
0x01,0x80,0x00,0x01,0x80,0x00,0x01,0x80,
0x00,0x01,0x80,0x00,0x01,0x80,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xf8,0x1f,0x00,
0xf8,0x1f,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x01,0x80,0x00,0x01,0x80,0x00,0x01,
0x80,0x00,0x01,0x80,0x00,0x01,0x80,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,

// Horz >

 0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x20,
 0x00,0x0c,0x30,0x00,0x1c,0x38,0x00,0x3c,
 0x3c,0x00,0x7c,0x3e,0x00,0xfc,0x3f,0x00,
 0xfc,0x3f,0x00,0x7c,0x3e,0x00,0x3c,0x3c,
 0x00,0x1c,0x38,0x00,0x0c,0x30,0x00,0x04,
 0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,

// Vert ^

 0x01,0x80,0x00,0x03,0xc0,0x00,0x07,0xe0,
 0x00,0x0f,0xf0,0x00,0x1f,0xf8,0x00,0x3f,
 0xfc,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x3f,0xfc,
 0x00,0x1f,0xf8,0x00,0x0f,0xf0,0x00,0x07,
 0xe0,0x00,0x03,0xc0,0x00,0x01,0x80,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
 
 // red car and black bg
//spr1
0x3f, 0xff, 0x4a, 0x2a, 0xac, 0x00,	0x00, 0x10, 0x05, 0x2a, 0xa0, 0x00,
0x1f, 0xe0, 0x00, 0x3f, 0xc4, 0x00, 0x1f, 0xc8, 0x00, 0x3f, 0x82, 0x00,	
0x03, 0xc4, 0x00, 0x69, 0xca, 0x00, 0x75, 0x84, 0x00, 0xf4, 0xca, 0x00,
0xf4, 0x84, 0x00, 0x75, 0xca, 0x00,	0x69, 0x84, 0x00, 0x03, 0xca, 0x00,	
0x3f, 0x84, 0x00, 0x1f, 0xca, 0x00,	0x3f, 0xc4, 0x00, 0x1f, 0x0e, 0x00,
0x2a, 0xa0, 0x00, 0x00, 	
 //spr2	
0xad, 0xff, 0xfc, 0x00, 0xff, 0xf9, 0x42, 0x7f, 0xfa, 0x08, 0x7f, 0xf9,
0x16, 0xff, 0xfc, 0x2e, 0x60, 0x00, 0x1e, 0xd5, 0x54, 0x2e, 0x5f, 0xfe,
0x1e, 0xdf, 0xfc, 0x2e, 0x5e, 0x0e, 0x1e, 0xdc, 0xa4, 0x2e, 0x59, 0xf6,
0x2e, 0x59, 0xf6, 0x1e, 0xdc, 0xa4, 0x2e, 0x5e, 0x0e, 0x1e, 0xdf, 0xfc,
0x2e, 0x5f, 0xfe, 0x1e, 0xd5, 0x54, 0x2e, 0x60, 0x00, 0x16, 0xff, 0xfc,
0x08, 0x7f, 0xf9, 0x00, 
//spr3
0x00, 0x10, 0x05, 0x2a, 0xac, 0x00, 0x3f, 0xff, 0x4a, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 
//spr4
0x42, 0x7f, 0xfa, 0x00, 0xff, 0xf9, 0xad, 0xff, 0xfc, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 
// sprblack 
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,
 
// sprblackdown
 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
 0xff,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 
 // sprite black 16*16

 0xff,0xff,0x00,0xff,0xff,0x00,0xff,0xff,
 0x00,0xff,0xff,0x00,0xff,0xff,0x00,0xff,
 0xff,0x00,0xff,0xff,0x00,0xff,0xff,0x00,
 0xff,0xff,0x00,0xff,0xff,0x00,0xff,0xff,
 0x00,0xff,0xff,0x00,0xff,0xff,0x00,0xff,
 0xff,0x00,0xff,0xff,0x00,0xff,0xff,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	
};

const char bar[]={0x3c,0x3c,0x24,0x24,0x24,0x24,0x3c,0x3c};

static byte framecount; 
char oldjoy[5]={0,0,0,0,0};

static char * const Screen = (char *)0xc000;
static char * const Sprites = (char *)0xd000;
static char * const Charset = (char *)0xc800;
static char * const Color = (char *)0xd800;
static char * const Music = (char *)0xa000;

static char StatusText[40];

signed char grid[GRID_SIZE*GRID_SIZE];
signed char selected_pos_x = -1, selected_pos_y = -1;
signed char prev_selected_pos_x = -1, prev_selected_pos_y = -1;
signed char selected = -1;
signed char JOYSTICK = -1;
signed char level = 0;
signed char KX,KY,KB;
	
unsigned char colgird[GRID_SIZE*GRID_SIZE];
unsigned char cursor_x = 0, cursor_y = 0;
unsigned char move_count = 0;
unsigned char game_won = 0;
unsigned char sprx;
unsigned char delayY,delayX;
unsigned char cmpt,lvlcol;
unsigned char RESTART =0;
	
uint8_t COLOR_FADE_1[COLOR_FADE_LENGTH] = { 0, 0, 6, 6, 3, 3, 1, 1, 1, 1, 1, 1, 3, 3, 6, 6 };
uint8_t COLOR_FADE_POS = 0;

int framecnt=0;
int x=0;

const char * Text =
"	                                     "
"PATACAISSE 64 IS A RUSHOUR LIKE GAME BAS"
"ED ON PATACAISSE VG5000 BY BDCIRON GFX D"
"ESIGN:BDCIRON (ADAPTED FOR C64) - TESTIN"
"G:HEAD-OVER-HEELS - CODE & SFX:7NOT6 -  "
"HAVE A NICE DAY!                        "
"                 ";
	 
RIRQCode scroll, restore;

#pragma stacksize(512)
#pragma heapsize(0)

__export const char music[] = {
	#embed 2336 0x7e "FLIP.SID" 
};

void music_init(char tune)
{
	__asm
	{
		lda		tune
		jsr		$a000
	}
}

void music_play(void)
{
	__asm
	{
		jsr		$a006
	}
}

void music_volume(char volume)
{
	*(volatile char *)0xd418= volume;
}

void music_silence(void)
{
	sid.voices[0].ctrl = 0;
	sid.voices[0].susrel = 0;
	sid.voices[1].ctrl = 0;
	sid.voices[1].susrel = 0;
	sid.voices[2].ctrl = 0;
	sid.voices[2].susrel = 0;
}

int load_level( int level){
	int pos= level*36;
	for(int i=pos;i<pos+36;i++)grid[i-pos]=levels[i];
	return 0;
}

void  detect_pieces_transform(){

	char v;
	signed char temp;
	
	for(int i=0;i<GRID_SIZE*GRID_SIZE;i++){

		if(grid[i]!=0){
			
				temp= grid[i];
				v = (temp<0)?-temp:temp;					
				colgird[i]=v/10;
				v-=colgird[i]*10;
				grid[i]=(temp<0)?-v:v;	
				
				if(colgird[i]==0)colgird[i]=2;
				else if(colgird[i]==2)colgird[i]=14;	
				else if(colgird[i]==3)colgird[i]=15;	
		}		
	}

}

int check_victory() {

	if (grid[2*GRID_SIZE + 5] == -1)return 1;
	 
	return 0;
}

void find_piece(){
      
     int  val = grid[cursor_y*GRID_SIZE + cursor_x];             
    
	 selected_pos_x = -1;
	 selected_pos_y = -1;	
			
	 if(val>0){
		 
		 	selected_pos_x = cursor_x;
			selected_pos_y = cursor_y;	
			return;
	 }
	 
     switch(val){
       
       	case -1:
     	case -2:
		
			selected_pos_x = cursor_x-1;
			selected_pos_y = cursor_y;
			
     		break;
     		
       	case -3:
		
			selected_pos_y = cursor_y;
			if( grid[cursor_y*GRID_SIZE + cursor_x-1]==3)selected_pos_x = cursor_x-1;
			else 	selected_pos_x = cursor_x-2;		
			
     		break;
     		
        case -4:
			
			selected_pos_x = cursor_x;
			selected_pos_y = cursor_y-1;
		
     		break;
     		
        case -5:
		
     		selected_pos_x = cursor_x;
			if( grid[(cursor_y-1)*GRID_SIZE + cursor_x]==5)selected_pos_y = cursor_y-1;
			else 	selected_pos_y = cursor_y-2;		
		
     		break;    		
     		
     	default:
     		break;     		
     }   
                        
}

void move_selected(signed char dx, signed char dy){

    char vertical=0;
    char size=2;
        
    char pos=selected_pos_x + selected_pos_y*GRID_SIZE;
	signed char pselect = grid[pos];
    char backcol=colgird[pos];

	char temp,postemp;
	
    if(dy!=0 && pselect<4)return; // piece horizontale
    if(dx!=0 && pselect>3)return; // piece verticale
	
    switch(pselect){    
    
        case 1: // Player horizontal, size=2  
        case 2: // voiture horizontal, size=2                
                break;
        
        case 3: // voiture horizontal, size=3               
                size=3;                
                break;
                
        case 4: // voiture vertical, size=2                
                vertical=1;                
                break;
                
        case 5: // voiture vertical, size=3                
                vertical=1;
                size=3;                
                break;    
                
        default:        
                break;                    
    }
	
	if(vertical){
	
		//check if posy + size == 0
		char deb = selected_pos_y + dy;
		char fin = selected_pos_y + dy + size -1;
		
		if( deb>5 || fin>5 || deb<0 || fin < 0 )return;
	
		char posdeb = deb*GRID_SIZE + selected_pos_x;
		char posfin = fin*GRID_SIZE + selected_pos_x;
		
		if(dy<0) postemp= posdeb;
		else  postemp= posfin;
		
		if( grid[postemp]!=0)return;
		
		postemp= posdeb;
		
		for(int i=0;i<size;i++){
			grid[pos+i*GRID_SIZE]=0;
			colgird[pos+i*GRID_SIZE]=0;	
		}
		
		for(int i=0;i<size;i++){
            	
         	if(i==0)grid[postemp+i*GRID_SIZE]=pselect;
            else grid[postemp+i*GRID_SIZE]=-pselect;
			colgird[postemp+i*GRID_SIZE]=backcol;		
        } 
        
		selected_pos_y += dy;  	
	}
	
	else {

		//check if posy + size == 0
		char deb = selected_pos_x + dx;
		char fin = selected_pos_x + dx + size -1;
		
		if( deb>5 || fin>5 || deb<0 || fin < 0 )return;
	
		char posdeb = selected_pos_y*GRID_SIZE + deb;
		char posfin = selected_pos_y*GRID_SIZE + fin;

		if(dx<0) postemp= posdeb;
		else  postemp= posfin;
		
		if( grid[postemp]!=0)return;
		
		postemp= posdeb;
		
        for(int i=0;i<size;i++){
			grid[pos+i]=0;
			colgird[pos+i]=0;		
		}
			
        for(int i=0;i<size;i++){
				
        	if(i==0)grid[postemp+i]=pselect;
            else grid[postemp+i]=-pselect;
			colgird[postemp+i]=backcol;
        } 
        
		selected_pos_x += dx;  		
	}		
}

void init_game(char lvl){
	
	load_level(lvl);
    detect_pieces_transform();	
    game_won = 0;
    move_count = 0;
    selected = -1;
	level = lvl;
}

void tileplot(unsigned char x,unsigned char y ,unsigned char n,unsigned char w,unsigned char h,char col){
	
	unsigned char tot,xx,yy;
	
	tot=n;
	
	for(unsigned char j=0;j<h;j++)	
		for(unsigned char i=0;i<w;i++){
			xx=x+i;
			yy=y+j;
			Screen[xx+40*yy]=tot;
			Color[xx+40*yy]= col;	
			tot++;		
		}		
}

void affiche_pieces(){

		signed char e;
		char ind,col;

		char cnt=0;
		
		for(int j=ydec;j<(GRID_SIZE*TILEMULT)+ydec;j+=TILEMULT){	
			for(int i=xdec;i<(GRID_SIZE*TILEMULT)+xdec;i+=TILEMULT){
					
				ind = ((j-ydec)/TILEMULT)*GRID_SIZE + (i-xdec)/TILEMULT;
				
				col= COLBACK;
			
				e=grid[ind];
				
				if(e>0){				
						
				col= colgird[ind];
				
				switch(e){				 
				 
				 	case 1:
				 	
				 		tileplot(i,j ,180,3,3,col);
				 		tileplot(i+3,j ,180+9,3,3,col);
				 			
				 		break;	 	
				 				
					case 2:
						
				 		tileplot(i,j ,216,3,3,col);
				 		tileplot(i+3,j ,216+9,3,3,col);
				 		
				 		break;	 			 
				 	
				 	case 3:
				 	
				 		if(cnt%2==0){
				 		
				 		tileplot(i,j ,153,3,3,col);
				 		tileplot(i+3,j ,153+9,3,3,col);				
				 		tileplot(i+6,j ,153+18,3,3,col);
				 		
				 		}
				 		else {
				 						 		
				 		tileplot(i,j ,126,3,3,col);
				 		tileplot(i+3,j ,126+9,3,3,col);				
				 		tileplot(i+6,j ,126+18,3,3,col);
				 		
				 		}
				 		
				 		cnt++;
				 	
				 		break;
				 					 		
				 	case 4:				 	
						
						tileplot(i,j ,198,3,3,col);
				 		tileplot(i,j+3 ,198+9,3,3,col);
				 		
				 		break;				 		
				 	
				 	case 5:				 		
						tileplot(i,j ,99,3,3,5/*col*/);
				 		tileplot(i,j+3 ,99+9,3,3,1);
				 		tileplot(i,j+6 ,99+18,3,3,1);
				 	
				 		break;
				 						 	
				 }
			
				}
				else if(e==0) {

				Color [(i+0)+(j+0)*40]=col;	
				Screen[(i+0)+(j+0)*40]=TILE_FOND;
			
				Color [(i+0)+(j+1)*40]=col;	
				Screen[(i+0)+(j+1)*40]=TILE_FOND;
				
				Color [(i+0)+(j+2)*40]=col;	
				Screen[(i+0)+(j+2)*40]=TILE_FOND;
							
				Color [(i+1)+(j+0)*40]=col;	
				Screen[(i+1)+(j+0)*40]=TILE_FOND;		
				
			    Color [(i+1)+(j+1)*40]=col;	
				Screen[(i+1)+(j+1)*40]=TILE_FOND;
			
				Color [(i+1)+(j+2)*40]=col;	
				Screen[(i+1)+(j+2)*40]=TILE_FOND;				
				
				Color [(i+2)+(j+0)*40]=col;	
				Screen[(i+2)+(j+0)*40]=TILE_FOND;
			
				Color [(i+2)+(j+1)*40]=col;	
				Screen[(i+2)+(j+1)*40]=TILE_FOND;		
				
				Color [(i+2)+(j+2)*40]=col;	
				Screen[(i+2)+(j+2)*40]=TILE_FOND;	
				
				}				
			
			}
		}
		
}

int Anim_FinLevel(){

	int bx1=24+8*STARTX;
	int bx2=24+8*(STARTX+3);
	int by1=50+8*9;
	int by2=50+8*11+5;
	
	if(sprx>200)
	{		
		for(int i=0;i<8;i++)spr_show(i,0 );
		return 1;

	}	
	else {		

		spr_move(0,bx1+sprx,by1);
		spr_move(1,bx2+sprx,by1);
		spr_move(2,bx1+sprx,by2);
		spr_move(3,bx2+sprx,by2);
		spr_move(4,bx1+sprx,by1);
		spr_move(5,bx2+sprx,by1);
		spr_move(6,bx1+sprx,by2);
		spr_move(7,bx2+sprx,by2);
		
		sprx+=2;	
	
	}
	
	return 0;	
}

void printxy(int x,char y,char flag,char col,char *format, ...){

	char len,i,c;
	va_list args;
	
	if(col>15)c=1;
	else c=col;
	
	memset(&drawTextBuffer, '\0', sizeof(drawTextBuffer));

	va_start(args, format);
	vsprintf(drawTextBuffer, format, args);
	va_end(args);
	
	len = strlen(drawTextBuffer);
	
	if(flag==CENTRED){
		x= 20-len/2 - x;
	}

	for( i=0; i<len; i++){
	 		Screen[i+x + 40*y] = drawTextBuffer[i];
	 		Color[i+x  + 40*y] = c;
	 		if(col>15){
	 			c++;
	 			if(c>15)c=1;
	 		}
	}	 	
}

void transition(void){

	char end=0;
	int i,j;
	
	i=3;j=3;
	
	while(!end)
	{	
			
		if(end==0){
			
			tileplot(i,j ,9,3,3,3);
			i+=3;
			
			if(i>=21){
				i=0;
				j+=3;
				if(j>=21){
					j=0;
					end=1;
					
				}
			}
			
		}
		
		vic_waitFrame();
	}
	
	oldjoy[4]=0;	
	
	memset(Screen,BLANK, 1000);
 	memset(Color, 0, 1000);
	
}

void transition2(char startx,char starty,char endx,char endy,char taillex,char tailley,char tile ,char color){

	char end=0;
	int i,j;
	
	i=startx*taillex;
	j=starty*tailley;
	
	while(!end)
	{	
			
		if(end==0){
			
			tileplot(i,j ,tile,taillex,tailley,color);
			i+=taillex;
			
			if(i>=taillex*endx){
				i=startx*taillex;
				j+=tailley;
				if(j>=tailley*endy){
					j=starty*tailley;
					end=1;
					
				}
			}
			
		}
		
		vic_waitFrame();
	}
	
	oldjoy[4]=0;	
	
	memset(Screen,BLANK, 1000);
 	memset(Color, 0, 1000);
	
}

void transition3(char startx,char starty,char w,char h,char tile,char col){
	
	int i,j;

	for(i=startx;i<startx+w;i++){					
			charplot(i,starty,tile,col) ;
	}
	
	for(j=1;j<h/2;j++){
		for(i=startx;i<startx+w;i++){		
			
			charplot(i,starty+j,tile,col) ;
			charplot(i,starty-j,tile,col) ;					
		}		
		
		vic_waitFrame();
	}
}

void drawctrl(){

	int i,j;
 	
 	for(int j=0;j<24;j+=3){	
		for(int i=0;i<39;i+=3){	
			if(i/3<1 || i/3>11 || j/3<1 || j/3>6)tileplot(i,j ,9,3,3,3);				
		}
	}
	
	printxy(1,4,CENTRED,16,(char *)"PATACAISSE C64 V0.%d",7);

	printxy(1,VERTICAL_START +0,CENTRED,16,(char *)"******************************");
	printxy(1,VERTICAL_START +1,CENTRED,16,(char *)"*      KEYBOARD CONTROL      *");
	printxy(1,VERTICAL_START +2,CENTRED,16,(char *)"*                            *");
	printxy(1,VERTICAL_START +3,CENTRED,16,(char *)"*              T             *");
	printxy(1,VERTICAL_START +4,CENTRED,16,(char *)"* MOVE        FGH   SPC      *");
	printxy(1,VERTICAL_START +5,CENTRED,16,(char *)"* RESET        R             *");
	printxy(1,VERTICAL_START +6,CENTRED,16,(char *)"* LEVEL DOWN  +/-            *");
	printxy(1,VERTICAL_START +7,CENTRED,16,(char *)"* KDB/JOY      J             *");	
	printxy(1,VERTICAL_START +8,CENTRED,16,(char *)"*                            *");
	printxy(1,VERTICAL_START +9,CENTRED,16,(char *)"******************************");	
	
	for(int i=0;i<39;i++)Color[i + 40*24] = 1;	
}

void drawintro(){

	int i,j;
	char col;	

	for(int j=0;j<24;j+=3){	
		for(int i=0;i<39;i+=3){	
			if(i/3<1 || i/3>11 || j/3<1 || j/3>6)tileplot(i,j ,9,3,3,3);				
		}
	}
	
	printxy(1,4,CENTRED,16,(char *)"PATACAISSE C64 V0.%d",7);
		
	// bus horizontal 126/135/144
	i=5*3;j=8;col=7;		
 	tileplot(i,j ,126,3,3,col);
	tileplot(i+3,j ,126+9,3,3,col);				
	tileplot(i+6,j ,126+18,3,3,col);
	
	// voit vertical 2 -234/243
	i=32;j=4*3;col=6;
	tileplot(i,j ,234,3,3,col);
 	tileplot(i,j+3 ,234+9,3,3,col);
 	
	//player rouge
	i=3*3;j=4*3;col=2;
 	tileplot(i,j ,180,3,3,col);
 	tileplot(i+3,j ,180+9,3,3,col);
	
	//voit horiz 2
	i=4*3;j=17;col=15;
	tileplot(i,j ,216,3,3,col);
 	tileplot(i+3,j ,216+9,3,3,col);
		
	//voit 3 Hor
	i=20;j=14;col=5;		
 	tileplot(i,j ,153,3,3,col);
	tileplot(i+3,j ,153+9,3,3,col);				
	tileplot(i+6,j ,153+18,3,3,col);

	//voit vertical 2 type b
	i=28;j=7;col=4;
	tileplot(i,j ,198,3,3,col);
 	tileplot(i,j+3 ,198+9,3,3,col);

	//camion H
	i=4;j=7;col=8;
	tileplot(i,j ,99,3,3,5/*col*/);
 	tileplot(i,j+3 ,99+9,3,3,1);
 	tileplot(i,j+6 ,99+18,3,3,1);
		
	printxy(30,19,0,1,(char *)"7NOT6");	
		
	for(int i=0;i<39;i++)Color[i + 40*24] = 1;
}

__interrupt void srcoll_music(void){

	music_play();
	
		   // Update raster IRQ for scroll line with new horizontal scroll offset		
		rirq_data(&scroll, 1, 7 - (x & 7));
		// Copy scrolled version of text when switching over char border
		if ((x & 7) == 0){
			
				char tmp =((x >> 3) & 255);
				if(tmp>215){
						char reste = 255-tmp;						
						memcpy((char *)Screen + 40 * 24, Text + tmp,reste);
						for(int i=reste;i<40-reste;i++)Screen[40 * 24 + i] = 0;						
				}
				else memcpy((char *)Screen + 40 * 24, Text + tmp,40);			
		}
		x++;
		
}

void intro(void){

	int i,j;
	char col=1;	
	
	framecnt=0;

 	music_init(0);
 	
	memset(Screen,BLANK, 1000);
 	memset(Color, 0, 1000);
 	
	drawintro();
	
	rirq_init(true);

	rirq_build(&scroll, 2);
	rirq_delay(&scroll, 11);
	rirq_write(&scroll, 1, &vic.ctrl2, 0);
	rirq_set(0, 49 + 24 * 8, &scroll);

	rirq_build(&restore, 2);
	rirq_write(&restore, 0, &vic.ctrl2, VIC_CTRL2_CSEL);
	rirq_call(&restore, 1, srcoll_music);
	rirq_set(1, 250, &restore);

	rirq_sort();

	rirq_start();

	int x=0;	
	int END=0;

	introscreen:
	
	for(;;)
	{	

	 	if(END==1){			

			 goto fin;
	 	}
	 		
	 	keyb_poll();	
		END=key_pressed(KSCAN_SPACE)?1:0;
		
		joy_poll(0);
				
		if(joyb[0] && oldjoy[4]==0 ){	
			END=1;					
			oldjoy[4]=1;
			JOYSTICK=1;
			break;
		}		
		
		if(joyb[0]==0)oldjoy[4]=0;			

		for(i=0; i<10; i++)	 		
	 		Color[i + 10 + 40*4] = ((col + i) % 15) + 1;
	  	 	
	 	COLOR_FADE_POS = (COLOR_FADE_POS + 1) & (COLOR_FADE_LENGTH - 1);
	 	uint8_t param1 = VERTICAL_START; 
	 	
	 	if(framecnt>=500){
		 
	 		for (int line = param1; line <= VERTICAL_START+9; ++line) { 
        		uint8_t fade_index = (COLOR_FADE_POS + param1 - VERTICAL_START) & (COLOR_FADE_LENGTH - 1); 
        	
        		for (uint8_t col = LINE_START; col < LINE_END; ++col) {
        	    	uint16_t color_offset = col + 40 * line;  
        	    	Color[color_offset] = COLOR_FADE_1[fade_index];
		        	fade_index = (fade_index + 1) & (COLOR_FADE_LENGTH - 1);
    	    	}
        		++param1;  
    		} 
	 	
	 	}

	 	framecnt++;
	 	 	
	 	if(framecnt==500){			
			transition3(3,13,33,16,0,0);
	 		drawctrl();				
	 	}
	 	else if(framecnt==1000){
	 		transition3(3,13,33,16,0,0);			
	 		drawintro();			
	 		framecnt=0; 		 	
	 	}
	 	
	 	if((framecnt&27) ==0 ){
	 		col = (col % 15) + 1; 	 		
	 	}	 	
	 	
		vic_waitFrame();
	 	
	}
		
	fin:
	
	music_volume(8);
	
	rirq_clear(0);
  	rirq_clear(1);	
	
	vic_waitBottom();
	
	oldjoy[4]=0;	
	music_volume(0);
	music_silence();	

	rirq_sort();
	
	music_volume(4);

	transition2(1,1,12,7,3,3,9,3);

	music_volume(0);	
	
	memset(Screen,BLANK, 1000);
 	memset(Color, 0, 1000);

}

void init_display()
{
	mmap_trampoline();

	// Disable interrupts while setting up
	__asm { sei };

	// Kill CIA interrupts
	cia_init();
	
	mmap_set(MMAP_RAM);	
	
    memcpy(Sprites, spdata,64*10);

	oscar_expand_lzo(Charset, customchar);
	
	memcpy(Music, music, 2206);

	mmap_set(MMAP_NO_ROM);
	
	vic_setmode(VICM_TEXT/*_MC*/, Screen, Charset);
	
	vic.color_border = 0;
	vic.color_back = 0;
	vic.color_back1 = 10;
	vic.color_back2 = 9;

    spr_init(Screen);
		
	// player sprite		
	spr_set(0, false, 24+8*24 , 50+8*9, SHAPE(3), 2, false, false, false); 
	spr_set(1, false, 24+8*27 , 50+8*9, SHAPE(4), 2, false, false, false); 	
	spr_set(2, false, 24+8*24 , 50+8*11+5, SHAPE(5), 2, false, false, false); 
	spr_set(3, false, 24+8*27 , 50+8*11+5, SHAPE(6), 2, false, false, false); 
	 		
	spr_set(4, false, 24+8*24 , 50+8*9, SHAPE(7), 0, false, false, false); 
	spr_set(5, false, 24+8*27 , 50+8*9, SHAPE(7), 0, false, false, false); 	
	spr_set(6, false, 24+8*24 , 50+8*11+5, SHAPE(8), 0, false, false, false);		
	spr_set(7, false, 24+8*27 , 50+8*11+5, SHAPE(8), 0, false, false, false); 

	// add bar to charset place 19
	for (char i = 0; i < 8; i++){	
		Charset[(CHARBAR<<3) + i]=bar[i];
	}
	
}

void start_game(char lvl)
{
	// cursor sprite
 	spr_color(0,1);
 	spr_image(0, SHAPE(0));	
 	spr_show(0, 1);
 	
 	spr_color(1,0);
 	spr_image(1, SHAPE(9));	
 	spr_show(1, 1);
 	
 	for(int i=2;i<8;i++)spr_show(i,0);
 		
 	cursor_x = 3;
 	cursor_y = 2;
 	
 	sprx= 0;
 	cmpt=0;
	
 	memset(Screen,BLANK, 1000);
 	memset(Color, 0, 1000);
	
	for(int j=0;j<24;j+=3){	
		for(int i=0;i<24;i+=3){	
			if(i/3<1 || i/3>6 || j/3<1 || j/3>6)tileplot(i,j ,9,3,3,3);				
		}
	}
	
	tileplot(8*3,2*3 ,9,3,3,3);	
	tileplot(9*3,2*3 ,9,3,3,3);
	tileplot(8*3,4*3 ,9,3,3,3);	
	tileplot(9*3,4*3 ,9,3,3,3);	
	
	for(int i=21;i<24+6;i++){
		charplot(i,9,20,7);
		charplot(i,11,21,7);
		charplot(i,10,0,0);
	}

	charplot(21,9,19,1);
	charplot(21,10,19,1);
	charplot(21,11,19,1);
	
	framecount=0;
	
	delayY = 0;
	delayX = 0;
		
    init_game(lvl);
    
    RESTART=0;

}

void affiche_status(){
		
		sprintf(StatusText,"LEVEL :%2d",level+1);	
		
		for(char ii=0; ii<9; ii++){
	 		Screen[ii+27 + 40] = StatusText[ii];
	 		Color[ii+27 + 40] = 1;
	 	}

		sprintf(StatusText,"MOVES :%3d",move_count);		
		
		for(char ii=0; ii<11; ii++){
	 		Screen[ii+27 + 3*40] = StatusText[ii];
	 		Color[ii+27 + 3*40] = 1;
	 	}

		if(JOYSTICK>0){
			Screen[25 + 3*40] = 2;
	 		Color[25 + 3*40] = 1;
		}
		else {
			Screen[25 + 3*40] = 3;
	 		Color[25 + 3*40] = 1;			
		}
	 	 	
	 	if(level>=0 && level<10){
	 		sprintf(StatusText,"BEGINNER");	
	 		lvlcol=VCOL_GREEN;
	 	}	
		else if(level>=10 && level<20){
			sprintf(StatusText,"INTERMED");
			lvlcol=VCOL_ORANGE;		
		}
		else if(level>=20 && level<30){
			sprintf(StatusText,"ADVANCED");
			lvlcol=VCOL_BLUE;		
		}
		else if(level>=30 && level<40){
			sprintf(StatusText,"EXPERT  ");
			lvlcol=VCOL_RED;		
		}
		
		for(char ii=0; ii<9; ii++){
	 		Screen[ii+27 + 18*40] = StatusText[ii];
	 		Color[ii+27 + 18*40] = lvlcol;
	 	}
	 	
}	 	

void draw_screen(){
	char col=1;	
	
	if(game_won==0)affiche_pieces();

	affiche_status();
	 	 
	if(game_won==1){
			
		if(Anim_FinLevel()){
			
			transition();
			RESTART=1;        	
				
		}
	 		
	 	if(cmpt==1)sprintf(StatusText,"LEVEL CLEAR");
	 	else {
	 		col=0;
	 		sprintf(StatusText,"           ");
	 	}	
		
		for(char ii=0; ii<11; ii++){
	 		Screen[ii+27 + 21*40] = StatusText[ii];
	 		Color[ii+27 + 21*40] = col;
		}		 			
			
	}	
}

void poll_input(){

		joy_poll(0);
		
		keyb_poll();
		
		KX=0;
		KY=0;
		KB=0;
		
		switch (keyb_key)
		{		
			case KSCAN_R + KSCAN_QUAL_DOWN :
				RESTART=1;   
				break;

			case KSCAN_PLUS + KSCAN_QUAL_DOWN :
			
			    level++;               
                if(level>NBLEVELS-1)level=0;
				
				RESTART=1;  
				break;
				
			case KSCAN_MINUS + KSCAN_QUAL_DOWN :
			
			    level--;               
                if(level<0)level=NBLEVELS-1;
                RESTART=1;				  
				break;			
			
			case KSCAN_J + KSCAN_QUAL_DOWN :
				 JOYSTICK=-JOYSTICK;
				break;	
					
		    case KSCAN_F + KSCAN_QUAL_DOWN :
				 KX=-1;
				break;	
						
			case KSCAN_H + KSCAN_QUAL_DOWN :
				 KX=1;
				break;	
				
			case KSCAN_T + KSCAN_QUAL_DOWN :
				 KY=-1;
				break;	
						
			case KSCAN_G + KSCAN_QUAL_DOWN :
				 KY=1;
				break;	
				
			case KSCAN_SPACE + KSCAN_QUAL_DOWN :
				 KB=1;
				break;							
		}				
	
}

void digi_setup(void)
{
	sid.voices[0].susrel=0xff;
	sid.voices[1].susrel=0xff;
	sid.voices[2].susrel=0xff;
	sid.voices[0].ctrl=0x49;
	sid.voices[1].ctrl=0x49;
	sid.voices[2].ctrl=0x49;
}

void cia2_wait()
{
	word timer = cia2.ta;
	while (cia2.ta < timer) ;
}

void digi_play(const char* snd, unsigned int len)
{

 	unsigned int i; 

  	cia2.cra = 0x00;
  	cia2.ta =163;  
  	cia2.cra = 0x11;

	for (i = 0; i < len; i++) {

	    cia2_wait();
	    sid.fmodevol = (snd[i] >> 4);
	
	    cia2_wait();
	    sid.fmodevol = (snd[i] & 15);
	}
	  
	cia2.cra = 0x00;  

}

void gameloop()
{
	char pdecx,pdecy;	

 	level=0;
 	lvlcol=0;
 	
 	digi_setup();
 	
 	start:
 	
	start_game(level);
		
	for(;;)
	{	

		poll_input();
		
		if(RESTART==1)goto start;

		if (selected == -1){
		
		if(JOYSTICK>0){
		
			if( joyx[0]<0  && oldjoy[0]==0 ){
				if(cursor_x > 0) cursor_x--;
				oldjoy[0]=1;
				delayX=0;
			}
		
			if( joyx[0]>0 && oldjoy[1]==0){
				if(cursor_x < GRID_SIZE-1) cursor_x++;
				oldjoy[1]=1;
				delayX=0;
			}

			if( joyy[0]<0  && oldjoy[2]==0){			
				if(cursor_y > 0) cursor_y--;
				oldjoy[2]=1;
				delayY=0;
			}
			
			if( joyy[0]>0 && oldjoy[3]==0){
				if(cursor_y < GRID_SIZE-1) cursor_y++;				
				oldjoy[3]=1;
				delayY=0;
			}
			
		 }
		 else {
		 
		 	if(  KX<0 && oldjoy[0]==0 ){
				if(cursor_x > 0) cursor_x--;
				oldjoy[0]=1;
				delayX=0;
			}
		
			if( KX>0 && oldjoy[1]==0){
				if(cursor_x < GRID_SIZE-1) cursor_x++;
				oldjoy[1]=1;
				delayX=0;
			}

			if( KY<0 && oldjoy[2]==0){			
				if(cursor_y > 0) cursor_y--;
				oldjoy[2]=1;
				delayY=0;
			}
			
			if( KY>0 && oldjoy[3]==0){
				if(cursor_y < GRID_SIZE-1) cursor_y++;				
				oldjoy[3]=1;
				delayY=0;
			}
		 
		 }

		}
		else {
		
		if(JOYSTICK>0){
		
			if( joyx[0]<0&& oldjoy[0]==0 ){
				move_selected( -1, 0);
				oldjoy[0]=1;
				delayX=0;
			}
		
			if( joyx[0]>0 && oldjoy[1]==0){
				move_selected( 1, 0);
				oldjoy[1]=1;
				delayX=0;
			}

			if( joyy[0]<0  && oldjoy[2]==0){			
				move_selected( 0, -1);
				oldjoy[2]=1;
				delayY=0;
			}
			
			if( joyy[0]>0 && oldjoy[3]==0){
				move_selected( 0, 1);   				
				oldjoy[3]=1;
				delayY=0;
			}
		}
		else {
				if( KX<0 && oldjoy[0]==0 ){
				move_selected( -1, 0);
				oldjoy[0]=1;
				delayX=0;
			}
		
			if( KX>0 && oldjoy[1]==0){
				move_selected( 1, 0);
				oldjoy[1]=1;
				delayX=0;
			}

			if( KY<0 && oldjoy[2]==0){			
				move_selected( 0, -1);
				oldjoy[2]=1;
				delayY=0;
			}
			
			if( KY>0 && oldjoy[3]==0){
				move_selected( 0, 1);   				
				oldjoy[3]=1;
				delayY=0;
			}		
		}	

		}
		
		if(JOYSTICK>0){
		
			if( joyy[0]==0 && ( oldjoy[2]==1 || oldjoy[3]==1 ) ){
				oldjoy[2]=0;
				oldjoy[3]=0;
				delayY=0;
			}	
			
			if( joyx[0]==0 && ( oldjoy[0]==1 || oldjoy[1]==1 )){
				oldjoy[0]=0;
				oldjoy[1]=0;
				delayX=0;
			}
		
		}
		else {
		
			if( KY==0 && ( oldjoy[2]==1 || oldjoy[3]==1 ) ){
				oldjoy[2]=0;
				oldjoy[3]=0;
				delayY=0;
			}	
			
			if( KX==0 && ( oldjoy[0]==1 || oldjoy[1]==1 )){
				oldjoy[0]=0;
				oldjoy[1]=0;
				delayX=0;
			}
					
		}
			
		if(delayY==TEMPXY){	

			oldjoy[2]=0;
			oldjoy[3]=0;			
			delayY=0;			
		}
		if(delayX==TEMPXY){	
		
			oldjoy[0]=0;
			oldjoy[1]=0;			
			delayX=0;			
		}

		if((joyb[0] || KB==1 )&& oldjoy[4]==0 ){	
								
			oldjoy[4]=1;
		
			if(game_won==1){   
				for(int i=0;i<8;i++)spr_show(i,0 );
				transition();
           		goto start;          
            }
            
			if (selected == -1 && game_won==0) {
						
              	find_piece();
				selected=1;
						
				if(selected_pos_x == -1 || selected_pos_y == -1)selected=-1;	
				else {
						prev_selected_pos_x = selected_pos_x;
						prev_selected_pos_y = selected_pos_y;
						
						if( grid[selected_pos_y*GRID_SIZE + selected_pos_x]<4)
							spr_image(0, SHAPE(1));
						else 
							spr_image(0, SHAPE(2));
				}						
                       
            }
            else if (selected != -1 && game_won==0)  {                                          
                      				   
				if( prev_selected_pos_x != selected_pos_x || prev_selected_pos_y != selected_pos_y) move_count++;
						
                selected = -1;
				
				spr_image(0, SHAPE(0));
				
				cursor_x = selected_pos_x;
				cursor_y = selected_pos_y;
						
            }	 
				
		}
		
		if(JOYSTICK>0){ 
			if( joyb[0]==0 )oldjoy[4]=0;	
		}
		else {
			if( KB==0 )oldjoy[4]=0;	
		}

		if(selected==1 && game_won==0 ){	
				
			char e=grid[selected_pos_y*GRID_SIZE + selected_pos_x];		
			
			pdecx=24+8*xdec + 4 + selected_pos_x*24;
			pdecy=50+8*ydec + 4 + selected_pos_y*24;	
			
			if(e==1 || e==2)pdecx+=12;
			else if(e==3)pdecx+=24;	
			else if (e==4)pdecy+=12;
			else if (e==5)pdecy+=24;	
			
			if (check_victory()) {
			
				for(int i=0;i<8;i++)spr_show(i,0 );
			    affiche_pieces();
			    digi_play(digisound, sizeof(digisound));
			    charplot(21,9,20,7);
				charplot(21,10,0,0);
				charplot(21,11,21,7);
			        
			    for(int k=0;k<  GRID_SIZE*GRID_SIZE;k++)
			    {
			    	if(grid[k]==1 || grid[k]==-1)grid[k]=0;
			    }
			
                game_won = 1;
                affiche_pieces();
                
                sprx=0;
                
                level++;               
                if(level>NBLEVELS-1)level=0;
                
                move_count++;  
                selected=-1;
                
           		spr_color(0,2);
				spr_image(0, SHAPE(3));	
				
				spr_color(1,2);
				spr_image(1, SHAPE(4));	
				
				for(int i=0;i<8;i++)spr_show(i,1 );
							
				cursor_x = selected_pos_x;
				cursor_y = selected_pos_y;	
							
          	}         	
                				
		}
		else {
		
			pdecx=24+8*xdec + 4 + cursor_x*24;
			pdecy=50+8*ydec + 4 + cursor_y*24;						
		}
		
		if(game_won==0){
		
			spr_move(0,pdecx,pdecy);	
			
			if(selected==1){
				spr_show(1,1 );
				spr_move(1,pdecx,pdecy);				
			}
			else{
				spr_show(1,0 );
			}
		}

		draw_screen();
			
		if(framecount==0 ){			
			cmpt = 1 - cmpt;	
		}	

		vic_waitFrame();
		
		delayY++;
		delayX++;
		
		framecount++;
    	if(framecount>16)framecount=0;      	 
	
	}

}

int main(void)
{	
	init_display();
	 
 	intro();
 	
	gameloop();

	return 0;
}

