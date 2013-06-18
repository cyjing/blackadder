all:
	$(CXX) $(CXXFLAGS) bitvector.cpp graph_representation.cpp parser.cpp network.cpp client_exec.cpp deploy.cpp  -o deploy $(LDFLAGS) -lblackadder -lconfig++ -ligraph
	
	$(CXX) $(CXXFLAGS) bitvector.cpp graph_representation.cpp tm_igraph.cpp tmRearrange.cpp -o tmRearrange $(LDFLAGS) -lblackadder -lconfig++ -ligraph
	

clean:
	rm -f deploy
