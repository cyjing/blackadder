#include <string.h>
#include <iostream>
#include <map>
#include <utility>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
	 
using namespace std;

typedef map<int, string> innerMap;

bool run = true;

static void * print(void *arg){
	while (run){
		sleep(1);
		cout<<"a"<<endl;
	}
}

int main(int argc, char *argv[]){
	pthread_t thread1;
	int s = pthread_create(&thread1, NULL, &print, NULL);
	int x = 0;	
	while (x<5){
	   sleep(1);
	   cout<<"T"<<endl;	
	   x++;
	}
	run = false;
	pthread_join(thread1, NULL);
	pthread_exit(NULL);
}
