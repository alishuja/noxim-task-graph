/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the processing element
 */

#ifndef __NOXIMPROCESSINGELEMENT_H__
#define __NOXIMPROCESSINGELEMENT_H__

#include <queue>
#include <systemc.h>
#include "NoximMain.h"
#include "NoximGlobalTrafficTable.h"
#include "NoximGlobalTaskGraph.h"
#include <dlfcn.h>
using namespace std;

SC_MODULE(NoximProcessingElement)
{

    // I/O Ports
    sc_in_clk clock;		// The input clock for the PE
    sc_in < bool > reset;	// The reset signal for the PE

    sc_in < NoximFlit > flit_rx;	// The input channel
    sc_in < bool > req_rx;	// The request associated with the input channel
    sc_out < bool > ack_rx;	// The outgoing ack signal associated with the input channel

    sc_out < NoximFlit > flit_tx;	// The output channel
    sc_out < bool > req_tx;	// The request associated with the output channel
    sc_in < bool > ack_tx;	// The outgoing ack signal associated with the output channel

    sc_in < int >free_slots_neighbor;

    // Registers
    int local_id;		// Unique identification number
    bool current_level_rx;	// Current level for Alternating Bit Protocol (ABP)
    bool current_level_tx;	// Current level for Alternating Bit Protocol (ABP)
    queue < NoximPacket > packet_queue;	// Local queue of packets
    bool transmittedAtPreviousCycle;	// Used for distributions with memory

    // Functions
    void rxProcess();		// The receiving process
    void txProcess();		// The transmitting process
    void tgProcess();		// Process to check if all required inputs of a taskgraph node have been recieved (only in taskgraph mode)
    void taskMappingProcess();	// Perform task mapping with external mapping module
    bool canShot(NoximPacket & packet);	// True when the packet must be shot
    NoximFlit nextFlit();	// Take the next flit of the current packet
    NoximPacket trafficRandom();	// Random destination distribution
    NoximPacket trafficTranspose1();	// Transpose 1 destination distribution
    NoximPacket trafficTranspose2();	// Transpose 2 destination distribution
    NoximPacket trafficBitReversal();	// Bit-reversal destination distribution
    NoximPacket trafficShuffle();	// Shuffle destination distribution
    NoximPacket trafficButterfly();	// Butterfly destination distribution
    NoximPacket trafficTaskgraph();	// For traffic generated by taskgraphs


    NoximGlobalTrafficTable *traffic_table;	// Reference to the Global traffic Table
    NoximGlobalTaskGraph *task_graph; 		// Reference to the Global task graph
    map <int, int > packet_received;		// Counts the number of packets received from each node (only in taskgraph mode)
    map <int, int > packet_received_average_time; //Stores average time of packet received from each node in a task graph
    map <int, int > packet_received_num;

    int processing_time_counter;		// Counts the processing time to 0
    bool processing_time_set;			// Is set to true if all taskgraph inputs are received.
    queue <int> taskgraph_output;		// For storing taskgraph outputs
    
    bool never_transmit;	// true if the PE does not transmit any packet 
    //  (valid only for the table based traffic)

    void fixRanges(const NoximCoord, NoximCoord &);	// Fix the ranges of the destination
    int randInt(int min, int max);	// Extracts a random integer number between min and max
    int getRandomSize();	// Returns a random size in flits for the packet
    void setBit(int &x, int w, int v);
    int getBit(int x, int w);
    double log2ceil(double x);

    // For critical path execution time calculation (only in taskgraph and task mapping mode)
    double first_packet_transmit_time;
    double first_packet_process_time;

    // Constructor
    SC_CTOR(NoximProcessingElement) {
	first_packet_transmit_time =-1;
	first_packet_process_time = -1;
	SC_METHOD(rxProcess);
	sensitive << reset;
	sensitive << clock.pos();

	SC_METHOD(tgProcess);
	sensitive <<reset;
	sensitive << clock.pos();
	
	SC_METHOD(taskMappingProcess);
	sensitive << reset.pos();


	SC_METHOD(txProcess);
	sensitive << reset;
	sensitive << clock.pos();
    }

};

#endif