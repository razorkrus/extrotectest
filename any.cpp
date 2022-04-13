#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;

int main(){

    int val = 100;
    int &r = val;
    cout << "r is: " << r << endl;
}