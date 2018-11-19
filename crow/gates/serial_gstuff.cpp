#include <crow/gates/serial_gstuff.h>

#include <crow/tower.h>

#include <gxx/gstuff/autorecv.h>
#include <gxx/gstuff/gstuff.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <termios.h>

const struct crow::gateway_operations crow_serial_gstuff_ops;

struct crow_serial_gstuff {
	int fd;

	struct crow::gateway gw;
	struct crow::packet * rpack;
	bool debug;

	struct gstuff_autorecv recver;
	//struct gstuff_sender sender;

} crow_serial_gstuff;


static inline void newline_handler(struct crow_serial_gstuff* g) 
{
	struct crow::packet * block = g->rpack;
	g->rpack = NULL;

	crow::packet_revert_g(block, g->gw.id);

	crow::packet_initialization(block, &g->gw);
	crow_travel(block);
}

struct crow::gateway* crow_create_serial_gstuff(const char* path, uint32_t baudrate, uint8_t id, bool debug) 
{
	int ret;

	struct crow_serial_gstuff* g = (struct crow_serial_gstuff*) 
		malloc(sizeof(struct crow_serial_gstuff));

	g->debug = debug;
	
	g->fd = open(path, O_RDWR | O_NOCTTY);
	if (g->fd < 0) {
		perror("serial::open");
		exit(0);
	}

	//dprf("%d, %s\n", g->fd, path);

	struct termios tattr, orig;
	ret = tcgetattr(g->fd, &orig);
	if (ret < 0) {
		perror("serial::tcgetattr");
		exit(0);
	}

	tattr = orig;  /* copy original and then modify below */

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
    tattr.c_cc[VMIN] = 0; tattr.c_cc[VTIME] = 0; /* immediate - anything       */

    cfsetispeed(&tattr, B115200);
	cfsetospeed(&tattr, B115200);
    
    /* put terminal in raw mode after flushing */
    ret = tcsetattr(g->fd,TCSAFLUSH,&tattr); 
	if (ret < 0) {
		perror("serial::tcsetattr");
	}

	g->rpack = NULL;
	g->gw.ops = &crow_serial_gstuff_ops;
	crow_link_gate(&g->gw, id);

	//gstuff_autorecv_init(&g->recver, callback_handler, g);
	
	//dprln("crow_create_serial_gstuff... exit");
	return &g->gw;
}

void crow_serial_gstuff_send(struct crow::gateway* gw, struct crow::packet* pack) 
{
	//dprln("crow_serial_gstuff_send");

	char buffer[pack->header.flen * 2 + 3];

	struct crow_serial_gstuff* g = mcast_out(gw, struct crow_serial_gstuff, gw);
	/*std::string str;
	gxx::io::std_string_writer strm(str);
	gxx::gstuff::sender sender(strm);

	sender.start_message();
	sender.write((char*)&pack->header, pack->header.flen);
	sender.end_message();

	mtx.lock();
	ser->write((uint8_t*)str.data(), str.size());
	mtx.unlock();
*/
	int len = gstuffing((char*)&pack->header, pack->header.flen, buffer);

	write(g->fd, buffer, len);

	crow_return_to_tower(pack, CROW_SENDED);
}

/*void crow_serial_gstuff_send(struct crow::gateway* gw, struct crow::packet* pack) 
{

}*/


void crow_serial_gstuff_nblock_onestep(struct crow::gateway* gw) 
{
	struct crow_serial_gstuff* g = mcast_out(gw, struct crow_serial_gstuff, gw);
	
	if (g->rpack == NULL) {
		g->rpack = (struct crow::packet*) 
			malloc(128 + sizeof(struct crow::packet) - sizeof(struct crow::header));
		gstuff_autorecv_setbuf(&g->recver, (char*)&g->rpack->header, 128);
	}

	char c;
	int len = read(g->fd, (uint8_t*)&c, 1);
	//int len = ser->read((uint8_t*)&c, 1);
	if (len == 1) {
		if (g->debug) {
			dprhex(c); dprchar('\t'); dprchar(c); dln();
		}
		
		int ret = gstuff_autorecv_newchar(&g->recver, c);

		switch (ret) {
			case GSTUFF_CRC_ERROR: dprln("warn: gstuff crc error"); break;
			case GSTUFF_NEWPACKAGE: newline_handler(g); break;
			default: break;
		}
	}
}

const struct crow::gateway_operations crow_serial_gstuff_ops = {
	.send = crow_serial_gstuff_send,
	.nblock_onestep = crow_serial_gstuff_nblock_onestep
};
