#include "sim.hpp"

int main(int argc, char* argv[]) {
    stateDRR state;
    if(argc > 1)
        SEED =  atoi(argv[1]);
    else
        SEED = rand()%50;
    
    M = 0.4;
    for(M=0.4; M<=2.0; M+=0.2) {
        state.initSimulation(M);
        state.simulate(M);
        state.dumpStats(M);
    }

    return 0;
}
void stateDRR::dumpStats(float M)
{
    cout<<"Dump stats for "<<M<<endl;
#ifdef DEBUG
    fprintf(stdout,"M %1.2lf\tT %1.2lf\tR %lf\tP %d\tSz %lf\tDt %lf\n", M,
            this->globalStats.totalSize / this->globalStats.lastDeparture,
            this->globalStats.totalResponse/this->globalStats.totalPackets,
            (int)this->globalStats.totalPackets,
            this->globalStats.totalSize, this->globalStats.lastDeparture);
#endif

    FILE *tput, *delay;
    char fname[64];
    for(int i=0; i<NUM_SOURCES;i++) {
        snprintf(fname, 64,"%d_tput_drr.dat", i);
        tput = fopen(fname, "a+");
        snprintf(fname, 64, "%d_delay_drr.dat", i);
        delay = fopen(fname, "a+");
        double rate;
        switch(i) {
            case TELNET1:
            case TELNET2:
            case TELNET3:
            case TELNET4:
                rate = M/10;
                break;
            case FTP1:
            case FTP2:
            case FTP3:
            case FTP4:
            case FTP5:
            case FTP6:
                rate = M/10;
                break;
            case ROGUE:
                rate = 0.5;
                break;
        }
#ifdef DEBUG
        fprintf(stdout,"M %1.2lf\tT %1.4lf\tR+W %8.4lf\tP %d\tSz %10.1lf\tDt %10.1lf\n", this->sourceStats[i].M,
            this->sourceStats[i].totalSize / this->sourceStats[i].lastDeparture,
            (this->sourceStats[i].totalResponse +  this->sourceStats[i].totalWait)/
                            this->sourceStats[i].totalPackets,
            (int)this->sourceStats[i].totalPackets,
            this->sourceStats[i].totalSize, this->sourceStats[i].lastDeparture);
        fprintf(stdout,"M %1.2lf\nPlotT\t%d\t%1.4lf\nPlotD\t%d\t%8.4lf\nP %d\tSz %10.1lf\tDt %10.1lf\n", this->sourceStats[i].M,i,
            this->sourceStats[i].totalSize / this->sourceStats[i].lastDeparture, i,
            (this->sourceStats[i].totalResponse +  this->sourceStats[i].totalWait)/
                            this->sourceStats[i].totalPackets,
            (int)this->sourceStats[i].totalPackets,
            this->sourceStats[i].totalSize, this->sourceStats[i].lastDeparture);
#endif

        double meanResponse = (this->sourceStats[i].totalResponse)/this->sourceStats[i].totalPackets;
        double meanWait = (this->sourceStats[i].totalWait)/this->sourceStats[i].totalPackets;
        double meanDelay = meanResponse+meanWait;
        double varResponse = ((this->sourceStats[i].responseSqr)/this->sourceStats[i].totalPackets)
                                - pow(meanResponse, 2);
        double varWait = ((this->sourceStats[i].waitSqr)/this->sourceStats[i].totalPackets)
                                - pow(meanWait, 2);
        fprintf(delay, "%1.4lf\t%1.4lf\n", M, meanDelay);
            
        fprintf(tput, "%1.4lf\t%lf\n", M, 
                std::abs(rate - (this->sourceStats[i].totalSize/this->sourceStats[i].lastDeparture)));
    
        fclose(tput);
        fclose(delay);
    }
    /* resets everything for next run */
    packetid.clear();
    lastArrival.clear();

    sourceStats.clear();
    flows.clear();
}
int exp_rand(int mean)
{
    int a=0;
    double b=0;
    while(a == 0 )
    {
        b = drand48();
        a = ( -1*log(b)*mean);
    }
    return a;
}
/* Only on departure */
void stateDRR::update_stats(int src)
{
    double responseTime;
    this->lastDeparture = this->clock;
    this->globalStats.lastDeparture = this->clock;
    this->currentPackets--;
    this->globalStats.totalPackets++;

    responseTime = this->clock - this->flows[src].front()->arrivalTime;
    this->globalStats.totalSize += this->flows[src].front()->packetSize;
    this->globalStats.totalResponse += responseTime;
    this->globalStats.totalWait += responseTime - this->flows[src].front()->packetSize/R;
    this->globalStats.responseSqr += pow(responseTime, 2);
    this->globalStats.waitSqr += pow(responseTime - this->flows[src].front()->packetSize/R, 2);
    

/* source stats */
    this->sourceStats[src].totalPackets++; 
    this->sourceStats[src].lastDeparture = this->clock; 
    this->sourceStats[src].totalSize +=
                    this->flows[src].front()->packetSize;
    this->sourceStats[src].totalResponse += responseTime;
    this->sourceStats[src].totalWait += 
                    responseTime - this->flows[src].front()->packetSize/R;
    
    this->sourceStats[src].responseSqr += pow(responseTime,2);
    this->sourceStats[src].waitSqr += pow(responseTime- this->flows[src].front()->packetSize/R, 2);
}

