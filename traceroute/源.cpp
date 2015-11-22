#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <WinSock2.h>
//#include <Ws2ipdef.h>
#include <WS2tcpip.h>
#include <Mstcpip.h>
#include <chrono>
#include "ip_icmp.h"
#include "ip.h"
#include "udp.h"
#include <thread>
#include <time.h>
using namespace std::chrono;


#define dport 65530/*33434*/
#define time_out 0
#define error -1

uint16_t localport;
int timeout = 2000;

uint8_t recv_v4(SOCKET sockfd, uint16_t desport, sockaddr_in *addr)
{
	int             len;
	char recvbuf[1500];
	int n = sizeof(sockaddr_in);
	clock_t t1, t2;

	t1 = clock();
	do
	{
		len = recvfrom(sockfd, recvbuf, 1500, 0, (sockaddr*)addr, &n);
		if (len < 0)
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
		icmplen = len - hlen1;

		if (icmp->icmp_type == ICMP_DEST_UNREACH || icmp->icmp_type == ICMP_TIME_EXCEEDED)
		{
			struct ip *ipsend;
			struct udp *udphdr;

			ipsend = &(icmp->icmp_ip);
			if (ipsend->ip_p != IPPROTO_UDP)
				continue;

			hlen2 = ipsend->ip_hl << 2;
			if (icmplen - 8 - hlen2 < 8)
				continue;

			udphdr = (struct udp *) (recvbuf + hlen1 + 8 + hlen2);

			if (udphdr->uh_sport != htons(localport) || udphdr->uh_dport != htons(desport))
				continue;

			return icmp->icmp_type;
		}
	} while (true);
}

void traceloop(int n, sockaddr *target, int len)
{
	SOCKET sendsoc = socket(AF_INET, SOCK_DGRAM, 0);
	SOCKET recvsoc = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sendsoc == INVALID_SOCKET || recvsoc == INVALID_SOCKET)
	{
		printf("socket failed with error = %d\n", WSAGetLastError());
		return;
	}
	setsockopt(recvsoc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, 4);
	//绑定端口
	sockaddr_in localaddr;
	memset(&localaddr, 0, sizeof(sockaddr_in));
	localaddr.sin_family = AF_INET;	
	localaddr.sin_port = htons(localport);
	auto yy = bind(sendsoc, (sockaddr*)&localaddr, sizeof(sockaddr_in));
	auto zz = connect(sendsoc, target, len);

	auto aa = getsockname(sendsoc, (sockaddr*)&localaddr, &len);
	localaddr.sin_port = 0;
	auto xx = bind(recvsoc, (sockaddr*)&localaddr, sizeof(sockaddr_in));

	int rcv = RCVALL_IPLEVEL;
	DWORD b = 0;
	WSAIoctl(recvsoc, SIO_RCVALL, &rcv, sizeof(rcv), 0, 0, &b, 0, 0);

	char buf[50];
	uint8_t recvflag;
	sockaddr_in remoteaddr;
	for (int ttl = 1; ttl <= n; ttl++)
	{
		auto aa=setsockopt(sendsoc, IPPROTO_IP, IP_TTL, (char*)&ttl, 4);
		for (int i = 0; i < 3; i++)
		{
			((sockaddr_in*)target)->sin_port = htons(dport + i);

			auto start = steady_clock::now();
			auto zz=sendto(sendsoc, NULL, 0, 0, target, len);
			recvflag = recv_v4(recvsoc, dport + i, &remoteaddr);
			auto end = steady_clock::now();
			switch (recvflag)
			{
			case time_out:
				printf("* ");
				continue;
			case ICMP_PORT_UNREACH:
				ttl = n;//break loop
				break;
			case error:
				return;
			default:
				break;
			}
			printf("%lfms ", duration_cast<duration<double, std::milli>>(end - start).count());
		}
		printf("%s\n", inet_ntop(AF_INET, &remoteaddr.sin_addr, buf, 50));
	}
}

int main(int argc, char **argv)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (argc == 2)
	{
		char buf[50];
		localport = (GetCurrentProcessId() & 0xffff) | 0x8000;

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
		printf("请使用:traceroute [ip]");
	}

	WSACleanup();
	return 0;
}