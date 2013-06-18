#include "tmRearrange.hpp"

#include "tm_igraph.hpp"
#include <sys/time.h>  
#include <signal.h>
#include <arpa/inet.h>
#include "blackadder.hpp"
#include <queue>

int fid_len = 32;

RV_leaf::RV_leaf(string &nodeID, string &leftRV, string &rightRV){
    self_ID = nodeID;
    string left, right;
    left_RV = NULL;
    right_RV = NULL;
    leftNodeNum = 0;
	rightNodeNum = 0;
    if (!leftRV.empty()){
		left_RV = new RV_leaf(leftRV, left, right);
		leftNodeNum++;
    }
    if (!rightRV.empty()){
		right_RV = new RV_leaf(rightRV, left, right);  
		rightNodeNum++;
    }
}
void RV_leaf::addChild(RV_leaf * child, int leftRight){
    if (leftRight == 0){
		left_RV = child;    
    }else{
		right_RV = child;
    }
}


void RV_leaf::addNode(RV_leaf * node, string &nodeID){
    if (left_RV==NULL){
		left_RV = node;
		nodeID = self_ID;
    }else if (right_RV==NULL){
    	right_RV = node;
		nodeID = self_ID;
    }else{
		if (leftNodeNum<=rightNodeNum){
			left_RV->addNode(node, nodeID);
			leftNodeNum++;
		}else{
			right_RV->addNode(node, nodeID);
			rightNodeNum++;
		}
    }
}

void RV_leaf::replace(RV_leaf * node, string &replaceNode){
	if (left_RV->self_ID == replaceNode){
		left_RV = node;
	}else if (right_RV->self_ID == replaceNode){
		right_RV = node;
	}
}

map<string, RV_leaf *> RVNodeMap;
RV_leaf * master;
bool run = true;
TMIgraph tm_igraph;
igraph_t graph;
map<string, int> reverse_node_index;
map<int, string> node_index;
map<string, int> reverse_edge_index;
map<string, Bitvector *> nodeID_iLID;
map<int, Bitvector *> vertex_iLID;
map<int, Bitvector *> edge_LID;
map<string, vector<string> > nodeLists;


void split1(string &str, vector<string> &nodes){
	string::size_type found = str.find_first_of(",");
    while (found!=std::string::npos)
    {
   		nodes.push_back(str.substr(found-PURSUIT_ID_LEN, PURSUIT_ID_LEN));
    	found=str.find_first_of(",",found+1);
    }
}
void addRVNode(string &ID, string &child){
	string left, right;
    RV_leaf * newNode;

    if (RVNodeMap.find(ID) == RVNodeMap.end()){
		newNode = new RV_leaf(ID, left, right);
    }else{
		newNode = RVNodeMap[ID];
    }

	if (!child.empty()){
		if (RVNodeMap.find(child) == RVNodeMap.end()){
			RV_leaf * childRVNode = new RV_leaf(child, left, right);
			childRVNode->parent_RV = newNode;
			RVNodeMap[child] = childRVNode;			
			newNode->addNode(childRVNode, child);
		}else{
			RV_leaf * childRVNode = RVNodeMap[child];
			childRVNode->parent_RV = newNode;		
			newNode->addNode(childRVNode, child);
		}
    }
	
	RVNodeMap[ID] = newNode;
}


int readRVTopology2(char *file_name) {
    int ret;
    Bitvector *lid;
    Bitvector *ilid;
    ifstream infile;
    string str;
    size_t found, first, second;
    FILE *instream;
    infile.open(file_name, ifstream::in);
    /*first the Global graph attributes - c igraph does not do it!!*/

    while (infile.good()) {
        getline(infile, str);
		found = str.find("<data key=\"FID_LEN\">");
        if (found != string::npos) {
            first = str.find(">");
            second = str.find("<", first);
            sscanf(str.substr(first + 1, second - first - 1).c_str(), "%d", &fid_len);
        }
        found = str.find("<data key=\"MASTER_RV\">");
        if (found != string::npos) {
            first = str.find(">");
            second = str.find("<", first);
            string masterRVnodeID = str.substr(first + 1, second - first - 1);
			//cout<<masterRVnodeID<<endl;
			string left, right;
			master = new RV_leaf(masterRVnodeID, left, right);
			RVNodeMap[masterRVnodeID] = master;
        }
    }

    infile.close();
    instream = fopen(file_name, "r");
    ret = igraph_read_graph_graphml(&graph, instream, 0);
    fclose(instream);
    if (ret < 0) {
        return ret;
    }
	
    for (int i = 0; i < igraph_vcount(&graph); i++) {
        string nID = string(igraph_cattribute_VAS(&graph, "NODEID", i));
        string iLID = string(igraph_cattribute_VAS(&graph, "iLID", i));		
        string nodes = string(igraph_cattribute_VAS(&graph, "netWork Node", i));
		vector<string> nodeList;
		split1(nodes, nodeList);
		nodeLists[nID] = nodeList;
		node_index.insert(pair<int, string>(i, nID));
        reverse_node_index.insert(pair<string, int>(nID, i));
        ilid = new Bitvector(iLID);
        nodeID_iLID.insert(pair<string, Bitvector *>(nID, ilid));
        vertex_iLID.insert(pair<int, Bitvector *>(i, ilid));
        //cout << "node " << i << " has NODEID " << nID << endl;
    }

	for (int i = 0; i < igraph_vcount(&graph); i++) {
		igraph_vs_t vertexOut;
	    igraph_vs_adj(&vertexOut, i, IGRAPH_OUT);

		igraph_vit_t vit;
		igraph_vit_create(&graph, vertexOut, &vit);	
		igraph_integer_t j;
    	igraph_vs_size(&graph, &vertexOut, &j);	
		
		string nodeID = node_index[i];

		while (!IGRAPH_VIT_END(vit)) {
  			long int e1 = IGRAPH_VIT_GET(vit);
			string e1NodeID = node_index[e1];
			addRVNode(nodeID, e1NodeID);
			IGRAPH_VIT_NEXT(vit);
		}
	}

    return ret;
}

