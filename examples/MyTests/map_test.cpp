#include <string.h>
#include <iostream>
#include <map>
#include <utility>
#include <time.h>
#include <stdio.h>
	 
using namespace std;

typedef map<int, string> innerMap;

int main()
{
   map<string, innerMap*> E;
   timespec rawtime;
   clock_gettime(CLOCK_MONOTONIC, &rawtime);
   
   double secs = (double) rawtime.tv_sec;
   double millsec = (double)rawtime.tv_nsec/1000000;
   

   double time1 = secs + millsec;
   double time2 = secs + millsec;

   secs += millsec;
   cout<<secs<<endl;   

   if (secs==time2){
      cout<<"they're the same"<<endl;
   }

   innerMap * node1 = new innerMap;
   (*node1)[1] = "id1";
	
   E["1000"] = node1;

   innerMap * newNode = E["1000"];

   if ((*newNode).find(1) == (*newNode).end()){
   	cout<<"not found"<<endl;
   }
   else {
	cout<<"found key"<<endl;
   }


   cout<<(*newNode)[1]<<endl;
}
