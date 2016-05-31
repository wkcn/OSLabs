#include <stdint.h>

#define WinCol 20
#define WinRow 12
#define UpdateTimes 60
#define NOHARMTIME 3
#define TotalBomb 20
#define PowerFlag 0x02

typedef uint8_t db;
typedef uint16_t dw;

typedef struct{
		dw graph;
		dw dir;
		dw pat;
		dw x, y;
		dw tx, ty;
		dw ani;
		dw v;
}Player;

typedef struct{
	dw graph;
	db used;
	db power;
	dw count;
	dw pat;
	dw x, y;
	dw tx, ty;
	dw ani;
	dw jump_count;
}Bomb;

typedef struct{
	db used;
	dw pat;
	dw count;
	dw x, y;
}Power;

extern Player Players[2]; 
extern uint8_t PASSED_DATA[WinRow][WinCol];
extern uint8_t STATE_DATA[WinRow][WinCol];
extern Bomb Bombs[TotalBomb];
extern db PlayerHP, BossHP; 
extern dw PlayerNOHARM, BossNOHARM;

uint16_t costMap[WinRow][WinCol];

__attribute__((regparm(2)))
uint8_t IsPassed(uint16_t x, uint16_t y){
	if (x >= WinCol)return 0;
	if (y >= WinRow)return 0;
	if (PASSED_DATA[y][x])return 0;
	if (STATE_DATA[y][x])return 0;
	return 1;
}

__attribute__((regparm(3)))
void PowerGo(Bomb *b, uint16_t ax, uint16_t ay){
	db power = b->power;
	uint16_t x = (b->x + 0x80) >> 8;
	uint16_t y = (b->y + 0x80) >> 8;
	for (db i = 0;i < power;++i){
		x += ax;
		y += ay;
		if (!IsPassed(x,y))break;
		costMap[y][x] += power;
		--power;
	}	
}

__attribute__((regparm(2)))
uint16_t GetCost(uint16_t x, uint16_t y){
	if (x >= WinCol)return 0;
	if (y >= WinRow)return 0;
	return costMap[y][x];
}

void Update(){
	if (PlayerNOHARM > 0)--PlayerNOHARM;
	else {
		Player *r = &Players[0];
		uint16_t x = (r->x + 0x80) >> 8;
		uint16_t y = (r->y + 0x80) >> 8;
		if (STATE_DATA[y][x] == PowerFlag){
			PlayerNOHARM = UpdateTimes * NOHARMTIME;
			if(PlayerHP > 0)--PlayerHP;
		}
	}
	if (BossNOHARM > 0)--BossNOHARM;
	else {
		Player *r = &Players[1];
		uint16_t x = (r->x + 0x80) >> 8;
		uint16_t y = (r->y + 0x80) >> 8;
		if (STATE_DATA[y][x] == PowerFlag){
			BossNOHARM = UpdateTimes * NOHARMTIME;
			if(BossHP > 0)--BossHP;
		}
	}
}

void AI(){
	Player *ai = &Players[1];
	if (ai->x != ai->tx || ai->y != ai->ty)return;
	for (int r = 0;r < WinRow;++r){
		for (int c = 0;c < WinCol;++c){
			costMap[r][c] = 0;
			if (STATE_DATA[r][c])costMap[r][c] += 5;
		}
	}
	for (int i = 0;i < TotalBomb;++i){
		if (!Bombs[i].used)continue;
		uint16_t x = (Bombs[i].x + 0x80) >> 8;
		uint16_t y = (Bombs[i].y + 0x80) >> 8;
		costMap[y][x] += Bombs[i].power;
		PowerGo(&Bombs[i],0,1);
		PowerGo(&Bombs[i],-1,0);
		PowerGo(&Bombs[i],1,0);
		PowerGo(&Bombs[i],0,-1);
	}
	uint16_t x = (ai->x + 0x80) >> 8;
	uint16_t y = (ai->y + 0x80) >> 8;
	uint16_t costF[4]; // down, left, right ,up
	costF[0] = GetCost(x-1,y+1) + GetCost(x,y+1) + GetCost(x+1,y+1);
	costF[3] = GetCost(x-1,y-1) + GetCost(x,y-1) + GetCost(x+1,y-1);
	costF[1] = GetCost(x-1,y-1) + GetCost(x-1,y) + GetCost(x-1,y+1);
	costF[2] = GetCost(x+1,y-1) + GetCost(x+1,y) + GetCost(x+1,y+1);

	uint8_t dir[4] = {0,1,2,3};
	//从小到大排列
	for (int i = 0;i < 4;++i){
		for (int j = 1;j < 4 - i;++j){
			if (costF[j - 1] > costF[j]){
				uint16_t c = costF[j - 1];
				costF[j - 1] = costF[j];
				costF[j] = c;
				uint8_t d = dir[j - 1];
				dir[j - 1] = dir[j];
				dir[j] = d;
			}
		}
	}

	if (costF[2] == 0){
		//相当安全
		//跟随角色
		Player *r = &Players[0];
		int q = 0;
		if (r->y > ai->y){
			dir[q++] = 0;
		}
		if (r->x < ai->x){
			dir[q++] = 1;
		}
		if (r->x > ai->x){
			dir[q++] = 2;
		}
		if (r->y < ai->y){
			dir[q++] = 3;
		}
	}

	for (int i = 0;i < 4;++i){

		uint16_t x = ((ai->x + 0x80) >> 8) & 0xFF;
		uint16_t y = ((ai->y + 0x80) >> 8) & 0xFF;

		switch(dir[i]){
			case 0:
				y += 1;break;
			case 1:
				x -= 1;break;
			case 2:
				x += 1;break;
			case 3:
				y -= 1;break;
		};

		if (!IsPassed(x,y))continue;
		//32位GCC在16位下， 无符号比较有点问题～
		if (x == 0)continue;
		if (y == 0)continue;
		ai->tx = x << 8;
		ai->ty = y << 8;
		break;
	}
}
