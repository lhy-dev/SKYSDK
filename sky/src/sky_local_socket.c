#include <errno.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netpacket/packet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "sky.h"


struct ip_udp_packet {
	struct iphdr ip;
	struct udphdr udp;
	uint8_t data[ 512 ];
} __attribute__ ((__packed__));


struct route_info
{
	struct in_addr dst_addr;
	struct in_addr src_addr;
	struct in_addr gateway;
	char if_name [ IF_NAMESIZE ];
};


/* Construct a ip/udp header for a packet, send packet */
SKY_API int sky_socket_send_raw_udp (
	int if_index,
	uint8_t const * dst_mac,
	uint32_t dst_ip,
	int dst_port,
	uint32_t src_ip,
	int src_port,
	void * buf,
	uint32_t len )
{
	struct sockaddr_ll dest_sll;
	struct ip_udp_packet packet;
	uint32_t packet_len;
	int sockfd;
	int rc;

	SKY_LOG_TRACE( "%s: if_index = %d\n", __func__, if_index );

	if ( len > sizeof( packet.data ) )
	{
		SKY_LOG_TRACE(
			"%s: sizeof packet.data = %d, len = %d\n",
			__func__,
			sizeof( packet.data ),
			len );
		return -1;
	}

	sockfd = socket( PF_PACKET, SOCK_DGRAM, htons( ETH_P_IP ) );
	if ( sockfd < 0 )
	{
		SKY_LOG_TRACE(
			"%s: fail to create socket, error = %d\n",
			__func__,
			errno );
		return -1;
	}

	sky_mem_clear( &packet, sizeof( packet ) );
	sky_mem_copy( packet.data, buf, len );

	packet_len = sizeof( packet ) - sizeof( packet.data ) + len;

	sky_mem_clear( &dest_sll, sizeof( dest_sll ) );

	dest_sll.sll_family = AF_PACKET;
	dest_sll.sll_protocol = htons( ETH_P_IP );
	dest_sll.sll_ifindex = if_index;
	dest_sll.sll_halen = 6;
	sky_mem_copy( dest_sll.sll_addr, dst_mac, 6 );

	rc = bind(
		sockfd,
		(struct sockaddr*) &dest_sll,
		sizeof( dest_sll ) );
	if ( rc < 0 )
	{
		SKY_LOG_TRACE(
			"%s: fail to bind, error = %d\n",
			__func__,
			errno );
		close( sockfd );
		return -1;
	}

	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = src_ip;
	packet.ip.daddr = dst_ip;
	packet.udp.source = src_port;
	packet.udp.dest = dst_port;
	/* size, excluding IP header: */
	packet.udp.len = htons( sizeof( packet.udp ) + len );
	/* for UDP checksumming, ip.len is set to UDP packet len */
	packet.ip.tot_len = packet.udp.len;
	packet.udp.check = sky_algorithm_inet_chksum(
		(uint16_t*) &packet,
		sizeof( packet.ip ) + sizeof( packet.udp ) + len );
	/* but for sending, it is set to IP packet len */
	packet.ip.tot_len = htons( packet_len );
	packet.ip.ihl = sizeof( packet.ip ) >> 2;
	packet.ip.version = IPVERSION;
	packet.ip.frag_off = htons( IP_DF );
	packet.ip.ttl = IPDEFTTL;
	packet.ip.check = sky_algorithm_inet_chksum(
		(uint16_t*) &packet.ip,
		sizeof( packet.ip ) );

	rc = sendto(
		sockfd,
		&packet,
		packet_len,
		0,
		(struct sockaddr*) &dest_sll,
		sizeof( dest_sll ) );
	if ( -1 == rc )
	{
		SKY_LOG_TRACE(
			"%s: fail to bind, error = %d\n",
			__func__,
			errno );
	}

	close( sockfd );

	return rc;
}

SKY_API bool sky_if_get_hwaddr (
	char const * if_name,
	uint8_t hw_addr[ 6 ] )
{
	int sockfd;
	struct ifreq ifr;
	int rc;

	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( 0 == sockfd )
	{
		SKY_LOG_FATAL(
			"%s: fail to call socket on %s, error = %d\n",
			__func__,
			if_name,
			errno );
		return false;
	}

	sky_mem_clear( &ifr, sizeof( ifr ) );
	sky_str_printf( ifr.ifr_name, sizeof( ifr.ifr_name ), if_name );

	rc = ioctl( sockfd, SIOCGIFHWADDR, &ifr );

	close( sockfd );

	if ( rc != 0 )
	{
		SKY_LOG_FATAL(
			"%s: fail to call ioctl on %s, error = %d\n",
			__func__,
			if_name,
			errno );
		return false;
	}

	sky_mem_copy( hw_addr, ifr.ifr_hwaddr.sa_data, sizeof( hw_addr ) );

	return true;
}

SKY_API uint32_t sky_if_get_ip ( char const * if_name )
{
	int sockfd;
	struct ifreq ifr;
	int rc;

	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( 0 == sockfd )
	{
		SKY_LOG_FATAL(
			"%s: fail to call socket on %s, error = %d\n",
			__func__,
			if_name,
			errno );
		return SKY_INADDR_ANY;
	}

	sky_mem_clear( &ifr, sizeof( ifr ) );
	sky_str_cpy( ifr.ifr_name, if_name, sizeof( ifr.ifr_name ) );

	rc = ioctl( sockfd, SIOCGIFADDR, &ifr );

	close( sockfd );

	if ( rc != 0 )
	{
		SKY_LOG_FATAL(
			"%s: fail to call ioctl on %s, error = %d\n",
			__func__,
			if_name,
			errno );
		return SKY_INADDR_ANY;
	}

	if ( ifr.ifr_addr.sa_family != AF_INET )
	{
		SKY_LOG_ERROR( "%s: %s sa_family != AF_INET\n", __func__, if_name );
		return SKY_INADDR_ANY;
	}

	return ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr;
}

