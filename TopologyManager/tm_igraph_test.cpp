
void TMIgraph::calculateFID(string &source, string &destination, Bitvector &resultFID, unsigned int &numberOfHops) {
    int vertex_id;
    igraph_vs_t vs;
    igraph_vector_ptr_t res;
    igraph_vector_t to_vector;
    igraph_vector_t *temp_v;
    igraph_integer_t eid;

    /*find the vertex id in the reverse index*/
    int from = (*reverse_node_index.find(source)).second;
    igraph_vector_init(&to_vector, 1);
    VECTOR(to_vector)[0] = (*reverse_node_index.find(destination)).second;
    /*initialize the sequence*/
    igraph_vs_vector(&vs, &to_vector);
    /*initialize the vector that contains pointers*/
    igraph_vector_ptr_init(&res, 1);
    temp_v = (igraph_vector_t *) VECTOR(res)[0];
    temp_v = (igraph_vector_t *) malloc(sizeof (igraph_vector_t));
    VECTOR(res)[0] = temp_v;
    igraph_vector_init(temp_v, 1);
    /*run the shortest path algorithm from "from"*/
    igraph_get_shortest_paths(&graph, &res, from, vs, IGRAPH_OUT);
    /*check the shortest path to each destination*/
    temp_v = (igraph_vector_t *) VECTOR(res)[0];

    /*now let's "or" the FIDs for each link in the shortest path*/
    for (int j = 0; j < igraph_vector_size(temp_v) - 1; j++) {
        igraph_get_eid(&graph, &eid, VECTOR(*temp_v)[j], VECTOR(*temp_v)[j + 1], true);
        Bitvector *lid = (*edge_LID.find(eid)).second;
        (resultFID) = (resultFID) | (*lid);
    }
    numberOfHops = igraph_vector_size(temp_v);

    /*now for the destination "or" the internal linkID*/
    Bitvector *ilid = (*nodeID_iLID.find(destination)).second;
    (resultFID) = (resultFID) | (*ilid);
    //cout << "FID of the shortest path: " << resultFID.to_string() << endl;
    igraph_vector_destroy((igraph_vector_t *) VECTOR(res)[0]);
    igraph_vector_destroy(&to_vector);
    igraph_vector_ptr_destroy_all(&res);
    igraph_vs_destroy(&vs);
}

