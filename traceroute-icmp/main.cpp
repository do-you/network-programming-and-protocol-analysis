#include <stdio.h>
#include <chrono>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "ip.h"
#include "ip_icmp.h"
using namespace std::chrono;

#define time_out -2
#define error -1

int datalen = 32;
int pid = 0;
int timeout = 2000;

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

int recv_v4(SOCKET sockfd, int nsend,sockaddr_in *addr)
{
	int             n;
	char  recvbuf[1500];
	int len = sizeof(sockaddr);
	clock_t t1, t2;

	t1 = clock();
	while(true)
	{
		n = recvfrom(sockfd, recvbuf, 1500, 0, (sockaddr*)addr, &len);
		if (n < 0)
		{
			if (WSAGetLastError() == WSAETIMEDOUT)
				return time_out;
			else
			{
				printf("recv fail with error code %d\n", WSAGetLastError());
				return error;
			}
		}
		t2 = clock();
		if (1000.0*(t2 - t1) / CLOCKS_PER_SEC >= timeout)
			return time_out;
		//process icmp
		int				hlen1, icmplen, hlen2;
		struct ip		*ip;
		struct icmp		*icmp;

		ip = (struct ip *) recvbuf;		/* start of IP header */
		hlen1 = ip->ip_hl << 2;		/* length of IP header */
		if (ip->ip_p != IPPROTO_ICMP)
			continue;			/* not ICMP */

		icmp = (struct icmp *) (recvbuf + hlen1);	/* start of ICMP header */
		if ((icmplen = n - hlen1) < 8)
			continue;				/* malformed packet */

		if (icmp->icmp_type == ICMP_ECHOREPLY && icmp->icmp_seq == nsend && icmp->icmp_id == pid)
			return ICMP_ECHOREPLY;
		else if (icmp->icmp_type == ICMP_TIME_EXCEEDED)
		{
			struct ip *ipsend;
			struct icmp *icmp2;

			ipsend = &(icmp->icmp_ip);
			if (ipsend->ip_p != IPPROTO_ICMP)
				continue;

			hlen2 = ipsend->ip_hl << 2;
			if (icmplen - 8 - hlen2 < 8)
				continue;

			icmp2 = (struct icmp *) (recvbuf + hlen1 + 8 + hlen2);

			if (icmp2->icmp_seq == nsend && icmp2->icmp_id == pid)
				return ICMP_TIME_EXCEEDED;
		}
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
	icmp->icmp_seq = nsent;
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

void traceloop(int n, sockaddr *target, int len)
{
	SOCKET sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd == INVALID_SOCKET)
	{
		printf("socket failed with error = %d\n", WSAGetLastError());
		return;
	}

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, 4);
	
	char buf[50];
	int recvflag;
	bool hadrecved;
	sockaddr_in remoteaddr;
	for (int ttl = 1; ttl <= n; ttl++)
	{
		auto aa = setsockopt(sockfd, IPPROTO_IP, IP_TTL, (char*)&ttl, 4);
		printf("%2d ", ttl);
		hadrecved = false;
		for (int i = 1; i <= 3; i++)
		{
			auto start = steady_clock::now();

			send_v4(sockfd, i, target, len);
			recvflag = recv_v4(sockfd, i, &remoteaddr);
			
			auto end = steady_clock::now();
			switch (recvflag)
			{
			case time_out:
				printf("        *");
				continue;
			case ICMP_ECHOREPLY:
				ttl = n;//break loop
				break;
			case error:
				return;
			default:
				break;
			}
			hadrecved = true;
			printf("%7.2lfms", duration_cast<duration<double, std::milli>>(end - start).count());
		}
		printf("  %s\n", hadrecved? inet_ntop(AF_INET, &remoteaddr.sin_addr, buf, 50) :"Request timed out.");
	}
}
int main(int argc, char **argv)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	pid = GetCurrentProcessId();

	if (argc == 2)
	{
		char buf[50];
		addrinfo hints, *result;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		int xx = getaddrinfo(argv[1], "0", &hints, &result);
		if (xx != 0)
			printf("getaddrinfo failed with error: %d\n", WSAGetLastError());
		else
		{
			printf("Tracing route to %s over a maximum of 20 hops\n", inet_ntop(AF_INET, &((sockaddr_in*)(result[0].ai_addr))->sin_addr, buf, 50));
			traceloop(20, result[0].ai_addr, result[0].ai_addrlen);
			freeaddrinfo(result);
		}
	}
	else
	{
		printf("��ʹ��:traceroute [ip]");
	}

	WSACleanup();
	return 0;
}