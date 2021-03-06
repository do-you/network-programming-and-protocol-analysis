#ifndef IP_H
#define IP_H

#define	__LITTLE_ENDIAN	1234
#define	__BIG_ENDIAN	4321
#define	__PDP_ENDIAN	3412

#include <winsock2.h>
#include <stdint.h>
#define __BYTE_ORDER __LITTLE_ENDIAN



struct ip
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned char ip_hl : 4;		/* header length */
	unsigned char ip_v : 4;		/* version */
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned int ip_v : 4;		/* version */
	unsigned int ip_hl : 4;		/* header length */
#endif
	uint8_t ip_tos;			/* type of service */
	u_short ip_len;			/* total length */
	u_short ip_id;			/* identification */
	u_short ip_off;			/* fragment offset field */
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	uint8_t ip_ttl;			/* time to live */
	uint8_t        ip_p;			/* protocol */
	u_short ip_sum;			/* checksum */
	struct in_addr ip_src, ip_dst;	/* source and dest address */
};

#define	IPVERSION	4               /* IP version number */
#define	IP_MAXPACKET	65535		/* maximum packet size */

/*
* Definitions for Explicit Congestion Notification (ECN)
*
* Taken from RFC-3168, Section 5.
*/

#define	IPTOS_ECN_MASK		0x03
#define	IPTOS_ECN(x)		((x) & IPTOS_ECN_MASK)
#define	IPTOS_ECN_NOT_ECT	0x00
#define	IPTOS_ECN_ECT1		0x01
#define	IPTOS_ECN_ECT0		0x02
#define	IPTOS_ECN_CE		0x03

/*
* Definitions for IP differentiated services code points (DSCP)
*
* Taken from RFC-2597, Section 6 and RFC-2598, Section 2.3.
*/

#define	IPTOS_DSCP_MASK		0xfc
#define	IPTOS_DSCP(x)		((x) & IPTOS_DSCP_MASK)
#define	IPTOS_DSCP_AF11		0x28
#define	IPTOS_DSCP_AF12		0x30
#define	IPTOS_DSCP_AF13		0x38
#define	IPTOS_DSCP_AF21		0x48
#define	IPTOS_DSCP_AF22		0x50
#define	IPTOS_DSCP_AF23		0x58
#define	IPTOS_DSCP_AF31		0x68
#define	IPTOS_DSCP_AF32		0x70
#define	IPTOS_DSCP_AF33		0x78
#define	IPTOS_DSCP_AF41		0x88
#define	IPTOS_DSCP_AF42		0x90
#define	IPTOS_DSCP_AF43		0x98
#define	IPTOS_DSCP_EF		0xb8

/*
* In RFC 2474, Section 4.2.2.1, the Class Selector Codepoints subsume
* the old ToS Precedence values.
*/

#define	IPTOS_CLASS_MASK		0xe0
#define	IPTOS_CLASS(class)		((class) & IPTOS_CLASS_MASK)
#define	IPTOS_CLASS_CS0			0x00
#define	IPTOS_CLASS_CS1			0x20
#define	IPTOS_CLASS_CS2			0x40
#define	IPTOS_CLASS_CS3			0x60
#define	IPTOS_CLASS_CS4			0x80
#define	IPTOS_CLASS_CS5			0xa0
#define	IPTOS_CLASS_CS6			0xc0
#define	IPTOS_CLASS_CS7			0xe0

#define	IPTOS_CLASS_DEFAULT		IPTOS_CLASS_CS0

/*
* Definitions for IP type of service (ip_tos) [deprecated; use DSCP
* and CS definitions above instead.]
*/
#define	IPTOS_TOS_MASK		0x1E
#define	IPTOS_TOS(tos)		((tos) & IPTOS_TOS_MASK)
#define	IPTOS_LOWDELAY		0x10
#define	IPTOS_THROUGHPUT	0x08
#define	IPTOS_RELIABILITY	0x04
#define	IPTOS_LOWCOST		0x02
#define	IPTOS_MINCOST		IPTOS_LOWCOST

