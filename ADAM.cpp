// 采用Modbus-RTU连接

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
    connect(false, this->slave_id); // 连接从机

    // 读取一次线圈状态,并对线圈状态进行更新
    if(modbus_read_bits(ctx, 0, total_coils, this->state_coils.data()) == -1) {
        cout << "Failed to read coils: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    return 0;
}

ADAM4168::ADAM4168(const ADAM& adam, int slave_id, int total_channels, float duty_cycles)
         : ADAM(adam.device, adam.baud, adam.check, adam.data_bit, adam.stop_bit) {
    this->slave_id = slave_id;
    this->total_channels = total_channels;
    InitPulse(duty_cycles); // 初始化脉冲频率
}
ADAM4168::~ADAM4168() {}

/**
 * @brief 连接从机，初始化脉冲频率
 * 
 * @param duty_cycles 
 * @return int 
 */
int ADAM4168::InitPulse(float duty_cycles) {
    connect(false, this->slave_id); // 连接从机
    SetMode({}); // 清空模式设置,否则写入脉冲频率会打开脉冲
    // 修改脉冲输出频率 （32bit）
    vector<uint16_t> Toffs(16, 0);
    vector<uint16_t> Tons(16, 0);
    // 设置时间周期为1s（ADAM模块单位为0.1ms）, 输入占空比, 自动计算脉冲频率
    for(int i = 0; i < 16; i += 2) {
        Tons[i] = 10000 * duty_cycles;
        Toffs[i] = 10000 - Tons[i];
    }
    // 写入脉冲输出高、低延时
    if(modbus_write_registers(ctx, 30, 16, Tons.data()) == -1 || modbus_write_registers(ctx, 72, 16, Toffs.data()) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    return 0;
}

/**
 * @brief 初始化通道：将需要脉冲的通道Digital output mode置1为脉冲模式，其余默认为电平模式.
 * 
 * @param PulseChannel 设置需要脉冲输出的通道号。
 * @return int 
 */
int ADAM4168::SetMode(const vector<int>& PulseChannel) {
    //设定指定引脚为脉冲输出（pulse）模式
    vector<uint16_t> channels_mode(this->total_channels, 0);
    for(auto i : PulseChannel) {
        channels_mode[i] = 1;
    }
    if(modbus_write_registers(ctx, 64, this->total_channels, channels_mode.data()) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    return 0;
}

/**
 * @brief 写入寄存器Set absolute pulse,对channels启动pulse_times次电平
 * (注意: 必须单个写入,否则会使得其他脉冲写入0=continue)
 * 
 * @param channels 通道号
 * @param pulse_times 输出次数，为-1时一直输出
 * @return int 
 */
int ADAM4168::StartPulse(const vector<int>& channels, uint16_t pulse_times) {
    connect(false, this->slave_id); // 连接从机
    SetMode(channels); // 设置指定通道为脉冲输出模式
    
    // 设置脉冲输出次数 （32bit）
    vector<uint16_t> pulse_times_all(this->total_channels, 0);
    for(auto channel : channels) {
        pulse_times_all[channel * 2] = pulse_times;
    }
    if(modbus_write_registers(ctx, 32, this->total_channels * 2, pulse_times_all.data()) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    return 0;
}

int ADAM4168::SetDO(int channels, bool value) {
    connect(false, this->slave_id); // 连接从机
    // 设置脉冲输出次数 （32bit）
    if(modbus_write_bit(ctx, 16 + channels, value) == -1) {
        cout << "Failed to write registers: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    return 0;
}

ADAM4068::ADAM4068(const ADAM& adam, int slave_id, int total_coils)
         : ADAM(adam.device, adam.baud, adam.check, adam.data_bit, adam.stop_bit) {
    this->slave_id = slave_id;
    this->total_coils = total_coils;
    this->state_coils = vector<uint8_t>(total_coils, false);
}
ADAM4068::~ADAM4068() {}

/**
 * @brief 写单个通道（写入位置为线圈）
 * 
 * @param ch 通道
 * @param flag 通道状态
 * @return int 
 */
int ADAM4068::write_coil(int ch, bool flag) {
    connect(false, this->slave_id); // 连接从机
    int mapped_ch = 16 + ch;
    // 16~23是modbus上的地址，0~7是用户输入的地址
    if(modbus_write_bit(ctx, mapped_ch, flag) == -1) {
        cout << "Failed to write coil: " << modbus_strerror(errno) << endl;
        modbus_close(ctx);
        modbus_free(ctx);
        return -1;
    }
    return 0;
}