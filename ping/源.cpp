#include <stdio.h>
#include <chrono>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ip.h"
#include "ip_icmp.h"
using namespace std::chrono;

int nsent = 1;
int datalen = 32;
int pid = 0;

uint16_t in_cksum(uint16_t * addr, int len);
//int recv_v4(SOCKET sockfd);
//void send_v4(SOCKET sockfd, sockaddr* sasend, int salen);
//void proc_v4(char *ptr, int len, steady_clock::time_point start);

uint16_t in_cksum(uint16_t *addr, int len)
{
	int				nleft = len;
	uint32_t		sum = 0;
	uint16_t		*w = addr;
	uint16_t		answer = 0;

	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	/* 4mop up an odd byte, if necessary */
	if (nleft == 1)
	{
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}

	/* 4add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}

int recv_v4(SOCKET sockfd, char *recvbuf)
{
	int				size;
	int             n;
	sockaddr_in     addr;
	int len = sizeof(sockaddr);

	n = recvfrom(sockfd, recvbuf, 1500, 0, (sockaddr*)&addr, &len);
	if (n < 0)
	{
		if (WSAGetLastError() == WSAETIMEDOUT)
			printf("receive time out\n");
		else
			printf("recv fail with error code %d\n", WSAGetLastError());
		return -1;
	}
	return n;
}

void proc_v4(char *recvbuf, int len, int nsent, steady_clock::time_point start)
{
	int				hlen1, icmplen;
	struct ip		*ip;
	struct icmp		*icmp;
	auto            end = steady_clock::now();
	double          rtt;

	ip = (struct ip *) recvbuf;		/* start of IP header */
	hlen1 = ip->ip_hl << 2;		/* length of IP header */
	if (ip->ip_p != IPPROTO_ICMP)
		return;				/* not ICMP */

	icmp = (struct icmp *) (recvbuf + hlen1);	/* start of ICMP header */
	if ((icmplen = len - hlen1) < 8)
		return;				/* malformed packet */

	if (icmp->icmp_type == ICMP_ECHOREPLY&&icmp->icmp_seq == nsent)
	{
		if (icmp->icmp_id != pid)
			return;			/* not a response to our ECHO_REQUEST */
		if (icmplen < 16)
			return;			/* not enough data to use */

		rtt = duration_cast<duration<double, std::milli>>(end - start).count();

		char strbuf[50];
		printf("%d bytes from %s: seq=%u, ttl=%d, rtt=%.3lf ms\n",
			datalen, inet_ntop(AF_INET, &ip->ip_src, strbuf, 50),
			icmp->icmp_seq, ip->ip_ttl, rtt);
	}
}
void send_v4(SOCKET sockfd, int nsent, sockaddr* sasend, int salen)
{
	int			len;
	struct icmp	*icmp;
	char sendbuf[1500];

	icmp = (struct icmp *) sendbuf;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_id = pid;
	icmp->icmp_seq = nsent++;
	memset(icmp->icmp_data, 0xa5, datalen);	/* fill with pattern */

	len = 8 + datalen;		/* checksum ICMP header and data */
	icmp->icmp_cksum = 0;
	icmp->icmp_cksum = in_cksum((u_short *)icmp, len);

	auto xx = sendto(sockfd, sendbuf, len, 0, sasend, salen);
	if (xx < 0)
	{
		printf("sendto fail with error code %d\n", WSAGetLastError());
		return;
	}
}

void ping(sockaddr* sasend, int salen)
{
	sockaddr_in bind_addr;
	memset(&bind_addr, 0, sizeof(bind_addr));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_addr.s_addr = INADDR_ANY;

	SOCKET sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	bind(sockfd, (sockaddr*)&bind_addr, sizeof(bind_addr));

	int timeout = 2000;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, 4);

	char recvbuf[1500];
	int n;
	for (int i = 1; i <= 500; i++)
	{
		auto start = steady_clock::now();

		send_v4(sockfd, i,sasend, salen);
		if ((n = recv_v4(sockfd, recvbuf)) != -1)
			proc_v4(recvbuf, n,i, start);
// 		Sleep(1000);
	}
}
int main(int argc, char **argv)
{
	WSADATA wsaData;
	int error = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (argc == 2)
	{
		addrinfo hints, *result;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		int xx = getaddrinfo(argv[1], "0", &hints, &result);
		if (xx != 0)
		{
			printf("getaddrinfo failed with error: %d\n", WSAGetLastError());
		}
		else
		{
			ping(result[0].ai_addr, result[0].ai_addrlen);
			freeaddrinfo(result);
		}
	}
	else
	{
		printf("ÓÃ·¨:ping [ip]\n");
	}

	WSACleanup();
	return 0;
}
