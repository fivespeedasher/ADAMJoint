#include "ADAM.h"
#include "BulbConstants.h"
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
    int slave_ID_6 = 6;
    int totalDI_4 = 16; // 从机DI总数
    int totalDI_5 = 16;
    int totalCH_6 = 8; // 从机通道总数
    int duty_cycles = 0.5; // 脉冲占空比
    ADAM4051 adam_4(adamPort1, slave_ID_4, totalDI_4);
    ADAM4051 adam_5(adamPort1, slave_ID_5, totalDI_5);
    ADAM4168 adam_6(adamPort1, slave_ID_6, totalCH_6, duty_cycles);

    // // 脉冲输出参数
    // const int LEFT = 2; // 左转向引脚通道
    // const int RIGHT = 0; // 右转向引脚通道
    // const uint16_t Blink = 3;  // 闪烁次数
    
    // // 电平输出参数
    // const uint16_t RUN = 1; // 行车灯
    
    bool RUN_status = true; // 行车灯状态

    adam_6.SetDO(RUNNING_LIGHT, RUN_status); // 打开行车灯
    while(true) {
        if(adam_4.read_coils() != -1) {
            vector<uint8_t> state_coils_4 = adam_4.state_coils;
            cout << "ADAM-4051(4): ";
            // 4051COM悬空时，DI=1表示接收到了低电平
            for(auto coil: state_coils_4) {
                cout << static_cast<int>(coil) << " ";
            }
            cout << endl;
            // if(state_coils_4[0] == 1) {
            //     // 启动左转向灯
            //     vector<int> PulseChannel = {TURNING}; // 脉冲通道
            //     adam_6.StartPulse(PulseChannel, BLINK); // 打开转向
            // }
        }
        if(adam_5.read_coils() != -1) {
            vector<uint8_t> state_coils_5 = adam_5.state_coils;
            cout << "ADAM-4051(5): ";
            for(auto coil: state_coils_5) {
                cout << static_cast<int>(coil) << " ";
            }
            cout << endl;
            // if(state_coils_5[0] == 1) {
            //     RUN_status = !RUN_status;
            //     adam_6.SetDO(RUNNING_LIGHT, RUN_status); // 打开行车灯
            // }
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