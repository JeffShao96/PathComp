/*
 * writes a udp packet and sends it through raw socket
 *
 * compile: gcc -g raw.c -o raw
 * 
 * This is working version of the raw socket, which creates
 * a UDP datagram through raw socket and send it to server
 *
 * */

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/ip.h>
#include <netinet/udp.h>

#include <string.h>

#define MESG_LENGTH 978
#define DEFAULT_TTL 60
#define BUFLEN 1542
#define PORT 9932
#define PORT1 9932
int breakout = 0, i;
char *serverIP; //定义指针变量
void udp_gen(char *packet, unsigned short sport,
			 unsigned short dport, unsigned short length);
void ip_gen(char *packet, unsigned char protocol, struct in_addr saddr,
			struct in_addr daddr, unsigned short length);
unsigned short in_cksum(unsigned short *addr, int len);
double receive();
void send_msg(char *host, double rate, int port);
int main(int argc, char *argv[])
{

	unsigned char packet[sizeof(struct iphdr) + sizeof(struct udphdr) + MESG_LENGTH];

	struct in_addr saddr, daddr;
	unsigned short sport, dport;
	struct sockaddr_in mysocket;
	struct udphdr *udphdr;
	int sockd, on = 1;

	sport = (unsigned short)atoi("2222");
	saddr.s_addr = inet_addr(argv[1]);
	dport = (unsigned short)atoi("9001");
	daddr.s_addr = inet_addr(argv[2]);
	//初始化发送端口和接收端口/地址
	printf("debug: create raw socket");

	if ((sockd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
	{
		perror("socket");
		exit(1);
	}

	if (setsockopt(sockd, IPPROTO_IP, IP_HDRINCL, (char *)&on, sizeof(on)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}
	//设置Socket，如果失败返回非0，可以通过WSAGetLastError()查看原因
	printf("before ip_gen");

	ip_gen(packet, IPPROTO_UDP, saddr, daddr, sizeof(packet));

	udphdr = (struct udphdr *)(packet + sizeof(struct iphdr));

	memset((packet + sizeof(struct udphdr) + sizeof(struct iphdr)),
		   '0', MESG_LENGTH);
	sprintf(packet + sizeof(struct udphdr) + sizeof(struct iphdr), "1");
	udp_gen((char *)udphdr, sport, dport,
			(sizeof(struct udphdr) + MESG_LENGTH));

	printf("sport: %d\n", ntohs(udphdr->source));
	printf("dport: %d\n", ntohs(udphdr->dest));
	printf("len: %d\n", udphdr->len);
	printf("check: %d\n", udphdr->check);

	memset(&mysocket, '\0', sizeof(mysocket));

	mysocket.sin_family = AF_INET;
	mysocket.sin_port = htons(dport);
	mysocket.sin_addr = daddr;

	printf("debug: before sendto\n");
	int i = 0;
	char *id = packet + sizeof(struct udphdr) + sizeof(struct iphdr);
	int timespend = 0;
	while (1)
	{
		int r = (int)receive();
		send_msg(serverIP, 1, 9932);
		struct timeval t1, t2, t3, t4;
		if (r == 1)
		{
			//Test for RTT
			gettimeofday(&t3, NULL);
			sprintf(id, "%d", -1);
			if (sendto(sockd, &packet, sizeof(packet), 0x0,
					   (struct sockaddr *)&mysocket,
					   sizeof(mysocket)) != sizeof(packet))
			{
				perror("sendto");
				exit(1);
			}
			gettimeofday(&t4, NULL);
			while ((t4.tv_sec - t3.tv_sec) * 1000000 + t4.tv_usec - t3.tv_usec < 10)
				gettimeofday(&t4, NULL);
		}
		else if (r > 1)
		{
			printf("Phase 1 recev %d\n", r);
			gettimeofday(&t1, NULL);
			for (i = 1; i <= r; i++)
			{
				gettimeofday(&t3, NULL);
				sprintf(id, "%d", -i);
				if (sendto(sockd, &packet, sizeof(packet), 0x0,
						   (struct sockaddr *)&mysocket,
						   sizeof(mysocket)) != sizeof(packet))
				{
					perror("sendto");
					exit(1);
				}
				gettimeofday(&t4, NULL);
				while ((t4.tv_sec - t3.tv_sec) * 1000000 + t4.tv_usec - t3.tv_usec < 10)
					gettimeofday(&t4, NULL);
			}
			//检查是否发送超时
			gettimeofday(&t2, NULL);
			sleep(1);
			send_msg(serverIP, 1, 9001);
			timespend = (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;
			printf("Phase 1 takes time %d\n", timespend);
		}
		else
		{
			r = -r;
			printf("Phase 2 recev %d\n", r);
			int count = 0;
			int interval = 40 * timespend / r;
			struct timeval t5, t6;
			gettimeofday(&t1, NULL);
			gettimeofday(&t5, NULL);
			for (i = 1; i <= r; i++)
			{
				if(count<19){
					gettimeofday(&t3, NULL);
					sprintf(id, "%d", -i);
					if (sendto(sockd, &packet, sizeof(packet), 0x0,
							   (struct sockaddr *)&mysocket,
							   sizeof(mysocket)) != sizeof(packet))
					{
						perror("sendto");
						exit(1);
					}
					count++;
					gettimeofday(&t4, NULL);
					while ((t4.tv_sec - t3.tv_sec) * 1000000 + t4.tv_usec - t3.tv_usec < 10)
						gettimeofday(&t4, NULL);
				}else{
					count = 0;
					gettimeofday(&t3, NULL);
					sprintf(id, "%d", -i);
					if (sendto(sockd, &packet, sizeof(packet), 0x0,
							   (struct sockaddr *)&mysocket,
							   sizeof(mysocket)) != sizeof(packet))
					{
						perror("sendto");
						exit(1);
					}
					gettimeofday(&t4, NULL);
					while ((t4.tv_sec - t3.tv_sec) * 1000000 + t4.tv_usec - t3.tv_usec < 10)
						gettimeofday(&t4, NULL);
					gettimeofday(&t6, NULL);
					//int blocktime = (t6.tv_sec - t5.tv_sec) * 1000000 + t6.tv_usec - t5.tv_usec;
					while ((t6.tv_sec - t5.tv_sec) * 1000000 + t6.tv_usec - t5.tv_usec < interval)
						gettimeofday(&t6, NULL);
					//printf("BlockTime %d\n", blocktime);
					gettimeofday(&t5, NULL);
				}
			}
			//检查是否发送超时
			gettimeofday(&t2, NULL);
			sleep(1);
			send_msg(serverIP, 1, 9001);
			timespend = (t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec;
			printf("Phase 2 takes time %d\n", timespend);
		}
	}
	exit(0);
}
double receive()
{
	struct sockaddr_in my_addr, cli_addr;
	int sockfd, i;
	double r;
	socklen_t slen;

	char buf[BUFLEN];
	slen = sizeof(cli_addr);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		err("socket");
	//    else
	//      printf("Server : Socket() successful\n");

	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT1);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
		err("bind 1");
	//    printf("Server : bind() successful\n");
	i = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *)&cli_addr, &slen);
	serverIP = inet_ntoa(cli_addr.sin_addr);
	//    printf("server IP is %s\n", serverIP);
	if (i < 0)
		err("recvfrom()");
	//无信号输入（经常发生，可能是RTT较长及电脑性能受限，运行时时常在复杂topo图中无法快速接收到Receiver发出的指令）
	sscanf(buf, "%lf", &r);
	//     printf("%lf\n",r);
	close(sockfd);
	return r;
	//返回接受到的指令
}
void send_msg(char *host, double rate, int port)
{
	struct sockaddr_in serv_addr;

	int sockfd, i, slen = sizeof(serv_addr);
	char buf[BUFLEN];
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		err("socket");
	//检查Socket是否创建成功
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	if (inet_aton(host, &serv_addr.sin_addr) == 0)
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	//用于将String的IP地址转换为32位的网络序列IP地址，失败返回 0
	sprintf(buf, "%lf", rate);
	if (sendto(sockfd, buf, 100, 0, (struct sockaddr *)&serv_addr, slen) == -1)
		//用于检查是否发送成功，成功返回发送字节数量，失败返回-1
		err("sendto()");
	close(sockfd);
	return;
}
void ip_gen(char *packet, unsigned char protocol, struct in_addr saddr,
			struct in_addr daddr, unsigned short length)
{

	struct iphdr *iphdr;

	iphdr = (struct iphdr *)packet;
	memset((char *)iphdr, '\0', sizeof(struct iphdr));

	iphdr->ihl = 5;
	iphdr->version = IPVERSION;

#ifdef IP_LEN_HORDER
	iphdr->tot_len = length;
#else
	iphdr->tot_len = htons(length);
#endif

	iphdr->id = htons(getpid());
	iphdr->ttl = DEFAULT_TTL;
	iphdr->protocol = protocol;
	iphdr->saddr = saddr.s_addr;
	iphdr->daddr = daddr.s_addr;
	iphdr->check = (unsigned short)in_cksum(
		(unsigned short *)iphdr,
		sizeof(struct iphdr));

	return;
	//定义发送的TCP/IP数据报头
}

void udp_gen(char *packet, unsigned short sport,
			 unsigned short dport, unsigned short length)
{
	struct udphdr *udp;

	udp = (struct udphdr *)packet;

	udp->source = htons(sport);
	udp->dest = htons(dport);
	udp->len = htons(length);
	udp->check = 0;

	return;
	//定义发送的UDP数据报头
}

unsigned short in_cksum(unsigned short *buf, int len)
{
	unsigned long sum;
	for (sum = 0; len > 0; len--)
		sum += *buf++;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	return (unsigned short)(~sum);
	//CheckSum检查
}
