
#ifndef CLICK_PING_HH
#define CLICK_PING_HH

#include "globalconf.hh"
#include "helper.hh"
#include "common.hh"

#include <pthread.h>
#include <map>
CLICK_DECLS

class Ping : public Element {
public:
    /**
     * @brief Constructor: it does nothing - as Click suggests
     * @return 
     */
    Ping();
    /**
     * @brief Destructor: it does nothing - as Click suggests
     * @return 
     */
    ~Ping();

    /**
     * @brief the class name - required by Click
     * @return 
     */
    const char *class_name() const {return "Ping";}
    /**
     * @brief the port count - required by Click - it can have multiple output ports that are connected with Click "network" Elements, like ToDevice and raw sockets. 
     * It can have multiple input ports from multiple "network" devices, like FromDevice or raw sockets.
     * @return 
     */
    const char *port_count() const {return "-/-";}
    /**
     * @brief a PUSH Element.
     * @return PUSH
     */
    const char *processing() const {return PUSH;}
    /**
     * @brief Element configuration. Ping needs a pointer to the GlovalConf Element so that it can read the Global Configuration.
     * Then, there is the number of (LIPSIN) links. 
     * For each such link the Ping reads the corresponding BABitvector
     */
    int configure(Vector<String>&, ErrorHandler*);
    /**@brief This Element must be configured AFTER the GlobalConf Element
     * @return the correct number so that it is configured afterwards
     */
    int configure_phase() const{return 400;}
    /**
     * @brief This method is called by Click when the Element is about to be initialized. There is nothing that needs initialization though.
     * @param errh
     * @return 
     */
    int initialize(ErrorHandler *errh);
    /**@brief Cleanups everything. 
     * 
     * If stage >= CLEANUP_CONFIGURED (i.e. the Element was configured), 
     */
    void cleanup(CleanupStage stage);
 
    /** @brief when packets are pushed to the Ping 
     *  It can be from two ports, 0 or 1
     *  From port 0, the packet was sent from the local RV to tell Ping to 
     *  either ping a connected node or reconfigure the node and BABitvector map
     * 
     *  From port 1, the packet was sent from the network and is a ping from
     *  another node
     */
    void push(int port, Packet *p);

  void createAndSendPing(double &secs, String &nodeID, BABitvector &nodeFID, string &ts, unsigned char &timeStampLen);

    /**
     * Handles the pings from the network
     * Can be of two types, an initial ping or a return ping
     * Can be determined by the node identification in the beginning of the 
     * packet. If it is the nodeID of the current node, then it is a responding
     * ping
     */
    int handlePing(Packet *p);

    void sendPing();

    void sendMessageToTM(String &nodeID);

    /**@brief A pointer to the GlobalConf Element for reading some global node configuration.
     */
    GlobalConf *gc;
    pthread_t thread1;
    map<String, BABitvector> nodeFIDs;
    PingUpdates nodePingInfo;
};

CLICK_ENDDECLS
#endif

  
