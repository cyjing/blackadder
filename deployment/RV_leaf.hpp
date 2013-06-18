#ifndef RV_LEAF_HPP
#define	RV_LEAF_HPP

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>

#include "network.hpp"

class RV_leaf{
public:
  NetworkNode* master_RV;
  RV_leaf* left_RV;
  RV_leaf* right_RV; 
  NetworkNode* parent_RV;
  NetworkNode* self_RV;

  RV_Tree(NetworkNode* master,NetworkNode* parent, NetworkNode* self);

  void add(NetworkNode* newRV); 
  int getNodeNums();

};

#endif	/*RV_LEAF_HPP */
