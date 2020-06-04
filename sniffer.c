#include<stdio.h>
#include<malloc.h>
#include<string.h>
#include<signal.h>
#include<stdbool.h>
#include<sys/socket.h>
#include<sys/types.h>

#include<linux/if_packet.h>
#include<netinet/in.h>       
#include<netinet/if_ether.h>    // for ethernet header
#include<netinet/ip.h>      // for ip header
#include<netinet/udp.h>     // for udp header
#include<netinet/tcp.h>
#include<arpa/inet.h>           // to avoid warning at inet_ntoa

#include "socket.h"

int total,tcp,udp,icmp,igmp,other,iphdrlen;

struct sockaddr saddr;
struct sockaddr_in source,dest;
FILE* dumpfile;

void ethernet_header(unsigned char* buffer,int buflen)
{
    struct ethhdr *eth = (struct ethhdr *)(buffer);
    fprintf(dumpfile,"\nEthernet Header\n");
    fprintf(dumpfile,"\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
    fprintf(dumpfile,"\t|-Destination Address    : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
    fprintf(dumpfile,"\t|-Protocol       : %d\n",eth->h_proto);

}

void ip_header(unsigned char* buffer,int buflen)
{
    struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));

    iphdrlen =ip->ihl*4;

    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = ip->saddr;     
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = ip->daddr;     

    fprintf(dumpfile , "\nIP Header\n");

    fprintf(dumpfile , "\t|-Version              : %d\n",(unsigned int)ip->version);
    fprintf(dumpfile , "\t|-Internet Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)ip->ihl,((unsigned int)(ip->ihl))*4);
    fprintf(dumpfile , "\t|-Type Of Service   : %d\n",(unsigned int)ip->tos);
    fprintf(dumpfile , "\t|-Total Length      : %d  Bytes\n",ntohs(ip->tot_len));
    fprintf(dumpfile , "\t|-Identification    : %d\n",ntohs(ip->id));
    fprintf(dumpfile , "\t|-Time To Live     : %d\n",(unsigned int)ip->ttl);
    fprintf(dumpfile , "\t|-Protocol         : %d\n",(unsigned int)ip->protocol);
    fprintf(dumpfile , "\t|-Header Checksum   : %d\n",ntohs(ip->check));
    fprintf(dumpfile , "\t|-Source IP         : %s\n", inet_ntoa(source.sin_addr));
    fprintf(dumpfile , "\t|-Destination IP    : %s\n",inet_ntoa(dest.sin_addr));
}

void payload(unsigned char* buffer,int buflen)
{
    int i=0;
    unsigned char * data = (buffer + iphdrlen  + sizeof(struct ethhdr) + sizeof(struct udphdr));
    fprintf(dumpfile,"\nData\n");
    int remaining_data = buflen - (iphdrlen  + sizeof(struct ethhdr) + sizeof(struct udphdr));
    for(i=0;i<remaining_data;i++)
    {
        if(i!=0 && i%16==0)
            fprintf(dumpfile,"\n");
        fprintf(dumpfile," %.2X ",data[i]);
    }

    fprintf(dumpfile,"\n");



}

void tcp_header(unsigned char* buffer,int buflen)
{
    fprintf(dumpfile,"\n*************************TCP Packet******************************");
    ethernet_header(buffer,buflen);
    ip_header(buffer,buflen);

    struct tcphdr *tcp = (struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
    fprintf(dumpfile , "\nTCP Header\n");
    fprintf(dumpfile , "\t|-Source Port          : %u\n",ntohs(tcp->source));
    fprintf(dumpfile , "\t|-Destination Port     : %u\n",ntohs(tcp->dest));
    fprintf(dumpfile , "\t|-Sequence Number      : %u\n",ntohl(tcp->seq));
    fprintf(dumpfile , "\t|-Acknowledge Number   : %u\n",ntohl(tcp->ack_seq));
    fprintf(dumpfile , "\t|-Header Length        : %d DWORDS or %d BYTES\n" ,(unsigned int)tcp->doff,(unsigned int)tcp->doff*4);
    fprintf(dumpfile , "\t|----------Flags-----------\n");
    fprintf(dumpfile , "\t\t|-Urgent Flag          : %d\n",(unsigned int)tcp->urg);
    fprintf(dumpfile , "\t\t|-Acknowledgement Flag : %d\n",(unsigned int)tcp->ack);
    fprintf(dumpfile , "\t\t|-Push Flag            : %d\n",(unsigned int)tcp->psh);
    fprintf(dumpfile , "\t\t|-Reset Flag           : %d\n",(unsigned int)tcp->rst);
    fprintf(dumpfile , "\t\t|-Synchronise Flag     : %d\n",(unsigned int)tcp->syn);
    fprintf(dumpfile , "\t\t|-Finish Flag          : %d\n",(unsigned int)tcp->fin);
    fprintf(dumpfile , "\t|-Window size          : %d\n",ntohs(tcp->window));
    fprintf(dumpfile , "\t|-Checksum             : %d\n",ntohs(tcp->check));
    fprintf(dumpfile , "\t|-Urgent Pointer       : %d\n",tcp->urg_ptr);

    payload(buffer,buflen);

fprintf(dumpfile,"*****************************************************************\n\n\n");
}

void udp_header(unsigned char* buffer, int buflen)
{
    fprintf(dumpfile,"\n*************************UDP Packet******************************");
    ethernet_header(buffer,buflen);
    ip_header(buffer,buflen);
    fprintf(dumpfile,"\nUDP Header\n");

    struct udphdr *udp = (struct udphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
    fprintf(dumpfile , "\t|-Source Port      : %d\n" , ntohs(udp->source));
    fprintf(dumpfile , "\t|-Destination Port : %d\n" , ntohs(udp->dest));
    fprintf(dumpfile , "\t|-UDP Length       : %d\n" , ntohs(udp->len));
    fprintf(dumpfile , "\t|-UDP Checksum     : %d\n" , ntohs(udp->check));

    payload(buffer,buflen);

    fprintf(dumpfile,"*****************************************************************\n\n\n");



}



void data_process(unsigned char* buffer,int buflen)
{
    struct iphdr *ip = (struct iphdr*)(buffer + sizeof (struct ethhdr));
    ++total;
    /* we will se UDP Protocol only*/ 
    switch (ip->protocol)    //see /etc/protocols file 
    {

        case 6:
            ++tcp;
            tcp_header(buffer,buflen);
            break;

        case 17:
            ++udp;
            udp_header(buffer,buflen);
            break;

        default:
            ++other;

    }
    printf("TCP: %d  UDP: %d  Other: %d  Toatl: %d  \r",tcp,udp,other,total);


}



int main()
{

    int saddr_len, buflen;
    SOCKET socket;
    struct hostent *local_host;  //localhost

    unsigned char* buffer = (unsigned char *)malloc(65536); 
    memset(buffer,0,65536);

    dumpfile=fopen("dump.txt","w");

    if(!dumpfile)
    {
        printf("Unable to open dump.txt\n");
        return -1;

    }

    printf("Starting .... \n");

    socket = create_raw_socket();

    local_host = get_host();

    bind_socket(socket, local_host);
    
    if(socket<0)
    {
        printf("Error in socket\n");
        return -1;
    }

    while(1)
    {
        saddr_len=sizeof saddr;
        buflen=recvfrom(socket,buffer,65536,0,&saddr,(socklen_t *)&saddr_len);


        if(buflen<0)
        {
            printf("Error in reading recvfrom function\n");
            return -1;
        }
        fflush(dumpfile);
        data_process(buffer,buflen);

    }

    close(socket);// signal to close socket 
    printf("DONE!\n");

}
