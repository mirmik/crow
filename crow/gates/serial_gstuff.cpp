/** @file */

#include <crow/gates/serial_gstuff.h>
#include <crow/tower.h>
#include <crow/warn.h>

#include <nos/io/serial_port.h>
#include <igris/util/bug.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <termios.h>

void crow::serial_gstuff::newline_handler()
{
    crow::compacted_packet *block = rpack;
    rpack = NULL;

    block->revert_gate(id);

    crow_packet_initialization(block, this);
    crow::nocontrol_travel(block, false);
}

struct crow::serial_gstuff *crow::create_serial_gstuff(const char *path,
        uint32_t baudrate,
        uint8_t id, bool debug)
{
    crow::serial_gstuff *g = new crow::serial_gstuff;

    g->debug = debug;

    g->fd = open(path, O_RDWR | O_NOCTTY);
    fcntl(g->fd, F_SETFL, fcntl(g->fd, F_GETFL) | O_NONBLOCK);

    if (g->fd < 0)
    {
        perror("serial::open");
        exit(0);
    }

    g->rpack = NULL;
    g->bind(id);

    return g;
}

void crow::serial_gstuff::send(crow::packet *pack)
{
    char buffer[512];

    header_v1 header = pack->extract_header_v1();
    struct iovec iov[] =
    {
        { &header, sizeof(header) },
        { pack->addrptr(), pack->addrsize() },
        { pack->dataptr(), pack->datasize() },
    };
    int size = gstuffing_v(iov, 3, buffer);

    write(fd, buffer, size);
    crow::return_to_tower(pack, CROW_SENDED);
}

void crow::serial_gstuff::nblock_onestep()
{
#define GSTUFF_MAXPACK_SIZE 512
    char buf[1024];
    ssize_t len = read(fd, (uint8_t *)buf, 1024);

    for (int i = 0; i < len; ++i)
    {
        char c = buf[i];

        if (debug)
        {
            debug_printhex_uint8(c);
            debug_putchar('\t');
            debug_putchar(c);
            debug_print_newline();
        }

        if (rpack == NULL)
        {
            rpack = (crow::compacted_packet *)malloc(GSTUFF_MAXPACK_SIZE +
                    sizeof(crow::compacted_packet) -
                    sizeof(crow::header_v1));
            new (rpack) crow::compacted_packet;
            rpack->destructor = +[](crow::packet*pack){free(pack);};
            gstuff_autorecv_setbuf(&recver, (char *)&rpack->header(),
                                   GSTUFF_MAXPACK_SIZE);
        }

        int ret = gstuff_autorecv_newchar(&recver, c);

        switch (ret)
        {
            case GSTUFF_CRC_ERROR:
                crow::warn("warn: gstuff crc error");
                break;

            case GSTUFF_NEWPACKAGE:
                newline_handler();
                break;

            default:
                break;
        }
    }
}

void crow::serial_gstuff::setup_serial_port(
    int baud, char parity, int databits, int stopbits)
{
    nos::serial_port port(fd);
    port.setup(baud, parity, databits, stopbits);
}