#ifndef __NOXIMCOMMUNICATION_H__
#define __NOXIMCOMMUNICATION_H__

// Structure used to store information into the table
struct NoximCommunication {
    int src;			// ID of the source node (PE)
    int dst;			// ID of the destination node (PE)
    float pir;			// Packet Injection Rate for the link
    float por;			// Probability Of Retransmission for the link
    int t_on;			// Time (in cycles) at which activity begins
    int t_off;			// Time (in cycles) at which activity ends
    int t_period;		// Period after which activity starts again
};

#endif
