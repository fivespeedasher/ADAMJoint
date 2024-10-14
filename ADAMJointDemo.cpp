#include <iostream>
#include <modbus.h>
#include <errno.h>
#include <vector>
#include <unistd.h> // sleep
#include <termios.h>
#include <fcntl.h>
#include <string.h>

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
    string device = "/dev/ttyUSB0";
    int baud = 9600;
    int check = 'N';
    int data_bit = 8;
    int stop_bit = 1;
    int slave_id = 4;
    modbus_t *ctx = nullptr;
    int total_coils = 16; // 读取线圈总数（DI总数）
    vector<uint8_t> state_coils(total_coils, false); // 线圈状态

    // 打开端口
    ctx = modbus_new_rtu(device.c_str(), baud, check, data_bit, stop_bit);
    if(ctx == nullptr) {
        cout << "Unable to create the libmodbus context: " << modbus_strerror(errno) << endl;
        return -1;
    }

    // 设置从机地址
    modbus_set_slave(ctx, slave_id);

    // connect
    if(modbus_connect(ctx) == -1) {
        cout << "Connection to slave failed: " << modbus_strerror(errno) << endl;
        modbus_free(ctx);
        modbus_close(ctx);
        return -1;
    }
    
    // 设置应答延时1s
    modbus_set_response_timeout(ctx, 0 ,1000000);

    // 启用调试模式
    modbus_set_debug(ctx, TRUE);

    // 读取线圈状态
    while(1) {
        if(modbus_read_bits(ctx, 0, total_coils, state_coils.data()) == -1) {
            cout << "Failed to read coils: " << modbus_strerror(errno) << endl;
            modbus_close(ctx);
            modbus_free(ctx);
            return -1;
        }
        cout << "Coils status: ";
        for(auto coil: state_coils) {
            cout << static_cast<int>(coil) << " ";
        }
        cout << endl << endl;
        sleep(1); // 1s

        // 检查是否有按键按下
        if (kbhit()) {
            cout << "Key pressed, exiting..." << endl;
            break;
        }
    }
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}