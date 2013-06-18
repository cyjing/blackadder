#include "ping.hh"

#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <set>
#include <map>
#include <sys/time.h> 
using namespace std;

CLICK_DECLS

//bool runSendPing = true;
/*
void * sendPingHandler (void * arg){
    while(runSendPing){
        Ping::sendPing();
        sleep(1);
    }
}*/

Ping::Ping() {
}

Ping::~Ping() {
    click_chatter("Ping: destroyed!");
}

int Ping::configure(Vector<String> &conf, ErrorHandler *errh) {
    gc = (GlobalConf *) cp_element(conf[0], this);
    click_chatter("*****************************************************PING CONFIGURATION*****************************************************");
    int number_of_links;
    

    if (gc->use_mac == true) {
        cp_integer(conf[1], &number_of_links);
        click_chatter("Ping: Number of Links: %d", number_of_links);

        for (int i = 0; i < number_of_links; i++) {
	    String node;
            cp_string(conf[2 + 3 * i], &node);
            BABitvector nodeFID = BABitvector(FID_LEN * 8);
            for (int j = 0; j < conf[4 + 3 * i].length(); j++) {
                if (conf[4 + 3 * i].at(j) == '1') {
                    nodeFID[conf[4 + 3 * i].length() - j - 1] = true;
                } else {
                    nodeFID[conf[4 + 3 * i].length() - j - 1] = false;
                }
            }
	    nodeFIDs[node] = nodeFID;
        }
    } else {
        cp_integer(conf[1], &number_of_links);
        click_chatter("Ping: Number of Links: %d", number_of_links);
        for (int i = 0; i < number_of_links; i++) {
            String node;
            cp_string(conf[2 + 3 * i], &node);
            BABitvector nodeFID= BABitvector(FID_LEN * 8);
            for (int j = 0; j < conf[4 + 3 * i].length(); j++) {
                if (conf[4 + 3 * i].at(j) == '1') {
                    nodeFID[conf[4 + 3 * i].length() - j - 1] = true;
                } else {
                    nodeFID[conf[4 + 3 * i].length() - j - 1] = false;
                }
            }
	    nodeFIDs[node] = nodeFID;
        }
    }
    return 0;
}

int Ping::initialize(ErrorHandler *errh){
  //pthread_create( &thread1, NULL, &sendPingHandler, NULL);
    timespec rawtime;
    clock_gettime(CLOCK_REALTIME, &rawtime);
    double secs = (double) rawtime.tv_sec;
    double millsec = (double)rawtime.tv_nsec/1000000;
    secs += millsec;
    std::ostringstream strs;
    strs.precision(17);
    strs << secs;
    string ts = strs.str();
    unsigned char timeStampLen = ts.length();

    map<String, BABitvector>::iterator nodeFIDs_iter;
    for (nodeFIDs_iter = nodeFIDs.begin(); nodeFIDs_iter != nodeFIDs.end(); nodeFIDs_iter++) {
        BABitvector nodeFID = nodeFIDs_iter->second;
	String node = nodeFIDs_iter->first;
	//createAndSendPing(secs, node, nodeFID, ts,timeStampLen);
    }
    return 0;
}

void Ping::cleanup(CleanupStage stage){
  if (stage >= CLEANUP_ROUTER_INITIALIZED){
    //runSendPing = false;
    //pthread_join(thread1, NULL);
  }
  click_chatter("Ping: Cleaned UP!");
}

void Ping::push(int in_port, Packet *p){
  int index;
  unsigned char numberOfIDs, IDLength;
  Vector<String> IDs;
  
  if (in_port == 0){
    /*from port 2 I receive pings from the network*/
    handlePing(p);
    p->kill();
  }else if (in_port == 1){
    //from port 1 I expect pings from the local RV
    String nodeID;
    nodeID = String((const char *) (p->data()), PURSUIT_ID_LEN);
    if(nodeID == gc->nodeID){
      sendPing();
    }else if (nodeFIDs.find(nodeID) == nodeFIDs.end()){
      //couldn't find the nodeID in 
      cout<<"fail"<<endl;
    }else{
      BABitvector nodeFID = nodeFIDs[nodeID];

      struct timeval tim;  
      gettimeofday(&tim, NULL);  
      double secs =tim.tv_sec+(tim.tv_usec/1000000.0);  

      std::ostringstream strs;
      strs.precision(17);
      strs << secs;
      string ts = strs.str();
      unsigned char timeStampLen = ts.length();
      createAndSendPing(secs, nodeID, nodeFID, ts,timeStampLen);
    }
    p->kill();
  }
}

