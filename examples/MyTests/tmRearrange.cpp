#include "tmRearrange.hpp"

int fid_len = 32;

RV_leaf::RV_leaf(string &nodeID, string &leftRV, string &rightRV){
    self_ID = nodeID;
    string left, right;
    if (!leftRV.empty()){
	left_RV = new RV_leaf(leftRV, left, right);
    }
    if (!rightRV.empty()){
	right_RV = new RV_leaf(rightRV, left, right);  
    }	
}
void RV_leaf::addChild(RV_leaf * child, int leftRight){
    if (leftRight == 0){
	left_RV = child;    
    }else{
	right_RV = child;
    }
}

void RV_leaf::failedRVReconfig(string & RVNode, map<string, RV_leaf *> RVNodeMap){
    if (RVNodeMap.find(RVNode) == RVNodeMap.end()){
	cout<<"error, RV node wasn't part of RV graph"<<endl;
    }else{
	RV_leaf * failedNode = RVNodeMap[RVNode];
	RV_leaf * leftNode = failedNode->left_RV;
	RV_leaf * rightNode = failedNode->right_RV;
    }
}


int RV_leaf::getNodes(){
  if (left_RV==NULL&& right_RV==NULL){
    return 1;
  }else if (left_RV==NULL){
    return right_RV->getNodes()+1;
  }else if (right_RV==NULL){
    return left_RV->getNodes()+1;
  }else{
    return left_RV->getNodes() + right_RV->getNodes()+1;
  }
}

string RV_leaf::print(){

    if (left_RV==NULL & right_RV == NULL){
	return self_ID;
    }else if(left_RV==NULL){
	string right = right_RV->print();
	cout<<right<<endl;
	return self_ID;
    }else if (right_RV==NULL){
	string right = left_RV->print();
	cout<<right<<endl;
	return self_ID;
    }else{
	string print = left_RV->print() + "    " +right_RV->print();
	cout<<print<<endl;
	return self_ID;
    }
}

map<string, RV_leaf *> RVNodeMap;
RV_leaf * master;
bool run = true;

bool exist(vector<Bitvector> &LIDs, Bitvector &LID) {
    for (int i = 0; i < LIDs.size(); i++) {
        if (LIDs[i] == LID) {
            //cout << "duplicate LID" << endl;
            return true;
        }
    }
    return false;
}

void calculateLID(vector<Bitvector> &LIDs, int index) {
    int bit_position;
    int number_of_bits = (index / (fid_len * 8)) + 1;
    Bitvector LID;
    do {
        LID = Bitvector(fid_len * 8);
        for (int i = 0; i < number_of_bits; i++) {
            /*assign a bit in a random position*/
            bit_position = rand() % (fid_len * 8);
            LID[bit_position] = true;
        }
    } while (exist(LIDs, LID));
    LIDs[index] = LID;
}
void addRVNode(string &ID, string &leftRV, string &rightRV){
    string left, right;
    if (RVNodeMap.find(ID) == RVNodeMap.end()){
	RV_leaf * newNode = new RV_leaf(ID, left, right);
	if (!leftRV.empty()){
	    if (RVNodeMap.find(leftRV) == RVNodeMap.end()){
		RV_leaf * leftRVNode = new RV_leaf(leftRV, left, right);
		leftRVNode->parent_ID = ID;
		RVNodeMap[leftRV] = leftRVNode;
		newNode->addChild(leftRVNode, 0);
	    }else{
		RV_leaf * leftRVNode = RVNodeMap[leftRV];
		leftRVNode->parent_ID = ID;
		newNode->addChild(leftRVNode, 0);
	    }
	}
	if (!rightRV.empty()){
	    if (RVNodeMap.find(rightRV) == RVNodeMap.end()){
	    	RV_leaf * rightRVNode = new RV_leaf(rightRV, left, right);
		rightRVNode->parent_ID = ID;
		RVNodeMap[rightRV] = rightRVNode;
		newNode->addChild(rightRVNode, 1);
	    }else{
		RV_leaf * rightRVNode = RVNodeMap[rightRV];
		rightRVNode->parent_ID = ID;
		newNode->addChild(rightRVNode, 1);
	    } 
	}
	RVNodeMap[ID] = newNode;
	if (master->self_ID.empty()){
	    master = newNode;
	}else if(leftRV == master->self_ID || rightRV == master->self_ID){
	    master = newNode;
	}
    }else{
	cout<<"RV node already exists"<<endl;
    }
}

void printRV(){
	cout<<master->print()<<endl;
}
int main(int argc, char *argv[]){
	string id = "00001";
	string start, left, right;
	master = new RV_leaf(start, left, right);
	addRVNode(id, left, right);
	string id1 = "00000";
	string id2 = "00002";
	addRVNode(id1, id, id2);
	id = "00003";
	id1 = "00004";
	addRVNode(id2, id , id1);
	printRV();
}
