Unused code

void failedRVReconfig(string & RVNode){

//Vertex set with a single vertex.
//int igraph_vs_1(igraph_vs_t *vs, igraph_integer_t vid);

    cout << "TM: " << igraph_vcount(&tm_igraph.graph) << " nodes" << endl;
    cout << "TM: " << igraph_ecount(&tm_igraph.graph) << " edges" << endl;

    igraph_integer_t vertex = tm_igraph.reverse_node_index[RVNode];

	int v = tm_igraph.reverse_node_index[RVNode];	
	igraph_integer_t i;


    igraph_vs_t vertexOut, vertexIn;
    igraph_vs_adj(&vertexIn, vertex, IGRAPH_IN);
    igraph_vs_adj(&vertexOut, vertex, IGRAPH_OUT);

    igraph_es_t edges, edgesIn, edgesOut;
    igraph_es_adj(&edges, vertex, IGRAPH_ALL);
    igraph_es_adj(&edgesIn, vertex, IGRAPH_IN);
    igraph_es_adj(&edgesOut, vertex, IGRAPH_OUT);
    
    //igraph_es_size(&tm_igraph.graph, &edgesOut, &i);
	//igraph_integer_t j;
    //igraph_vs_size(&tm_igraph.graph, &vertexOut, &j);	

	igraph_vit_t vit, vitParentNode;
	igraph_vit_create(&tm_igraph.graph, vertexOut, &vit);
	igraph_vit_create(&tm_igraph.graph, vertexIn, &vitParentNode);	
	long int parent = IGRAPH_VIT_GET(vitParentNode);
	long int e1 = IGRAPH_VIT_GET(vit);
	IGRAPH_VIT_NEXT(vit);
	long int e2 = IGRAPH_VIT_GET(vit);

    igraph_delete_edges(&tm_igraph.graph, edges);

	cout << "TM: " << igraph_vcount(&tm_igraph.graph) << " nodes" << endl;
    cout << "TM: " << igraph_ecount(&tm_igraph.graph) << " edges" << endl;

	igraph_add_edge(&tm_igraph.graph, parent, e1);
	tm_igraph.addToRVIgraph(e2);

    cout << "TM: " << igraph_vcount(&tm_igraph.graph) << " nodes" << endl;
    cout << "TM: " << igraph_ecount(&tm_igraph.graph) << " edges" << endl;
}

void addRVNode(string &ID, string &leftRV, string &rightRV){
    string left, right;
    RV_leaf * newNode;
    if (RVNodeMap.find(ID) == RVNodeMap.end()){
		newNode = new RV_leaf(ID, left, right);
    }else{
		newNode = RVNodeMap[ID]; 
    }
    if (!leftRV.empty()){
		if (RVNodeMap.find(leftRV) == RVNodeMap.end()){
			RV_leaf * leftRVNode = new RV_leaf(leftRV, left, right);
			leftRVNode->parent_RV = newNode;
			RVNodeMap[leftRV] = leftRVNode;
			newNode->addChild(leftRVNode, 0);
		}else{
			RV_leaf * leftRVNode = RVNodeMap[leftRV];
			leftRVNode->parent_RV = newNode;
			newNode->addChild(leftRVNode, 0);
		}
    }
    if (!rightRV.empty()){
		if (RVNodeMap.find(rightRV) == RVNodeMap.end()){
			RV_leaf * rightRVNode = new RV_leaf(rightRV, left, right);
			rightRVNode->parent_RV = newNode;
			RVNodeMap[rightRV] = rightRVNode;
			newNode->addChild(rightRVNode, 1);
		}else{
			RV_leaf * rightRVNode = RVNodeMap[rightRV];
			rightRVNode->parent_RV = newNode;
			newNode->addChild(rightRVNode, 1);
		} 
	}

    RVNodeMap[ID] = newNode;
    if (master->self_ID.empty()){
		master = newNode;
    }else if(leftRV == master->self_ID || rightRV == master->self_ID){
		master = newNode;
    }
}
