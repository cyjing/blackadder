/*
* Copyright (C) 2010-2011  George Parisis and Dirk Trossen
* All rights reserved.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License version
* 2 as published by the Free Software Foundation.
*
* Alternatively, this software may be distributed under the terms of
* the BSD license.
*
* See LICENSE and COPYING for more details.
*/
#include "globalconf.hh"

CLICK_DECLS

GlobalConf::GlobalConf() {

}

GlobalConf::~GlobalConf() {
    click_chatter("GlobalConf: destroyed!");
}

int GlobalConf::configure(Vector<String> &conf, ErrorHandler *errh) {
    String mode;
    String defRVFID;
    String internalLID;
    
    String masterRV = String();
    String parentRV = String();
    String leftRV = String();
    String rightRV = String();
    
    masterID = String();
    parentID = String();
    leftID = String();
    rightID = String();

    String TMFID_str = String();
    click_chatter("*******************************************************GLOBAL CONFIGURATION*******************************************************");
    if (cp_va_kparse(conf, this, errh,
            "MODE", cpkM, cpString, &mode,
            "NODEID", cpkM, cpString, &nodeID,
            "DEFAULTRV", cpkM, cpString, &defRVFID,
            "iLID", cpkM, cpString, &internalLID,
	    "masterRVFID", cpkN, cpString, &masterRV,
	    "masterID", cpkN, cpString, &masterID,
	    "parentRVFID", cpkN, cpString, &parentRV,
	    "parentID", cpkN, cpString, &parentID,
	    "leftRVFID", cpkN, cpString, &leftRV,
            "leftID", cpkN, cpString, &leftID,
            "rightRVFID", cpkN, cpString, &rightRV,
            "rightID", cpkN, cpString, &rightID,
            "TMFID", cpkN, cpString, &TMFID_str,
            cpEnd) < 0) {
        return -1;
    }

    masterRVFID = BABitvector(FID_LEN * 8);
    parentRVFID = BABitvector(FID_LEN * 8);
    leftRVFID = BABitvector(FID_LEN * 8);
    rightRVFID = BABitvector(FID_LEN * 8);

    if (masterRV.compare("none")!=0){
      if( initializeFIDConfig(masterRV, masterRVFID)<0){
	click_chatter("fail master rv");
	return -1;
      }else{
	isRV = true;
      }
    }else{
      isRV = false;
    }
    if(parentRV.compare("none")!=0){
      if (initializeFIDConfig(parentRV, parentRVFID)<0){
	click_chatter("fail parent rv");
	return -1;
      }    
    }
    if (leftRV.compare("none")==0){
        RVhasLeft = false;
    }else{
        RVhasLeft = true;
        if (initializeFIDConfig(leftRV, leftRVFID)<0){
	  click_chatter("fail left rv");
	  return -1;
	}
    }
    if (rightRV.compare("none")==0){
        RVhasRight = false;
    }else{
        RVhasRight = true;
        if (initializeFIDConfig(rightRV, rightRVFID)<0){
	  click_chatter("fail right rv");
	  return -1;
	}
    }


    if (mode.compare(String("mac")) == 0) {
        use_mac = true;
        //click_chatter("Forwarder will run using ethernet frames");
    } else if (mode.compare(String("ip")) == 0) {
        use_mac = false;
        //click_chatter("Forwarder will run using ip raw sockets");
    } else {
        errh->fatal("wrong MODE argument");
        return -1;
    }
    
    /*create the RV root scope: /FFFFFFFF.....depending on the PURSUIT_ID_LEN*/
    RVScope = String();
    for (int j = 0; j < PURSUIT_ID_LEN; j++) {
        RVScope += (char) 255;
    }
    TMScope = String();
    for (int j = 0; j < PURSUIT_ID_LEN - 1; j++) {
        TMScope += (char) 255;
    }
    TMScope += (char) 254;
    notificationIID = String();
    for (int j = 0; j < PURSUIT_ID_LEN - 1; j++) {
        notificationIID += (char) 255;
    }
    notificationIID += (char) 253;
    notificationIID += nodeID;
    click_chatter("GlobalConf: NodeID: %s", nodeID.c_str());
    if (TMFID_str.length() != 0) {
        TMFID = BABitvector(FID_LEN * 8);
	if (initializeFIDConfig(TMFID_str, TMFID)<0){
	  click_chatter("fail tm rv");
	  return -1;
	}
        click_chatter("GlobalConf: FID to the TM: %s", TMFID.to_string().c_str());
    }

    if (nodeID.length() != NODEID_LEN) {
        errh->fatal("NodeID should be %d bytes long...it is %d bytes", NODEID_LEN, nodeID.length());
        return -1;
    }
    defaultRV_dl = BABitvector(FID_LEN * 8);
    if (initializeFIDConfig(defRVFID, defaultRV_dl)<0)
	  return -1;

    iLID = BABitvector(FID_LEN * 8);
    if (initializeFIDConfig(internalLID, iLID)<0)
	  return -1;

    click_chatter("GlobalConf: RV Scope: %s", RVScope.quoted_hex().c_str());
    click_chatter("GlobalConf: TM scope: %s", TMScope.quoted_hex().c_str());
    nodeRVScope = RVScope + nodeID;
    nodeTMScope = TMScope + nodeID;
    click_chatter("GlobalConf: Information ID to publish RV requests: %s", nodeRVScope.quoted_hex().c_str());
    click_chatter("GlobalConf: Information ID to publish TM requests: %s", nodeTMScope.quoted_hex().c_str());
    click_chatter("GlobalConf: Information ID to subscribe for receiving all notifications: %s", notificationIID.quoted_hex().c_str());
    click_chatter("GlobalConf: default FID for RendezVous node %s", defaultRV_dl.to_string().c_str());
    if (defaultRV_dl == iLID) {
        click_chatter("GlobalConf: I am the RV node for this domain");
    }
    //click_chatter("GlobalConf: configured!");
    return 0;
}
int GlobalConf::initializeFIDConfig(String rvNode, BABitvector &node){
    if (rvNode.length()!= FID_LEN * 8) {
      click_chatter(" should be %d bits...it is %d bits", FID_LEN * 8, rvNode.length());
      return -1;
    }

    for (int j = 0; j < rvNode.length(); j++) {
       if (rvNode.at(j) == '1') {
	 node[rvNode.length() - j - 1] = true;
       } else {
	 node[rvNode.length() - j - 1] = false;
       }
    }
  
  return 0;
}
int GlobalConf::initialize(ErrorHandler *errh) {
    //click_chatter("GlobalConf: initialized!");
    return 0;
}

void GlobalConf::cleanup(CleanupStage stage) {
    click_chatter("GlobalConf: Cleaned Up!");
}

CLICK_ENDDECLS
EXPORT_ELEMENT(GlobalConf)
