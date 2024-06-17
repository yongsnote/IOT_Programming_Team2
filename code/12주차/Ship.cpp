#include <iostream>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <ctime>

#define ENUM_SET

using namespace std;

//ENUM을 활용해서 0,1,2,3,4를 설정
typedef enum : int
{
    SHOOT,
    LEFT,
    RIGHT,
    OK
} MOVE;

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
    int x = 4;
    int y = 7;
    int canShoot = true;

    void Shoot()
    {
        if(canShoot)
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
        y = 8;
        canShoot = true;
    }


    //조작
    void controll(int input)
    {
        switch (input)
        {
            case SHOOT:
                Shoot();
                break;
            case LEFT:
                x = (x-1) < 0 ? 0: (x-1);
                break;
            case RIGHT:
                x = (x+1) > 7 ? 7: (x+1);
                break;
            default:
                break;
        }
    }

    //함선 디스플레이 좌표
    vector<pair<int,int>> getShipPos()
    {
        vector<pair<int,int>> v;
        v.push_back(make_pair(y, x)); //몸통
        v.push_back(make_pair(y-1, x)); //앞부분

        if(x-1>=0)
            v.push_back(make_pair(y, x-1)); //좌
        if(x+1<8)
            v.push_back(make_pair(y, x+1)); //우

        return v;
    }

    //총알 움직임 로직은 main 에서
    void setBulletPos(int y) { b.y = y; }
    pair<int, int> getBulletPos() { return make_pair(b.y, b.x); }

    //다시 총 쏠 수 있게 총이 끝까지 가면 canShoot true로
    bool canShootNow() { return canShoot; }

    void reload() {canShoot = 1;}
};