void stateDRR::initSimulation(float M)
{
    std::queue<events *> eq;
    r = M/(NUM_SOURCES - 1);
    //SEED VALUE variation
    srand48(SEED);

    this->clock = 0;

    this->totalArrival = 0;
    this->currentPackets = 0;
    this->packetid.reserve(NUM_SOURCES);
    this->lastArrival.reserve(NUM_SOURCES);
    this->sourceStats.reserve(NUM_SOURCES);
    this->deficit.reserve(NUM_SOURCES);

    this->flows.reserve(NUM_SOURCES);
    /* clear globalstats */
    this->globalStats.totalPackets = 0;
    this->globalStats.totalSize = 0;
    this->globalStats.lastDeparture = 0;
    this->globalStats.totalResponse = 0;
    this->globalStats.totalWait = 0;

    for(int i=0;i<NUM_SOURCES;i++) {
        flows.push_back(queue<events *>());
        this->deficit[i] = 0;   //set to zero and later check for (deficit[i]+Q)
        this->packetid[i]=1;
        this->lastArrival[i] = 0;

        this->sourceStats[i].M = M;
        this->sourceStats[i].totalPackets = 1;
        this->sourceStats[i].totalResponse = 0;
        this->sourceStats[i].totalWait = 0;
        this->sourceStats[i].responseSqr = 0;
        this->sourceStats[i].waitSqr = 0;
        this->sourceStats[i].totalSize = 0;
        this->sourceStats[i].lastDeparture = 0;

        events *newEvent = new events(PKT_ARRIVE, i, this->packetid[i], 0, 0);
        switch(i) {
            case TELNET1:
            case TELNET2:
            case TELNET3:
            case TELNET4:
                newEvent->packetSize = exp_rand(L_TELNET);
                break;
            case FTP1:
            case FTP2:
            case FTP3:
            case FTP4:
            case FTP5:
            case FTP6:
                newEvent->packetSize = exp_rand(L_FTP);
                break;
            case ROGUE:
                newEvent->packetSize = L_ROGUE;
                break;
        }
        newEvent->eventTime = 0;
        newEvent->arrivalTime = newEvent->eventTime;
        newEvent->departureTime = INVAL;

        /* Update stats */
        this->totalArrival++;
        
        this->eventQueue.push(newEvent);
#ifdef DEBUG
        cout<<"+("<<newEvent->type<<", "<<newEvent->sourceId<<", "<<newEvent->packetId<<", "<<newEvent->packetSize<<")";
#endif
    }

}
/*
 * For arrival events, use exponential distribution.
 * For departure events, function assumes <src,pkt> have sufficient deficits.
 */
void stateDRR::schedule_event(int type, int pkt, int src)
{
    events *evt = new events(type, src, pkt, 0, 0);

    switch(type) {
        case PKT_ARRIVE:
            switch(src) {
                case TELNET1:
                case TELNET2:
                case TELNET3:
                case TELNET4:
            #ifdef DEBUG
                    cout<<"Arr "<<this->lastArrival[src]<<", "<<exp_rand(L_TELNET/r)<<endl;
            #endif
                    evt->eventTime = this->lastArrival[src] + exp_rand(L_TELNET/r);
                    this->lastArrival[src] = evt->eventTime;
                    evt->packetSize = exp_rand(L_TELNET);
                    break;
                case FTP1:
                case FTP2:
                case FTP3:
                case FTP4:
                case FTP5:
                case FTP6:
                    evt->eventTime = this->lastArrival[src] + exp_rand(L_FTP/r);
                    this->lastArrival[src] = evt->eventTime;
                    evt->packetSize = exp_rand(L_FTP);
                    break;
                case ROGUE:
                    evt->eventTime = this->lastArrival[src] + exp_rand(L_ROGUE*2/ROGUE_SRC);
                    this->lastArrival[src] = evt->eventTime;
                    evt->packetSize = exp_rand(L_ROGUE);
                    break;
            }
            evt->arrivalTime = evt->eventTime;
            evt->departureTime = INVAL;
            
            this->eventQueue.push(evt);
#ifdef DEBUG
            cout<<"sched\t"<<evt->eventTime<<"\tA\tpkt "<<evt->packetId<<"\tsrc "<<evt->sourceId<<"\ttime "<<evt->eventTime;
            cout<<"\tQsize "<<this->eventQueue.size()<<endl;
#endif
            break;
        case PKT_DEPART:
            evt->eventTime = this->lastDeparture + 
                                this->flows[src].front()->packetSize/R;
            evt->packetSize = this->flows[src].front()->packetSize;
            evt->sourceId = src;
            evt->packetId = this->flows[src].front()->packetId;
            evt->departureTime = evt->eventTime;
            evt->arrivalTime = this->flows[src].front()->arrivalTime;
            this->eventQueue.push(evt);
#ifdef DEBUG
            cout<<"sched\t"<<evt->eventTime<<"\tD\tpkt "<<evt->packetId<<"\tsrc "<<evt->sourceId;
            cout<<"\ttime "<<evt->eventTime<<"\tQsize "<<this->eventQueue.size()<<endl;
#endif
            break;
    }
}

