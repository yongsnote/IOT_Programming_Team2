#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <math.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/stat.h> 
#include <time.h>  
#include <ctime>
#include <cmath>


#define CLCD "/dev/clcd"
#define DOT_DEV "/dev/dot"
#define FND_DEV "/dev/fnd"
#define FND_MAX 8
static char tactswDev[] = "/dev/tactsw";

// 1/600초를 정의
#define TIME_QUANTUM 1667
using namespace std;

void init(); //초기화 함수
void setStartGamePos(); //게임 초기 좌표 설정
void setPlayerPos(); //플레이어 좌표
bool enemyGoingDown(); //적이 내려오는 함수
void bulletLogic(); //총알 이동 및 적 처치 로직


//ENUM을 활용해서 0,1,2,3,4를 설정
//typedef enum : int
//{
//    SHOOT,
//    LEFT,
//    RIGHT,
//    OK
//} MOVE;

//dot matrix
int dot_fd;
bool matrix[8][8];
// 모든 매트릭스 초기화
void clear() { memset(matrix, 0, sizeof(bool) * 8 * 8); }
// Matrix에 원하는 점을 세팅
void setPoint(int y, int x, bool val) { matrix[y][x] = val; }
// 매트릭스로 출력
void drawToMatrix(int microSec)
{
    dot_fd = open(DOT_DEV, O_WRONLY);
    unsigned char rows[8];
    for (int i = 0; i < 8; i++)
    {
        rows[i] = 0;
        for (int j = 0; j < 8; j++)
        {
            if (matrix[i][j])
            {
                //쉬프트 연산을 통해 16진수로 한줄 표시
                rows[i] |= 1 << j;
            }
        }
    }
    write(dot_fd, &rows, sizeof(rows));
    usleep(microSec);
    close(dot_fd);
}

//TactSwitch
int getButtonTACT()
{
    unsigned char b;
    int tactswFd = -1;
    tactswFd = open(tactswDev, O_RDONLY);
    read(tactswFd, &b, sizeof(b));
    close(tactswFd);
    switch (b)
    {
    case 2: //공격
        return 0;
    case 4: //왼쪽 이동
        return 1;
    case 6: //오른쪽 이동
        return 2;
    case 5: //확인 버튼
        return 3;

    default:
        return -1;
    }
}

//CLCD
int clcd_fd;
// CharacterLCD에 출력
void printCLCD(string S)
{
    char* cstr = new char[S.length() + 1];
    strcpy(cstr, S.c_str());
    clcd_fd = open(CLCD, O_WRONLY);
    write(clcd_fd, cstr, 32);
    close(clcd_fd);
}

string scoreSpacer(int I, int space)
{
    //c++11 to_string 이 사용이 안되어 char 배열 에서 string으로 변환
    char s[10];
    sprintf(s, "%d", I);
    string STR(s);

    for (int i = STR.length(); i < space; i++)
    {
        STR = '0' + STR;
    }
    return STR;
}
void beforeGameCLCD()
{
    string s1 = "     Galaga     ";
    string s2 = "   Press Start  ";
    printCLCD(s1 + s2);
}

void gamingCLCD(int score, int highScore)
{
    string s1 = "Score   : " + scoreSpacer(score, 4) + "  ";
    string s2 = "HighScore: " + scoreSpacer(highScore, 4) + "  ";
    printCLCD(s1 + s2);
}

void gameOverCLCD(int score)
{
    string s1 = "Score   : " + scoreSpacer(score, 4) + "  ";
    string s2 = "   Game Over    ";
    printCLCD(s1 + s2);
}

//FND
int fnd_fd = -1;
unsigned char asc_to_fnd(int n)
{
    //0 ~ 9까지의 숫자들
    unsigned char FND_BITS[11] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xD8, 0x80, 0x98, 0xFF };
    return n >= 0 && n < 10 ? FND_BITS[n] : FND_BITS[10];
}

