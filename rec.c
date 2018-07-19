#include <pcap.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>

#define BUFLEN 1050
#define PORT 9932
#define PORT1 9001
typedef int bool;
#define TRUE 1
#define FALSE 0

struct record_info
{
	long src;
	int seq;
	long t1;
	long t2;
};
struct record_info info[250000];
void err(char *s)
{
	perror(s);
	exit(1);
}
struct timeval tod[5000], tio[5000];
int count = 0;

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
	my_addr.sin_port = htons(PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
		err("bind 1");
	//    printf("Server : bind() successful\n");
	i = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *)&cli_addr, &slen);
	//serverIP=inet_ntoa(cli_addr.sin_addr);
	//    printf("server IP is %s\n", serverIP);
	if (i < 0)
		err("recvfrom()");
	sscanf(buf, "%lf", &r);
	//     printf("%lf\n",r);
	close(sockfd);
	return r;
}

void send_msg(char *host, double rate)
{
	struct sockaddr_in serv_addr;

	int sockfd, i, slen = sizeof(serv_addr);
	char buf[BUFLEN];
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		err("socket");

	bzero(&serv_addr, sizeof(serv_addr));
	//bzero
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	if (inet_aton(host, &serv_addr.sin_addr) == 0)
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	//判断是否成功把IP专成数字形式
	sprintf(buf, "%lf", rate);
	if (sendto(sockfd, buf, BUFLEN, 0, (struct sockaddr *)&serv_addr, slen) == -1)
		err("sendto()");
	close(sockfd);
	return;
}

int absolute(int a, int b)
{
	if (a > b)
		return a - b;
	else
		return b - a;
}

// void inter_pkts_num(int info[], int len, double *p1, double *p2)
// {

//check the multiple peaks

// 	if (count > 1)
// 	{
// 		printf("%f, %f\n", m1, m2);
// 		*p1 = m1;
// 		*p2 = m2;
// 	}
// 	else
// 	{
// 		printf("%f, %f\n", m1, m2);
// 		*p1 = m1;
// 		*p2 = m2;
// 	}
// }

