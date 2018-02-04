#include "../include/simulator.h"
#include<cstdlib>
#include<stdio.h>
#include <queue>
#include <iostream>
#include<cstring>
#define TIMER_TIME 10.0

using namespace std;
struct pkt a_packet;
struct pkt packet;
struct msg message;
int sequence_number;
int acknowledgement_number;
int calculate_checksum(struct pkt packet);
int corrupted(struct pkt packet);
int latest_sequence_number;
static int transport = 0;
std::queue<msg> amessageQueue;
int no_sent_packets = 0;
int acorrectchecksum;
int bcorrectchecksum;
char emptypayload[20];
int counter = 0;
/********************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
 
 This code should be used for PA2, unidirectional data transfer
 protocols (from A to B). Network properties:
 - one way network delay averages five time units (longer if there
 are other messages in the channel for GBN), but can be larger
 - packets can be corrupted (either the header or the data portion)
 or lost, according to user-defined probabilities
 - packets will be delivered in the order in which they were sent
 (although some can be lost).
 **********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */

// The declaration and implementation of queue : www.stackoverflow.com
int calculate_checksum(struct pkt packet)
{
    int val_checksum = 0;
    packet.checksum = 0;
    val_checksum = val_checksum + packet.seqnum;
    val_checksum = val_checksum + packet.acknum;
    for(int i=0; i<sizeof(packet.payload); i++)
    {
        val_checksum = val_checksum + (packet.payload[i] - '0');
    }
    //cout<<"The value of check sum is"<<val_checksum<<endl;
    return val_checksum;
}

void A_output(struct msg message)
{
    // cout << counter++ << endl;

    if(!transport)
    {
        if(!amessageQueue.empty())
        {
            acknowledgement_number = 0;
            a_packet.seqnum = sequence_number;
            a_packet.acknum = acknowledgement_number;
            strncpy(a_packet.payload,amessageQueue.front().data,sizeof(amessageQueue.front().data));
            a_packet.checksum = calculate_checksum(a_packet);
            tolayer3(0, a_packet);
            transport = 1;
            //no_sent_packets++;
            starttimer(0, TIMER_TIME);
            amessageQueue.pop();
            //Remove sent element
            amessageQueue.push(message);
            //Enter recent element
        }
        else
        {
            acknowledgement_number = 0;
            a_packet.seqnum = sequence_number;
            a_packet.acknum = acknowledgement_number;
            strncpy(a_packet.payload,message.data,sizeof(message.data));
            a_packet.checksum = calculate_checksum(a_packet);
            tolayer3(0, a_packet); //Send to receiver
            transport = 1;
            //no_sent_packets++;
            starttimer(0, TIMER_TIME);
        }
    }
    else
    {
        amessageQueue.push(message);
    }
}

/*
struct pkt a_packet;
 acknowledgement_number = 0;
 a_packet.seqnum = sequence_number;
 a_packet.acknum = acknowledgement_number;
 strncpy(a_packet.payload,message.data,20);
 //a_packet.checksum = calculate_checksum(a_packet);
 tolayer3(0, a_packet);
 */



/* called from layer 3, when a packet arrives for layer 4 */

void A_input(struct pkt packet)
{
    acorrectchecksum = calculate_checksum(packet);
    if(packet.acknum == sequence_number && acorrectchecksum == packet.checksum)
    {
        stoptimer(0);
        transport = 0;
        if(sequence_number == 0)
            sequence_number = 1;
        else
            sequence_number = 0;
    }
    
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    tolayer3(0, a_packet);
    starttimer(0, TIMER_TIME);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    
    sequence_number = 0;
   // no_sent_packets = 0;
    transport = 0;
    
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    
    bcorrectchecksum = calculate_checksum(packet);
    if(bcorrectchecksum == packet.checksum && packet.seqnum != latest_sequence_number)
    {
        struct pkt acknowledgement;
       // char emptypayload[20] = "";
        acknowledgement.seqnum = 0;
        acknowledgement.acknum = packet.seqnum;
        strncpy(acknowledgement.payload,emptypayload,sizeof(emptypayload));
        acknowledgement.checksum = calculate_checksum(acknowledgement);
        tolayer3(1,acknowledgement);
    }
    else if(bcorrectchecksum == packet.checksum && packet.seqnum == latest_sequence_number)
    {
        tolayer5(1, packet.payload);
        struct pkt acknowledgement;
        acknowledgement.seqnum = 0;
        acknowledgement.acknum = packet.seqnum;
        strncpy(acknowledgement.payload,emptypayload,sizeof(emptypayload));
        acknowledgement.checksum = calculate_checksum(acknowledgement);
        tolayer3(1,acknowledgement);
        if(latest_sequence_number == 0)
            latest_sequence_number = 1;
        else
            latest_sequence_number = 0;
    }
    
    
    //tolayer5(1, packet.payload);
    
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
     latest_sequence_number = 0;
}
