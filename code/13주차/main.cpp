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


#define CLCD "/dev/clcd"
#define DOT_DEV "/dev/dot"
#define FND_DEV "/dev/fnd"
#define FND_MAX 8
static char tactswDev[] = "/dev/tactsw";

// 1/600�ʸ� ����
#define TIME_QUANTUM 1667
using namespace std;

void init(); //�ʱ�ȭ �Լ�
void setStartGamePos(); //���� �ʱ� ��ǥ ����
void setPlayerPos(); //�÷��̾� ��ǥ
bool enemyGoingDown(); //���� �������� �Լ�
void bulletLogic(); //�Ѿ� �̵� �� �� óġ ����


//ENUM�� Ȱ���ؼ� 0,1,2,3,4�� ����
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
// ��� ��Ʈ���� �ʱ�ȭ
void clear() { memset(matrix, 0, sizeof(bool) * 8 * 8); }
// Matrix�� ���ϴ� ���� ����
void setPoint(int y, int x, bool val) { matrix[y][x] = val; }
// ��Ʈ������ ���
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
    case 2: //����
        return 0;
    case 4: //���� �̵�
        return 1;
    case 6: //������ �̵�
        return 2;
    case 5: //Ȯ�� ��ư
        return 3;

    default:
        return -1;
    }
}

//CLCD
int clcd_fd;
// CharacterLCD�� ���
void printCLCD(string S)
{
    char* cstr = new char[S.length() + 1];
    strcpy(cstr, S.c_str());
    clcd_fd = open(CLCD, O_WRONLY);
    write(clcd_fd, cstr, 32);
    close(clcd_fd);
}
//Score�� ȿ�������� ����ϴ� �޼ҵ�
string scoreSpacer(int I, int space)
{
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


typedef struct bullet
{
    int x;
    int y;
} bullet;

// ������ �Լ�
class Ship
{
private:
    bullet b;
    int x;
    int y;
    int canShoot;

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
    Ship()
    {
        x = 4;
        y = 7;
        canShoot = true;
    }


    //����
    void controll(int input)
    {
        switch (input)
        {
        case 0:
            Shoot();
            break;
        case 1:
            x = (x - 1) < 0 ? 0 : (x - 1);
            break;
        case 2:
            x = (x + 1) > 7 ? 7 : (x + 1);
            break;
        default:
            break;
        }
    }

    //�Լ� ��ǥ
    vector< pair<int, int> > getShipPos()
    {
        vector< pair<int, int> > v;
        v.push_back(make_pair(y, x)); //����
        v.push_back(make_pair(y - 1, x)); //�պκ�

        if (x - 1 >= 0)
            v.push_back(make_pair(y, x - 1)); //��
        if (x + 1 < 8)
            v.push_back(make_pair(y, x + 1)); //��

        return v;
    }

    //�Ѿ� ������ ������ main ����
    void setBulletPos(int y) { b.y = y; }
    pair<int, int> getBulletPos() { return make_pair(b.y, b.x); }

    //�ٽ� �� �� �� �ְ� ���� ������ ���� canShoot true��
    bool canShootNow() { return canShoot; }

    void reload() { canShoot = true; }
};


int timer;
int score;
int highScore;
Ship ship;
bool isalive;
int command;
int downCounter;

int main(int argc, const char* argv[])
{
    highScore = 0;

    while (true)
    {
        //���� ������ Ȯ�ι�ư
        beforeGameCLCD();
        while (getButtonTACT() != 3)
        {
            printf("Press StartButton \n");
        }

        init();

        //�ѹ��� Ȯ��
        while (getButtonTACT() != 3)
        {
            printf("one more\n");
            drawToMatrix(TIME_QUANTUM * 20);
        }

        gamingCLCD(score, highScore);

        while (isalive)
        {
            //�Ѿ�
            bulletLogic();

            //���
            drawToMatrix(TIME_QUANTUM * 20); // 20/600��
            //FN.draw(TIME_QUANTUM * 9); // 9/600��

            //��� �������� 

            command = getButtonTACT();

            if (command > -1 && command < 3)
            {
                printf("command - %d \n", command);
                ship.controll(command);
                setPlayerPos();
            }

            timer++;
            // ���� ����ϴ� �͵� usleep�� ���
            // 20�� �����Ҷ����� 0���� �ʱ�ȭ
            // 20�� �����ϸ� 1�ʰ� ������
            if (!(timer % 30))
            {
                timer = 0;
                //5�ʸ��� �������� ����
                downCounter++;

                printf("downCounter - %d\n", downCounter);

                if (downCounter == 5)
                {
                    isalive = enemyGoingDown();
                    downCounter = 0;
                }
                //FN.next();
            }
        }

        //���� ���� ���ӿ���
        gameOverCLCD(score);
        highScore = score > highScore ? score : highScore;
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
                setPoint(i, j, true);
        }
        for (int i = 0; i < 90; i++)
        {
            drawToMatrix(TIME_QUANTUM * 20);
        }

        //Ȯ�� ���� �� ���� ���
        while (getButtonTACT() != 3)
        {
            printf("waiting for restart\n");
        }
    }
    
    return 0;
}


//���� ���� �� �ʱ�ȭ �Լ�
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

    setStartGamePos();
}

void setStartGamePos()
{
    //�� ���� - ���� ����
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 8; j++)
            setPoint(i, j, 1);
    }

    //�� 2�� �ʱ�ȭ ��
    //�÷��̾� ��ǥ ����
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


//�� �پ� �������� �Լ� - false: gameover, true: keepgoing
bool enemyGoingDown()
{
    for (int i = 0; i < 8; i++)
    {
        if (matrix[5][i])
            return false;
    }

    for (int i = 5; i > 0; i--)
    {
        for (int j = 0; j < 8; j++)
        {
            setPoint(i, j, matrix[i - 1][j]);
        }
    }

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            printf(matrix[i][j] ? "1" : "0");
        }
        printf("\n");
    }
    printf("\n");

    for (int i = 0; i < 8; i++)
        setPoint(0, i, 1);

    return true;
}

void bulletLogic()
{
    //���� �Ƚ����� �Ѿ� ��ġ�� �ʿ� ����
    if (ship.canShootNow()) return;

    pair<int, int> bulletPos = ship.getBulletPos();
    int by = bulletPos.first - 1;
    if (by < 0)
    {
        printf("bullet end\n");
        ship.reload();
    }
    else if (matrix[by][bulletPos.second])
    {
        printf("enemy slained\n");
        setPoint(by, bulletPos.second, 0); //�� óġ
        score++;
        ship.reload();

        //���� ���� �� �ٽ� ���
        gamingCLCD(score, highScore);
    }
    else
    {
        printf("bullet go straight\n");
        setPoint(by, bulletPos.second, 1); // ������ġ set
        ship.setBulletPos(by);
    }

    //���� ��ġ 0
    setPoint(bulletPos.first, bulletPos.second, 0);
}