void Ping::createAndSendPing(double &secs, String &nodeID, BABitvector &nodeFID, string &ts, unsigned char &timeStampLen){   
      // cout.precision(17);
      //cout<<secs<< "  sending ping"<<endl;

      if (nodePingInfo.find(nodeID) == nodePingInfo.end()){
	PingInfo * newNode = new pair<double, bool>;
	newNode->first = secs;
	newNode->second = true;
	nodePingInfo.set(nodeID, newNode);
      }else {
	PingInfo * newNode = nodePingInfo.get(nodeID);
	if((newNode->second == false) && (secs - newNode->first > 2)){
	  if (nodeID!=gc->nodeID){
	    cout.precision(17);
	    cout<<nodeID.c_str()<< "  i think a node disconnected from this many seconds ago  "<< secs-newNode->first<<endl;
	    sendMessageToTM(nodeID);
	  }
	}else{
	  if (newNode->second == true){
	    newNode->first = secs;
	  }
	  newNode->second = false;
	}
	nodePingInfo.set(nodeID, newNode);
      } 
      WritablePacket *p1;
      p1 = Packet::make(50, NULL, FID_LEN + 2 * PURSUIT_ID_LEN + sizeof(timeStampLen)+ timeStampLen, 50);

      memcpy(p1->data(), nodeFID._data, FID_LEN);
      memcpy(p1->data() + FID_LEN, gc->nodeID.c_str(), PURSUIT_ID_LEN);
      memcpy(p1->data() + FID_LEN + PURSUIT_ID_LEN, nodeID.c_str(), PURSUIT_ID_LEN);
      memcpy(p1->data() + FID_LEN + 2 * PURSUIT_ID_LEN, &timeStampLen, sizeof(timeStampLen));
      memcpy(p1->data() + FID_LEN + 2 * PURSUIT_ID_LEN + sizeof(timeStampLen), ts.c_str(),timeStampLen);
      output(0).push(p1);
}

int Ping::handlePing(Packet *p){
    String senderID, receiverID;
    unsigned char timeStampLen;
    String timeStamp;

    senderID = String((const char *) (p->data()), PURSUIT_ID_LEN);
    receiverID = String((const char *) (p->data())+ PURSUIT_ID_LEN, PURSUIT_ID_LEN);
    timeStampLen = *(p->data()+ 2 * PURSUIT_ID_LEN);
    timeStamp = String((const char *) (p->data() + 2 * PURSUIT_ID_LEN + sizeof(timeStampLen)), timeStampLen);

    double seconds = atof(timeStamp.c_str());
    //cout<<timeStamp.c_str()<< "   "<<receiverID.c_str()<<"   timestamp"<<endl;

    if (senderID == gc->nodeID){
      // cout<<timeStamp.c_str()<< "   "<<receiverID.c_str()<<"   timestamp"<<endl;
        click_chatter("received ping back");
	if (nodePingInfo.find(receiverID) == nodePingInfo.end()){
	  cout<<"  error, ping cant find info"<<endl;
	  //error, should never happen
	}
	else {
  	    PingInfo * newNode = nodePingInfo.get(receiverID);
	    newNode->second = true;
	}
    }else{
      //click_chatter("send ping back");
      if (nodePingInfo.find(senderID) == nodePingInfo.end()){
  	    PingInfo * newNode = new pair<double, bool>;
	    if (newNode->first < seconds){
	      //newNode->first = seconds;
	    }
	    //nodePingInfo.set(senderID, newNode);
	    //newNode->second = true;
	}
	else {
	    PingInfo * newNode = nodePingInfo.get(senderID);
	    if (newNode->first < seconds){
	      //newNode->first = seconds;
	    }
	    //newNode->second = true;
        }
	WritablePacket *p1;
        BABitvector nodeFID = nodeFIDs[senderID];
	p1 = Packet::make(50, NULL, FID_LEN + 2*PURSUIT_ID_LEN + sizeof(timeStampLen)+ timeStampLen, 50);
	memcpy(p1->data(), nodeFID._data, FID_LEN);
	memcpy(p1->data() + FID_LEN, senderID.c_str(), PURSUIT_ID_LEN);
	memcpy(p1->data() + FID_LEN + PURSUIT_ID_LEN, gc->nodeID.c_str(), PURSUIT_ID_LEN);
	memcpy(p1->data() + FID_LEN + 2 * PURSUIT_ID_LEN, &timeStampLen, sizeof(timeStampLen));
	memcpy(p1->data() + FID_LEN + 2 * PURSUIT_ID_LEN + sizeof(timeStampLen), timeStamp.c_str(),timeStampLen);
        output(0).push(p1);
    }
    return 0;
}

