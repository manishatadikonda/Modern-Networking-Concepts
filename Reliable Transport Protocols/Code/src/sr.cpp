#include "../include/simulator.h"
#include<cstdlib>
#include<stdio.h>
#include <queue>
#include <iostream>
#include <cstring>
#include <algorithm>
#define TIMER_TIME 100.0

using namespace std;
struct pkt a_packet;
struct msg message;
struct pkt b_packet;
vector<pkt> senderReferenceQueue;
queue<pkt> extraSenderReference;
queue<pkt> amessageQueue;
vector<pkt> senderSecondQueue;
//vector<bool> ackQueue;
int next_sequence_number;
int base;
//int seq = 0;
int w_size;
int receiver_base = 1;
int timer_started = 0;
struct time_object{
    int sequence_number;
    float beginning_time;
    int set_timer;
    int ack_rcvd;
};
char emptypayload[20];
vector<time_object> timerQueue;
vector<pkt> receiverBuffer;
int just = 0;
float min_time;
int found_minimum;
int min_pack_num;
struct sort_condition
{
    inline bool operator() (const pkt& pkt1, const pkt& pkt2){
        return (pkt1.seqnum < pkt2.seqnum);
    }
};
struct sort_condition_sender
{
    inline bool operator() (const pkt& pkt1, const pkt& pkt2){
        return (pkt1.acknum < pkt2.acknum);
    }
};

