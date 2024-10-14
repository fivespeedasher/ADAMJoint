/***
 * 用于读取的信号
 */

#include <iostream>
#include <errno.h>
#include <vector>
#include <unistd.h> // sleep
#include <string.h>
#include "ADAM.h"

using namespace std;

/**
 * @brief Construct a new ADAM::ADAM object
 * 
 * @param device 串口号
 * @param baud 波特率
 * @param check 奇偶校验
 * @param data_bit 数据位
 * @param stop_bit 停止位
 * @param slave_id 从机地址
 * @param total_coils 线圈总数
 */
ADAM::ADAM(string device, int baud, char check, int data_bit, int stop_bit) {
    this->device = device; // this指针指向调用成员函数的对象，消除歧义
    this->baud = baud;
    this->check = check;
    this->data_bit = data_bit;
    this->stop_bit = stop_bit;
    modbus_t *ctx = nullptr;
}
ADAM::~ADAM() {disconnect();}

/**
 * @brief 打开端口、创建连接
 * 
 * @param debug true:启用调试模式，会打印报文信息
 * @return int 
 */
int ADAM::connect(bool debug, int slave_id) {
    this->ctx = modbus_new_rtu(this->device.c_str(), this->baud, this->check, this->data_bit, this->stop_bit);
    
    // 打开端口
    if(this->ctx == nullptr) {
        cerr << "Unable to create the libmodbus context: " << modbus_strerror(errno) << endl;
        return -1;
    }

    // 设置从机地址
    modbus_set_slave(this->ctx, slave_id);

    // connect
    if(modbus_connect(this->ctx) == -1) {
        cerr << "Connection to slave failed: " << modbus_strerror(errno) << endl;
        modbus_free(this->ctx);
        modbus_close(this->ctx);
        return -1;
    }
    
    // 设置应答延时1s
    modbus_set_response_timeout(ctx, 0 ,1000000);

    // 启用调试模式
    modbus_set_debug(ctx, debug);
    return 0;
}

int ADAM::disconnect() {
    modbus_close(this->ctx);
    modbus_free(this->ctx);
    return 0;
}

ADAM4051::ADAM4051(const ADAM& adam, int slave_id, int total_coils)
         : ADAM(adam.device, adam.baud, adam.check, adam.data_bit, adam.stop_bit) {
    this->slave_id = slave_id;
    this->total_coils = total_coils;
    this->state_coils = vector<uint8_t>(total_coils, false);
}
ADAM4051::~ADAM4051() {}

/**
 * @brief 使用modbus_read_bits实时读取所有DI线圈的状态
 */
int ADAM4051::read_coils() {
    // 使用类的成员变量
    // string device = this->device;
    // int baud = this->baud;
    // char check = this->check;
    // int data_bit = this->data_bit;
    // int stop_bit = this->stop_bit;
    // int slave_id = this->slave_id;
    // int total_coils = this->total_coils; // 读取线圈总数（DI总数）
    // modbus_t *ctx = this->ctx;

    connect(false, this->slave_id); // 连接从机

    // 读取一次线圈状态,并对线圈状态进行更新
    if(modbus_read_bits(ctx, 0, total_coils, this->state_coils.data()) == -1) {
        cout << "Failed to read coils: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    // cout << "Coils status: ";
    // for(auto coil: coils) {
    //     cout << static_cast<int>(coil) << " ";
    // }
    // cout << endl << endl;
    return 0;
}