int failedRVReconfig(string & RVNode){
	RV_leaf * newNode;
	RV_leaf * parent;
    if (RVNodeMap.find(RVNode) == RVNodeMap.end()){
		cout<<"no node found, error"<<endl;
		return -1;
    }else{
		newNode = RVNodeMap[RVNode];
    }

	RVNodeMap.erase(RVNode);  
	
	string newNodeID;
	if(newNode->parent_RV!=NULL){
		parent = newNode->parent_RV;;
		if(newNode->left_RV!=NULL){
			parent->replace(newNode->left_RV, newNode->self_ID);
			if(newNode->right_RV!=NULL){
				master->addNode(newNode->right_RV, newNodeID);
			}
		}else if(newNode->right_RV!=NULL){
			parent->replace(newNode->right_RV, newNode->self_ID);
		}
		delete newNode;
	}else{
		if(newNode->left_RV!=NULL){
			master = newNode->left_RV;
			if(newNode->right_RV!=NULL){
				master->addNode(newNode->right_RV, newNodeID);
			}
		}else if (newNode->right_RV!=NULL){
			master = newNode->right_RV;
		}
		delete newNode;
	}
}

string resp_id = string();
string resp_prefix_id = "FFFFFFFFFFFFFFFD";
string resp_bin_id = hex_to_chararray(resp_id);
string resp_bin_prefix_id = hex_to_chararray(resp_prefix_id);


void tmOutput(string& source, string& destination){

    Bitvector * FID_newRV = tm_igraph.calculateFID(source, destination);

    int response_size;

	char *response = (char *) malloc(response_size);
    string response_id = resp_bin_prefix_id + nodeID;

    memcpy(response, &request_type, sizeof (request_type));
    //memcpy(response + sizeof (request_type), , );

    ba->publish_data(response_id, IMPLICIT_RENDEZVOUS, FID_newRV->_data, FID_LEN, response, response_size);

	//unsigned char request_type = MATCH_PUB_SUBS;

	// rv_reconfig_type
	// parent fid
	// left fid
	// right fid


	//ToDo -> when sending initial message about broken 
	//packet_len = /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN/*END OF API*/\
            /*PAYLOAD*/ + sizeof (request_type) + ;

}

void printRV(){
	queue<RV_leaf *> currentLevel, nextLevel;
	currentLevel.push(master);
	cout<<master->self_ID<<endl;
	

	while(!currentLevel.empty()){
		RV_leaf * node = currentLevel.front();
		currentLevel.pop();

		if(node->left_RV!=NULL){
			nextLevel.push(node->left_RV);
			cout<<node->left_RV->self_ID;
		}
		if(node->right_RV!=NULL){
			nextLevel.push(node->right_RV);			
			cout<<node->right_RV->self_ID;
		}
		if(currentLevel.empty()){
			cout<<endl;
			std::swap(nextLevel, currentLevel);
		}
	}
		
}
int main(int argc, char *argv[]){

    cout << "TM: starting - process ID: " << getpid() << endl;
    if (argc != 2) {
        cout << "TM: the topology file is missing" << endl;
        exit(0);
    }
    /*read the graphML file that describes the topology*/
    /*if (tm_igraph.readRVTopology(argv[1]) < 0) {
        cout << "TM: couldn't read topology file...aborting" << endl;
        exit(0);
    }
    /***************************************************/


	timeval tim;  	
    gettimeofday(&tim, NULL);	
    double t1=tim.tv_sec+(tim.tv_usec/1000000.0); 	

	if (readRVTopology2(argv[1]) < 0) {
        cout << "TM: couldn't read topology file...aborting" << endl;
        exit(0);
    }
	
	printRV();
	string id = "00000001";
	failedRVReconfig(id);
	gettimeofday(&tim, NULL);  
	printRV();
	double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
	cout.precision(17);  	
	cout<<t2-t1<<endl;	

}
