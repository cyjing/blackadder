/*
 * Copyright (C) 2010-2011  George Parisis and Dirk Trossen
 * All rights reserved.
 * TCLAP and PlanetLab support By Dimitris Syrivelis
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of
 * the BSD license.
 *
 * See LICENSE and COPYING for more details.
 */

#include "network.hpp"
#include "graph_representation.hpp"
#include "parser.hpp"
#include "client_exec.hpp"
#include <tclap/CmdLine.h>


using namespace std;

int main(int argc, char **argv) {
    int ret;

    string filename;
    string transferfilename;
    string experimentfile;
    string tgzfile;
    bool autogenerate = false;
    bool experiment_deploy = false;
    bool transfer_binaries = false;
    bool monitor_tool_stub = false;

    /**parse command line block based on TCLAP (template lib)
     */
    try {
        TCLAP::CmdLine cmd("BlackAdder Universal Deployment Tool ", ' ', "0.2");
        TCLAP::ValueArg<std::string> configfileArg("c", "configfile", "Configuration file that contains graph attributes OR describes a graph (when -a is not used)", true, "homer", "string");
        TCLAP::ValueArg<std::string> experimentfileArg("d", "experimentfile", "Experiment description deployment file that contains information to remotely deploy applications and locally collect their STDOUT", false, "None", "string");
        TCLAP::SwitchArg autoSwitch("a", "auto", " Enable graph autogeneration - a autogenerated.cfg and edgevertices.cfg files are emitted at WRITE_CONF folder. The former contains the graph to repeat the experiment and the later the leaf nodes", cmd, false);
        TCLAP::ValueArg<std::string> tgzfileArg("t", "tgzfile", "tar gzipped file that gets transferred and extracted at USER home folders on all experiment targets", false, "None", "string");
        TCLAP::SwitchArg MonToolStubSwitch("m", "montoolstub", " Enable Java monitor tool stub in the Click configuration files. This injects counters in click configs	that are inspected at runtime via port 55555. It will be ommited for kernel versions", cmd, false);

        cmd.add(configfileArg);
        cmd.add(experimentfileArg);
        cmd.add(tgzfileArg);

        cmd.parse(argc, argv);
        /**get experiment file argument
         */
        experimentfile = experimentfileArg.getValue();
        /**if experiment file argument is not the default value the experiment deploy flag should be true
         */
        if (experimentfile != string("None")) {
            experiment_deploy = true;
        }
        /**get config filename. This is mandatory argument so we can skip existence check
         */
        filename = configfileArg.getValue();
        /** check if autogenerate flag was set
         */
        autogenerate = autoSwitch.getValue();
        tgzfile = tgzfileArg.getValue();
        /**if experiment file argument is not the default value the experiment deploy flag should be true
         */
        if (tgzfile != string("None")) {
            transfer_binaries = true;
        }
        /**Get monitor tool stub flag
         */
        monitor_tool_stub = MonToolStubSwitch.getValue();


    } catch (TCLAP::ArgException &e)
        /**catch any command parsing exceptions
         */ {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }

    /**if autogenerated flag is set, a autogenerated file is built and it is then loaded with the proper chain of commands  
     */
    string autoconffile = "";
    if (autogenerate) {
        /**create an empty network representation. 
         */
        Domain pdm = Domain();
        /**create a parser object. the configuration file and the network domain are the parameters. 
         */
        Parser parserp(((char *) filename.c_str()), &pdm);
        /**Build PlanetLab Domain out of the planetlab available node file input
         */
        ret = parserp.buildPlanetLabDomain();
        /**Global domain parameteres should be explicitly called at this stage
         */
        ret = parserp.getGlobalDomainParameters();
        /**Check for errors
         */
        if (ret < 0) {
            cout << "Something went wrong" << endl;
            return EXIT_FAILURE;
        }
        /**create a graph representation of the network domain. if autogenerated is true, the an igraph instance will be now created using the Barabasi-Albert model 
         */
        GraphRepresentation graphp = GraphRepresentation(&pdm, autogenerate);
        /** the igraph barabasi instance previously created will be traversed, to build 
            the deployment tool internal representation so that click configuration file generation and deployment execution
            can be carried out by the same code with the non-autogenerated graph input. 
         */
        graphp.BuildInputMap();
        graphp.ChooseTheBestTMRVNode();
        /**Write albert-barabasi autogenerated standard configuration file format
         */
        autoconffile = pdm.writeConfigFile("autogenerated.cfg");
        /**Calculate leaf Vertices
         */
        graphp.OutputLeafVertices("edgevertices.cfg");

    }

    /**create an empty network representation. 
     */
    Domain dm = Domain();
    /**create a parser object. the configuration file and the network domain are the parameters. 
     */
    string parserinputfile = "";
    if (autogenerate) {
        parserinputfile = autoconffile;
    } else {
        parserinputfile = filename;
    }
    Parser parser(((char *) parserinputfile.c_str()), &dm);
    /**In the typical scenario parse the configuration file and build the network domain (add nodes and connections).
     */
    ret = parser.buildNetworkDomain();
    /** Check for errors
     */
    if (ret < 0) {
        cout << "Something went wrong" << endl;
        return EXIT_FAILURE;
    }
    /**create a graph representation of the network domain. if autogenerated is true, the an igraph instance will be now created using the Barabasi-Albert model 
     */
    GraphRepresentation graph = GraphRepresentation(&dm, autogenerate);
    /**assign Link Identifiers and internal link identifiers using a randomly generated set of LIDs.
     */
    dm.assignLIDs();
    /**transform the network domain representation to iGraph representation.
     */
    graph.buildIGraphTopology();

    /**Calculate the default forwarding identifiers from each node to the domain's Rendezvous Node.
     */
    graph.calculateRVFIDs();

	graph.buildRVIGraphTopology();

    /**Calculate the default forwarding identifiers from each node to the domain's Topology Manager.
     */
    graph.calculateTMFIDs();
	/**Calculate forwarding identifiers from each RV node to parent,master,and any children
     */
    graph.calculateRVNodesRecursive();
    /**discover the MAC addresses (when needed) for each connection in the network domain.
     */
    dm.discoverMacAddresses();
    /**write all Click/Blackadder Configuration files.
     */
    dm.writeClickFiles(monitor_tool_stub);
    /**Tranfer a .tgz file from path to all nodes and also issue a tar zxvf command.
     */
    if (transfer_binaries) {
        dm.scpClickBinary(tgzfile);
    }
    /**copy the Click configuration to each blackadder node.
     */
    dm.scpClickFiles();
    /**Start Click using the copied configuration file.
     */
    dm.startClick();
    /**set some graph attributes for the topology manager
     */
    igraph_cattribute_GAN_set(&graph.igraph, "FID_LEN", dm.fid_len);
    igraph_cattribute_GAS_set(&graph.igraph, "TM", dm.TM_node->label.c_str());
    cout << "TM is " << dm.TM_node->label << endl;
    igraph_cattribute_GAS_set(&graph.igraph, "TM_MODE", dm.TM_node->running_mode.c_str());
    FILE * outstream_graphml = fopen(string(dm.write_conf + "topology.graphml").c_str(), "w");
    igraph_write_graph_graphml(&graph.igraph, outstream_graphml);
    fclose(outstream_graphml);

	igraph_cattribute_GAN_set(&graph.igraphRV, "FID_LEN", dm.fid_len);
    igraph_cattribute_GAS_set(&graph.igraphRV, "MASTER_RV", dm.Master_RV_node->label.c_str());
    FILE * outstream_RVgraphml = fopen(string(dm.write_conf + "RVtopology.graphml").c_str(), "w");
    igraph_write_graph_graphml(&graph.igraphRV, outstream_RVgraphml);
    fclose(outstream_RVgraphml);


    /** Copy the .graphml file to the Topology Manager node.
     */
    dm.scpTMConfiguration("topology.graphml");
    /**Start the Topology Manager at the right node.
     */
    cout << experimentfile;
    if (experiment_deploy) {
        ClientExec cex(&dm);
        cex.LoadCfg(string(experimentfile));
        cex.DeployExperiment();
    }
    //dm.startTM();
}