/* ******************************************************************
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
    // // // cout<<"The value of check sum is"<<val_checksum<<endl;
    return val_checksum;
}
// -------------
void A_output(struct msg message)
{
            //sender_reference_count++;
            a_packet.seqnum = next_sequence_number;
            a_packet.acknum = 0;
            // // // cout<< "Data:"<<message.data <<endl;
            strncpy(a_packet.payload, message.data, sizeof(a_packet.payload));
            a_packet.checksum = calculate_checksum(a_packet);
          //  senderReferenceQueue.push_back(a_packet);
    if((next_sequence_number < base + w_size) && (extraSenderReference.size() < w_size))
    {
        senderReferenceQueue.push_back(a_packet);
        extraSenderReference.push(a_packet);
        tolayer3(0, a_packet); //Sending to layer 3
        struct time_object packobj;
        // Setting parameters for Packoobj
        packobj.sequence_number = a_packet.seqnum;
        float present_time = get_sim_time();
        packobj.beginning_time = present_time;
        // // // cout<<"sim time 1 "<<present_time<<endl;
        packobj.set_timer = 0;
        packobj.ack_rcvd = 0;
        timerQueue.push_back(packobj); //Pushing into timer queue
        if(timer_started == 0)
        {
            // // // cout<<"Entered timer_started loop"<<endl;
            min_time = 100000;
            for(int i = 0; i < timerQueue.size(); i++)
            {
                if((timerQueue[i].beginning_time < min_time) && (timerQueue[i].set_timer == 0) && (timerQueue[i].ack_rcvd == 0))
                {
                    // // // cout<<"Entered if in A_output"<<endl;
                    found_minimum = 1;
                    min_time = timerQueue[i].beginning_time;
                    min_pack_num = timerQueue[i].sequence_number - 1;
                }
            }
                if(found_minimum == 1)
                {
                    float present_time = get_sim_time();
                    // // // cout<<"Entered fnd min in A_out"<<endl;
                    float effective_timer = TIMER_TIME - (present_time - min_time);
                    // // // cout<<"Sim time 2"<<get_sim_time()<<endl;
                    starttimer(0 , effective_timer);
                    timer_started = 1;
                    timerQueue[min_pack_num].set_timer =  1;
                    // // cout << "effective_timer" << effective_timer << endl;
                }
        }
    }
    else
    {
        // // cout<<"entered second loop"<<endl;
        amessageQueue.push(a_packet);
    }
    next_sequence_number++;
}
//Corrected A output

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    // // cout<<"entered a input"<<endl;
    int aexpectedchecksum = calculate_checksum(packet);
    int count =0;
    int index;
    if(aexpectedchecksum == packet.checksum)
    {
        // // cout<<"first if in a input"<<endl;
        // // cout<<"First loop"<<endl;
        if((base <= packet.acknum) && (packet.acknum < base+w_size) && (senderSecondQueue.size() < w_size))
        {
            for(int i =0; i<timerQueue.size();i++)
            {
                if(timerQueue[i].sequence_number == packet.acknum && timerQueue[i].ack_rcvd == 0)
                {
                    // // cout<<"Entered ack setting loop"<<endl;
                    timerQueue[i].ack_rcvd = 1;
                    senderSecondQueue.push_back(packet);
                    sort(senderSecondQueue.begin(), senderSecondQueue.end(), sort_condition_sender());
                    if(timerQueue[i].set_timer == 1)
                        {
                            // // cout<<"i:"<<i;
                            // // cout<<"timer loop"<<endl;
                            stoptimer(0);
                            //timerQueue.erase(timerQueue.begin());
                            timerQueue[i].set_timer =0;
                            timer_started = 0;
                        }
                            break;
                }
            }
               // timerQueue.insert(timerQueue.begin()+packet.acknum, packet);
                if(packet.acknum == base)
                {
                    // // cout<<"Entered if"<<endl;
                   // senderReferenceQueue.erase(senderReferenceQueue.begin());
                   // timerQueue.erase(timerQueue.begin());
                   // senderSecondQueue.erase(senderSecondQueue.begin());
                    for(int i=0;i<senderSecondQueue.size();i++)
                    {
                    if(senderSecondQueue[i].seqnum == 0 && senderSecondQueue[i].acknum == base)
                    {
                        base++;
                        count++;
                    }
                        else
                            break;
                    }
                    // // cout<<"After for"<<endl;
                    // // cout<<"count"<<count<<endl;
                    while(count!=0)
                    {
                        extraSenderReference.pop();
                        // // cout<<"Extra sender reference size"<<extraSenderReference.size()<<endl;
                        // // cout<<"first"<<endl;
                        // senderReferenceQueue.erase(senderReferenceQueue.begin());
                        senderSecondQueue.erase(senderSecondQueue.begin()+count-1);
                        // // cout<<"first"<<endl;
                    if(amessageQueue.size()>0)
                    {
                        // // cout<<"Entered second queue code"<<endl;
                            struct pkt d_packet = amessageQueue.front();
                        senderReferenceQueue.push_back(d_packet);
                            extraSenderReference.push(d_packet);
                            amessageQueue.pop();
                        tolayer3(0,d_packet);
                        struct time_object packobj;
                        packobj.sequence_number = d_packet.seqnum;
                        float present_time = get_sim_time();
                        packobj.beginning_time = present_time;
                        packobj.set_timer = 0;
                        packobj.ack_rcvd = 0;
                        timerQueue.push_back(packobj);
                            //  packobj.timeout_time = packobj.beginning_time + TIMER_TIME;
                            // // cout<<"End of timer push"<<endl;
                        if(timer_started == 0)
                        {
                            min_time = 1000000;
                            for(int i = 0; i < timerQueue.size(); i++)
                            {
                                if(timerQueue[i].beginning_time < min_time && timerQueue[i].set_timer == 0 && timerQueue[i].ack_rcvd == 0)
                                {
                                    
                                    found_minimum = 1;
                                    min_time = timerQueue[i].beginning_time;
                                    min_pack_num = timerQueue[i].sequence_number - 1;
                                }
                            }
                            // // cout<<"Inside timer loops"<<endl;
                            if(found_minimum == 1)
                            {
                                float present_time = get_sim_time();
                                float effective_timer = TIMER_TIME - (present_time - min_time);
                                // // // cout<<"Sim time 2"<<get_sim_time()<<endl;
                                starttimer(0 , effective_timer);
                                timer_started = 1;
                                timerQueue[min_pack_num].set_timer =  1;
                                // // cout << "effective_timer middle A_input" << effective_timer << endl;
                            }
                        }
                    }
                        count--;
                    }
                }
        }
    }
    // // cout<<"sender size"<<senderSecondQueue.size()<<endl;
            if(timer_started == 0 && senderSecondQueue.size()>0)
            {
                min_time = 1000000;
                for(int i = 0; i < timerQueue.size(); i++)
                {
                    if(timerQueue[i].beginning_time < min_time && timerQueue[i].set_timer == 0 && timerQueue[i].ack_rcvd == 0)
                    {
                        // // cout<<"i:"<<i;
                        // // cout<<"Set timer"<<timerQueue[i].set_timer<<endl;
                        // // cout<<"Ack rcvd"<<timerQueue[i].ack_rcvd;
                        found_minimum = 1;
                        min_time = timerQueue[i].beginning_time;
                        min_pack_num = timerQueue[i].sequence_number - 1;
                    }
                }
                if(found_minimum == 1)
                {
                    float present_time = get_sim_time();
                    float effective_timer = TIMER_TIME - (present_time - min_time);
                    // // // cout<<"Sim time 2"<<get_sim_time()<<endl;
                    starttimer(0 , effective_timer);
                    timer_started = 1;
                    timerQueue[min_pack_num].set_timer =  1;
                    // // cout << "effective_timer end A_input" << effective_timer << endl;
                }
            }
}
/* called when A's timer goes off */
//Corrected A_input
void A_timerinterrupt()
{
    int required = 0;
    timer_started = 0;
    int timer_interrupt = 0;
    for(int i=0;i<timerQueue.size();i++)
    {
        if(timerQueue[i].set_timer == 1 && timerQueue[i].ack_rcvd == 0)
        {
            // // cout<<"i:"<<i;
            // // cout<<"Set timer"<<timerQueue[i].set_timer<<endl;
            // // cout<<"Ack rcvd"<<timerQueue[i].ack_rcvd;
            //Resending that packet
            struct time_object packobj;
            required = timerQueue[i].sequence_number;
            packobj.sequence_number = timerQueue[i].sequence_number;
            float present_time = get_sim_time();
            timerQueue[i].beginning_time = present_time;
            timerQueue[i].set_timer = 0;
            timerQueue[i].ack_rcvd = 0;
           // timerQueue.erase(timerQueue.begin()+i);
           // timerQueue.push_back(packobj);
            // // cout<<"Looping in first for"<<endl;
            timer_interrupt = 1;
            break;
        }
    }
    int i=1;
    while((i<next_sequence_number) && (timer_interrupt == 1))
    {
        // // cout<<"Entered while in timer"<<endl;
        if(senderReferenceQueue[i-1].seqnum == required)
        {
            tolayer3(0, senderReferenceQueue[i-1]);
            break;
        }
        i++;
    }
    
    min_time = 1000000;
    found_minimum = 0;
    for(int i = 0; i < timerQueue.size(); i++)
    {
        if(timerQueue[i].beginning_time < min_time && timerQueue[i].set_timer == 0 && timerQueue[i].ack_rcvd == 0)
        {
            // // cout<<"Set timer"<<timerQueue[i].set_timer<<endl;
            // // cout<<"Ack rcvd"<<timerQueue[i].ack_rcvd;
            // // cout<<"Entered if in timer"<<endl;
            found_minimum = 1;
            min_time = timerQueue[i].beginning_time;
            min_pack_num = timerQueue[i].sequence_number - 1;
        }
    }
    if(found_minimum == 1)
    {
        // // cout<<"Entered fnd min in timer"<<endl;
        float present_time = get_sim_time();
        float effective_timer = TIMER_TIME - (present_time - min_time);
        // // // cout<<"Sim time 2"<<get_sim_time()<<endl;
        timer_started = 1;
        timerQueue[min_pack_num].set_timer =  1;
        // // cout << "effective_timer" << effective_timer << endl;
        starttimer(0 , effective_timer);
    }
    
}
/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    timer_started = 0;
    base = 1;
    next_sequence_number = 1;
    w_size = getwinsize();
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    int bexpectedchecksum = calculate_checksum(packet);
    if(bexpectedchecksum == packet.checksum)
    {
        // // cout<<"Entered B_input loop"<<endl;
        if((receiver_base <= packet.seqnum) && (packet.seqnum < receiver_base + w_size) && (receiverBuffer.size() < w_size))
        {
                    // // cout<<"Entered first in B_input"<<endl;
                    receiverBuffer.push_back(packet);
                    sort(receiverBuffer.begin(), receiverBuffer.end(), sort_condition());
                    b_packet.seqnum = 0;
                    b_packet.acknum = packet.seqnum;
                    strncpy(b_packet.payload, emptypayload, sizeof(b_packet.payload));
                    b_packet.checksum = calculate_checksum(b_packet);
                    tolayer3(1, b_packet);
            // Vector sort functionality
        if(packet.seqnum == receiver_base)
                {
                   // tolayer5(1, packet.payload);
                    //receiverBuffer.erase(receiverBuffer.begin());
                    int count = 0;
                    // // cout<<"Entered base loop"<<endl;
                    for(int i = 0; i<receiverBuffer.size();i++)
                    {
                        if(receiverBuffer[i].seqnum == receiver_base)
                        {
                            tolayer5(1, receiverBuffer[i].payload);
                            receiver_base++;
                            count++;
                        }
                        else
                            break;
                    }
                    // // cout<<"Exited for loop"<<endl;
                    while(count!=0)
                    {
                    // // cout<<"Erasing receiver buffer"<<endl;
                    receiverBuffer.erase(receiverBuffer.begin()+count-1);
                    count--;
                    }
                }
        }
        else if((receiver_base - w_size < packet.seqnum) && (packet.seqnum <= receiver_base-1))
        {
            // // cout<<"Entered third in B_input"<<endl;
            b_packet.seqnum = 0;
            b_packet.acknum = packet.seqnum;
            strncpy(b_packet.payload, emptypayload, sizeof(b_packet.payload));
            b_packet.checksum = calculate_checksum(b_packet);
            tolayer3(1, b_packet);
        }
        // // cout<<"Exiting end if loop"<<endl;
    }
}
//Corrected B_input
/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    receiver_base = 1;
}