// 지정된 시간동안 FND에 출력
void drawFND(int microSec, int level)
{
    fnd_fd = open(FND_DEV, O_WRONLY);
    unsigned char Hex_Code[4];
    memset(Hex_Code, 0x00, sizeof(Hex_Code));

    int TargetNum = level;
    int i;
    for (i = 1; i < 4 + 1; i++)
    {
        int Num = TargetNum / pow(10, 4 - i);
        TargetNum -= Num * pow(10, 4 - i);
        Hex_Code[i - 1] = asc_to_fnd(Num);
    }
    write(fnd_fd, Hex_Code, 4);
    usleep(microSec);
    close(fnd_fd);
}

//총알 구조체, x좌표 y좌표
typedef struct bullet
{
    int x;
    int y;
} bullet;

// 조종할 함선
class Ship
{
private:
    bullet b;
    int x;
    int y;
    bool canShoot;

    //한 번에 한 총알 발사 및 총알의 좌표 설정
    void Shoot()
    {
        if (canShoot)
        {
            b.x = x;
            b.y = 6;
            canShoot = false;
        }
    }

public:
    Ship() //함선 초기화
    {
        x = 4;
        y = 7;
        canShoot = true;
    }


    //조작
    void controll(int input)
    {
        switch (input)
        {
        case 0:
            Shoot();
            break;
        case 1:
            x = (x + 1) > 7 ? 7 : (x + 1);
            break;
        case 2:
            x = (x - 1) < 0 ? 0 : (x - 1);
            break;
        default:
            break;
        }
    }

    //함선 좌표
    vector< pair<int, int> > getShipPos()
    {
        vector< pair<int, int> > v;
        v.push_back(make_pair(y, x)); //몸통
        v.push_back(make_pair(y - 1, x)); //앞부분

        //왼쪽 및 오른쪽 끝 부분에는 날개 한 부분이 안보이게
        if (x - 1 >= 0)
            v.push_back(make_pair(y, x - 1)); //좌
        if (x + 1 < 8)
            v.push_back(make_pair(y, x + 1)); //우

        return v;
    }

    //총알 움직임 로직은 main 에서
    void setBulletPos(int y) { b.y = y; }
    pair<int, int> getBulletPos() { return make_pair(b.y, b.x); }

    //현재 총을 쏠 수 있는 상태인지 가져오는 함수
    bool canShootNow() { return canShoot; }

    //재장전
    void reload() { canShoot = true; }
};


int timer; //시간초 세는 타이머
int score; //점수
int highScore; //최고 점수
int level; //적이 내려온 횟수(레벨)
Ship ship; //플레이어
bool isalive; //생존 여부
int command; //입력 키
int downCounter; //적이 내려오는 주기 카운터

int main(int argc, const char* argv[])
{
    highScore = 0;

    while (true)
    {
        //게임 시작전 확인버튼
        beforeGameCLCD();
        while (getButtonTACT() != 3)
        {
            printf("Press StartButton \n");
        }

        init();

        //한번더 확인
        while (getButtonTACT() != 3)
        {
            printf("one more\n");
            drawToMatrix(TIME_QUANTUM * 20);
        }

        //clcd 출력
        gamingCLCD(score, highScore);

        //플레이어 사망 전까지 루프
        while (isalive)
        {
            //총알
            bulletLogic();

            //출력- dotmatrix, fnd
            drawToMatrix(TIME_QUANTUM * 25); // 25/600초
            drawFND(TIME_QUANTUM * 5, level); // 5/600초

            //tact switch 명령 가져오기 
            command = getButtonTACT();

            if (command > -1 && command < 3)
            {
                printf("command - %d \n", command);
                ship.controll(command);
                setPlayerPos();
            }

            timer++;
            // 위의 출력하는 것들 usleep초 계산
            // 30/600 이니 20번 수행하면 1초가 지나감
            if (!(timer % 20))
            {
                timer = 0;
                //5초마다 내려오는 적들
                downCounter++;
                printf("downCounter - %d\n", downCounter);

                if (downCounter == 5)
                {
                    //플레이어가 살아있으면 적이 내려옴
                    isalive = enemyGoingDown();
                    downCounter = 0;
                    level++;
                }
            }
        }

        //여기 오면 게임오버
        gameOverCLCD(score);
        highScore = score > highScore ? score : highScore;
        
        //dotmatrix 모든 점 3초동안 점멸
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
                setPoint(i, j, true);
        }
        for (int i = 0; i < 90; i++)
        {
            drawToMatrix(TIME_QUANTUM * 20);
        }

        //확인 누를 때 까지 대기
        while (getButtonTACT() != 3)
        {
            printf("waiting for restart\n");
        }
    }
    
    return 0;
}


