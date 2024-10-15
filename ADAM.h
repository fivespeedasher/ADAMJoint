#ifndef ADAM_H
#define ADAM_H
#include <modbus.h>
#include <vector>
#include <string>

using namespace std;

class ADAM
{
public:
    ADAM(string device, int baud, char check, int data_bit, int stop_bit);
    ~ADAM();
    string device;
    int baud;
    int check;
    int data_bit;
    int stop_bit;
    modbus_t *ctx;
    int connect(bool debug, int slave_id);
    int disconnect();
protected:
    int slave_id;
    int total_coils;
};

class ADAM4051 : public ADAM
{
public:
    ADAM4051(const ADAM& adam, int slave_id, int total_coils);
    ~ADAM4051();
    vector<uint8_t> state_coils;
    int read_coils();
private:
    int slave_id;
    int total_coils;
    using ADAM::connect; // 防止派生类外部访问，using在派生类中用于改变基类成员的访问权限，不需写形参
    using ADAM::disconnect; 
};

class ADAM4168 : public ADAM
{
public:
    ADAM4168(const ADAM& adam, int slave_id, int total_channels);
    ~ADAM4168();
    int InitPulse(float duty_cycles);
    int SetMode(const vector<int>& PulseChannel);
    int StartPulse(const vector<int>& channels, uint16_t pulse_times);
    int SetDO(int channels, bool value);
private:
    int slave_id;
    int total_channels;
    using ADAM::connect; // 防止派生类外部访问，using在派生类中用于改变基类成员的访问权限，不需写形参
    using ADAM::disconnect; 
};
#endif