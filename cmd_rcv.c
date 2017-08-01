#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "struct.h"

#define Ref_BufferSize 64
#define BroadcastWaitTime 3
#define WaitTime 10
#define True  1
#define False 0
//extern int port_init(int num);
//extern void DTV_Close();
#define RCVBUFSIZE     1500
#define SNDBUFSIZE     1500
#define RCV_PORT       50000
#define SND_PORT       50002

int gRcvSocket;
int gSndSocket;
int exit_flag = False;

struct sockaddr_in rcvAddr;
struct sockaddr_in gSndAddr;




  int rcv_socket_init(void)
{
	int sock;
	//struct sockaddr_in rcvAddr;
	sock = socket(AF_INET,SOCK_DGRAM,0); 


	memset(&rcvAddr, 0, sizeof(rcvAddr));
	rcvAddr.sin_family = AF_INET;
	//rcvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	rcvAddr.sin_addr.s_addr = inet_addr("0.0.0.0");
	rcvAddr.sin_port = htons(RCV_PORT);
	if (bind(sock, (struct sockaddr *) &rcvAddr, sizeof(struct sockaddr)) < 0)
	{
		printf("error bind failed");
		close(sock);
		sock = -1;
	}
	return sock;
}




int receive_packet(unsigned char *rx_buf)
{
	int rec_size=0;
	int recvaddr_len=0;
	int i=0;

	recvaddr_len=sizeof(struct sockaddr_in);

	rec_size=recvfrom(gRcvSocket, (void *)rx_buf, RCVBUFSIZE, 0, (struct sockaddr *)&rcvAddr, &recvaddr_len);
	//if(rec_size!=RCVBUFSIZE)
	//{
	printf("rec_size = %d rec_buf %s\n", rec_size,rx_buf);
	//}

	printf("rx_buf data:");
//	for(i=0;i<rec_size;i++)
//	{
//	printf("%02x ",rx_buf[i]);
//	}
	printf("\n");
	return rec_size;

}



void Stop(int signo)
{
    printf("oops! stop!!!\n");
    exit_flag = True;

	close(gRcvSocket);
	//close(gSndSocket);

    _exit(0);
}

int src_type_parse(char *buff, int len)
{
	int i=0, j=0 ;
	int cnt;
	//char type[3];
	int flag =0;
    char type[2];
	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "srctype=", 8) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+8;
			 printf("find src_type \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			printf(" buff[i] = %02x\n",  buff[i]);
		//	return 1;
			 break;
		}

		if(flag == 1)
		{
			type[j++] = buff[i];

		}


	}

	return  atoi(type);

	//return  0;
}

int src_uri_parse(char *buff, int len, char *dst)
{
	int i=0, j=0 ;
	int cnt;
	//char type[3];
	int flag =0;

	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "srcrcvuri=", 10) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+10;
			 printf("find srcrcvuri \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			printf(" buff[i] = %02x\n",  buff[i]);
			  return 1;
		    // break;
		}

		if(flag == 1)
		{
			dst[j++] = buff[i];

		}

	}
	return  0;
}

int sink_type_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[2];
	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sinktype=", 9) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+9;
			 printf("find sinktype \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			printf(" buff[i] = %02x\n",  buff[i]);
			//return 1;
			 break;
		}

		if(flag == 1)
		{
			type[j++] = buff[i];

		}

	}
	return  atoi(type);
}



int sink_src_port_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[6];
	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sinksrcport=", 12) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+12;
			 printf("find sinkplv=true \n");
		//	 return TRUE;
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			printf(" buff[i] = %02x\n",  buff[i]);
			//return 1;
			 break;
		}

		if(flag == 1)
		{
			type[j++] = buff[i];

		}

	}
	return  atoi(type);
}

int sink_keep_alive_parse(char *buff, int len)
{
	int i=0, j=0;

	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sinkplv=true", 12) == 0)
		{

			 printf("find sinkplv=true \n");
			 return TRUE;
		}

	}
	return FALSE;
}


