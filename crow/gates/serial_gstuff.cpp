/** @file */

#include <crow/gates/serial_gstuff.h>
#include <crow/tower.h>
#include <crow/warn.h>

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
    int ret;

    crow::serial_gstuff *g = new crow::serial_gstuff;

    g->debug = debug;

    g->fd = open(path, O_RDWR | O_NOCTTY);
    fcntl(g->fd, F_SETFL, fcntl(g->fd, F_GETFL) | O_NONBLOCK);

    if (g->fd < 0)
    {
        perror("serial::open");
        exit(0);
    }

    struct termios tattr, orig;
    ret = tcgetattr(g->fd, &orig);

    if (ret < 0)
    {
        perror("serial::tcgetattr");
        exit(0);
    }

    tattr = orig; /* copy original and then modify below */

    /* input modes - clear indicated ones giving: no break, no CR to NL,
       no parity check, no strip char, no start/stop output (sic) control */
    tattr.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    /* output modes - clear giving: no post processing such as NL to CR+NL */
    tattr.c_oflag &= ~(OPOST);

    /* control modes - set 8 bit chars */
    tattr.c_cflag |= (CS8);

    /* local modes - clear giving: echoing off, canonical off (no erase with
       backspace, ^U,...),  no extended functions, no signal chars (^Z,^C) */
    tattr.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    /* control chars - set return condition: min number of bytes and timer */
    tattr.c_cc[VMIN] = 0;
    tattr.c_cc[VTIME] = 0; /* immediate - anything       */

    if (baudrate == 115200)
    {
        cfsetispeed(&tattr, B115200);
        cfsetospeed(&tattr, B115200);
    }
    else
    {
        BUG();
    }

    tattr.c_cflag &= ~(PARENB);
    tattr.c_cflag &= ~(PARODD);

    /* put terminal in raw mode after flushing */
    ret = tcsetattr(g->fd, TCSAFLUSH, &tattr);

    if (ret < 0)
    {
        perror("serial::tcsetattr");
    }

    g->rpack = NULL;
    g->bind(id);

    return g;
}

void crow::serial_gstuff::send(crow::packet *pack)
{
    char buffer[512];

    int len = gstuffing((char *)&dynamic_cast<compacted_packet*>(pack)->header(), pack->full_length(), buffer);
    write(fd, buffer, len);
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