/*
* Definitions for IP precedence (also in ip_tos) [also deprecated.]
*/
#define	IPTOS_PREC_MASK			IPTOS_CLASS_MASK
#define	IPTOS_PREC(tos)			IPTOS_CLASS(tos)
#define	IPTOS_PREC_NETCONTROL		IPTOS_CLASS_CS7
#define	IPTOS_PREC_INTERNETCONTROL	IPTOS_CLASS_CS6
#define	IPTOS_PREC_CRITIC_ECP		IPTOS_CLASS_CS5
#define	IPTOS_PREC_FLASHOVERRIDE	IPTOS_CLASS_CS4
#define	IPTOS_PREC_FLASH		IPTOS_CLASS_CS3
#define	IPTOS_PREC_IMMEDIATE		IPTOS_CLASS_CS2
#define	IPTOS_PREC_PRIORITY		IPTOS_CLASS_CS1
#define	IPTOS_PREC_ROUTINE		IPTOS_CLASS_CS0

/*
* Definitions for options.
*/
#define	IPOPT_COPY		0x80
#define	IPOPT_CLASS_MASK	0x60
#define	IPOPT_NUMBER_MASK	0x1f

#define	IPOPT_COPIED(o)		((o) & IPOPT_COPY)
#define	IPOPT_CLASS(o)		((o) & IPOPT_CLASS_MASK)
#define	IPOPT_NUMBER(o)		((o) & IPOPT_NUMBER_MASK)

#define	IPOPT_CONTROL		0x00
#define	IPOPT_RESERVED1		0x20
#define	IPOPT_DEBMEAS		0x40
#define	IPOPT_MEASUREMENT       IPOPT_DEBMEAS
#define	IPOPT_RESERVED2		0x60

#define	IPOPT_EOL		0		/* end of option list */
#define	IPOPT_END		IPOPT_EOL
#define	IPOPT_NOP		1		/* no operation */
#define	IPOPT_NOOP		IPOPT_NOP

#define	IPOPT_RR		7		/* record packet route */
#define	IPOPT_TS		68		/* timestamp */
#define	IPOPT_TIMESTAMP		IPOPT_TS
#define	IPOPT_SECURITY		130		/* provide s,c,h,tcc */
#define	IPOPT_SEC		IPOPT_SECURITY
#define	IPOPT_LSRR		131		/* loose source route */
#define	IPOPT_SATID		136		/* satnet id */
#define	IPOPT_SID		IPOPT_SATID
#define	IPOPT_SSRR		137		/* strict source route */
#define	IPOPT_RA		148		/* router alert */

/*
* Offsets to fields in options other than EOL and NOP.
*/
#define	IPOPT_OPTVAL		0		/* option ID */
#define	IPOPT_OLEN		1		/* option length */
#define	IPOPT_OFFSET		2		/* offset within option */
#define	IPOPT_MINOFF		4		/* min value of above */

#define	MAX_IPOPTLEN		40

/* flag bits for ipt_flg */
#define	IPOPT_TS_TSONLY		0		/* timestamps only */
#define	IPOPT_TS_TSANDADDR	1		/* timestamps and addresses */
#define	IPOPT_TS_PRESPEC	3		/* specified modules only */

/* bits for security (not byte swapped) */
#define	IPOPT_SECUR_UNCLASS	0x0000
#define	IPOPT_SECUR_CONFID	0xf135
#define	IPOPT_SECUR_EFTO	0x789a
#define	IPOPT_SECUR_MMMM	0xbc4d
#define	IPOPT_SECUR_RESTR	0xaf13
#define	IPOPT_SECUR_SECRET	0xd788
#define	IPOPT_SECUR_TOPSECRET	0x6bc5

/*
* Internet implementation parameters.
*/
#define	MAXTTL		255		/* maximum time to live (seconds) */
#define	IPDEFTTL	64		/* default ttl, from RFC 1340 */
#define	IPFRAGTTL	60		/* time to live for frags, slowhz */
#define	IPTTLDEC	1		/* subtracted when forwarding */

#define	IP_MSS		576		/* default maximum segment size */

#endif // IP_H