void stateDRR::handle_arrival(int pkt_id, int src_id)
{
#ifdef DEBUG
    fprintf(stdout, "[%ld] Enqueue\t %d\tsrc %d\tpkt %d\tsz %ld\n", this->clock,
            this->eventQueue.top()->type, this->eventQueue.top()->sourceId,
            this->eventQueue.top()->packetId, this->flows[src_id].size());
#endif
    this->flows[src_id].push(this->eventQueue.top());
    this->eventQueue.pop();
#ifdef DEBUG
    cout<<this->clock<<"\tA\t"<<"pkt "<<pkt_id<<"\t src "<<src_id;
    cout<<"\tflow "<<this->flows[src_id].size()<<"\tevent "<<this->eventQueue.size();
    cout<<"\teQ top "<<this->eventQueue.top()->sourceId<<", "<<this->eventQueue.top()->packetId<<endl;
#endif
    this->currentPackets++;
    this->packetid[src_id]++;
    this->totalArrival++;

    if(this->totalArrival <= SIM_LOOP)
       schedule_event(PKT_ARRIVE, this->packetid[src_id], src_id);

    if(this->currentPackets == 1) {
        /* 
         * Since DRR is also work conserving like RR, when only one packet is
         * in the system, no need to check the turn. Serve the solitary packet.
         */
        while(this->deficit[src_id] < this->flows[src_id].front()->packetSize) {
            deficit[src_id] += QUANTUM;
        }

        this->lastDeparture = this->flows[src_id].front()->arrivalTime;
        this->deficit[src_id] -=  this->flows[src_id].front()->packetSize;
        schedule_event(PKT_DEPART, this->flows[src_id].front()->packetId, src_id);
        
    }
}

void stateDRR::handle_departure(int pkt, int src)
{
    update_stats(src);
#ifdef DEBUG
    fprintf(stdout, "[%ld] Dequeue\t %d\ts %d\tp %d\t sz %ld\tserv %ld\tdef %d\n", this->clock,
            this->flows[src].front()->type, this->flows[src].front()->sourceId,
            this->flows[src].front()->packetId, this->flows[src].size(), 
            this->globalStats.totalPackets, this->deficit[src]);
#endif
    this->flows[src].pop();
    this->eventQueue.pop();
#ifdef DEBUG
    if(this->flows[src].size() > 0) {
    cout<<this->clock<<" \t D \t pkt "<<pkt<<" \tsrc "<<src<<endl;
    }
#endif
    
    if(this->currentPackets >= 1) {
        /* When there are multiple packets,must schedule next departure 
         * to follow work-preserving model.
         * Next departure must be from the next immediate active flow. 
         * When number of flows are small, we can quickly iterate over instead of storing separate list*/
        while(1) {
            /* loop over until next departure scheduled*/
            src = (src+1)%(NUM_SOURCES);
            while(flows[src].size() == 0) {
#ifdef DEBUG 
                cout<<"[Dequeue]\tEmpty "<<src<<" def"<<this->deficit[src]<<endl;
#endif
                src = (src+1)%(NUM_SOURCES);
            }
            /* increment deficit for each active  flow _before_ scheduling */
            this->deficit[src] += QUANTUM;
            /* check for deficit of the active flow */
            if(this->deficit[src] >= this->flows[src].front()->packetSize) {
                schedule_event(PKT_DEPART, this->flows[src].front()->packetId, src);
                this->deficit[src] -=  this->flows[src].front()->packetSize;
                break;
            }
#ifdef DEBUG
            cout<<"[Dequeue]\tSkipped: "<<src<<" def: "<<this->deficit[src]<<" pkt ";
            cout<<this->flows[src].front()->packetSize<<endl;
#endif
        }
    }
}
void stateDRR::simulate(float totalLoad)
{
    r = totalLoad/10;
    cout<<"Simulate for r "<<r<<" seed "<<SEED<<endl;
    while(this->eventQueue.empty() == false) {
        events* eventHead = this->eventQueue.top();
        this->clock = eventHead->eventTime;
        switch(eventHead->type) {
            case PKT_ARRIVE:
                handle_arrival(eventHead->packetId, eventHead->sourceId);
                break;
            case PKT_DEPART:
                this->lastDeparture = this->clock;

                this->lastDeparture = this->clock;
                handle_departure(eventHead->packetId, eventHead->sourceId);
                break;
            default:
                cout<<"Invalid event"<<endl;
                break;
        }
    }
}

