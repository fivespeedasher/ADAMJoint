#include <iostream>
#include <modbus.h>
#include <errno.h>
#include <vector>
#include <unistd.h> // sleep
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include "ADAM.h"

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

    // 初始化通道
    int slave_ID_6 = 6; // 从机地址
    int totalCH_6 = 8; // 从机通道总数
    float duty_cycles = 0.5; // 脉冲占空比

    // 初始化
    ADAM4168 adam_6(adamPort1, slave_ID_6, totalCH_6);
    if(adam_6.InitPulse(duty_cycles) == 0) {
        cout << "Set pulse channel successfully!" << endl;
    }

    // 脉冲输出参数
    const int LEFT = 3; // 左转向引脚通道
    const int RIGHT = 0; // 右转向引脚通道
    const uint16_t Blink = 3;  // 闪烁次数
    
    // 电平输出参数
    const uint16_t RUN = 1; // 行车灯

    vector<int> PulseChannel = {RIGHT}; // 脉冲通道
    adam_6.StartPulse(PulseChannel, Blink); // 打开转向
    adam_6.SetDO(RUN, false); // 打开行车灯
    adamPort1.disconnect();
    return 0;
}