 #include "../include/simulator.h"
#include<cstdlib>
#include<stdio.h>
#include <queue>
#include <iostream>
#include <cstring>
#define TIMER_TIME 20.0

using namespace std;
struct pkt a_packet;
struct msg message;
struct pkt b_packet;
std::queue<msg> amessageQueue;
std::queue<msg> referenceQueue;
char emptypayload[20];
int base = 1;
int next_sequence_number = 1;
int w_size;
int expected_sequence_number = 1;
int reference_count = 0;
int second_queue = 0;
struct pkt c_packet;
int temp = 1;
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
   // cout<<"The value of check sum is"<<val_checksum<<endl;
    return val_checksum;
}

void A_output(struct msg message)
{
    
        if((next_sequence_number < (base + w_size)) && (reference_count < w_size))
            {
                referenceQueue.push(message);
                reference_count++;
                a_packet.seqnum = next_sequence_number;
                a_packet.acknum = 0;
                // cout<< "Data:"<<message.data <<endl;
                strncpy(a_packet.payload, message.data, sizeof(a_packet.payload));
                a_packet.checksum = calculate_checksum(a_packet);
                tolayer3(0, a_packet);
               // cout << "2nd loop" << endl;
                if(next_sequence_number == base)
                    starttimer(0,TIMER_TIME);
                next_sequence_number++;
            }
        else
        {
            second_queue++;
            amessageQueue.push(message);
            // cout<<"In this loop"<<endl;
        }

}
/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    //  cout<< "A input begn" << endl;
    int aexpectedchecksum = calculate_checksum(packet);
    if(aexpectedchecksum == packet.checksum)
    {
        //cout<< "A input" << endl;
        base = packet.acknum + 1;
        int count = 0;
        while(temp!=base)
        {
            referenceQueue.pop();
            reference_count--;
            temp++;
            count++;
        }
        while(count!=0)
        {
            if(second_queue>0)
            {
                struct msg dummy = amessageQueue.front();
                        //struct msg dummy = amessageQueue.front();
                        struct pkt d_packet;
                        d_packet.seqnum = next_sequence_number;
                        d_packet.acknum = 0;
                        strncpy(d_packet.payload, dummy.data, sizeof(d_packet.payload));
                        d_packet.checksum = calculate_checksum(d_packet);
                        tolayer3(0, d_packet);
                        starttimer(0, TIMER_TIME);
                         cout<< "In the A- input deepest"<<endl;
                        amessageQueue.pop();
                        second_queue--;
                        referenceQueue.push(dummy);
                        reference_count++;
                        next_sequence_number++;
                        count--;
                    }
                    else{
                        break;
                    }
        }

        if(base == next_sequence_number)
        {
            stoptimer(0);
            // cout<<"after deleting"<<endl;
           // cout<<referenceQueue.front().data<<endl;
            // reference_count += red;
            temp = base;
    }
    else
    {
        stoptimer(0);
        starttimer(0,TIMER_TIME);
    }
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    starttimer(0, TIMER_TIME);
     // cout<<"First Inside Timer"<<endl;
     // cout<<"Base"<<base<<endl;
     // cout<<"nextsn"<<next_sequence_number<<endl;
    // cout<<"Size"<<referenceQueue.size()<<endl;
    // cout<<referenceQueue.front().data<<endl;
    std::queue<msg> timer_queue;
    timer_queue = referenceQueue;
    for(int i = base; i < next_sequence_number; i++)
    {
       cout<<"Inside timer"<<endl;
        cout<<timer_queue.front().data<<endl;
        c_packet.seqnum = i;
        c_packet.acknum = 0;
        strncpy(c_packet.payload, timer_queue.front().data, sizeof(c_packet.payload));
        c_packet.checksum = calculate_checksum(c_packet);
        tolayer3(0, c_packet);
        timer_queue.pop();
    }
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    w_size = getwinsize();
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    int bexpectedchecksum = calculate_checksum(packet);
    if((packet.seqnum == expected_sequence_number) && (bexpectedchecksum == packet.checksum))
    {
       // cout<< " B data:"<< packet.payload<<endl;
        tolayer5(1, packet.payload);
      // cout<< "B loop" << endl;
        b_packet.seqnum = 0;
        b_packet.acknum = expected_sequence_number;
        strncpy(b_packet.payload,emptypayload, sizeof(b_packet.payload));
        b_packet.checksum = calculate_checksum(b_packet);
        tolayer3(1, b_packet);
        expected_sequence_number++;
    }
    else if((packet.seqnum != expected_sequence_number))
    {
        tolayer3(1, b_packet);
    }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    expected_sequence_number = 1;
    b_packet.seqnum = 0;
    b_packet.acknum = 0;
    strncpy(b_packet.payload, emptypayload, sizeof(b_packet.payload));
    b_packet.checksum = calculate_checksum(b_packet);
}
