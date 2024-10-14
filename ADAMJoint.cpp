#include "ADAM.h"
#include <iostream>
#include <vector>
#include <termios.h> //TCSANOW
#include <string.h>
#include <fcntl.h> // fcntl

using namespace std;

bool kbhit() {
    termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }

    return false;
}

int main() {
    string device1 = string("/dev/ttyUSB0");

    // 基类ADAM: 放置串口连接的参数
    ADAM adamPort1(device1, 9600, 'N', 8, 1);

    int slave_ID_4 = 4; // 从机地址
    int slave_ID_5 = 5;
    int totalDI_4 = 16; // 从机DI总数
    int totalDI_5 = 16;
    ADAM4051 adam_4(adamPort1, slave_ID_4, totalDI_4);
    ADAM4051 adam_5(adamPort1, slave_ID_5, totalDI_5);

    // adam_4.connect(false); // 以不调试的形式连接
    // adam_5.connect(false);
    // adamPort1.connect(false); // 以不调试的形式连接

    while(true) {
        if(adam_4.read_coils() != -1) {
            vector<uint8_t> state_coils_4 = adam_4.state_coils;
            cout << "ADAM-4051(4): ";
            for(auto coil: state_coils_4) {
                cout << static_cast<int>(coil) << " ";
            }
            cout << endl;
        }
        if(adam_5.read_coils() != -1) {
            vector<uint8_t> state_coils_5 = adam_5.state_coils;
            cout << "ADAM-4051(5): ";
            for(auto coil: state_coils_5) {
                cout << static_cast<int>(coil) << " ";
            }
            cout << endl;
        }

        sleep(1);

        // 检查是否有按键按下
        if (kbhit()) {
            cout << "Key pressed, exiting..." << endl;
            adamPort1.disconnect();
            break;
        }
    }
    return 0;
}