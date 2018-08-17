#ifndef  STRUCT
#define  STRUCT


#include <string.h>
#include <gst/gst.h>

#include <sys/wait.h>
#include <sys/types.h>

#include <gio/gio.h>


//typedef struct
//{
//	gint source_id;  //check headbeat
//	GIOChannel* io_channel;
//}Heatbeat;


//
//typedef struct
//{
////	char src_ip[20];
//	char dst_ip[20];
////	int src_port;
//	int dst_port;
//	int keep_alive;
//	int time_cnt;
////	int sock_fd;
//	char callid[100];
//}SinkAddress;
//

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

  char callid[100];

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
  //int tcp_client_count;
  //int tcp_client_callid;
  //GList *Address_list;
  char jftcpstring[301];
 // Tcpclientsocketinfo  *tcpclientsock;
  //GHashTable *tcpclienthashtb;   // call id as key SinkAddress as value
} Sink;

typedef struct
{
	GstElement *src;
	GstElement *h264depay;  //for rtsp
	GstElement *h264parse;  // rtsp
	GstElement *audiodepay;  //for rtsp
	GstElement *aacparse;

	GstElement *mpegtsmux;
	GstElement *rndbuffersize;
	GstElement *tee;
	GstElement *capsfilter;

    char dst_ip[20];
    char src_ip[20];
    char rtspaddr[100];
    int dst_port;
    int src_port;
  //  GSocket *sndkeepalive_socket;
	char keep_alive_str[100];
	int keep_alive_str_lenth;
   // GSocketAddress *address;

  //  guint timesourceid;
 //   char src_uri[100];
    int keep_alive_flag;

    int type;   // TCP/RTP/UDP Jftcp
}Source;


typedef struct
{
	Sink  *sink;  //getting from  sink_hashtable
	Source source;
    GstElement *pipeline;
	GstBus *bus;
	char sipuri[100];  //  = sip_uri
	char callid[100];
	GMainLoop* loop;
	GThread * gthread;
	 GMutex sink_hash_mutex;
	 GHashTable *sink_hashtable; //sink type, udp/rtp/tcp  // call_id
     int pipeline_flag;
	 GMutex pipeline_mutex;  // used for rtsp distribute
	//Sink *tcpsink;  // Only have 1 tcpsink
	//char tcpclientsinkcallid[50];
} GstCustom;

#endif