void Ping::sendPing(){
    /*Publish a update to the Master RV*/
    /********FOR THE API*********/
    unsigned char numberOfIDs = 1;
    unsigned char numberOfFragments = 2;
    /****************************/
 
    struct timeval tim;  
    gettimeofday(&tim, NULL);  
    double secs =tim.tv_sec+(tim.tv_usec/1000000.0);  
    std::ostringstream strs;
    strs.precision(17);
    strs << secs;
    string ts = strs.str();
    unsigned char timeStampLen = ts.length();
   
    createAndSendPing(secs, gc->parentID, gc->parentRVFID, ts,timeStampLen);
    if(gc->RVhasLeft){
      // cout<<gc->leftID.c_str()<<"  left gc"<<endl;
      createAndSendPing(secs, gc->leftID,  gc->leftRVFID, ts,timeStampLen);
    }
    if(gc->RVhasRight){
      //cout<<gc->rightID.c_str()<< "   right gc"<<endl;
      createAndSendPing(secs, gc->rightID,  gc->leftRVFID, ts,timeStampLen);
    }
}
void Ping::sendMessageToTM(String &nodeID){
    int packet_len;
    WritablePacket *p;
    /********FOR THE API*********/
    unsigned char typeForAPI = PUBLISH_DATA;
    unsigned char IDLenForAPI = 2 * PURSUIT_ID_LEN / PURSUIT_ID_LEN;
    unsigned char strategy = IMPLICIT_RENDEZVOUS;
    /****************************/
    unsigned char request_type = RV_RESPONSE;

    /*allocate the packet*/

    packet_len = /*For the blackadder API*/ sizeof (typeForAPI) + sizeof (IDLenForAPI) + 2 * PURSUIT_ID_LEN + sizeof (strategy) + FID_LEN/*END OF API*/\
            /*PAYLOAD*/ + sizeof (request_type) + PURSUIT_ID_LEN;
    p = Packet::make(50, NULL, packet_len, 0);
    /*For the API*/
    memcpy(p->data(), &typeForAPI, sizeof (typeForAPI));
    memcpy(p->data() + sizeof (typeForAPI), &IDLenForAPI, sizeof (IDLenForAPI));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI), gc->nodeTMScope.c_str(), gc->nodeTMScope.length());
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length(), &strategy, sizeof (strategy));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy), gc->TMFID._data, FID_LEN);

    /*Put the payload*/
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN, &request_type, sizeof (request_type));
    memcpy(p->data() + sizeof (typeForAPI) + sizeof (IDLenForAPI) + gc->nodeTMScope.length() + sizeof (strategy) + FID_LEN + sizeof (request_type), nodeID.c_str(), PURSUIT_ID_LEN);

    p->set_anno_u32(0, RV_ELEMENT);
    output(1).push(p);
}
CLICK_ENDDECLS
EXPORT_ELEMENT(Ping)