SKY_API uint32_t sky_if_get_netmask ( char const * if_name )
{
	int sockfd;
	struct ifreq ifr;
	int rc;

	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( 0 == sockfd )
	{
		SKY_LOG_FATAL(
			"%s: fail to call socket on %s, error = %d\n",
			__func__,
			if_name,
			errno );
		return SKY_INADDR_ANY;
	}

	sky_mem_clear( &ifr, sizeof( ifr ) );
	sky_str_printf( ifr.ifr_name, sizeof( ifr.ifr_name ), if_name );

	rc = ioctl( sockfd, SIOCGIFNETMASK, &ifr );

	close( sockfd );

	if ( rc != 0 )
	{
		SKY_LOG_FATAL(
			"%s: fail to call ioctl on %s, error = %d\n",
			__func__,
			if_name,
			errno );
		return SKY_INADDR_ANY;
	}

	if ( ifr.ifr_addr.sa_family != AF_INET )
	{
		SKY_LOG_ERROR( "%s: %s sa_family != AF_INET\n", __func__, if_name );
		return SKY_INADDR_ANY;
	}

	return ((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr.s_addr;
}

static int read_nl_sock (
	int sock_fd,
	char *buf,
	size_t buf_size,
	int seq_num,
	int pid )
{
	struct nlmsghdr *nl_hdr;
	int read_len = 0;
	int msg_len = 0;

	do
	{
		/* Receive response from the kernel */
		if ( ( read_len = recv( sock_fd, buf, buf_size - msg_len, 0 ) ) < 0 )
		{
			SKY_LOG_ERROR( "%s: socket read error!\n", __func__ );
			return -1;
		}

		nl_hdr = (struct nlmsghdr *) buf;

		/* Check if the header is valid */
		if ( ( NLMSG_OK( nl_hdr, read_len ) == 0 )
			|| ( nl_hdr->nlmsg_type == NLMSG_ERROR ) )
		{
			SKY_LOG_ERROR( "%s: error in received packet!\n", __func__ );
			return -1;
		}

		/* Check if the its the last message */
		if ( nl_hdr->nlmsg_type == NLMSG_DONE )
		{
			break;
		}
		else
		{
			/* Else move the pointer to buffer appropriately */
			buf += read_len;
			msg_len += read_len;
		}

		/* Check if its a multi part message */
		if ( ( nl_hdr->nlmsg_flags & NLM_F_MULTI ) == 0 )
		{
			/* return if its not */
			break;
		}
	} while ( ( nl_hdr->nlmsg_seq != seq_num ) || ( nl_hdr->nlmsg_pid != pid ) );

	return msg_len;
}

SKY_API uint32_t sky_if_get_gateway ( char const * if_name )
{
	uint32_t gateway_addr = SKY_INADDR_ANY;
	struct nlmsghdr * nl_msg;
	struct route_info route_info;
	char msg_buf [ 8192 ]; // pretty large buffer
	int sock;
	int len;
	int msg_seq = 0;

	/* Create Socket */
	sock = socket( PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE );
	if ( sock < 0 )
	{
		SKY_LOG_ERROR(
			"%s: fail to create socket! error = %d\n",
			__func__,
			errno );
		return gateway_addr;
	}

	/* Initialize the buffer */
	sky_mem_clear( msg_buf, sizeof( msg_buf ) );

	/* point the header and the msg structure pointers into the buffer */
	nl_msg = (struct nlmsghdr *) msg_buf;

	/* Fill in the nlmsg header*/
	nl_msg->nlmsg_len = NLMSG_LENGTH( sizeof(struct rtmsg) ); // Length of message.
	nl_msg->nlmsg_type = RTM_GETROUTE; // Get the routes from kernel routing table .

	nl_msg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST; // The message is a request for dump.
	nl_msg->nlmsg_seq = msg_seq++; // Sequence of the message packet.
	nl_msg->nlmsg_pid = getpid(); // PID of process sending the request.

	/* Send the request */
	if ( send( sock, nl_msg, nl_msg->nlmsg_len, 0 ) < 0 )
	{
		SKY_LOG_ERROR(
			"%s: write to socket failed, error = %d\n",
			__func__,
			errno );
		close( sock );
		return gateway_addr;
	}

	/* Read the response */
	len = read_nl_sock( sock, msg_buf, sizeof( msg_buf ), msg_seq, getpid() );
	if ( len < 0 )
	{
		SKY_LOG_ERROR(
			"%s: read from socket failed, error = %d\n",
			__func__,
			errno );
		close( sock );
		return gateway_addr;
	}

	/* Parse and print the response */
	for ( ; NLMSG_OK( nl_msg, len ); nl_msg = NLMSG_NEXT( nl_msg, len ) )
	{
		sky_mem_clear( &route_info, sizeof( route_info ) );
		if ( parse_routes( nl_msg, &route_info ) < 0 )
		{
			continue;  // don't check route_info if it has not been set up
		}

		if ( sky_str_cmp( route_info.if_name, if_name ) == 0 )
		{
			// Check if default gateway
			if ( sky_str_str(
				sky_inet_ntoa( route_info.dst_addr.s_addr ),
				"0.0.0.0" ) )
			{
				gateway_addr = route_info.gateway.s_addr;
				break;
			}
		}
	}
	close( sock );

	return gateway_addr;
}

