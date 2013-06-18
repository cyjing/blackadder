#include <string.h>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <vector>

#include <cstdlib>
#include <fstream>

#include "bitvector.hpp"
using namespace std;


class RV_leaf{
public:
  RV_leaf* left_RV;
  RV_leaf* right_RV; 
  string parent_ID;
  string self_ID;

  RV_leaf(string &nodeID, string &leftRV, string &rightRV);

  //void addParent(string parent);
  void addChild(RV_leaf * child, int leftRight);
  int getNodes();
  string print();
  void failedRVReconfig(string & RVNode, map<string, RV_leaf *> RVNodeMap);

};
