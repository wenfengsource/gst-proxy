#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <sys/wait.h>
#include <sys/types.h>

#define Ref_BufferSize 64
#define BroadcastWaitTime 3
#define WaitTime 10
#define True  1
#define False 0
//extern int port_init(int num);
//extern void DTV_Close();
#define RCVBUFSIZE     1500
#define SNDBUFSIZE     1500

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
	rcvAddr.sin_port = htons(5562);
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
	printf("rec_size = %d\n", rec_size);
	//}

	printf("rx_buf data:");
	for(i=0;i<rec_size;i++)
	{
	printf("%02x ",rx_buf[i]);
	} 
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







