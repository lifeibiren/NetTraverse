#include "tuntap.hpp"

void do_read(void *buf, size_t *size)
{

}

void do_write(void *buf, size_t size)
{

}

#define MBUF_ETHER (1U << 0)
#define MBUF_ARP   (1U << 1)
#define MBUF_IP    (1U << 2)
#define MBUF_IPv6  (1U << 3)
#define MBUF_TCP   (1U << 4)
#define MBUF_UDP   (1U << 5)

#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip6.h>
#include <netinet/in.h>
#include <string.h>

struct mbuf {
    uint8_t *raw;
    uint16_t len;

    struct ether_header *ether_frame;
    uint16_t ether_len;
    uint16_t ether_off;

    struct arp_frame *arp_frame;
    uint16_t arp_len;

    struct ip *ip_frame;
    uint16_t ip_len;
    uint16_t ip_off;

    struct tcp_head_t {
        struct tcphdr *tcp_frame;
        uint16_t tcp_len;
        uint16_t tcp_off;
        uint32_t tcp_seq;
        uint32_t tcp_ack;

        bool ack()
        {
            return tcp_frame->ack == 1;
        }
        bool syn()
        {
            return tcp_frame->syn == 1;
        }
        bool fin()
        {
            return tcp_frame->fin == 1;
        }
        bool psh()
        {
            return tcp_frame->psh == 1;
        }
        bool rst()
        {
            return tcp_frame->rst == 1;
        }
    } tcp_head;


    uint8_t protocol_mask;


    int parser_ether()
    {
        if (len < sizeof(ether_frame)) {
            return -1;
        }
        ether_frame = (struct ether_header *)raw;
        ether_len = len;
        ether_off = sizeof(struct ether_header);
        protocol_mask |= MBUF_ETHER;

        switch (ntohs(ether_frame->ether_type)) {
            case ETHERTYPE_IP: {
                if (parse_ip()) {
                    return -1;
                }
                break;
            }
            case ETHERTYPE_ARP: {
                if (parse_arp()) {
                    return -1;
                }
                break;
            }
            default:
                break;
        }
        printf("ether_type %x\n", ntohs(ether_frame->ether_type));

        return 0;
    }

    int parse_ip()
    {
        if (ether_len < sizeof(struct ip)) {
            return -1;
        }
        ip_frame = (struct ip *)((uint8_t *)ether_frame + ether_off);
        ip_off = ip_frame->ip_hl << 2;
        ip_len = ntohs(ip_frame->ip_len) - ip_off;
        if (ip_off > ip_len) {
            return -1;
        }
        protocol_mask |= MBUF_IP;

        switch (ip_frame->ip_p) {
            case IPPROTO_TCP: {
                if (parse_tcp()) {
                    return -1;
                }
                break;
            }
            case IPPROTO_UDP:
                break;
            case IPPROTO_ICMP:
                break;
        }

        printf("ip frame length %d\n", ip_len);

        return 0;
    }

    int parse_arp()
    {

        return 0;
    }

    int parse_tcp()
    {
        if (ip_len < sizeof(struct tcphdr)) {
            return -1;
        }
        tcp_head.tcp_frame = (struct tcphdr *)((uint8_t *)ip_frame + ip_off);
        tcp_head.tcp_off = tcp_head.tcp_frame->doff << 2;
        tcp_head.tcp_len = ip_len - tcp_head.tcp_off;
        tcp_head.tcp_seq = ntohl(tcp_head.tcp_frame->seq);
        tcp_head.tcp_ack = ntohl(tcp_head.tcp_frame->ack);


        printf("seq %u ack %u\n", tcp_head.tcp_seq, tcp_head.tcp_ack);
        return 0;
    }
};




static void do_receive(int fd, struct mbuf *buf)
{
    ssize_t ret = read(fd, buf->raw, 1518);
    if (ret < 0) {
        return;
    }
    buf->len = ret;
    if (buf->parser_ether()) {
        return;
    }
}

void protocol_test(void)
{
    tap tap1("tap1");

    int fd = tap1.fd();


    do {
        mbuf *buf = new mbuf;
        bzero(buf, sizeof(*buf));
        buf->raw = new uint8_t[1518];

        do_receive(fd, buf);
    } while (1);

}
