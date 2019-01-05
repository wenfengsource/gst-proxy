#ifndef CMD_RCV
#define CMD_RCV

extern int gRcvSocket;
extern int gSndSocket;
int rcv_socket_init(void);
int receive_packet(unsigned char *rx_buf);
void Stop(int signo);


#define MPEGPS         1
#define MPEGTS         2


#endif
