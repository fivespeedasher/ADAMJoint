/* test.cpp */
#include "test.h"
#include <iostream>
using namespace std;

Father::Father(string family_name, string given_name) {
    this->family_name = family_name;
    this->given_name = given_name;
}
Father::~Father() {}

void Father::print() {
    cout << "family_name: " << family_name << ", given_name: " << given_name << endl;
}

Son::Son(const Father& father, string given_name) : Father(father.family_name, given_name) {
}
Son::~Son() {}