int sipuri_parse(char *buff, int len, char *sipuri)
{
	int i=0, j=0 ;
	int cnt;
	//char type[3];
	int flag =0;

	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sipuri=", 7) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+7;
			 printf("find sipuri \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			printf(" buff[i] = %02x\n",  buff[i]);
			  return 1;
			// break;
		}

		if(flag == 1)
		{
			sipuri[j++] = buff[i];

		}

	}
	return  0;

}

int request_address(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[2];
	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "reqadd=ok", 9) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+9;
			 printf("find reqadd=ok \n");
			 return 1;
		}

	}
	return 0;
}


int callid_parse(char *buff, int len, char *callid)
{
	int i=0, j=0 ;
	int cnt;
	//char type[3];
	int flag =0;

	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "callid=", 6) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+6;
			 printf("find callid \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			printf(" buff[i] = %02x\n",  buff[i]);
			  return 1;
			// break;
		}

		if(flag == 1)
		{
			callid[j++] = buff[i];

		}

	}
	return  0;

}

int sink_dst_uri_parse(char *buff, int len, char *dst_ip, int *dst_port)
{
	int i=0, j=0, k=0;
	int cnt;
	//char type[3];
	int flag =0;
    char port[6];
    int maohaoflag = 0;

    *dst_port = 0;
	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sinkdsturi=", 11) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+11;
			 printf("find sinkdsturi \n");
		}

		if((flag == 1) && (buff[i+1]== 0x3b))
		{
			printf(" buff[i] = %02x\n",  buff[i]);
			*dst_port =   atoi(port);

			printf("sink dst port = %d \n" ,*dst_port);
			return 1;
			break;
		}

		if(flag == 1)
		{
			if(maohaoflag == 0 && buff[i] != ':')
			{
				dst_ip[j++] = buff[i];

			}
			else
			{
		    	//	printf("find : \n");
				maohaoflag = 1;
				port[k++] = buff[i+1];
			}

		}

	}
	return 0;
//	return  atoi(type);
}

int invite_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[2];
	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "invite=ok", 9) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+9;
			 printf("find invite=ok \n");
			 return 1;
		}

	}
	return 0;
}

int bye_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[2];
	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "bye=ok", 6) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+6;
			 printf("find bye=ok \n");
			 return 1;
		}

	}
	return 0;
}

//设置非阻塞
static void setnonblocking(int sockfd) {
    int flag = fcntl(sockfd, F_GETFL, 0);
    if (flag < 0) {
    	perror("fcntl F_GETFL fail");
        return;
    }
    if (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) < 0) {
        perror("fcntl F_SETFL fail");
    }
}

int  rcv_keep_alive_socket_init(Sink *Sink)
{
//	 int sockfd;
//	 struct sockaddr_in servaddr, cliaddr;
//	 Sink->src_fd = socket(AF_INET, SOCK_DGRAM, 0);
//	if (sockfd == -1) {
//		perror("socket failed:");
//		return -1;
//	}
//	 setnonblocking(Sink->src_fd);
//
//	bzero(&servaddr, sizeof(servaddr));
//
//	servaddr.sin_family = AF_INET;
//	servaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
//	servaddr.sin_port = htons(Sink->src_port);
//
//	if(bind(Sink->src_fd, (sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
//		perror("bind failed:");
//		return -1;
//	}
//	return sockfd;

}


void rcv_keep_alive_socket_read(Sink *Sink)
{
//	char mesg[1500] = {};
//	struct sockaddr_in  cliaddr;
//	socklen_t len = sizeof(cliaddr);
//
//	GList list;
//   // int n = recvfrom(sockfd, mesg, 1024, 0, (sockaddr *)&cliaddr, &len);
//	while(recvfrom(Sink->src_fd, mesg, 1024, 0, (sockaddr *)&cliaddr, &len) >=0)
//	{
//		//if()
//		printf("ip= %s \n",inet_ntoa(cliaddr.sin_addr));
//   		printf("port= %d \n", ntohs(cliaddr.sin_port));
//		printf("len= %d \n", len);
//	}
//	printf(" msg=%s \n",  mesg);

}

void  rcv_keep_alive_socket_close(int fd)
{
	close(gSndSocket);
}



