#include <iostream>
#include <fcntl.h>
#include <unistd.h>

static char tactswDev[] = "/dev/tactsw";

class TactSW
{
private:
public:
    int get()
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
};