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



 int snd_socket_init(void)
{
	int sock;
	sock = socket(AF_INET,SOCK_DGRAM,0);

	return sock;
}

void send_packet(unsigned char *tx_buf ,int length, char *ip, int port)//SNDBUFSIZE=30
{

	int i=0;
	struct sockaddr_in gSndAddr;

//	printf("ip = %s\n",ip);

	memset(&gSndAddr, 0, sizeof(gSndAddr));
	gSndAddr.sin_family = AF_INET;
	gSndAddr.sin_addr.s_addr = inet_addr(ip);

//	gSndAddr.sin_addr.s_addr = src_Addr.sin_addr.s_addr;

	//printf("ip= %s",inet_ntoa(src_Addr.sin_addr));

	gSndAddr.sin_port = htons(port);

	sendto(gSndSocket, tx_buf, length, 0, (struct sockaddr*)&gSndAddr, sizeof(struct sockaddr_in));


}



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
		printf("error bind failed \n");
		close(sock);
		sock = -1;
		exit(0);
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





int src_type_parse(char *buff, int len)
{
	int i=0, j=0 ;
	int cnt;
	//char type[3];
	int flag =0;
    char type[2];
	 memset(type,0,2);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "srctype=", 8) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+8;
			// printf("find src_type %d\n", atoi(&buff[i]));
			 return atoi(&buff[i]);
		}

	}

	return  0;
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
	//		 printf("find srcrcvuri \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			//printf(" buff[i] = %02x\n",  buff[i]);
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
	 memset(type,0,2);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sinktype=", 9) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+9;
	//		 printf("find sinktype %d\n", atoi(&buff[i]));
			return atoi(&buff[i]);
		}
 
	}
	return  0;
}



int sink_src_port_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[6];
	 memset(type,0,6);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sinksrcport=", 12) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+12;
	//		 printf("find sinksrcport=%d \n", atoi(&buff[i]));
		 	 return atoi(&buff[i]);
		}

		 

	}
	return  0;
}


int sink_dst_port_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[6];
	 memset(type,0,6);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sinkdstport=", 12) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+12;
	//		 printf("find sinkdstport=%d \n", atoi(&buff[i]));
		 	 return atoi(&buff[i]);
		}
	}
	return 0;
}

int source_dst_port_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[6];
	memset(type,0,6);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sourcedstport=", 14) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+14;
		//	 printf("find sourcedstport= %d \n", atoi(&buff[i]));
		 	 return atoi(&buff[i]);
		}
 

	}
	return  0;
}

int source_src_port_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[6];
	memset(type,0,6);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sourcesrcport=", 14) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+14;
			// printf("sourcesrcport= %d \n", atoi(&buff[i]));
		 	 return atoi(&buff[i]);
		}
	}
	return  0;
}

int rcv_min_port_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[6];
	memset(type,0,6);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "RCV_PORT_MIN=", 13) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+13;
			 printf("RCV_PORT_MIN= %d\n", atoi(&buff[i]));
		 	 return atoi(&buff[i]);
		}

	}
	return  60000;
}

int rcv_max_port_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[6];
	memset(type,0,6);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "RCV_PORT_MAX=", 13) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+13;
			 printf("RCV_PORT_MAX= %d\n", atoi(&buff[i]));
		 	 return atoi(&buff[i]);
		}
	}
	return  61000;
}

int snd_min_port_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[6];
	memset(type,0,6);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "SND_PORT_MIN=", 13) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+13;
			 printf("SND_PORT_MIN=%d \n" ,atoi(&buff[i]));
			
		 	 return atoi(&buff[i]);
		}

		 

	}
	return  61001;
}

int snd_max_port_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[6];
	memset(type,0,6);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "SND_PORT_MAX=", 13) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+13;
			 printf("SND_PORT_MAX= %d \n", atoi(&buff[i]));
		 	 return atoi(&buff[i]);
		}

	}
	return  62000;
}

int sink_keep_alive_parse(char *buff, int len)
{
	int i=0, j=0;

	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sinkplv=ok", 10) == 0)
		{

		//	 printf("find sinkplv=ok \n");
			 return TRUE;
		}

	}
	return FALSE;
}

int source_keep_alive_parse(char *buff, int len)
{
	int i=0, j=0;

	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sourcekplv=ok", 13) == 0)
		{

		//	 printf("find sourcekplv=true \n");
			 return TRUE;
		}

	}
	return FALSE;
}

