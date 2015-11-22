#ifndef __NETINET_UDP_H
#define __NETINET_UDP_H    1

#include <stdint.h>

/* UDP header as specified by RFC 768, August 1980. */

struct udp
{
      uint16_t uh_sport;		/* source port */
      uint16_t uh_dport;		/* destination port */
      uint16_t uh_ulen;		/* udp length */
      uint16_t uh_sum;		/* udp checksum */
};

/* UDP socket options */
#define UDP_CORK	1	/* Never send partially complete segments.  */
#define UDP_ENCAP	100	/* Set the socket to accept
				   encapsulated packets.  */

/* UDP encapsulation types */
#define UDP_ENCAP_ESPINUDP_NON_IKE 1	/* draft-ietf-ipsec-nat-t-ike-00/01 */
#define UDP_ENCAP_ESPINUDP	2	/* draft-ietf-ipsec-udp-encaps-06 */
#define UDP_ENCAP_L2TPINUDP	3	/* rfc2661 */

#define SOL_UDP            17      /* sockopt level for UDP */

#endif /* netinet/udp.h */
