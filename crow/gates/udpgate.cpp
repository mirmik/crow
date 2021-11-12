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
#include <sys/uio.h>

#include <memory>

void crow::udpgate::nblock_onestep()
{
    crow::header_v1 header;

    struct sockaddr_in sender;
    socklen_t sendsize = sizeof(sender);
    memset(&sender, 0, sizeof(sender));

    ssize_t len = recvfrom(sock, &header, sizeof(crow::header_v1), MSG_PEEK,
                           (struct sockaddr *)&sender, &sendsize);

    if (len <= 0)
        return;

    //size_t flen = header.flen;

    crow::morph_packet *block = nullptr;
    //if (!block)
    //    block = crow_allocate_packet(flen - sizeof(crow::header_v1));

    if (!block)
    {
        block = new crow::morph_packet();
        block->parse_header(header);
        block->allocate_buffer(block->addrsize(), block->datasize());
    }

    struct iovec iov[] =
    {
        {&header, sizeof(header)},
        {block->addrptr(), block->addrsize()},
        {block->dataptr(), block->datasize()}
    };

    //recvmsg(sock, &msg, 0);
    readv(sock, iov, 3);

    //len = recvfrom(sock, &block->header(), flen, 0, (struct sockaddr *)&sender,
    //               &sendsize);

    crow_packet_initialization(block, this);

    igris::buffer vec[3] = {{(char *)&id, 1},
        {(char *)&sender.sin_addr.s_addr, 4},
        {(char *)&sender.sin_port, 2}
    };

    block->revert(vec, 3);

    crow::packet *pack = block;
    block = NULL;

    crow::nocontrol_travel(pack, fastsend);
}

int crow::udpgate::open(uint16_t port)
{
    int ret;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

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

void crow::udpgate::send(crow::packet *pack)
{
    uint32_t *addr = (uint32_t *)(pack->stageptr() + 1);
    uint16_t *port = (uint16_t *)(pack->stageptr() + 5);

    struct sockaddr_in ipaddr;
    socklen_t iplen = sizeof(struct sockaddr_in);
    memset(&ipaddr, 0, iplen);

    //Конвертация не требуется, т.к. crow использует сетевой порядок записи
    //адреса.
    ipaddr.sin_port = *port;
    ipaddr.sin_addr.s_addr = *addr;

    header_v1 header = pack->extract_header_v1();

    char buf[header.flen];
    memcpy(buf, &header, sizeof(header));
    memcpy(buf + sizeof(header), pack->addrptr(), pack->addrsize());
    memcpy(buf + sizeof(header) + pack->addrsize(), pack->dataptr(), pack->datasize());

    //sendto(sock, (const char *)&dynamic_cast<crow::compacted_packet *>(pack)->header(), pack->full_length(), 0,
    //       (struct sockaddr *)&ipaddr, iplen);
    sendto(sock, buf, pack->full_length(), 0,
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

std::shared_ptr<crow::udpgate> crow::create_udpgate_safe(uint8_t id,
        uint16_t port)
{
    int sts;

    auto g = std::make_shared<crow::udpgate>();
    if ((sts = g->open(port)))
        return g;

    if ((sts = g->bind(id)))
    {
        g->close();
        return g;
    }

    return g;
}