//게임 시작 시 초기화 함수
void init()
{
    dot_fd = -1;
    memset(matrix, 0, sizeof(bool) * 8 * 8);
    ship = Ship();
    score = 0;
    timer = 1;
    isalive = true;
    command = 0;
    downCounter = 0;
    level = 1;

    setStartGamePos();
}

//게임 시작 시 좌표 기본 설정
void setStartGamePos()
{
    //적 생성 - 상위 두줄
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 8; j++)
            setPoint(i, j, 1);
    }

    //밑 2줄 초기화 후
    //플레이어 좌표 설정
    for (int i = 0; i < 8; i++)
    {
        setPoint(6, i, 0);
        setPoint(7, i, 0);
    }

    vector< pair<int, int> > v = ship.getShipPos();
    for (int i = 0; i< v.size(); i++)
    {
        setPoint(v[i].first, v[i].second, 1);
    }
}

//플레이어가 이동할 때 마다 좌표 재설정 해야됨
//플레이어 좌표 세팅 해주는 함수
void setPlayerPos()
{
    for (int i = 0; i < 8; i++)
    {
        setPoint(6, i, 0);
        setPoint(7, i, 0);
    }

    vector< pair<int, int> > v = ship.getShipPos();
    for (int i = 0; i < v.size(); i++)
    {
        setPoint(v[i].first, v[i].second, 1);
    }
}


//한 줄씩 내려오는 함수 - false: gameover, true: keepgoing
bool enemyGoingDown()
{
    //내려올 시 플레이어 함선 앞부분에 닿으면 게임오버
    for (int i = 0; i < 8; i++)
    {
        if (matrix[5][i])
            return false;
    }

    //한줄씩 밑으로 이동
    for (int i = 5; i > 0; i--)
    {
        for (int j = 0; j < 8; j++)
        {
            setPoint(i, j, matrix[i - 1][j]);
        }
    }

    //디버깅용 프린트
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            printf(matrix[i][j] ? "1" : "0");
        }
        printf("\n");
    }
    printf("\n");

    //가장 윗줄 적으로 채우기
    for (int i = 0; i < 8; i++)
        setPoint(0, i, 1);

    return true;
}

void bulletLogic()
{
    //총을 안쐈으면 총알 위치가 필요 없음
    if (ship.canShootNow()) return;

    //총알은 y좌표만 이동, x좌표는 쏜 곳에서 고정
    pair<int, int> bulletPos = ship.getBulletPos();
    int by = bulletPos.first - 1;
    if (by < 0) //총알이 범위를 벗어나면 재장전
    {
        printf("bullet end\n");
        ship.reload();
    }
    else if (matrix[by][bulletPos.second]) //적을 맞췄다면
    {
        printf("enemy slained\n");
        setPoint(by, bulletPos.second, 0); //적 처치
        score++; //점수 증가
        ship.reload(); //재장전

        //점수 변경 시 다시 출력
        gamingCLCD(score, highScore);
    }
    else //총알이 이동 중일 때
    {
        printf("bullet go straight\n");
        setPoint(by, bulletPos.second, 1); // 다음위치로 y좌표 set
        ship.setBulletPos(by);
    }

    //이전 위치 0
    setPoint(bulletPos.first, bulletPos.second, 0);
}