int remote_ip_parse(char *buff, int len, char *remote_ip)
{
	int i=0, j=0 ;
	int cnt;
	//char type[3];
	int flag =0;

	memset(remote_ip,0,20);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "myip=", 5) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+5;
			 printf("find myip \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			//printf(" buff[i] = %02x\n",  buff[i]);
			  return 1;
			// break;
		}

		if(flag == 1)
		{
			remote_ip[j++] = buff[i];

		}

	}
	return  0;

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
	 	//	 printf("find sipuri \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
		//	printf(" buff[i] = %02x\n",  buff[i]);
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

int rtspaddr_parse(char *buff, int len, char *rtspaddr)
{
	int i=0, j=0 ;
	int cnt;
	//char type[3];
	int flag =0;

	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "rtspaddr=", 9) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+9;
	 	//	 printf("find sipuri \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
		//	printf(" buff[i] = %02x\n",  buff[i]);
			  return 1;
			// break;
		}

		if(flag == 1)
		{
			rtspaddr[j++] = buff[i];

		}

	}
	return  0;

}

int jftcpstring_parse(char *buff, int len, char *jftcp)
{
	int i=0, j=0 ;
	int cnt;
	//char type[3];
	int flag =0;

	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "jftcpstring=", 12) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+12;
	//		 printf("find sipuri \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			//printf(" buff[i] = %02x\n",  buff[i]);
			  return 1;
			// break;
		}

		if(flag == 1)
		{
			jftcp[j++] = buff[i];

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
 //   printf("find buff =%s \n", buff);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "reqadd=ok", 9) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+9;
		//	 printf("find reqadd=ok \n");
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
			 i=i+7;
		//	 printf("find callid \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			//printf(" buff[i] = %02x\n",  buff[i]);
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
			// printf("find sinkdsturi \n");
		}

		if((flag == 1) && (buff[i+1]== 0x3b))
		{
		//	printf(" buff[i] = %02x\n",  buff[i]);
			*dst_port =   atoi(port);

		//	printf("sink dst port = %d \n" ,*dst_port);
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

int sink_dst_ip_parse(char *buff, int len, char *dst_ip)
{
	int i=0, j=0, k=0;
	int cnt;
	//char type[3];
	int flag =0;
    char port[6];
    int maohaoflag = 0;
	memset(dst_ip,0,20);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sinkdstip=", 10) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+10;
		//	 printf("find sinkdstip \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
		//	printf(" buff[i] = %02x\n",  buff[i]);
			return 1;
            break;

		}

		if(flag == 1)
		{
			dst_ip[j++] = buff[i];
		}

	}
	return 0;
//	return  atoi(type);
}

int sink_src_ip_parse(char *buff, int len, char *dst_ip)
{
	int i=0, j=0, k=0;
	int cnt;
	//char type[3];
	int flag =0;
    char port[6];
    int maohaoflag = 0;
    memset(dst_ip,0,20);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sinksrcip=", 10) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+10;
		//	 printf("find sinksrcip \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
		//	printf(" buff[i] = %02x\n",  buff[i]);
			return 1;
            break;

		}

		if(flag == 1)
		{
			dst_ip[j++] = buff[i];
		}

	}
	return 0;
//	return  atoi(type);
}

int source_src_ip_parse(char *buff, int len, char *dst_ip)
{
	int i=0, j=0, k=0;
	int cnt;
	//char type[3];
	int flag =0;
    char port[6];
    int maohaoflag = 0;
    memset(dst_ip,0,20);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sourcesrcip=", 12) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+12;
		//	 printf("find sourcesrcip \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
		//	printf(" buff[i] = %02x\n",  buff[i]);
			return 1;
            break;

		}

		if(flag == 1)
		{
			dst_ip[j++] = buff[i];
		}

	}
	return 0;
//	return  atoi(type);
}


int source_dst_ip_parse(char *buff, int len, char *dst_ip)
{
	int i=0, j=0, k=0;
	int cnt;
	//char type[3];
	int flag =0;
    char port[6];
    int maohaoflag = 0;
    memset(dst_ip,0,20);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sourcedstip=", 12) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+10;
	//		 printf("find sourcedstip \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
		//	printf(" buff[i] = %02x\n",  buff[i]);
			return 1;
            break;

		}

		if(flag == 1)
		{
			dst_ip[j++] = buff[i];
		}

	}
	return 0;
