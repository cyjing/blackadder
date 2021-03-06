
#include <map>

#include "RV_leaf.hpp"

RV_leaf::RV_leaf(NetworkNode* master,NetworkNode* parent, NetworkNode* self) {
    parent_RV = parent;
    left_RV = NULL;
    right_RV = NULL;
    Master_RV = master;
    self_RV = self;
}

void RV_leaf::add(NetworkNode* newRV){
  if (left_RV==NULL){
    left_RV = new RV_Tree(Master_RV, self_Rv, newRV);
  }else if (right_RV==NULL){
    right_RV = new RV_Tree(Master_RV, self_Rv, newRV);
  }else{
    int leftNodes = left_RV->getNodes();
    int rightNodes = right_RV->getNodes();
    if (leftNodes<=rightNodes){
      left_RV->add(newRV);
    }else{
      right_RV->add(newRV);
  }
}

int RV_leaf::getNodeNums(){
  if (left_RV==NULL&& right_RV==NULL){
    return 1;
  }else if (left_RV==NULL){
    return right_RV->getNodes()+1;
  }else if (right_RV==NULL){
    return left_RV->getNodes()+1;
  }else{
    return lefr_RV->getNodes() + right_RV->getNodes()+1;
  }
}
