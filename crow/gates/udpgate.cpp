/** @file */

#include <crow/gates/udpgate.h>
#include <crow/tower.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void crow::udpgate::nblock_onestep()
{
    crow_header header;

    struct sockaddr_in sender;
    socklen_t sendsize = sizeof(sender);
    memset(&sender, 0, sizeof(sender));

    ssize_t len = recvfrom(sock, &header, sizeof(crow_header), MSG_PEEK,
                           (struct sockaddr *)&sender, &sendsize);

    if (len <= 0)
        return;

    size_t flen = header.flen;

    if (!block)
        block = crow::allocate_packet(flen - sizeof(crow_header));
    //(crow_packet *) malloc(flen + sizeof(crow_packet) -
    // sizeof(crow_header));

    len = recvfrom(sock, &block->header, flen, 0, (struct sockaddr *)&sender,
                   &sendsize);

    crow_packet_initialization(block, this);

    igris::buffer vec[3] = {
        {(char*)&id, 1}, 
        {(char*)&sender.sin_addr.s_addr, 4}, 
        {(char*)&sender.sin_port, 2}};

    crow_packet_revert(block, vec, 3);

    crow_packet *pack = block;
    block = NULL;

    crow::nocontrol_travel(pack, fastsend);
}

int crow::udpgate::open(uint16_t port)
{
    int ret;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    const int optVal = 1;
    const socklen_t optLen = sizeof(optVal);

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&optVal, optLen);

    if (sock < 0)
    {
        perror("udp socket open:");
        exit(0);
    }

    struct sockaddr_in ipaddr;
    socklen_t iplen = sizeof(struct sockaddr_in);
    memset(&ipaddr, 0, iplen);
    ipaddr.sin_port = htons(port);
    ipaddr.sin_family = PF_INET;

    if (port != 0)
    {
        ret = ::bind(sock, (struct sockaddr *)&ipaddr, iplen);

        if (ret != 0)
        {
            perror("crow::udpgate::bind");
            exit(-1);
        }
    }

    else
    {
        // dynamic port assign
        ret = 0;
    }

    int flags = fcntl(sock, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(sock, F_SETFL, flags);

    return ret;
}

void crow::udpgate::close()
{
    if (sock > 0)
    {
        ::shutdown(sock, SHUT_RDWR);
        ::close(sock);
        sock = -1;
    }
}

void crow::udpgate::send(crow_packet *pack)
{
    uint32_t *addr = (uint32_t *)(crow_packet_stageptr(pack)  + 1);
    uint16_t *port = (uint16_t *)(crow_packet_stageptr(pack) + 5);

    struct sockaddr_in ipaddr;
    socklen_t iplen = sizeof(struct sockaddr_in);
    memset(&ipaddr, 0, iplen);

    //Конвертация не требуется, т.к. crow использует сетевой порядок записи
    //адреса.
    ipaddr.sin_port = *port;
    ipaddr.sin_addr.s_addr = *addr;

    sendto(sock, (const char *)&pack->header, pack->header.flen, 0,
           (struct sockaddr *)&ipaddr, iplen);
    crow::return_to_tower(pack, CROW_SENDED);
}

int crow::create_udpgate(uint8_t id, uint16_t port)
{
    int sts;

    crow::udpgate *g = new crow::udpgate;
    if ((sts = g->open(port)))
        return sts;

    if ((sts = g->bind(id)))
    {
        g->close();
        return sts;
    }

    return 0;
}