//	return  atoi(type);
}

int Nat_ip_parse(char *buff, int len, char *dst_ip)
{
	int i=0, j=0, k=0;
	int cnt;
	int flag =0;
	memset(dst_ip,0,20);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "NAT_IP=", 7) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+7;
			 printf("find NAT_IP ==");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
	 		printf(" %s\n",  dst_ip);
			return 1;
            break;

		}

		if(flag == 1)
		{
			dst_ip[j++] = buff[i];
		}

	}
	return 0;
}

int rcv_ip_parse(char *buff, int len, char *dst_ip)
{
	int i=0, j=0, k=0;
	int cnt;
	int flag =0;
	memset(dst_ip,0,20);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "LOCAL_IP=", 9) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+9;
			 printf("find LOCAL_IP =");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
		 	printf(" %s\n", dst_ip);
			return 1;
            break;

		}

		if(flag == 1)
		{
			dst_ip[j++] = buff[i];
		}

	}
	return 0;
}

int audio_codec_parse(char *buff, int len)
{
	int i=0, j=0, k=0;
	int cnt;
	int flag =0;

    if(strstr(buff,"AUDIO_CODEC=AAC"))
	{
    	return 1;   // CODEC_AAC
	}
    else if(strstr(buff,"AUDIO_CODEC=AAC"))
    {
    	return 2;   // CODEC_PCMULAW
    }
    // default
	return 2;
}


int control_ip_parse(char *buff, int len, char *dst_ip)
{
	int i=0, j=0, k=0;
	int cnt;
	int flag =0;
	memset(dst_ip,0,20);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "CONTROL_IP=", 11) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+11;
			 printf("find CONTROL_IP == ");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
	 		printf("%s\n",  dst_ip);
			return 1;
            break;

		}

		if(flag == 1)
		{
			dst_ip[j++] = buff[i];
		}

	}
	return 0;
}


int get_total_session(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[2];
	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "getsession", 10) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+10;
			 printf("find getsession=ok \n");
			 return 1;
		}

	}
	return 0;
}

void keep_alive_string_parse(char *buff, int len, char *str)
{

	int i=0, j=0, k=0;
	int cnt;
	int flag =0;
	//memset(dst_ip,0,20);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sndkplvstring=", 14) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+14;
		//	 printf("find sndkplvstring \n");
		}

		if((flag == 1) && (buff[i]== 0x3b))
		{
			//printf(" buff[i] = %02x\n",  buff[i]);
			return 1;
            break;

		}

		if(flag == 1)
		{
			str[j++] = buff[i];
		}

	}
	return 0;
}

int keep_alive_string_len_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[6];
	memset(type,0,6);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "sndkplvlen=", 11) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+11;
		//	 printf("find sndkplvlen=%d \n", atoi(&buff[i]));
		 	 return atoi(&buff[i]);
		}
 

	}
	return  0;
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
		//	 printf("find invite=ok \n");
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
	//		 printf("find bye=ok \n");
			 return 1;
		}

	}
	return 0;
}

int Nat_parse(char *buff, int len)
{
	int i=0, j=0;
	int cnt;
	//char type[3];
	int flag =0;
    char type[2];
	//memset(type,0,3);
	for(i =0 ;i < len ; i++)
	{
		if(memcmp(&buff[i], "nat=ok", 6) == 0)
		{
			 cnt = i;
			 flag = 1;
			 i=i+6;
	//		 printf("find nat=ok \n");
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

int udp_port_available_check(int port)
{
	//int port = 8888;
	int flag = 0;
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
	if(bind(fd, (struct sockaddr *)(&addr), sizeof(struct sockaddr)) < 0)
	{
		printf("port %d has been used.\n", port);
		close(fd);
		return flag = -1;
	}

	close(fd);
	return flag = 1;

}


int tcp_port_available_check(int port)
{
	//int port = 8888;
	int flag = 0;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, "0.0.0.0", &addr.sin_addr);
	if(bind(fd, (struct sockaddr *)(&addr), sizeof(struct sockaddr)) < 0)
	{
		printf("port %d has been used.\n", port);
		close(fd);
		return flag = -1;
	}

	close(fd);
	return flag = 1;

}

