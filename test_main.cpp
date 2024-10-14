/* test_main.cpp */
#include "test.h"
#include <iostream>

using namespace std;

int main(){
    Father father("Smith", "John");
    father.print();
    Son son(father, "Tom");
    son.print();
    return 0;
}