void TMIgraph::failedNodeReconfig(string & node){
    cout<<"entering failed reconfig phase"<<endl;

    if (reverse_node_index.find(node) == reverse_node_index.end()){
      //this node isn't even part of the topology
      //error, or this is a repeat message from another part of the graph
        cout<<"couldn't find node"<<endl;
    }else{
        igraph_vs_t vertexOut;
	int i = reverse_node_index[node];
	igraph_vs_adj(&vertexOut, i, IGRAPH_OUT);
	igraph_integer_t j;
	igraph_vs_size(&graph, &vertexOut, &j);
	if (j<=1){
  	    cout<<"it is a leaf node"<<endl;
	    //igraph_es_t edges;
	    //igraph_es_adj(&edges, i, IGRAPH_ALL);
	    //igraph_delete_edges(&graph, edges);
	    //message about how one of the neighbors got deleted?? Otherwise no action is needed. Edge is kept for now because I need to figure out how to delete an edge without shifting all the edges IDs in the igraph
	}else{
	    //get all the graph ids of nodes connected to failed node
	    igraph_vit_t vit;
            igraph_vit_create(&graph, vertexOut, &vit);
            vector<long int> vertexID;
	    int size_response = 0;

	    string failedIP = nodeID_IP[node];
	    unsigned char failedIPLen = failedIP.length();

	    vector< pair<string, string> > oldIP_LID;
            while(!IGRAPH_VIT_END(vit)){
              vertexID.push_back(IGRAPH_VIT_GET(vit));
	      IGRAPH_VIT_NEXT(vit);
	      igraph_integer_t eid;
	      igraph_get_eid(&graph, &eid, i, IGRAPH_VIT_GET(vit), true);
	      string LID1 = string(igraph_cattribute_EAS(&graph, "LID", eid));
	      string nID = string(igraph_cattribute_VAS(&graph, "NODEID", eid));
	      string IP = nodeID_IP[nID];
	      oldIP_LID.push_back(make_pair(LID1, IP));
	      size_response+= LID1.length() + sizeof(unsigned char) + IP.length() + sizeof(unsigned char);
	      igraph_get_eid(&graph, &eid, IGRAPH_VIT_GET(vit), i, true);
	      string LID2 = string(igraph_cattribute_EAS(&graph, "LID", eid));
	      nID = string(igraph_cattribute_VAS(&graph, "NODEID", eid));
	      IP = nodeID_IP[nID];	      
	      oldIP_LID.push_back(make_pair(LID2, IP));
	      size_response+= LID2.length() + sizeof(unsigned char) + IP.length() + sizeof(unsigned char);
            }
	   
	    int vertex_id;
	    igraph_vs_t vs;
	    igraph_vector_ptr_t res;
	    igraph_vector_t to_vector;
	    igraph_vector_t *temp_v;
	    igraph_integer_t eid;

			//find the best new star node that is connected to the failed node
            long int from = (*reverse_node_index.find(node)).second;
            igraph_vector_init(&to_vector, 1);
            VECTOR(to_vector)[0] = (*reverse_node_index.find(nodeID)).second;
            igraph_vs_vector(&vs, &to_vector);
            igraph_vector_ptr_init(&res, 1);
            temp_v = (igraph_vector_t *) VECTOR(res)[0];
            temp_v = (igraph_vector_t *) malloc(sizeof (igraph_vector_t));
            VECTOR(res)[0] = temp_v;
            igraph_vector_init(temp_v, 1);
            igraph_get_shortest_paths(&graph, &res, from, vs, IGRAPH_OUT);
            temp_v = (igraph_vector_t *) VECTOR(res)[0];

	    //iterate through all connected vertex
            long int vertexNext =  VECTOR(*temp_v)[1];
 
	    //iterate through all connected edges
	    //TODO figure out how to delete the edges while keeping track of the edge ids
	    igraph_es_t edges;
	    igraph_es_adj(&edges, i, IGRAPH_ALL);
	    igraph_eit_t eit;
	    igraph_eit_create(&graph, edges, &eit);

	    string oldLID;
	    unsigned char oldLIDLength;

	    while(!IGRAPH_EIT_END(eit)){
	        long int edgeID = IGRAPH_EIT_GET(eit);
			Bitvector* lid =  (*edge_LID.find(edgeID)).second;
			//edge_LID.erase(edgeID);
			//reverse_edge_index.erase(lid->to_string());
	        IGRAPH_EIT_NEXT(eit);
	    }

            //igraph_delete_edges(&graph, edges);

	    //update RV configuration
	    if (RVNodeMap.find(node) != RVNodeMap.end()){
	    	string a = failedRVReconfig(node);
	    }

        long int newStar = vertexNext;
	    int response_size = 0;
        unsigned char numIPaddresses = 0;
	    // (node,(IP, LID))
        vector<pair<string, pair<string, string> > > IPs;

            /*response
                this all goes to the localProxy and it knows that it was from just a network publication
                1) code, ie define #NEWCONNECTIONS
                2) number of IP addresses
                3) length of nodeID
                4) nodeID
                5) length of IP
                6) IP
                7) repeat for each IP that needs to be sent over
            */
	    
	    //assign new LIDs for the new edges added
	    //TODO make sure new LIDs aren't the same as ones in click config file
	    int LIDcounter =0;
	    int totalLIDs =  (vertexID.size()-1)*2;
	    vector<Bitvector> LIDs(totalLIDs);
	    for (int j= 0 ; j< totalLIDs; j++){
	        calculateLID(LIDs, j);
	    }

	    unsigned char response_type = NEWCONNECTIONS_NODE;
	    unsigned char numOfResponse = 1;
	    unsigned char lenOfNodeID, IPlen, LIDlen;
	    string IP, LID;

	    Bitvector *lid;
        for (int j= 0 ; j< vertexID.size(); j++){
            long int newConnection = vertexID.at(j);
            if (newConnection != vertexNext){
                igraph_add_edge(&graph, newConnection, vertexNext);
                igraph_add_edge(&graph, vertexNext, newConnection);

                string newNodeID = node_index[newConnection];
			    IP = nodeID_IP[nodeID];
			    IPlen = IP.length();
			    LID = LIDs.at(LIDcounter).to_string();
			    lid = new Bitvector(LID);
			    //TODO get rid of magic number, LIDlen in chuncks of fid_len total LIDlen = 8*fid_len
			    int temp = LID.length()/fid_len;
			    LIDlen = temp;
			    string LIDtoStar = LIDs.at(LIDcounter+1).to_string();
			    
			    igraph_integer_t eid;
			    igraph_get_eid(&graph, &eid, newConnection, vertexNext, true);
			    reverse_edge_index.insert(pair<string, int>(LID, eid));
			    edge_LID.insert(pair<int, Bitvector *>(eid, lid));
			    igraph_cattribute_EAS_set(&graph, "LID", eid, LID.c_str());
	
			    lid = new Bitvector(LIDtoStar);
			    igraph_get_eid(&graph, &eid, vertexNext, newConnection, true);
			    reverse_edge_index.insert(pair<string, int>(LIDtoStar, eid));
			    edge_LID.insert(pair<int, Bitvector *>(eid, lid));		    
			    igraph_cattribute_EAS_set(&graph, "LID", eid, LIDtoStar.c_str());
	
			    lenOfNodeID = newNodeID.length();
			    int lenOfArray = sizeof(response_type) + sizeof(numOfResponse) + sizeof(failedIPLen) + failedIPLen + sizeof(IPlen) + IPlen + sizeof(LIDlen) + LID.length();
                char *responseSmall = (char *) malloc(lenOfArray);

                memcpy(responseSmall, &response_type, sizeof (response_type));
                memcpy(responseSmall + sizeof (response_type), &numOfResponse, sizeof (numOfResponse));
			    memcpy(responseSmall + sizeof (response_type) + sizeof (numOfResponse), &failedIPLen , sizeof(failedIPLen));
                memcpy(responseSmall + sizeof (response_type) + sizeof (numOfResponse) + sizeof(failedIPLen), failedIP.c_str(), failedIP.length());
			    memcpy(responseSmall + sizeof (response_type) + sizeof (numOfResponse) + sizeof(failedIPLen) + failedIPLen, &IPlen, sizeof(IPlen));
			    memcpy(responseSmall + sizeof (response_type) + sizeof (numOfResponse) + sizeof(failedIPLen) + failedIPLen + sizeof(IPlen), IP.c_str(), IPlen);
			    memcpy(responseSmall + sizeof (response_type) + sizeof (numOfResponse) + sizeof(failedIPLen) + failedIPLen + sizeof(IPlen) + IPlen, &LIDlen, sizeof(LIDlen));
			    memcpy(responseSmall + sizeof (response_type) + sizeof (numOfResponse) + sizeof(failedIPLen) + failedIPLen + sizeof(IPlen) + IPlen + sizeof(LIDlen), LID.c_str(), LID.length());
			    numIPaddresses ++;
                response_size +=   sizeof(IPlen) + IPlen + sizeof(LIDlen) + LID.length();
                whatImReturning.push_back(make_pair(make_pair(newNodeID, lenOfArray), responseSmall));
                IPs.push_back(make_pair(newNodeID, make_pair(nodeID_IP[newNodeID], LIDtoStar)));
			    LIDcounter+=2;
                }
            }
	    
	    	response_type = NEWCONNECTIONS_NODE;
	    	response_size += sizeof(response_type) + sizeof(numIPaddresses)+ sizeof(failedIPLen) + failedIPLen;
            char *response = (char *) malloc(response_size);           
	    	string newStarNodeID = node_index[vertexNext];

	    	lenOfNodeID = nodeID.length();
            memcpy(response, &response_type, sizeof (response_type));
            memcpy(response + sizeof (response_type), &numIPaddresses, sizeof (numIPaddresses));
	    	memcpy(response + sizeof (response_type) + sizeof (numIPaddresses), &failedIPLen, sizeof(failedIPLen));
	    	memcpy(response + sizeof (response_type) + sizeof (numIPaddresses) + sizeof(failedIPLen), failedIP.c_str(), failedIPLen);

	    	int index = 0;
            for (int k = 0; k < IPs.size(); k++){
                string newNodeID = IPs.at(k).first;
				unsigned char newNodeLen = newNodeID.length();
               	string IP = IPs.at(k).second.first;
				unsigned char IPlen = IP.length();
				string LID = IPs.at(k).second.second;
				//TODO get rid of magic number, LIDlen in chuncks of fid_len total LIDlen = 8*fid_len
				int temp = (LID.length()/fid_len);
				unsigned char LIDlen = temp;
                memcpy(response + sizeof (response_type) + sizeof (numIPaddresses) + sizeof(failedIPLen) + failedIPLen + index , &IPlen, sizeof(IPlen));
                memcpy(response + sizeof (response_type) + sizeof (numIPaddresses) + sizeof(failedIPLen) + failedIPLen + index + sizeof(IPlen), IP.c_str(), IPlen);
				memcpy(response + sizeof (response_type) + sizeof (numIPaddresses) + sizeof(failedIPLen) + failedIPLen + index + sizeof(IPlen) + IPlen, &LIDlen, sizeof(LIDlen));
				memcpy(response + sizeof (response_type) + sizeof (numIPaddresses) + sizeof(failedIPLen) + failedIPLen + index + sizeof(IPlen) + IPlen + sizeof(LIDlen), LID.c_str(), LID.length());
				index+=  sizeof(IPlen) + IPlen + sizeof(LIDlen) + LID.length();
            }
            whatImReturning.push_back(make_pair(make_pair(newStarNodeID, response_size), response));

	    
	    	response_type = NEWCONNECTIONS_STAR;
	   		size_response += sizeof(response_type) + sizeof(numIPaddresses);

            char *response1 = (char *) malloc(size_response);         
	    	memcpy(response, &response_type, sizeof (response_type));
            memcpy(response + sizeof (response_type), &numIPaddresses, sizeof (numIPaddresses));
	    	memcpy(response + sizeof (response_type) + sizeof (numIPaddresses), &failedIPLen, sizeof(failedIPLen));
	    	memcpy(response + sizeof (response_type) + sizeof (numIPaddresses) + sizeof(failedIPLen), failedIP.c_str(), failedIPLen);

	    	int indexA = 0;
	    	for (int j = 0; j < oldIP_LID.size(); j++){
	        	string IP = oldIP_LID.at(i).first;
				unsigned char IPlen = IP.length();
				string LID = oldIP_LID.at(i).second;
				int temp = (LID.length()/fid_len);
				unsigned char LIDlen = temp;
				memcpy(response + sizeof (response_type) + sizeof (numIPaddresses) + sizeof(failedIPLen) + failedIPLen + index , &IPlen, sizeof(IPlen));
                memcpy(response + sizeof (response_type) + sizeof (numIPaddresses) + sizeof(failedIPLen) + failedIPLen + index + sizeof(IPlen), IP.c_str(), IPlen);
				memcpy(response + sizeof (response_type) + sizeof (numIPaddresses) + sizeof(failedIPLen) + failedIPLen + index + sizeof(IPlen) + IPlen, &LIDlen, sizeof(LIDlen));
				memcpy(response + sizeof (response_type) + sizeof (numIPaddresses) + sizeof(failedIPLen) + failedIPLen + index + sizeof(IPlen) + IPlen + sizeof(LIDlen), LID.c_str(), LID.length());
				index+=  sizeof(IPlen) + IPlen + sizeof(LIDlen) + LID.length();
		    }
		    whatImReturning.push_back(make_pair(make_pair(node_index[vertexNext], size_response), response1));
        }
    }
}
