/*
 * Author: Sahil Deshpande
 */

#include<iostream>
#include<vector>
#include<string>
#include<algorithm>
#include<cmath>
#include<queue>
#include<stdio.h>

using namespace std;

//events
#define PKT_ARRIVE 1 
#define PKT_DEPART 2 

//sources
#define NUM_SOURCES 11
#define L_TELNET    512     //(bits)
#define L_FTP       8192    //(bits)
#define L_ROGUE     5000    //(bits)
#define TELNET_SRC  4
#define FTP_SRC     6
#define ROGUE_SRC   1
#define QUANTUM     100     //(bits)

#define Z_CRITICAL  1.645 //multiplier for 90% confidence interval

enum {
    TELNET1,
    TELNET2,
    TELNET3,
    TELNET4,
    FTP1,
    FTP2,
    FTP3,
    FTP4,
    FTP5,
    FTP6,
    ROGUE
};

//constants
#define R           1       //(bps)
#define SIM_LOOP    100000
#define INVAL       -1
#define INIT        0
//#define SEED        10
int SEED;
//log files
enum logtype {LOG_ERROR, LOG_WARNING, LOG_INFO};
#define RR_LOG        "./rr.log"
#define DRR_LOG       "./drr.log"

//global var
float r;
double M;

//inline



//class declarations
//events class object forms priority queue
class events {
public:
    int type;
    int sourceId;
    int packetId;
    int packetSize;
    unsigned long eventTime;
    unsigned long arrivalTime;
    unsigned long departureTime;
    //constructor
    events(int t, int sr, int p, int sz, unsigned long ev):type(t), sourceId(sr), packetId(p), packetSize(sz), eventTime(ev){}
//    events() : type(0), sourceId(0), packetId(0), packetSize(0), eventTime(0){}
    //compare
};

//statistics
class stats {
public:
    //global stats
    float M;
    unsigned long totalPackets; //served
    double totalResponse;
    double totalWait;
    double responseSqr;
    double waitSqr;
    double totalSize;
    double lastDeparture;
public:
    stats():M(0), totalPackets(0), totalResponse(0), totalWait(0), totalSize(0), lastDeparture(INVAL){}
    //update stats - LOG
    //print
    //plot
    //~stats();
};

//packets form fifo queue
struct compare : public std::binary_function<events*, events*, bool>
{
    bool operator()(const events* l, const events* g)const
    {
        if(l->eventTime > g->eventTime) return 1;
        if(l->eventTime == g->eventTime && 
                l->arrivalTime > g->arrivalTime) return 1;
        if(l->eventTime == g->eventTime &&
                l->arrivalTime == g->arrivalTime &&
                l->departureTime > g->departureTime) return 1;

        return 0;
    }
};

class stateRR {
public:
    long clock;
    long totalArrival;
    long currentPackets;
    vector<long> packetid;
    vector<long> lastArrival;
    long lastDeparture;

    vector<stats> sourceStats;  //per source stats
    stats globalStats;          //global stats
    std::priority_queue<events*, vector<events*>, compare> eventQueue;
    std::vector<std::queue<events *> > flows;   //circles between src # 0 to 10
    
    void initSimulation(float);
    void simulate(float);
    void handle_arrival(int, int);
    void handle_departure(int, int);
    void schedule_event(int , int , int );
    void update_stats(int);
    void dumpStats(float);
    //void plotStats(void);
};


