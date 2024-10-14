/* test.h */
#ifndef TEST_H
#define TEST_H

#include <string>
using namespace std;
class Father {
public:
    Father(string family_name, string given_name);
    ~Father();
    void print();
    string family_name;
protected:
    string given_name;
};

class Son : public Father {
public:
    Son(const Father& father, string given_name);
    ~Son();
};
#endif