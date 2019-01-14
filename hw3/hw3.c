#include<stdio.h>
#include <time.h>
#include <sys/time.h>
#include<string.h>
#include<stdlib.h>
#include<pcap.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <netinet/ip.h>	
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

static void dump_ethernet(u_int32_t length, const u_char *content){
	struct ether_header *ethernet = (struct ether_header *)content;
	u_int16_t type = ntohs(ethernet->ether_type);	 
	switch (type) {
	    case ETHERTYPE_IP:
		dump_ip(length, content);
		break;	
	}
}


void dump_ip(u_int32_t length, const u_char *content){
	char src_ip[INET_ADDRSTRLEN] = {0};
	char dst_ip[INET_ADDRSTRLEN] = {0};
	struct ip *ip = (struct ip *)(content + ETHER_HDR_LEN);
	snprintf(src_ip, sizeof(src_ip), "%s", inet_ntoa(ip->ip_src));
	snprintf(dst_ip, sizeof(dst_ip), "%s", inet_ntoa(ip->ip_dst));
	
	printf("\tSrc. IP Address:  %s\n", src_ip);
	printf("\tDes. IP Address:  %s\n", dst_ip);

	u_char protocol = ip->ip_p;
	switch (protocol) {
	    case IPPROTO_UDP:
		dump_udp(length, content);
		break;
		 
	    case IPPROTO_TCP:
		dump_tcp(length, content);
		break;
	    
	}
}

void dump_udp(u_int32_t length, const u_char *content) {
    struct ip *ip = (struct ip *)(content + ETHER_HDR_LEN);
    struct udphdr *udp = (struct udphdr *)(content + ETHER_HDR_LEN + (ip->ip_hl << 2)); //取得UDP封包要先略過ethernet跟ip層其中ip層長度是ip_hl的4被

    u_int16_t src_port = ntohs(udp->uh_sport);
    u_int16_t des_port = ntohs(udp->uh_dport);
    printf("\tProtocol :\tUDP\n");	    
    printf("\tSrc. Port:%u\n",src_port);
    printf("\tDes. Port:%u\n",des_port);
  
}

void dump_tcp(u_int32_t length, const u_char *content) {
    struct ip *ip = (struct ip *)(content + ETHER_HDR_LEN);
    struct tcphdr *tcp = (struct tcphdr *)(content + ETHER_HDR_LEN + (ip->ip_hl << 2));

    u_int16_t src_port = ntohs(tcp->th_sport);
    u_int16_t des_port = ntohs(tcp->th_dport);
    printf("\tProtocol :  TCP\n");
    printf("\tSrc. Port:  %u\n",src_port);
    printf("\tDes. Port:  %u\n",des_port);
    
}

	
int main(int argc , char *argv[]){
	char errbuf[PCAP_ERRBUF_SIZE];
	const char *file = strdup(*(argv+1));
	int limit = 0;
	printf("agrc:%d\n",argc);
	if(argc == 3)limit = atoi(argv[2]);
	printf("limit:%d\n",limit);
	pcap_t *handle = pcap_open_offline(file,errbuf);

	if(!handle){
		fprintf(stderr,"pcap_open_offline: %s\n",errbuf);
		exit(1);
	}
	printf("Open: %s\n",file);
	
	while(1){
		
		static int cnt = 0;
		struct pcap_pkthdr *header = NULL;
		const u_char *content = NULL;
		int ret = pcap_next_ex(handle,&header,&content);
		
		if(ret == 1){		
			printf("No. %d\n", ++cnt);
			struct tm *ltime;
			char timestr[16];
			time_t ht;
			 
			ht = header->ts.tv_sec;
			ltime = localtime(&ht);
			strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);
			 
			//print header
			printf("\tTime: %s.%.6d\n", timestr, (int)header->ts.tv_usec);
			printf("\tLength: %d bytes\n", header->len);
			printf("\tCapture length: %d bytes\n", header->caplen);
			 
			//dump ethernet
			dump_ethernet(header->caplen, content);
		        printf("\n\n");

			if(limit > 0)if(limit == cnt)break;	

		}else if(ret == 0){
			printf("Timeout\n");
		}else if(ret == -1){
			fprintf(stderr,"pcap_next_ex(): %s\n",pcap_geterr(handle));
		}else if(ret == -2){
			printf("No more packet from file\n");
			break;
		}
	}	
	pcap_close(handle);
}
