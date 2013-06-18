/*
*/
#include <iostream>
#include <stdio.h>
#include <time.h>
using namespace std;

int main(int argc, char* argv[]) {
    time_t rawtime;
    time (&rawtime);
    cout<< ctime (&rawtime) <<endl;
    return 0;
}

