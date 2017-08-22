#ifndef  STRUCT
#define  STRUCT


#include <string.h>
#include <gst/gst.h>

#include <sys/wait.h>
#include <sys/types.h>

#include <gio/gio.h>


typedef struct
{
	gint source_id;  //check headbeat
	GIOChannel* io_channel;
}Heatbeat;



typedef struct
{
//	char src_ip[20];
	char dst_ip[20];
//	int src_port;
	int dst_port;
	int keep_alive;
	int time_cnt;
//	int sock_fd;
	char callid[50];
}SinkAddress;


typedef struct
{
	GSocket *sock;
	char ip[20];
	int port;
	char callid[50];
	char sipuri[100];
}Tcpclientsocketinfo;


typedef struct
{
  GstPad *teepad;
  GstElement *queue;
  GstElement *conv;
  GstElement *depay;
  GstElement *sink;
  //gboolean removing;

  char dst_ip[20];
  int dst_port;

  char *dst_uri;

  int src_port;
  char src_ip[20];

  char callid[50];

  char sipuri[100];

  int src_fd;  // socket used for receive keep_alive data
  int keep_alive_flag;
 // SinkAddress *sinkaddress;

  //Heatbeat  rcv_beatheart;
 // int snd_port;
  int type;  // UDP/RTP/TCP

  GSocket *sndkeepalive_socket;
  guint sourceid;   // detect keep_alive source id
  int Nat_Traversal;  // NAT
  int Get_Nat_address_flag;
  //GHashTable Multi_Address; //  address as key, keep_live as value
  int tcp_client_count;
  //int tcp_client_callid;
  //GList *Address_list;

 // Tcpclientsocketinfo  *tcpclientsock;
  GHashTable *tcpclienthashtb;   // call id as key SinkAddress as value
} Sink;

typedef struct
{
	GstElement *src;
	GstElement *rndbuffersize;
	GstElement *tee;
	GstElement *capsfilter;

    char dst_ip[20];
    char src_ip[20];
    int dst_port;
    int src_port;
    GSocket *sndkeepalive_socket;

    GSocketAddress *address;

    guint timesourceid;
    char src_uri[50];
    int keep_alive_flag;

    int type;   // TCP/RTP/UDP
}Source;


typedef struct
{
	Sink *sink;  //getting from  sink_hashtable
	Source source;
    GstElement *pipeline;
	GstBus *bus;
	char sip_uri[100];  //  = sip_uri
	GMainLoop* loop;
	GThread * gthread;
	GHashTable *sink_hashtable; //sink type, udp/rtp/tcp  // call_id

	Sink *tcpsink;  // Only have 1 tcpsink

} GstCustom;

#endif