int main(int argc, char *argv[])
{
	int i, id, j = 0;
	struct timeval t1, t2, t3, t4;
	int negpktsize;
	char *serverIP;
	struct sockaddr_in my_addr, cli_addr;
	int sockfd;
	double r;
	socklen_t slen;
	char buf[BUFLEN];
	struct timeval first, curr;
	slen = sizeof(cli_addr);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		err("socket");
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT1);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
		err("bind");
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
				   sizeof(timeout)) < 0)
		perror("setsockopt failed\n");

	int pktsize;
	long rtt1[10], rtt2[10];
	double s1 = 0, s2 = 0;
	count = 0;
	for (i = 0; i < 10; i++)
	{
		gettimeofday(&t1, NULL);
		send_msg(argv[1], 1); //atoi(argv[3])
		j = recvfrom(sockfd, buf, BUFLEN, 0, NULL, 0);
		if (j < 0)
			printf("recvfrom\n");
		//没有接收到信息，连接失败
		else
		{
			gettimeofday(&t2, NULL);
			rtt1[i] = t2.tv_usec - t1.tv_usec;
			if (rtt1[i] < 0)
				rtt1[i] += 1000000;
			s1 += rtt1[i];
			count++;
		}
	}
	if (count > 0)
		s1 /= count;
	else
	{
		printf("no recv for rtt measured\n");
		exit(0);
	}
	printf("Pre-Phase:\nrtt measure for %s, %d times, rtt1=%fms\n", argv[1], count, s1 / 1000);
	count = 0;
	for (i = 0; i < 10; i++)
	{
		gettimeofday(&t1, NULL);
		send_msg(argv[2], 1); //atoi(argv[3])
		j = recvfrom(sockfd, buf, BUFLEN, 0, NULL, 0);
		if (j < 0)
			perror("recvfrom");
		else
		{
			gettimeofday(&t2, NULL);
			rtt2[i] = t2.tv_usec - t1.tv_usec;
			if (rtt2[i] < 0)
				rtt2[i] += 1000000;
			s2 += rtt2[i];
			count++;
		}
	}
	if (count > 0)
		s2 = s2 / count;
	else
	{
		printf("no recv for rtt measured\n");
		exit(0);
	}
	printf("rtt measure for %s, %d times, rtt2=%fms\n", argv[2], count, s2 / 1000);

	printf("rtt1=%fms, rtt2=%fms\n", s1 / 1000, s2 / 1000);
	gettimeofday(&t1, NULL);
	if (s1 > s2)
	{
		send_msg(argv[1], atoi(argv[3]));
		gettimeofday(&t2, NULL);
		while ((t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec < (s1 - s2))
			gettimeofday(&t2, NULL);
		send_msg(argv[2], atoi(argv[3]));
	}
	else
	{
		send_msg(argv[2], atoi(argv[3]));
		gettimeofday(&t2, NULL);
		while ((t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec < (s2 - s1))
			gettimeofday(&t2, NULL);
		send_msg(argv[1], atoi(argv[3]));
	}
	//	receive(); //wait for return;
	int n, p1 = 0, p2 = 0;
	int e = atoi(argv[3]);
	unsigned long a1;
	int array[50000];
	int fastpath = 0;
	count = 0;
	while (count < 2)
	{
		i = recvfrom(sockfd, buf, BUFLEN, 0, NULL, 0);
		if (i < 0)
			perror("recvfrom");
		//如果读取失败，接受窗口报错
		if (i < 300)
			count++;
		//检查终止信号
		sscanf(buf, "%d", &n);
		if (count < 1)
			array[j++] = n;
		//将Buffer中的内容记录到array中
		if (n < 0)
			p1++;
		else
			p2++;
		//printf("recv %d\n", n);
		//将buffer打印出来
	}
	printf("Phase 1:\nSendA=%d\nSendB=%d\n ", p1, p2);
	//计数（收到多少个消息）
	//inter_pkts_num(array, j, &m1, &m2);

	int j1 = -1, j2 = -1;
	int lasti = 0, curi = 0, lastj = 0, curj = 0;
	int h[200][2];
	for (i = 0; i < 200; i++)
	{
		h[i][0] = 0;
		h[i][1] = 0;
	}
	//判断0-200之间的直方
	for (i = 0; i < j; i++)
	{

		if (array[i] < 0)
		{
			if (j1 >= 0)
			{
				int index = absolute(curj, lastj);
				if (index < 199)
					h[index][0]++;
				lastj = curj;
				curi = array[i];
				//加入直方图
			}
			else
			{
				curi = array[i];
				lasti = array[i];
			}
			//初始化
			j1 = i;
		}
		else
		{
			if (j2 >= 0)
			{
				int index = absolute(curi, lasti);
				if (index < 199)
					h[index][1]++;
				lasti = curi;
				curj = array[i];
			}
			else
			{
				curj = array[i];
				lastj = curj;
			}
			j2 = i;
		}
	}
	int max1 = 1, max2 = 1;
	for (i = 1; i < 200; i++)
	{
		if (h[i][0] * i > h[max1][0] * max1 && h[i][0] > 3)
			max1 = i;
		//r1
		if (h[i][1] * i > h[max2][1] * max2 && h[i][1] > 3)
			max2 = i;
		//r2
	}
	double m1, m2;
	if (max1 > 1)
		m1 = (h[max1][0] * max1 + h[max1 - 1][0] * (max1 - 1) + h[max1 + 1][0] * (max1 + 1)) * 1.0 / (h[max1][0] + h[max1 - 1][0] + h[max1 + 1][0]);
	else
		m1 = (h[max1][0] * max1 + h[max1 + 1][0] * (max1 + 1)) * 1.0 / (h[max1][0] + h[max1 + 1][0]);
	max1 = max2;
	if (max1 > 1)
		m2 = (h[max1][1] * max1 + h[max1 - 1][1] * (max1 - 1) + h[max1 + 1][1] * (max1 + 1)) * 1.0 / (h[max1][1] + h[max1 - 1][1] + h[max1 + 1][1]);
	else
		m2 = (h[max1][1] * max1 + h[max1 + 1][1] * (max1 + 1)) * 1.0 / (h[max1][1] + h[max1 + 1][1]);
	//Algorithm 3

	for (i = 0; i < 200; i++)
	{
		if (h[i][1] != 0)
			printf("A %d:\t%d\n", i, h[i][1]);
	}

	//检查有没有Multiple Peaks
	int check_count = 0;
	if (m1 > m2)
	{
		for (i = 2; i < 200; i++)
		{
			if (h[i - 1][0] < h[i][0] && h[i + 1][0] < h[i][0])
				check_count++;
		}
	}
	else
	{
		for (i = 2; i < 200; i++)
		{
			if (h[i - 1][1] < h[i][1] && h[i + 1][1] < h[i][1])
				check_count++;
		}
	}
	if (check_count == 1)
	{
		printf("%f, %f\n", m1, m2);
	}
	else
	{
		//Phase 2
		gettimeofday(&t1, NULL);
		if(m1>m2){
			if (s1 > s2)
			{
				send_msg(argv[1], -atoi(argv[3]));
				gettimeofday(&t2, NULL);
				while ((t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec < (s1 - s2))
					gettimeofday(&t2, NULL);
				send_msg(argv[2], atoi(argv[3]));
			}
			else
			{
				send_msg(argv[2], atoi(argv[3]));
				gettimeofday(&t2, NULL);
				while ((t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec < (s2 - s1))
					gettimeofday(&t2, NULL);
				send_msg(argv[1], -atoi(argv[3]));
			}
		}else{
			if (s1 > s2)
			{
				send_msg(argv[1], atoi(argv[3]));
				gettimeofday(&t2, NULL);
				while ((t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec < (s1 - s2))
					gettimeofday(&t2, NULL);
				send_msg(argv[2], -atoi(argv[3]));
			}
			else
			{
				send_msg(argv[2], -atoi(argv[3]));
				gettimeofday(&t2, NULL);
				while ((t2.tv_sec - t1.tv_sec) * 1000000 + t2.tv_usec - t1.tv_usec < (s2 - s1))
					gettimeofday(&t2, NULL);
				send_msg(argv[1], atoi(argv[3]));
			}
		}
		
		p1 = 0;
		p2 = 0;
		count = 0;
		j = 0;
		while (count < 2)
		{
			i = recvfrom(sockfd, buf, BUFLEN, 0, NULL, 0);
			if (i < 0)
				perror("recvfrom");
			//如果读取失败，接受窗口报错
			if (i < 300)
			{
				count++;
			}
			//检查终止信号
			sscanf(buf, "%d", &n);
			if (count < 1)
				array[j++] = n;
			//将Buffer中的内容记录到array中
			if (n < 0)
				p1++;
			else
				p2++;
			//printf("recv %d\n", n);
			//将buffer打印出来
		}
		printf("Phase 2 received Packages\nSendA=%d\nSendB=%d\n ", p1, p2);
		//计数（收到多少个消息）
		//inter_pkts_num(array, j, &m1, &m2);

		j1 = -1, j2 = -1;
		lasti = 0, curi = 0, lastj = 0, curj = 0;
		for (i = 0; i < 200; i++)
		{
			h[i][0] = 0;
			h[i][1] = 0;
		}
		//判断0-200之间的直方
		for (i = 0; i < j; i++)
		{

			if (array[i] < 0)
			{
				if (j1 >= 0)
				{
					int index = absolute(curj, lastj);
					if (index < 199)
						h[index][0]++;
					lastj = curj;
					curi = array[i];
					//加入直方图
				}
				else
				{
					curi = array[i];
					lasti = array[i];
				}
				//初始化
				j1 = i;
			}
			else
			{
				if (j2 >= 0)
				{
					int index = absolute(curi, lasti);
					if (index < 199)
						h[index][1]++;
					lasti = curi;
					curj = array[i];
				}
				else
				{
					curj = array[i];
					lastj = curj;
				}
				j2 = i;
			}
		}
		for (i = 0; i < 200; i++)
		{
			if (h[i][1] != 0)
				printf("A %d:\t%d\n", i, h[i][1]);
		}
		max1 = 1, max2 = 1;
		for (i = 1; i < 200; i++)
		{
			if (h[i][0] * i > h[max1][0] * max1 && h[i][0] > 3)
				max1 = i;
			//r1
			if (h[i][1] * i > h[max2][1] * max2 && h[i][1] > 3)
				max2 = i;
			//r2
		}
		m1, m2;
		if (max1 > 1)
			m1 = (h[max1][0] * max1 + h[max1 - 1][0] * (max1 - 1) + h[max1 + 1][0] * (max1 + 1)) * 1.0 / (h[max1][0] + h[max1 - 1][0] + h[max1 + 1][0]);
		else
			m1 = (h[max1][0] * max1 + h[max1 + 1][0] * (max1 + 1)) * 1.0 / (h[max1][0] + h[max1 + 1][0]);
		max1 = max2;
		if (max1 > 1)
			m2 = (h[max1][1] * max1 + h[max1 - 1][1] * (max1 - 1) + h[max1 + 1][1] * (max1 + 1)) * 1.0 / (h[max1][1] + h[max1 - 1][1] + h[max1 + 1][1]);
		else
			m2 = (h[max1][1] * max1 + h[max1 + 1][1] * (max1 + 1)) * 1.0 / (h[max1][1] + h[max1 + 1][1]);

		printf("The result of Phase 2 is %f, %f\n", m1, m2);
	}

	FILE *fp = fopen("hihi", "w");
	for (i = 0; i < j; i++)
		fprintf(fp, "%d\n", array[i]);
	fclose(fp);
	//将结果写入hihi文件中

	close(sockfd);
	return 0;
}
