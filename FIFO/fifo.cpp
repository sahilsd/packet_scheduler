#include "sim.hpp"

int main(int argc, char* argv[]) {
    fifoqueue fifo;
    if(argc > 1)
        SEED =  atoi(argv[1]);
    else
        SEED = rand()%50;
    
    /* simulate for different total load values */
    for(M=0.4; M<=2.0; M+=0.2) {
        fifo.initSimulation(M);
        fifo.simulate(M);
        fifo.dumpStats(M);
    }

    return 0;
}
void fifoqueue::dumpStats(float M)
{
    cout<<"Dump stats for "<<M<<endl;
#ifdef DEBUG
    fprintf(stdout,"M %lf\tT %lf\tR+W %lf\tP %d\tSz %lf\tDt %lf\n", M,
            this->globalStats.totalSize / this->globalStats.lastDeparture,
            this->globalStats.totalResponse+this->globalStats.totalWait/
                    this->globalStats.totalPackets,
            (int)this->globalStats.totalPackets,
            this->globalStats.totalSize, this->globalStats.lastDeparture);
#endif
    FILE *tput, *delay;
    char fname[64];
    for(int i=0; i<NUM_SOURCES;i++) {
        snprintf(fname, 64,"%d_tput_fifo.dat", i);
        tput = fopen(fname, "a+");
        snprintf(fname, 64, "%d_delay_fifo.dat", i);
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
    fprintf(stdout,"M %1.2lf\nT %1.4lf\tR+W %8.4lf\tP %d\tSz %10.1lf\tDt %10.1lf\n", this->sourceStats[i].M,
            this->sourceStats[i].totalSize / this->sourceStats[i].lastDeparture,
            (this->sourceStats[i].totalResponse + this->sourceStats[i].totalWait)/
                            this->sourceStats[i].totalPackets,
            (int)this->sourceStats[i].totalPackets,
            this->sourceStats[i].totalSize, this->sourceStats[i].lastDeparture);
    fprintf(stdout," PlotT \t%d\tT %1.4lf\nPlotD %d\tR+W %8.4lf\t\nP %d\tSz %10.1lf\tDt %10.1lf\n", i,
            this->sourceStats[i].totalSize / this->sourceStats[i].lastDeparture, i,
            (this->sourceStats[i].totalResponse + this->sourceStats[i].totalWait)/
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
            /* deviaton = Z_{90%} * {sigma}/{n^0.5} */
            fprintf(delay, "%1.4lf\t%1.4lf\n", M, meanDelay);
            fprintf(tput, "%1.4lf\t%lf\n", M,
                   std::abs(rate - this->sourceStats[i].totalSize/this->sourceStats[i].lastDeparture));

            fclose(tput);
            fclose(delay);

    }
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
/* Update only on departure. Store packet size and delays. */
void fifoqueue::update_stats()
{
    double responseTime;
    this->lastDeparture = this->clock;
    this->globalStats.lastDeparture = this->clock;
    this->currentPackets--;
    this->globalStats.totalPackets++;

    /* global stats */
    responseTime = this->clock - this->fifoQueue.front()->arrivalTime;
    this->globalStats.totalSize += this->fifoQueue.front()->packetSize;
    this->globalStats.totalResponse += responseTime;
    this->globalStats.totalWait += responseTime - this->fifoQueue.front()->packetSize/R;

    this->globalStats.responseSqr += pow(responseTime, 2);
    this->globalStats.waitSqr += pow(responseTime - this->fifoQueue.front()->packetSize/R, 2);


    /* per source stats */
    this->sourceStats[this->fifoQueue.front()->sourceId].totalPackets++; 
    this->sourceStats[this->fifoQueue.front()->sourceId].lastDeparture = this->clock; 
    this->sourceStats[this->fifoQueue.front()->sourceId].totalSize +=
                    this->fifoQueue.front()->packetSize;
    this->sourceStats[this->fifoQueue.front()->sourceId].totalResponse += responseTime;
    this->sourceStats[this->fifoQueue.front()->sourceId].totalWait += 
                    responseTime - this->fifoQueue.front()->packetSize/R;
    
    this->sourceStats[this->fifoQueue.front()->sourceId].responseSqr += pow(responseTime, 2);
    this->sourceStats[this->fifoQueue.front()->sourceId].waitSqr +=
                    pow(responseTime - this->fifoQueue.front()->packetSize/R, 2);
    
    this->sourceStats[this->fifoQueue.front()->sourceId].totalDelaySqr +=
                    pow(responseTime+responseTime - this->fifoQueue.front()->packetSize/R , 2);
}

/* 
 * Allocates necessary resources and resets all counters to 0 
 * Also takes care of scheduling 1st arrival packet for each source
 */
void fifoqueue::initSimulation(float M)
{
    r = M/(NUM_SOURCES - 1);
    //SEED VALUE variation
    srand48(SEED);

    this->clock = 0;

    this->totalArrival = 0;
    this->currentPackets = 0;
    this->packetid.reserve(NUM_SOURCES);
    this->lastArrival.reserve(NUM_SOURCES);
    this->sourceStats.reserve(NUM_SOURCES);

    /* clear globalstats */
    this->globalStats.totalPackets = 1;
    this->globalStats.totalSize = 0;
    this->globalStats.lastDeparture = 0;
    this->globalStats.totalResponse = 0;
    this->globalStats.totalWait = 0;
    this->globalStats.responseSqr = 0;
    this->globalStats.waitSqr = 0;

    for(int i=0;i<NUM_SOURCES;i++) {
        this->packetid[i]=1;
        this->lastArrival[i] = 0;

        this->sourceStats[i].M = M;
        this->sourceStats[i].totalPackets = 1;
        this->sourceStats[i].totalResponse = 0;
        this->sourceStats[i].totalWait = 0;
        this->sourceStats[i].responseSqr = 0;
        this->sourceStats[i].waitSqr = 0;
        this->sourceStats[i].totalDelaySqr = 0;
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
        cout<<"+("<<newEvent->type<<", "<<newEvent->sourceId<<", "<<newEvent->packetId<<", "<<newEvent->packetSize<<")"<<endl;
#endif 
    }

}

/* 
 * Takes care of adding Arrival and Departure events to eventQueue.
 * Since the system keeps evolving with each event, the next events
 * are scheduled while serving the current event. Which means,
 * handling current arrival will lead to next arrival for the source.
 */
void fifoqueue::schedule_event(int type, int pkt, int src)
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
            cout<<"\tfifo "<<this->fifoQueue.size()<<"\tQsize "<<this->eventQueue.size()<<endl;
        #endif
            break;
        case PKT_DEPART:
            evt->eventTime = this->lastDeparture + 
                                this->fifoQueue.front()->packetSize/R;
            evt->packetSize = this->fifoQueue.front()->packetSize;
            evt->sourceId = this->fifoQueue.front()->sourceId;
            evt->packetId = this->fifoQueue.front()->packetId;
            evt->departureTime = evt->eventTime;
            evt->arrivalTime = this->fifoQueue.front()->arrivalTime;
            this->eventQueue.push(evt);
        #ifdef DEBUG
            cout<<"sched\t"<<evt->eventTime<<"\tD\tpkt "<<evt->packetId<<"\tsrc "<<evt->sourceId;
            cout<<"\ttime "<<evt->eventTime<<"\tQsize "<<this->eventQueue.size()<<endl;
        #endif
            break;
    }
}

