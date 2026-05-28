#if !defined __SOCK_PROTO_H__
#define __SOCK_PROTO_H__
#include <stdint.h>
#ifdef WIN32
#pragma pack(push,1)
#elif defined __GNUC__
#pragma pack(push,1)
#endif
#ifdef __cplusplus
namespace tcpip{
#endif
#define FIN  0x01
#define SYN  0x02
#define RST  0x04
#define PSH  0x08
#define ACK  0x10
#define URG  0x20
#define ECN  0x40
#define CWR  0x80
#if (defined WIN32)||(defined _WINDOWS_)
	#define  ETH_ALEN 6  //定义了以太网接口的MAC地址的长度为6个字节
	#define  ETH_P_IP 0x0800 //IP协议
	#define  ETH_P_ARP 0x0806  //地址解析协议(Address Resolution Protocol)
	#define  ETH_P_RARP 0x8035  //返向地址解析协议(Reverse Address Resolution Protocol)
	#define  ETH_P_IPV6 0x86DD  //IPV6协议
	struct ethhdr
	{
		unsigned char h_dest[ETH_ALEN]; //目的MAC地址
		unsigned char h_source[ETH_ALEN]; //源MAC地址
		uint16_t h_proto ; //网络层所使用的协议类型
	};
#else
#include <net/ethernet.h> 
#endif
	struct pesudo_tcphdr { 
		uint32_t saddr;
		uint32_t daddr;
		uint8_t unused;
		uint8_t protocol;
		uint16_t tcplen;
	}; 

	struct ip_header
	{
		uint8_t ip_version;//version:4b. headerLen:4b
		uint8_t ip_tos;// type of service
		uint16_t ip_len;//ip packet total len, include the header
		uint16_t ip_id;//Identifier, used for Fragment, when the id is same, the packet is one of the BIG packet
		uint16_t ip_off;//flags:3b,NOUSE DF MF. fragment offset: 13b.
		uint8_t ip_ttl;//when ttl is 0, should drop it.
		uint8_t ip_protocol; //6 tcp; 17 udp; 1 icmp;
		uint16_t checksum;
		uint32_t source_ip_address;
		uint32_t destination_ip_address;
	};

	struct udp_header
	{
		uint16_t srcPort;
		uint16_t dstPort;
		uint16_t dataLen;//length of data and the udp header
		uint16_t udpChecksum;
	};

	struct tcp_header {
		uint16_t	srcPort;
		uint16_t	dstPort;
		uint32_t	seqnum;
		uint32_t	acknum;
		uint8_t		header_len;
		uint8_t		flags;
		uint16_t	window_size;
		uint16_t	checksum;
		uint16_t	urgentPointer;
	};

	static inline bool sn_before(uint32_t a, uint32_t b){
		return (int32_t)(a-b)<0;
	}
#define sn_after(a,b) sn_before((b),(a))

	struct icmp_header{
		uint8_t		type;
		uint8_t		code;
		uint16_t	checksum; //include icmp header and data
	};

	enum icmp_type:uint8_t{
		_echo_reply = 0,
		_target_unreachable = 3,
		_source_quench = 4,
		_route_redirect = 5,
		_echo_request = 8,
		_router_query = 9,
		_router_notify = 10,
		_timeout_report = 11,
		_param_error_reort = 12,
		_timestamp_request = 13,
		_timestamp_reply = 14,
		_information_request = 15,
		_information_reply = 16,
		_addr_mask_request = 17,
		_addr_mask_reply = 18,
	};

	struct icmp_echo{//
		uint16_t	id;	//identification
		uint16_t	seqNum;//sequence number
	};

	enum icmp_code_type3:uint8_t{
		_code_net_unreach = 0,//net unreachable
		_code_host_unreach = 1,//host unreachable
		_code_proto_unreach = 2,//protocol unreachable
		_code_port_unreach = 3,//port unreachable
		_code_frag_need = 4,//fragmentation needed and DF set
		_code_source_route_fail = 5,//source route failed
	};
	
#ifdef __cplusplus
}
#endif

#ifdef WIN32
#pragma pack(pop)
#elif defined __GNUC__
#pragma pack(pop)
#endif

#endif //__SOCK_PROTO_H__
