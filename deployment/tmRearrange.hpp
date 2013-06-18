#include <string.h>
#include <map>
#include <vector>
#include <set>
#include <igraph/igraph.h>
#include <climits>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "bitvector.hpp"
using namespace std;


class RV_leaf{
public:
  RV_leaf* left_RV;
  RV_leaf* right_RV; 
  RV_leaf* parent_RV;
  string self_ID;
  int leftNodeNum, rightNodeNum;

  RV_leaf(string &nodeID, string &leftRV, string &rightRV);

  //void addParent(string parent);
  void addChild(RV_leaf * child, int leftRight);
  void addNode(RV_leaf * node, string &nodeID);
  void replace(RV_leaf * node, string &replaceNode);
};
