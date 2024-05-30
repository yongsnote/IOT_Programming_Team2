// Header File
#include<stdio.h>          		// 입출력 관련 
#include<stdlib.h>         		// 문자열 변환, 메모리 관련 
#include<unistd.h>       		// POSIX 운영체제 API에 대한 액세스 제공 
#include<fcntl.h>			// 타겟시스템 입출력 장치 관련 
#include<sys/types.h>    		// 시스템에서 사용하는 자료형 정보 
#include<sys/ioctl.h>    		// 하드웨어의 제어와 상태 정보 
#include<sys/stat.h>     		// 파일의 상태에 대한 정보 
#include <string.h>       		// 문자열 처리 
#include <time.h>         		// 시간 관련 
#include <iostream>
#include <vector>
#include <cstdlib>

#include "DotMatrix.cpp"
#include "Ship.cpp"
#include "TactSW.cpp"
#include "CharacterLCD.cpp"
#include "FND.cpp"

using namespace std;

int timer;
int score;
int highScore;
Ship ship;
DotMatrix dot;
TactSW tact;
CharacterLCD clcd;
FND fnd;

void init(); //초기화 함수
void setPlayerPos(); // 플레이어 좌표 동기화 함수
bool enemyGoingDown(); //적이 내려오는 함수
void bulletLogic(); //총알 이동 및 적 처치 로직




void main()
{
     while(1)
     {
        
     }
}





//게임 시작 시 초기화 함수
void init()
{
    score = 0;
    timer = 0;
    dot.clear();

    //적 생성 - 상위 두줄
    for(int i = 0; i<2; i++)
    {
        for(int j = 0; j<8; j++)
            dot.setPoint(i, j, 1);
    }

    setPlayerPos();
}

void setPlayerPos()
{
    //밑 2줄 초기화 후
    //플레이어 좌표 설정
    for(int i = 0; i<8; i++)
    {
        dot.setPoint(6,i,0);   
        dot.setPoint(7,i,0);   
    }

    for(auto a : ship.getShipPos())
        dot.setPoint(a.first, a.second, 1);
}

//한 줄씩 내려오는 함수 - false: gameover, true: keepgoing
bool enemyGoingDown()
{
    for(int i = 0; i<8; i++)
    {
        if(dot.getPoint(5,i) == 1)
            return 0;
    }

    for(int i = 5; i>0; i--)
    {
        for(int j = 0; j<8; j++)
        {
            if(isExist)
                dot.setPoint(i, j, dot.getPoint(i-1, j-1));
        }
    }

    for(int i = 0; i<8; i++)
        dot.setPoint(0,i, 1);

    return 1;
}

void bulletLogic()
{
    //총을 안쐈으면 총알 위치가 필요 없음
    if(ship.canShootNow()) return;

    pair<int, int> bulletPos = ship.getBulletPos();
    int by = bulletPos.first - 1;
    if(by < 0)
    {
        ship.reload();
    }
    else if(dot.getPoint(by, bulletPos.second) == 1)
    {
        ship.reload();
        score++;
        dot.setPoint(by, bulletPos.second, 0); //적 처치
    }
    else
    {
        dot.setPoint(by, bulletPos.second, 1); // 다음위치 set
    }

    //이전 위치 0
    dot.setPoint(bulletPos.first, bulletPos.second, 0);
}