/* 
 * Pop the current head of eventQueue which points to an arrival of <src,pkt>.
 * Schedule next arrival for src,pkt+1 and also very first departure.
 * Adds eventQueue head to fifoQueue.
 */
void fifoqueue::handle_arrival(int pkt_id, int src_id)
{
#ifdef DEBUG
    fprintf(stdout, "[%ld] Enqueue\t %d\tsrc %d\tpkt %d\tsz %ld\n", this->clock,
            this->eventQueue.top()->type, this->eventQueue.top()->sourceId,
            this->eventQueue.top()->packetId, this->fifoQueue.size());
#endif
    this->fifoQueue.push(this->eventQueue.top());
    this->eventQueue.pop();
#ifdef DEBUG
    cout<<this->clock<<"\tA\t"<<"pkt "<<pkt_id<<"\t src "<<src_id;
    cout<<"\tfifo "<<this->fifoQueue.size()<<"\tevent "<<this->eventQueue.size();
    cout<<"\teQ top "<<this->eventQueue.top()->sourceId<<", "<<this->eventQueue.top()->packetId;
    cout<<"\tfifo tail "<<this->fifoQueue.back()->sourceId<<", "<<this->fifoQueue.back()->packetId<<endl;
#endif

    this->currentPackets++;
    this->packetid[src_id]++;
    this->totalArrival++; /* few trivial arrival stats */

    if(this->totalArrival < SIM_LOOP)
       schedule_event(PKT_ARRIVE, this->packetid[src_id], src_id);

    if(this->currentPackets == 1) { 
       /* whenever packet count reaches 1, need to schedule it as soon as possible */
       this->lastDeparture = this->fifoQueue.front()->arrivalTime;
       schedule_event(PKT_DEPART, this->fifoQueue.front()->packetId, this->fifoQueue.front()->sourceId);
    }
}

/*
 * Remove departure event from eventQueue head and remove fifo packet.
 * Increment statistics for tracking packets, sizes and delays.
 */
void fifoqueue::handle_departure(int pkt, int src)
{
    update_stats();
#ifdef DEBUG
    fprintf(stdout, "[%ld] Dequeue\t %d\ts %d\tp %d\t sz %ld\tserved %ld\n", this->clock,
            this->fifoQueue.front()->type, this->fifoQueue.front()->sourceId,
            this->fifoQueue.front()->packetId, this->fifoQueue.size(), this->globalStats.totalPackets+1);
#endif
    this->fifoQueue.pop();
    this->eventQueue.pop();
#ifdef DEBUG
    if(this->fifoQueue.size() > 0) {
    cout<<this->clock<<" \t D \t pkt "<<pkt<<" \tsrc "<<src;
    cout<<"\tnewhead "<<this->fifoQueue.front()->sourceId<<"\tsize "<<this->fifoQueue.size()<<endl;
    }
#endif
    if(this->currentPackets >= 1) {
        schedule_event(PKT_DEPART, this->fifoQueue.front()->packetId, this->fifoQueue.front()->sourceId);
    }
}
/*
 * Iterates through complete eventQueue till it gets empty.
 * Called for every totalLoad value of M.
 */
void fifoqueue::simulate(float totalLoad)
{
    r = totalLoad/(NUM_SOURCES-1);
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

