#include <string.h>
#include <gst/gst.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <gio/gio.h>

#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <sys/wait.h>
#include <sys/types.h>

#include "cmd_rcv.h"
#include "hashtable.h"
#include "struct.h"

//#define LOGSAVE

#define VERSION                                 "1.0.0"

#define UDP    1
#define RTP    2
//#define TCP    3
#define RTSP   4
//#define JFTCP  5
#define TCPCLIENT 3
#define TCPSERVER 5

//#define DEBUG        1

static int rcv_port_min;
static int rcv_port_max;
static int snd_port_min;
static int snd_port_max;
#define  RCV_PORT_MIN   rcv_port_min
#define  RCV_PORT_MAX   rcv_port_max
#define  SND_PORT_MIN   snd_port_min
#define  SND_PORT_MAX   snd_port_max

#define  PORT_STEP      2
#define KEEP_ALIVE_TIME_OUT     33   //sink got keep alive time 30s
// UDP port
static int Cur_Rcv_Udp_Port=60000;
static int Cur_Snd_Udp_Port=62002;
// TCP port
static int Cur_Rcv_Tcp_Port=60000;
static int Cur_Snd_Tcp_Port=62002;

static char g_remote_ip[20];

static char g_nat_ip[20];
static char g_rcv_ip[20];
//static char g_snd_ip[20];

#define LOCAL_IP   g_rcv_ip

unsigned char tx_buf[1500];
unsigned char rx_buf[1500];

#define SND_PORT_ACK   50000   //
#define LISTEN_PORT    50001   // receive command port num
#define SND_PORT       50002   //

#define CODEC_AAC     1
#define CODEC_PCMULAW  2
int g_audio_codec= CODEC_PCMULAW ;
int cnt;

//GHashTable *Hashtbl_Udp_Source_rcv_port; // callid and port
//GList * Udp_Source_rcv_port;

//GHashTable *Hashtbl_udp_sink_snd_port; // callid and port
//GList * udp_sink_snd_port;

//GHashTable *Hashtbl_Tcp_Source_rcv_port; // callid and port
//GList * Tcp_Source_rcv_port;
//GHashTable *Hashtbl_Tcp_sink_snd_port; // callid and port
//GList * Tcp_sink_snd_port;
//static GMutex sink_snd_port_mutex;
//static GMutex source_rcv_port_mutex;

//GList *sink_bind_port_list = NULL;
//GList *source_rcv_port_list = NULL;

GMainLoop* loop;

GstCustom gstdata;

GHashTable *gsthashtbale;
static GMutex gst_mutex;
static GMutex snd_data_mutex;
static GMutex tcpclienthash_mutex;

static GSocket *backup_sock;
static GstElement *backup_element;
static GSocket *g_socket;
int fd;
extern int exit_flag;
void *new_pipeline_thread( gpointer *arg);
//void add_source(GMainContext *context);
void foreach_gst_hashtab(gpointer key, gpointer value, gpointer user_data);
void port_for_each(gpointer key, gpointer value, gpointer user_data);

static gboolean
message_cb (GstBus * bus, GstMessage * message, gpointer user_data)
{
	GstCustom *gst_ptr;
	gst_ptr = (GstCustom*)user_data;


	const GstStructure *st = gst_message_get_structure (message);

#if 0
   // GError *err = NULL;
      gchar *name, *debug = NULL;

      name = gst_object_get_path_string (message->src);
     // gst_message_parse_warning (message, &err, &debug);

      printf ("message: from element %s: type = %d\n", name, GST_MESSAGE_TYPE (message));
   //   if (debug != NULL)
    //    printf ("Additional debug info:\n%s\n", debug);

   //   g_error_free (err);
    //  g_free (debug);
      g_free (name);
#endif
	switch (GST_MESSAGE_TYPE (message)) {

//	case GST_MESSAGE_ASYNC_DONE:
//			printf("GST_MESSAGE_ASYNC_DONE \n");
//		break;
	case GST_MESSAGE_STATE_CHANGED:
	{


		// GError *err = NULL;
		gchar *name, *debug = NULL;

		name = gst_object_get_path_string (message->src);
		// gst_message_parse_warning (message, &err, &debug);

 	 //  printf ("message: from element %s\n", name);
		//   if (debug != NULL)
		//    printf ("Additional debug info:\n%s\n", debug);

		//   g_error_free (err);
		//  g_free (debug);
		g_free (name);

		GstState old_state, new_state, pending_state;
		gst_message_parse_state_changed (message, &old_state, &new_state, &pending_state);

		// if (GST_MESSAGE_SRC (message) == GST_OBJECT (gst_ptr->sink->sink))
	 //	g_print ("State set to %s\n", gst_element_state_get_name (new_state));  // Need comment
		 break;

			// if (GST_MESSAGE_SRC (message) == GST_OBJECT (gst_ptr->sink->sink))
		   // if (GST_MESSAGE_SRC (message) == GST_OBJECT (gst_ptr->source.tee))
#if 0
			{
			   //  data->current_state = new_state;
				g_print ("State set to %s\n", gst_element_state_get_name (new_state));
			   if (old_state == READY && new_state == NULL)
			   {
				  /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
				  printf("received data from sipuri %s \n", gst_ptr->sipuri);
				  g_mutex_lock (&snd_data_mutex);
				  bzero(tx_buf,sizeof(tx_buf));
				  sprintf(tx_buf, "sipuri=%s;getdata=true;",gst_ptr->sipuri);
				  send_packet(tx_buf,strlen(tx_buf),g_remote_ip,SND_PORT);
				  g_mutex_unlock (&snd_data_mutex);
			   }
			}
#endif
	}
	break;
	case GST_MESSAGE_ERROR:{

	  gchar *ele_name = gst_object_get_name(message->src);
	  GError *err = NULL;
	  gchar *name, *debug = NULL;
	  char *pt = NULL, *pt1 =NULL, *pt2 = NULL;
	  name = gst_object_get_path_string (message->src);
	  gst_message_parse_error (message, &err, &debug);
	
	  printf ("ERROR: from element %s ---: %s\n", ele_name, err->message);
	  if (debug != NULL)
		printf ("Additional debug info:\n%s\n", debug);

	  pt = strstr(ele_name,"tcpclientsink:"); // get callid for pt
      pt1 = strstr(ele_name,"multiudpsink:"); // get callid for pt
      pt2 = strstr(ele_name,"tcpserversink:"); // get callid for pt

	  if(gst_message_has_name(message,"tcpserversink"))
	  {
		  printf("tcpserversink error message \n");
	  }
	  g_error_free (err);
	  g_free (debug);
	  g_free (name);


//	Sink *sink =NULL;
 	  if(pt !=NULL)
	  {

			
		  	printf("GstTCPClientSink error message \n");
		  
//			g_mutex_lock (&snd_data_mutex);
//			bzero(tx_buf,sizeof(tx_buf));
//			sprintf(tx_buf, "self;callid=%s;sipuri=%s;bye=ok;sinktype=3;",pt+14, gst_ptr->sipuri);
//			send_packet(tx_buf,strlen(tx_buf),"0.0.0.0",LISTEN_PORT);
//			g_mutex_unlock (&snd_data_mutex);
//
//			g_mutex_lock (&snd_data_mutex);
//			bzero(tx_buf,sizeof(tx_buf));
//			sprintf(tx_buf,"code=1003;sipuri=%s;callid=%s;",gst_ptr->sipuri,gst_ptr->callid);
//			send_packet(tx_buf,strlen(tx_buf),g_remote_ip,SND_PORT);
//			g_mutex_unlock (&snd_data_mutex);
//
//			g_free (ele_name);
//			break;

	  }

	if(pt1 != NULL) // multiudpsink
	{

		 printf("%s error message \n", pt1);

//		g_mutex_lock (&snd_data_mutex);
//		bzero(tx_buf,sizeof(tx_buf));
//		sprintf(tx_buf, "self;callid=%s;sipuri=%s;bye=ok;sinktype=1;",pt1+13, gst_ptr->sipuri);
//		send_packet(tx_buf,strlen(tx_buf),"0.0.0.0",LISTEN_PORT);
//		g_mutex_unlock (&snd_data_mutex);
//
//		g_mutex_lock (&snd_data_mutex);
//		bzero(tx_buf,sizeof(tx_buf));
//	//	sprintf(tx_buf,"sipuri=%s;callid=%s;senddata=false;",gst_ptr->sipuri,pt1+13);   //  it not happen in normal state
//		sprintf(tx_buf,"code=1001;sipuri=%s;callid=%s;",gst_ptr->sipuri,gst_ptr->callid);
//		send_packet(tx_buf,strlen(tx_buf),g_remote_ip,SND_PORT);
//		g_mutex_unlock (&snd_data_mutex);
//
//		g_free (ele_name);
//		break;
	}

	if(pt2 != NULL) // tcpserversink
	{
		 printf("tcpserversink error message \n");

//		g_mutex_lock (&snd_data_mutex);
//		bzero(tx_buf,sizeof(tx_buf));
//		sprintf(tx_buf, "self;callid=%s;sipuri=%s;bye=ok;sinktype=5;",pt2+14, gst_ptr->sipuri);
//		send_packet(tx_buf,strlen(tx_buf),"0.0.0.0",LISTEN_PORT);
//		g_mutex_unlock (&snd_data_mutex);
//
//		g_mutex_lock (&snd_data_mutex);
//		bzero(tx_buf,sizeof(tx_buf));
//		//sprintf(tx_buf,"sipuri=%s;callid=%s;senddata=false;",gst_ptr->sipuri,pt2+14);   // it not happen in normal state
//		sprintf(tx_buf,"code=1001;sipuri=%s;callid=%s;",gst_ptr->sipuri,gst_ptr->callid);
//		send_packet(tx_buf,strlen(tx_buf),g_remote_ip,SND_PORT);
//		g_mutex_unlock (&snd_data_mutex);
//
//		g_free (ele_name);
//		break;
	}

		g_free (ele_name);
		g_mutex_lock (&snd_data_mutex);
		bzero(tx_buf,sizeof(tx_buf));
		sprintf(tx_buf, "self;callid=%s;sipuri=%s;bye=ok;",gst_ptr->callid, gst_ptr->sipuri);
		send_packet(tx_buf,strlen(tx_buf),"0.0.0.0",LISTEN_PORT);
		g_mutex_unlock (&snd_data_mutex);

	break;

	}

    case GST_MESSAGE_WARNING:{
      GError *err = NULL;
      gchar *name, *debug = NULL;

      name = gst_object_get_path_string (message->src);
      gst_message_parse_warning (message, &err, &debug);

      printf ("WARNING: from element %s: %s\n", name, err->message);
      if (debug != NULL)
        printf ("Additional debug info:\n%s\n", debug);

      g_error_free (err);
      g_free (debug);
      g_free (name);
      break;
    }
    case GST_MESSAGE_ELEMENT:
     // need remove, this is only used for comment stop pipeline
    //	break;
    	 // g_print ("Timeout \n");
    	g_print("GST_MESSAGE_ELEMENT = %s \n",gst_structure_get_name(st));
      /* We don't care for messages other than timeouts */
      if (gst_structure_has_name (st, "GstUDPSrcReceivedata") || gst_structure_has_name (st, "tcpserversrcgetdata"))
      {
		  printf("received data from sipuri %s \n", gst_ptr->sipuri);

      }

      else if (gst_structure_has_name (st, "GstUDPSrcTimeout") || gst_structure_has_name (st, "tcpserversrctimeout"))
	 {
		g_print ("got data Timeout received from sipuri %s\n", gst_ptr->sipuri);

		g_mutex_lock (&snd_data_mutex);
		bzero(tx_buf,sizeof(tx_buf));
		sprintf(tx_buf, "self;callid=%s;sipuri=%s;bye=ok;",gst_ptr->callid, gst_ptr->sipuri);
		send_packet(tx_buf,strlen(tx_buf),"0.0.0.0",LISTEN_PORT);
		g_mutex_unlock (&snd_data_mutex);

	 }
     break;

    case GST_MESSAGE_EOS:

		g_print ("Got EOS\n");

		g_mutex_lock (&snd_data_mutex);
		bzero(tx_buf,sizeof(tx_buf));
		sprintf(tx_buf, "self;callid=%s;sipuri=%s;bye=ok;",gst_ptr->callid, gst_ptr->sipuri);
		send_packet(tx_buf,strlen(tx_buf),"0.0.0.0",LISTEN_PORT);
		g_mutex_unlock (&snd_data_mutex);


      break;
    default:
      break;
  }

  return TRUE;
}

void cb_udp_client_add  (GstElement* object, gchararray arg0, gint arg1, gpointer user_data)
{
	printf("add arg0= %s\n");
	printf("add arg1 = %d \n", arg1);
	//GstUDPClient
}

void cb_udp_client_remove  (GstElement* object, gchararray arg0, gint arg1, gpointer user_data)
{
	printf("remove arg0= %s\n");
	printf("remove arg1 = %d \n", arg1);
}

//int tcp_client_remove(Sink *sink)
//{
//    ///GSocket *sock;
//    Tcpclientsocketinfo *tcpclientsock = g_hash_table_lookup (sink->tcpclienthashtb,sink->callid);
//    if(tcpclientsock != NULL && tcpclientsock->sock != NULL)
//	{
//    	g_signal_emit_by_name(sink->sink,"remove", tcpclientsock->sock);
//	}
//}

int cb_tcp_not_our_client_remove(Sink *sink)
{
    ///GSocket *sock;
   // Tcpclientsocketinfo *tcpclientsock = g_hash_table_lookup (sink->tcpclienthashtb,sink->callid);
   // if(tcpclientsock != NULL && tcpclientsock->sock != NULL)
	{
    	g_signal_emit_by_name(backup_element,"remove",backup_sock);
	}
	printf("remove not our client \n");
	return FALSE;
}



void cb_tcp_client_add  (GstElement* object, GSocket *arg0, gpointer user_data)
{

	 printf(" tcp client add arg0= \n");
	 GstCustom *gst = (GstCustom *)user_data;
	//printf( "sink address = %p \n", sink);
	//printf("size = %d \n",g_hash_table_size(sink->tcpclienthashtb));
	//printf( "tcpclienthashtb address = %p \n",  sink->tcpclienthashtb);

	int port ;
		 GInetSocketAddress *addr =
		G_INET_SOCKET_ADDRESS (g_socket_get_remote_address (arg0,
			NULL));
	gchar *ip =
		g_inet_address_to_string (g_inet_socket_address_get_address (addr));
	port = g_inet_socket_address_get_port (addr);
	printf ( "added new--- client ip %s:%u with socket %p \n",
		 ip, port, user_data);

	g_object_unref (addr);


	printf("sink->dst_ip = %s port= %d \n",gst->sink.dst_ip,gst->sink.dst_port);
	if(g_strcmp0(ip, gst->sink.dst_ip) == 0 && port == gst->sink.dst_port)
	{
		printf("tcp client is we assigned client \n");

		//tcpclientsock->sock = (GSocket*) user_data;
		//return 1;
	}
	else
	{
		 //g_signal_emit_by_name(object,"remove", arg0);
		printf("tcp client is not we assigned client \n");

		g_mutex_lock (&snd_data_mutex);
		bzero(tx_buf,sizeof(tx_buf));
		sprintf(tx_buf, "self;callid=%s;sipuri=%s;bye=ok;",gst->callid, gst->sipuri);
		send_packet(tx_buf,strlen(tx_buf),"0.0.0.0",LISTEN_PORT);
		g_mutex_unlock (&snd_data_mutex);

	}

	g_free (ip);

	return;

}

void cb_tcp_client_remove(GstElement* object,GSocket* arg0, gpointer arg1, gpointer user_data)
{
	//printf(" tcp111 client remove arg0= \n");
	GstCustom *gst = (GstCustom *)user_data;
	gpointer item_ptr = NULL;


	int port ;
		 GInetSocketAddress *addr =
			        G_INET_SOCKET_ADDRESS (g_socket_get_remote_address (arg0,
			            NULL));
			    gchar *ip =
			        g_inet_address_to_string (g_inet_socket_address_get_address (addr));
			    port = g_inet_socket_address_get_port (addr);
		    printf ( "tcp server remove client=== ip %s:%u with socket %p \n",
			         ip, port, user_data);

			    g_object_unref (addr);



//			    if(g_strcmp0(ip, tcpclientsock->ip) == 0 && port == tcpclientsock->port)
//				{
//			    	printf("tcp client is we assigned client \n");
//			    	//g_signal_emit_by_name(object,"remove", arg0);
//			    	tcpclientsock->sock = (GSocket*) user_data;
//			    	return 1;
//				}

			    g_free (ip);


				g_mutex_lock (&snd_data_mutex);
				bzero(tx_buf,sizeof(tx_buf));
				sprintf(tx_buf, "self;callid=%s;sipuri=%s;bye=ok;",gst->callid, gst->sipuri);
				send_packet(tx_buf,strlen(tx_buf),"0.0.0.0",LISTEN_PORT);

//				bzero(tx_buf,sizeof(tx_buf));
//				sprintf(tx_buf, "code=1001;sipuri=%s;callid=%s;",gst->sipuri,gst->callid);
//				send_packet(tx_buf,strlen(tx_buf),g_remote_ip,SND_PORT);
				g_mutex_unlock (&snd_data_mutex);

     return;

}


/* This function will be called by the pad-added signal */
static void pad_added_handler_for_rtsp (GstElement *src, GstPad *new_pad, gpointer decode)
{
  GstPad *sink_pad ;
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstCustom *ptr=(GstCustom*)decode;
  const gchar *new_pad_type = NULL;

  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));


  /* If our converter is already linked, we have nothing to do here */
//  if (gst_pad_is_linked (sink_pad)) {
//    g_print ("  We are already linked. Ignoring.\n");
//    goto exit;
//  }

    if (!strncmp(GST_PAD_NAME (new_pad), "recv_rtp_src_0", 14))
    { // Video pad
         g_print (" video pad\n");
         sink_pad = gst_element_get_static_pad (ptr->source.h264depay,"sink");

        g_print ("Received data from '%s':\n", ptr->sipuri);
        g_mutex_lock (&snd_data_mutex);
       	bzero(tx_buf,sizeof(tx_buf));
    //   	sprintf(tx_buf, "sipuri=%s;getdata=true;",ptr->sipuri);
       //	send_packet(tx_buf,strlen(tx_buf),g_remote_ip,SND_PORT);
       	g_mutex_unlock (&snd_data_mutex);

    }
    else
    {
    	g_print (" audio pad\n");
    	sink_pad = gst_element_get_static_pad (ptr->source.audiodepay,"sink");
        //goto exit;
    }

  /* Attempt the link */
  ret = gst_pad_link (new_pad, sink_pad);
  if (GST_PAD_LINK_FAILED (ret)) {
    g_print ("  Type is '%s' but link failed.\n", new_pad_type);
  } else {
    g_print ("  Link succeeded (type '%s').\n", new_pad_type);
  }

exit:
  /* Unreference the new pad's caps, if we got them */
  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);

  /* Unreference the sink pad */
  gst_object_unref (sink_pad);
}

 int Create_source_sink_pipeline( GstCustom *gstcustom)
{
	 gstcustom->pipeline = gst_pipeline_new (NULL);


	if(gstcustom->source.type == UDP || gstcustom->source.type == RTP)
	{
		gstcustom->source.src = gst_element_factory_make ("udpsrc", "udpsrc");

		//gstcustom->source.queue = gst_element_factory_make ("queue", "queue");

		gstcustom->source.tee = gst_element_factory_make ("tee", "tee");

		if (!gstcustom->pipeline || !gstcustom->source.src || !gstcustom->source.tee) {
			g_error ("Failed to create elements");
			return -1;
		}


		//g_object_set (src, "caps", gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING,"video","payload",G_TYPE_INT,33, NULL), NULL);
		g_object_set (gstcustom->source.src , "caps", gst_caps_new_simple("video/mpegts", "packetsize",G_TYPE_INT,188, NULL), NULL);


		gst_bin_add_many (GST_BIN (gstcustom->pipeline), gstcustom->source.src, /*gstcustom->source.queue ,*/ gstcustom->source.tee,NULL);
		if (!gst_element_link_many (gstcustom->source.src/*,gstcustom->source.queue */, gstcustom->source.tee, NULL))
		{
		    g_error ("Failed to link elements");
		    return -2;
		}

		//g_signal_connect (dbin, "pad-added", G_CALLBACK (pad_added_cb), NULL);

		//loop = g_main_loop_new (NULL, FALSE);


		g_object_set (gstcustom->source.src, "timeout",40000000000);   // 30s
		g_object_set (gstcustom->source.src, "buffer-size",212992);   // 208KB  Set to Max value

		printf("gstcustom->sipuri = %s \n",gstcustom->sipuri);

		// need keep alive
		//if(gstcustom->source.sndkeepalive_socket == NULL)
		{
			printf("using udpsrc default socket \n");
			//  g_object_set (gstcustom->source.src, "uri",gstcustom->source.src_uri, NULL);
			g_object_set (gstcustom->source.src, "port", gstcustom->source.dst_port,NULL);
			g_object_set (gstcustom->source.src, "address", "0.0.0.0",NULL);
			// Disabling this might result in minor performance improvements
			g_object_set (gstcustom->source.src, "retrieve-sender-address", FALSE,NULL);

			g_object_set (gstcustom->source.src, "keep-alive-time",5,NULL); // for test
			if(gstcustom->source.keep_alive_str_lenth == 0)
			{
				g_object_set (gstcustom->source.src, "keep-alive-len", 10, NULL);
				g_object_set (gstcustom->source.src, "keep-alive-string","hello_word",NULL);
			}
			else
			{
				g_object_set (gstcustom->source.src, "keep-alive-len", gstcustom->source.keep_alive_str_lenth,NULL);
				g_object_set (gstcustom->source.src, "keep-alive-string", gstcustom->source.keep_alive_str,NULL);
			}
			g_object_set (gstcustom->source.src, "nat_flag",0,NULL);  // need add nat transfer ?


		}


	}
	else if(gstcustom->source.type == TCPCLIENT) // tcpclientsrc
	{
			gstcustom->source.src = gst_element_factory_make ("tcpclientsrc", "tcpclientsrc");
			gstcustom->source.tee = gst_element_factory_make ("tee", "tee");
			//gstcustom->source.rndbuffersize = gst_element_factory_make ("rndbuffersize", "rndbuffersize");
			gstcustom->source.capsfilter = gst_element_factory_make ("capsfilter", "capsfilter");

			if (!gstcustom->pipeline || !gstcustom->source.src || /*!gstcustom->source.rndbuffersize ||*/ !gstcustom->source.capsfilter || !gstcustom->source.tee) {
				g_error ("Failed to create elements");
				return -1;
			}


			//g_object_set (src, "caps", gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING,"video","payload",G_TYPE_INT,33, NULL), NULL);
			g_object_set (gstcustom->source.capsfilter , "caps", gst_caps_new_simple("video/mpegts", "packetsize",G_TYPE_INT,188, NULL), NULL);


			gst_bin_add_many (GST_BIN (gstcustom->pipeline), gstcustom->source.src/*,gstcustom->source.rndbuffersize*/,gstcustom->source.capsfilter, gstcustom->source.tee,NULL);
		if (!gst_element_link_many (gstcustom->source.src,/*gstcustom->source.rndbuffersize,*/gstcustom->source.capsfilter, gstcustom->source.tee, NULL))
		{
			g_error ("Failed to link elements");
			return -2;
		}


			g_object_set (gstcustom->source.src, "port", gstcustom->source.src_port,NULL);
			printf("tcp srcport=%d srcip = %s \n", gstcustom->source.src_port, gstcustom->source.src_ip);
			g_object_set (gstcustom->source.src, "host", gstcustom->source.src_ip,NULL);
			g_object_set (gstcustom->source.src, "timeout", 30 ,NULL); // read data time-out


			g_object_set (gstcustom->source.src, "bindip", LOCAL_IP ,NULL); // read data time-out
			g_object_set (gstcustom->source.src, "bindport", gstcustom->source.dst_port ,NULL);

	}

	else if(gstcustom->source.type == TCPSERVER)
	{
			gstcustom->source.src = gst_element_factory_make ("tcpserversrc", "tcpserversrc");
			gstcustom->source.tee = gst_element_factory_make ("tee", "tee");
		//	gstcustom->source.rndbuffersize = gst_element_factory_make ("rndbuffersize", "rndbuffersize");
			gstcustom->source.capsfilter = gst_element_factory_make ("capsfilter", "capsfilter");

			if (!gstcustom->pipeline || !gstcustom->source.src || /*!gstcustom->source.rndbuffersize ||*/ !gstcustom->source.capsfilter || !gstcustom->source.tee) {
				g_error ("Failed to create elements");
				return -1;
			}


			//g_object_set (src, "caps", gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING,"video","payload",G_TYPE_INT,33, NULL), NULL);
			g_object_set (gstcustom->source.capsfilter , "caps", gst_caps_new_simple("video/mpegts", "packetsize",G_TYPE_INT,188, NULL), NULL);


			gst_bin_add_many (GST_BIN (gstcustom->pipeline), gstcustom->source.src/*,gstcustom->source.rndbuffersize*/,gstcustom->source.capsfilter, gstcustom->source.tee,NULL);
		if (!gst_element_link_many (gstcustom->source.src,/*gstcustom->source.rndbuffersize,*/gstcustom->source.capsfilter, gstcustom->source.tee, NULL))
		{
			g_error ("Failed to link elements");
			return -2;
		}




			//  g_object_set (gstcustom->source.src, "uri",gstcustom->source.src_uri, NULL);
			g_object_set (gstcustom->source.src, "port", gstcustom->source.dst_port,NULL);
			//printf("tcp srcport=%d srcip = %s \n", gstcustom->source.src_port, gstcustom->source.src_ip);
			g_object_set (gstcustom->source.src, "host", "0.0.0.0",NULL);
			//g_object_set (gstcustom->source.src, "timeout", 10 ,NULL); // read data time-out
			g_object_set (gstcustom->source.src, "timeout",40);


	}
	else if(gstcustom->source.type == RTSP)
	{
			gstcustom->source.src = gst_element_factory_make ("rtspsrc", "rtspsrc");
			gstcustom->source.h264depay = gst_element_factory_make ("rtph264depay", "rtph264depay");
			gstcustom->source.h264parse = gst_element_factory_make ("h264parse", "h264parse");

			gstcustom->source.mpegtsmux = gst_element_factory_make ("mpegtsmux", "mpegtsmux");
			gstcustom->source.tee = gst_element_factory_make ("tee", "tee");

			if(g_audio_codec==CODEC_AAC)
			{
				gstcustom->source.aacparse = gst_element_factory_make ("aacparse", "aacparse");
				gstcustom->source.audiodepay = gst_element_factory_make ("rtpmp4gdepay", "rtpmp4gdepay");


			//	gstcustom->source.audiodepay = gst_element_factory_make ("rtppcmudepay", "rtppcmudepay");

				if (!gstcustom->pipeline || !gstcustom->source.src || !gstcustom->source.h264depay || !gstcustom->source.h264parse
								|| !gstcustom->source.audiodepay || !gstcustom->source.mpegtsmux || !gstcustom->source.tee) {
								g_error ("Failed to create elements");
								return -1;
				}


				gst_bin_add_many (GST_BIN (gstcustom->pipeline), gstcustom->source.src ,gstcustom->source.h264depay,gstcustom->source.h264parse,
						gstcustom->source.aacparse,gstcustom->source.audiodepay,gstcustom->source.mpegtsmux, gstcustom->source.tee,NULL);

				if (!gst_element_link_many ( gstcustom->source.h264depay,gstcustom->source.h264parse,gstcustom->source.mpegtsmux, gstcustom->source.tee, NULL))
				{
					g_error ("Failed to link elements");
					return -2;
				}
				if (!gst_element_link_many ( gstcustom->source.audiodepay,gstcustom->source.aacparse,gstcustom->source.mpegtsmux, NULL))
				{
					g_error ("Failed to link elements");
					return -2;
				}
			}
			else if(g_audio_codec==CODEC_PCMULAW)
			{
				gstcustom->source.audiodepay = gst_element_factory_make ("rtppcmudepay", "rtppcmudepay");

				if (!gstcustom->pipeline || !gstcustom->source.src || !gstcustom->source.h264depay || !gstcustom->source.h264parse
								|| !gstcustom->source.audiodepay || !gstcustom->source.mpegtsmux || !gstcustom->source.tee) {
								g_error ("Failed to create elements");
								return -1;
				}


				gst_bin_add_many (GST_BIN (gstcustom->pipeline), gstcustom->source.src ,gstcustom->source.h264depay,gstcustom->source.h264parse,gstcustom->source.audiodepay,gstcustom->source.mpegtsmux, gstcustom->source.tee,NULL);
				if (!gst_element_link_many ( gstcustom->source.h264depay,gstcustom->source.h264parse,gstcustom->source.mpegtsmux, gstcustom->source.tee, NULL))
				{
					g_error ("Failed to link elements");
					return -2;
				}
				if (!gst_element_link_many ( gstcustom->source.audiodepay,gstcustom->source.mpegtsmux, NULL))
				{
					g_error ("Failed to link elements");
					return -2;
				}

			}



		 g_signal_connect (gstcustom->source.src, "pad-added", (GCallback)pad_added_handler_for_rtsp, gstcustom);

		// char tmp[100];
		// memset(tmp,0,100);
		// sprintf(tmp,"rtsp://%s:554/Streaming/Channels/101",gstcustom->source.src_ip);
		// printf("==========%s======%s \n", gstcustom->source.src_ip, tmp);
		 printf("rtsp = %s \n", gstcustom->source.rtspaddr);
		 g_object_set (gstcustom->source.src, "location",gstcustom->source.rtspaddr, NULL);
		 g_object_set (gstcustom->source.h264parse, "config-interval",1, NULL);
		 g_object_set (gstcustom->source.mpegtsmux, "alignment",7, NULL);

	}

	gstcustom->bus = gst_pipeline_get_bus (GST_PIPELINE (gstcustom->pipeline));
	gst_bus_add_signal_watch (gstcustom->bus);
	g_signal_connect (G_OBJECT (gstcustom->bus), "message", G_CALLBACK (message_cb), gstcustom);
	//gst_object_unref (GST_OBJECT (gstcustom->bus));




	  GstPadTemplate *templ;
	 templ =
	        gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (gstcustom->source.tee),
	        "src_%u");
	    gstcustom->sink.teepad = gst_element_request_pad (gstcustom->source.tee, templ, NULL, NULL);
	    gstcustom->sink.queue = gst_element_factory_make ("queue", gstcustom->callid);


	    if(gstcustom->sink.type == UDP || gstcustom->sink.type == RTP)
	    {
			gchar tmp[100];
			sprintf(tmp,"multiudpsink:%s",gstcustom->callid);
			gstcustom->sink.sink = gst_element_factory_make ("multiudpsink", tmp);

			if(gstcustom->sink.sndkeepalive_socket != NULL)
			{
				g_object_set (gstcustom->sink.sink, "socket",  gstcustom->sink.sndkeepalive_socket , NULL);
				g_object_set (gstcustom->sink.sink, "close-socket",  FALSE , NULL);    //stop pipeline not close socket
			}
			else
			{
				if(gstcustom->sink.src_port != 0)
				g_object_set (gstcustom->sink.sink, "bind-address",LOCAL_IP,"bind-port",gstcustom->sink.src_port, NULL);
			}

			g_object_set (gstcustom->sink.sink, "send-duplicates", FALSE, NULL);

			g_signal_connect (gstcustom->sink.sink, "client-added",G_CALLBACK (cb_udp_client_add), gstcustom);
			g_signal_connect (gstcustom->sink.sink, "client-removed",G_CALLBACK (cb_udp_client_remove), gstcustom);

			g_signal_emit_by_name (gstcustom->sink.sink, "add", gstcustom->sink.dst_ip, gstcustom->sink.dst_port, NULL);

			//g_object_set (gstcustom->sink->sink, "buffer-size", 10240, NULL);

			printf("gstcustom->sink.dst_ip %s gstcustom->sink.dst_port = %d \n",  gstcustom->sink.dst_ip, gstcustom->sink.dst_port);

	    }

	    else if(gstcustom->sink.type == TCPSERVER)
	    {
			gchar tmp[100];
			sprintf(tmp,"tcpserversink:%s",gstcustom->callid);

	    	gstcustom->sink.sink = gst_element_factory_make ("tcpserversink", tmp);
	    	g_object_set (gstcustom->sink.sink, "port", gstcustom->sink.src_port, NULL);  // Listen port
	    	g_object_set (gstcustom->sink.sink, "host", "0.0.0.0", NULL);
	     	g_object_set (gstcustom->sink.sink, "timeout", 20000000000, NULL);  // client is not inactivity timeout: 20s
	     	g_object_set (gstcustom->sink.sink, "client-connect-timeout", 20, NULL); // client connect timeout : 20s

	    	g_signal_connect (gstcustom->sink.sink, "client-added",G_CALLBACK (cb_tcp_client_add), gstcustom);
	    	g_signal_connect (gstcustom->sink.sink, "client-removed",G_CALLBACK (cb_tcp_client_remove),  gstcustom);


//	    	 g_object_set (gstcustom->sink.sink,
//	    	      "unit-format", GST_FORMAT_TIME,
//	    	      "units-max", (gint64) 7 * GST_SECOND,
//	    	      "units-soft-max", (gint64) 3 * GST_SECOND,
//	    	      "recover-policy", 3 /* keyframe */ ,
//	    	      "timeout", (guint64) 10 * GST_SECOND,
//	    	      "sync-method", 1 /* next-keyframe */ ,
//	    	      NULL);
	    }
		else if(gstcustom->sink.type == TCPCLIENT)
		{
			gchar tmp[100];
			sprintf(tmp,"tcpclientsink:%s",gstcustom->callid);

			gstcustom->sink.sink = gst_element_factory_make ("tcpclientsink", tmp);
			g_object_set (gstcustom->sink.sink, "host", gstcustom->sink.dst_ip, NULL);
			g_object_set (gstcustom->sink.sink, "port", gstcustom->sink.dst_port , NULL);
			g_object_set (gstcustom->sink.sink, "jftcpstring", gstcustom->sink.jftcpstring , NULL);
			g_object_set (gstcustom->sink.sink, "jftcpflag", 0 , NULL);
			g_object_set (gstcustom->sink.sink, "bindport", gstcustom->sink.src_port , NULL);
			g_object_set (gstcustom->sink.sink, "bindip", LOCAL_IP ,NULL);

		//	g_object_set (gstcustom->sink->sink, "block", FALSE ,NULL);
		}

	    g_object_set (gstcustom->sink.queue, "leaky", 1, NULL);  // buffer leak data

	     g_object_set (gstcustom->sink.sink, "sync", FALSE ,NULL);
	   //  g_object_set (gstcustom->sink->sink, "max-lateness", 20000000 ,NULL);

   //     gst_bin_add_many (GST_BIN (gstcustom->pipeline), gst_object_ref (gstcustom->sink.queue),gst_object_ref (gstcustom->sink.sink), NULL);
         gst_bin_add_many (GST_BIN (gstcustom->pipeline), gstcustom->sink.queue,gstcustom->sink.sink, NULL);
	   // gst_bin_add_many (GST_BIN (gstcustom->pipeline),gstcustom->sink->queue, gstcustom->sink->sink, NULL);
	    if(gst_element_link_many (gstcustom->sink.queue, gstcustom->sink.sink, NULL) != TRUE)
	    {
	    	printf("linked error \n");
	    }

	    GstPad *sinkpad;

	    sinkpad = gst_element_get_static_pad (gstcustom->sink.queue, "sink");
	  //  gst_pad_link (gstcustom->sink->teepad, sinkpad);

	    if (gst_pad_link (gstcustom->sink.teepad, sinkpad) != GST_PAD_LINK_OK)
		{
			printf ("Tee could not be linked.\n");
			// need add error process
		}

	    gst_object_unref (sinkpad);

	   printf("start player \n");
	 //  GstStateChangeReturn tmp;
	    gst_element_set_state (gstcustom->pipeline, GST_STATE_PLAYING);
	// printf("GstStateChangeReturn = %d \n");
	  return TRUE;

}

 int Release_udptoudp_Pipeline( GstCustom *gstcustom)
 {
	 gst_element_set_state (gstcustom->pipeline, GST_STATE_NULL);
 }


static gboolean
cb_have_data (GstPad    *pad,
      GstBuffer *buffer,
      gpointer   u_data)
{
   
  printf("have data \n");
  return TRUE;
}

void foreach_gst_hashtab(gpointer key, gpointer value, gpointer user_data)
{
	 GstCustom *gstdata;
	 gstdata = (GstCustom *)value;
	 cnt += sprintf(tx_buf+cnt,"callid=%s\n",key);
	// printf("cnt = %d %s \n", cnt, tx_buf);
}

 static void *cmd_thread(int len)
{
	 int rcv_size;
	// GstCustom *gst_ptr;
	signal(SIGINT, Stop);
	char remote_ip[20];
	//printf("cmd thread created \n");


	 fflush(stdout);

	rcv_size = len;
	gRcvSocket = rcv_socket_init();
	while(1)
	{

		int src_type = 0, sink_type = 0, invite_flag =0, bye_flag = 0, 	sink_keep_alive_flag=0, source_keep_alive_flag =0,
				sink_dst_port= 0, sink_src_port = 0,source_src_port =0, source_dst_port=0, NAT_Flag = 0;

		char sipuri[100], sink_dst_uri[30], sink_dst_ip[20], sink_src_ip[20], source_src_ip[20],source_dst_ip[20];
		char gst_hashtable_key[100],  callid[100], rtspaddr[100];


		printf("waitting cmd .... \n");
		memset(rx_buf, 0 ,1500);

		rcv_size = receive_packet(rx_buf);

		if(remote_ip_parse(rx_buf,rcv_size, remote_ip))
		{
		 	//tcp_client_remove();
			printf("remote ip = %s \n",remote_ip);
		    g_stpcpy(g_remote_ip, remote_ip);
		    //return 0;
			continue;
		}

		g_mutex_lock (&gst_mutex);
		if(get_total_session(rx_buf,rcv_size) == 1 && gsthashtbale !=NULL)
		{
			cnt = 0;
			g_mutex_lock (&snd_data_mutex);
			bzero(tx_buf,sizeof(tx_buf));
			g_hash_table_foreach(gsthashtbale, foreach_gst_hashtab, NULL);

			printf("tx_buf  %s \n", tx_buf);
			send_packet(tx_buf, strlen(tx_buf), g_remote_ip,SND_PORT) ;
			g_mutex_unlock (&snd_data_mutex);
		}
		g_mutex_unlock (&gst_mutex);


		//printf("sipuri = %s \n", sipuri);
		memset(callid, 0 ,100);
		if(callid_parse(rx_buf,rcv_size,callid) == 0)
		{
			printf("not find callid \n");
			//return 0;
			 continue;
		}

		memset(sipuri, 0 ,100);
		if(sipuri_parse(rx_buf,rcv_size,sipuri) == 0)
		{
			printf("not find sipuri \n");
			//return 0;
			 continue;
		}


		//printf("callid = %s \n", callid);

		src_type = src_type_parse(rx_buf, rcv_size);
		sink_type =  sink_type_parse(rx_buf, rcv_size);

		if(request_address(rx_buf,rcv_size)  == 1)
		{
			//snd address to request
			char *ptr;
			char str[255];
            int snd_port =0;
            int rcv_port = 0;
			if(src_type == UDP)
			{
				while(1)
				{
					   if(udp_port_available_check(Cur_Rcv_Udp_Port) == 1)
					   {
						   printf("port = %d is available for udp rcv \n", Cur_Rcv_Udp_Port);

							rcv_port = Cur_Rcv_Udp_Port;

							Cur_Rcv_Udp_Port +=PORT_STEP;

						   if(Cur_Rcv_Udp_Port >= RCV_PORT_MAX)
						   {
							   Cur_Rcv_Udp_Port = RCV_PORT_MIN;
						   }

						   break;
					   }
					   else
					   {
						   printf("port = %d is not available in udp rcv list  \n", Cur_Rcv_Udp_Port);
						   Cur_Rcv_Udp_Port +=PORT_STEP;

						   if(Cur_Rcv_Udp_Port >= RCV_PORT_MAX)
						   {
							   Cur_Rcv_Udp_Port = RCV_PORT_MIN;
						   }
					   }

				}

			}
			else if(src_type == TCPCLIENT  || src_type == TCPSERVER)
			{
				while(1)
				{
					   if(tcp_port_available_check(Cur_Rcv_Tcp_Port) == 1)
					   {
						   printf("port = %d is available for tcp rcv \n", Cur_Rcv_Tcp_Port);

							rcv_port = Cur_Rcv_Tcp_Port;

							Cur_Rcv_Tcp_Port +=PORT_STEP;

						   if(Cur_Rcv_Tcp_Port >= RCV_PORT_MAX)
						   {
							   Cur_Rcv_Tcp_Port = RCV_PORT_MIN;
						   }

						   break;
					   }
					   else
					   {
						   printf("port = %d is not available in tcp rcv list  \n", Cur_Rcv_Tcp_Port);
						   Cur_Rcv_Tcp_Port +=PORT_STEP;

						   if(Cur_Rcv_Tcp_Port >= RCV_PORT_MAX)
						   {
							   Cur_Rcv_Tcp_Port = RCV_PORT_MIN;
						   }
					   }

				}
			}

			if(sink_type == UDP)
			{
				while(1)
				{
					   if(udp_port_available_check(Cur_Snd_Udp_Port) == 1)
					   {
						   printf("port = %d is available for udp snd \n", Cur_Snd_Udp_Port);

						   snd_port = Cur_Snd_Udp_Port;

						 	Cur_Snd_Udp_Port +=PORT_STEP;

						   if(Cur_Snd_Udp_Port >= SND_PORT_MAX)
						   {
							   Cur_Snd_Udp_Port = SND_PORT_MIN;
						   }

						   break;
					   }
					   else
					   {
						   printf("port = %d is not available in udp snd list  \n", Cur_Snd_Udp_Port);
						   Cur_Snd_Udp_Port +=PORT_STEP;

						   if(Cur_Snd_Udp_Port >= SND_PORT_MAX)
						   {
							   Cur_Snd_Udp_Port = SND_PORT_MIN;
						   }
					   }

				}

			}

			else if(sink_type == TCPCLIENT || sink_type == TCPSERVER)
			{
				while(1)
				{
					   if(tcp_port_available_check(Cur_Snd_Tcp_Port) == 1)
					   {
						   printf("port = %d is available for tcp snd \n", Cur_Snd_Tcp_Port);

						   snd_port = Cur_Snd_Tcp_Port;

							Cur_Snd_Tcp_Port +=PORT_STEP;

						   if(Cur_Snd_Tcp_Port >= SND_PORT_MAX)
						   {
							   Cur_Snd_Tcp_Port = SND_PORT_MIN;
						   }

						   break;
					   }
					   else
					   {
						   printf("port = %d is not available in tcp snd list  \n", Cur_Snd_Tcp_Port);
						   Cur_Snd_Tcp_Port +=PORT_STEP;

						   if(Cur_Snd_Tcp_Port >= SND_PORT_MAX)
						   {
							   Cur_Snd_Tcp_Port = SND_PORT_MIN;
						   }
					   }

				}
			}


		    g_mutex_lock (&snd_data_mutex);
		    bzero(tx_buf,sizeof(tx_buf));

		    sprintf(tx_buf,"sipuri=%s;callid=%s;sourcedstip=%s;sourcedstport=%d;sinksrcip=%s;sinksrcport=%d",callid,callid,g_nat_ip,rcv_port,g_nat_ip,snd_port);
		    send_packet(tx_buf,strlen(tx_buf),g_remote_ip,SND_PORT_ACK);
		    g_mutex_unlock (&snd_data_mutex);
		     printf("tx_buf -----%s \n", tx_buf);
		  //   return 0;
			 continue;
		}

		//string  parse
		invite_flag = invite_parse(rx_buf, rcv_size);
		bye_flag    =  bye_parse(rx_buf, rcv_size);

		sink_src_port =  sink_src_port_parse(rx_buf, rcv_size);
		sink_dst_port =  sink_dst_port_parse(rx_buf, rcv_size);
//		if(sink_dst_port ==0)
//		{
//			continue;
//		}
		source_src_port = source_src_port_parse(rx_buf, rcv_size);
		source_dst_port = source_dst_port_parse(rx_buf, rcv_size);
        if(source_dst_port ==0 && invite_flag == 1 && src_type == UDP)
        {
        	printf("not find source_dst_port \n");
        	//return 0;
        	 continue;
        }

		sink_keep_alive_flag = sink_keep_alive_parse(rx_buf, rcv_size);
		source_keep_alive_flag = source_keep_alive_parse(rx_buf, rcv_size);
		NAT_Flag = Nat_parse(rx_buf, rcv_size);

//		if(src_uri_parse(rx_buf, rcv_size, src_uri) == 0)
//		{
//			printf("not find src_uri \n");
//			continue;
//		}
//

		if((invite_flag ==1) && (sink_dst_ip_parse(rx_buf, rcv_size,sink_dst_ip) == 0))
		{
			printf("not find sink_dst_ip \n");
			//return 0;
			 continue;
		}

//		if((invite_flag ==1) && (sink_src_ip_parse(rx_buf, rcv_size,sink_src_ip) == 0))
//		{
//			printf("not find sink_src_ip \n");
//
//		}
		int tmp = source_src_ip_parse(rx_buf, rcv_size,source_src_ip) ;
		if(((source_keep_alive_flag == 1) || src_type == TCPCLIENT) && (tmp == 0))
		{
		//	printf("not find source src ip \n");
		//	return 0;
			//continue;
		}
//		if(source_dst_ip_parse(rx_buf, rcv_size,source_dst_ip) == 0)
//		{
//			printf("not find source_dst_ip \n");
//
//		}
        bzero(rtspaddr,100);
		rtspaddr_parse(rx_buf, rcv_size,rtspaddr);

       // sleep(1);
		if(invite_flag == TRUE)
		{
			g_mutex_lock (&snd_data_mutex);
			send_packet("invite-true",11,g_remote_ip,SND_PORT_ACK);
			g_mutex_unlock (&snd_data_mutex);
			//printf("tx_buf -----%s \n", tx_buf);

			g_mutex_lock (&gst_mutex);
			GstCustom *tmp = g_hash_table_lookup (gsthashtbale, callid);
			g_mutex_unlock (&gst_mutex);

			switch(src_type)
			{

			case TCPCLIENT:
			case UDP:  //udp type src
			case TCPSERVER:
			case RTSP:
			{
				//sprintf(gst_hashtable_key,"udp://%s",src_uri);


				if(tmp != 0)  // src session id is esstibition
				{
					printf("src session callid id= is esstibition %s \n",callid);
				}
				else     // src session is not build, create new pipeline
				{
					if(sink_type == UDP || sink_type == TCPSERVER || sink_type == TCPCLIENT)
					{
					   // check udp type src pipeline if is created
						 printf("create pipeline \n");
						 GstCustom *gst_ptr = g_new0(GstCustom, 1);
						// sprintf(gst_ptr->sipuri,"udp://%s",src_uri);
						// gst_ptr->source.src_uri = g_strdup_printf("udp://%s",src_uri);
						 g_stpcpy(gst_ptr->callid, callid);
						 g_stpcpy(gst_ptr->sipuri, sipuri);
						// sprintf(gst_ptr->source.src_uri,"udp://%s",src_uri);
						 g_mutex_lock (&gst_mutex);
						 g_hash_table_insert (gsthashtbale,  g_strdup(gst_ptr->callid), gst_ptr);
						 g_mutex_unlock (&gst_mutex);

						 g_stpcpy(gst_ptr->sink.src_ip, sink_src_ip);
						 g_stpcpy(gst_ptr->sink.dst_ip, sink_dst_ip);
						 gst_ptr->sink.src_port = sink_src_port;
						 gst_ptr->sink.dst_port = sink_dst_port;
						 gst_ptr->sink.type = sink_type;
						 gst_ptr->source.type = src_type;
						 bzero(gst_ptr->source.rtspaddr,100);
						 g_stpcpy(gst_ptr->source.rtspaddr, rtspaddr);

						 gst_ptr->source.keep_alive_str_lenth = keep_alive_string_len_parse(rx_buf, rcv_size);
						 memset(gst_ptr->source.keep_alive_str, 0 ,100);
						 keep_alive_string_parse(rx_buf, rcv_size,  gst_ptr->source.keep_alive_str);

						// printf("+++++++++++++source_src_ip = %s \n", source_src_ip);

						 g_stpcpy(gst_ptr->source.src_ip, source_src_ip);
						 g_stpcpy(gst_ptr->source.dst_ip, source_dst_ip);

						 gst_ptr->source.src_port = source_src_port;
						 gst_ptr->source.dst_port = source_dst_port;

						 gst_ptr->sink.Nat_Traversal = NAT_Flag;
						 gst_ptr->sink.Get_Nat_address_flag = FALSE;
						 gst_ptr->sink.keep_alive_flag = sink_keep_alive_flag;
						 gst_ptr->source.keep_alive_flag = source_keep_alive_flag;
						 create_keep_alive_socket_for_sink(&(gst_ptr->sink));


						if(sink_type == TCPCLIENT)
						{	
							memset(gst_ptr->sink.jftcpstring,0,301);
							jftcpstring_parse(rx_buf, rcv_size, gst_ptr->sink.jftcpstring);
						}
						 gst_ptr->gthread  = g_thread_new("jieshouip:port", new_pipeline_thread , gst_ptr);

					}
					else if(sink_type == RTP)
					{

					}

				}

			}
 				break;

			default:
				break;
			}
		}
		else if(bye_flag == TRUE)
		{

                if(strstr(rx_buf,"self") == NULL)
                	send_packet("bye_true",8,g_remote_ip,SND_PORT_ACK);

				g_mutex_lock (&gst_mutex);
				GstCustom *tmp = g_hash_table_lookup (gsthashtbale, callid);
				g_mutex_unlock (&gst_mutex);
				if(tmp != 0)  // src session is builded
				{
					 char *ptr=NULL;

					//g_mutex_lock (&source_rcv_port_mutex);
					//ptr = g_hash_table_lookup(Hashtbl_Udp_Source_rcv_port ,tmp->callid);
					//g_mutex_unlock (&source_rcv_port_mutex);

					 g_mutex_lock (&snd_data_mutex);
					// sprintf(tx_buf,"code=1002;sipuri=%s;callid=%s;port=%d;",tmp->sipuri,tmp->callid,GPOINTER_TO_INT(ptr));
					 sprintf(tx_buf,"code=1002;sipuri=%s;callid=%s;port=%d;",tmp->sipuri,tmp->callid,tmp->source.dst_port);
					 send_packet(tx_buf,strlen(tx_buf),g_remote_ip,SND_PORT);
					 g_mutex_unlock (&snd_data_mutex);

					gst_element_set_state(GST_ELEMENT (tmp->pipeline),GST_STATE_NULL);
					usleep(5000); // solve bugs for tcp connect fail, can't quit the loop
					printf("set status to NUll \n");

					g_main_loop_quit (tmp->loop);

					printf(" waitting thread exit \n");
					g_thread_join(tmp->gthread);

					gst_bus_remove_signal_watch(tmp->bus);
					gst_object_unref (GST_OBJECT (tmp->bus));

					g_mutex_lock (&gst_mutex);
					if(g_hash_table_remove(gsthashtbale,tmp->callid))
					{
						printf("remove session from gsthashtbale of callid %s  \n", tmp->callid);
					}
					g_mutex_unlock (&gst_mutex);

				}
				else
				{
					printf("not find session of call id  %s \n" ,callid);
				}
			}
//


		}
		printf("exit cmd loop \n");
}

// release send port and keep alive socket
 void free_all_keepalive_for_sink( gpointer user_data)
 {


	 GstCustom *gstdata;
	 gstdata = (GstCustom *)user_data;

	 if(gstdata->sink.type == UDP || gstdata->sink.type == RTP)
	 {
		// g_mutex_lock (&sink_snd_port_mutex);
		// g_list_remove(udp_sink_snd_port,GINT_TO_POINTER(gstdata->sink.src_port));
		// g_hash_table_remove(Hashtbl_udp_sink_snd_port,gstdata->callid);  // remove send port for sink
	//	 g_mutex_unlock (&sink_snd_port_mutex);
		 rmv_keepalive_socket_for_sink(&(gstdata->sink));
	 }
	 else if(gstdata->sink.type == TCPCLIENT || gstdata->sink.type == TCPSERVER)
	 {	
	//	g_mutex_lock (&sink_snd_port_mutex);
		// g_list_remove(Tcp_sink_snd_port,GINT_TO_POINTER(gstdata->sink.src_port));
		//g_hash_table_remove(Hashtbl_Tcp_sink_snd_port,gstdata->callid);
	//	g_mutex_unlock (&sink_snd_port_mutex);
	 }

 }


 void port_for_each(gpointer key, gpointer value, gpointer user_data)
 {
	 printf("%s value = %s, port= %d \n",user_data,  key, GPOINTER_TO_INT(value));
 }


 void foreach_sink_hashtab(gpointer key, gpointer value, gpointer user_data)
 {
	 Sink  *sink;
	 sink = (Sink *)value;

	 printf("call id = %s \n", key);

	 cnt += sprintf(tx_buf+cnt,"callid=%s",key);
 }



 void *new_pipeline_thread( gpointer *arg)
 {
	GstCustom *gstdata;
	gstdata = (GstCustom *)arg;
	// gst_init (NULL, NULL);
	gstdata->loop =  g_main_loop_new (NULL, FALSE);
	printf("create new thread for new pipeline\n");

	Create_source_sink_pipeline(gstdata);

	g_main_loop_run (gstdata->loop);

	 printf("exit loop ===    \n"   );

	//gst_element_set_state (gstdata->pipeline, GST_STATE_NULL);
	//g_main_loop_quit (gstdata->loop);

	g_main_loop_unref (gstdata->loop);
	gst_object_unref (gstdata->pipeline);
 //   usleep(5000); // here only 5ms

	free_all_keepalive_for_sink(gstdata);

    printf("exit thread sipuri  %s \n", gstdata->sipuri);

//	 g_mutex_lock (&source_rcv_port_mutex);
//	 if(gstdata->source.type == UDP || gstdata->source.type == RTP)
//	 {// g_hash_table_remove(Hashtbl_Udp_Source_rcv_port, gstdata->callid);
//		 printf("remove port = %d \n ",gstdata->source.dst_port);
//		// g_list_remove(Udp_Source_rcv_port,GINT_TO_POINTER(gstdata->source.dst_port));
//	 }
//	 else if(gstdata->source.type == TCPCLIENT || gstdata->source.type == TCPSERVER)
//	 {
//		 //g_hash_table_remove(Hashtbl_Tcp_Source_rcv_port, gstdata->callid);
//		// g_list_remove(Tcp_Source_rcv_port,GINT_TO_POINTER(gstdata->source.dst_port));
//	 }
//	 g_mutex_unlock (&source_rcv_port_mutex);

 }


 gboolean task_process_callback(GIOChannel *channel)
 {
     static int i = 0;
  //   static gchar buf[100];
       gsize len = 0;
     short cmd;

    int flag =0;
     GError *error = NULL;

     memset(rx_buf, 0 ,1500);

     printf("task callback \n");
     flag =  g_io_channel_read_chars(channel,rx_buf, 1500,&len, error);
     printf("str %s  len %d error= %d\n", rx_buf, len ,error);

   //  g_io_channel_read (channel,rx_buf, 1500,&len);//, error);

     if(len > 0)
    	 cmd_thread(len);

     return TRUE;
 }

static guint g_source_id;
GIOChannel* g_channel ;


static gboolean
keep_alive_timed_out_cb (GSocket      *client,
			 GIOCondition  cond,
			 gpointer      user_data)
{

	 GstCustom *gst= (GstCustom*)user_data;
	 if(gst->sink.keep_alive_flag == FALSE && gst->sink.Nat_Traversal == FALSE)
	 {
		 //Create socket for receive data
		 return 1;
	 }


    GSocketAddress *src_address =NULL;
    GSocketAddress **p_saddr;
    GError *error = NULL;
    int len;
    char buf[100];
//   //  sockaddr_in *addr;
    p_saddr =   &src_address ;

//	printf(" len=%d, buf=%s  add=%p\n", len, buf, src_address);

    len= g_socket_receive_from (client,
    		p_saddr,
                            buf,
                           100,
                           NULL,
                           &error);

    if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT))
	{

    //	printf ("keep alive timeout from socket: %s\n",	  error->message);

     	printf(" sipuri %s, callid %s not receive keeplive \n", gst->sipuri,gst->callid);

     	g_mutex_lock (&snd_data_mutex);
     	bzero(tx_buf,sizeof(tx_buf));

     //	if(sink_size >1)
     //	{
     //		sprintf(tx_buf, "sipuri=%s;callid=%s;last=no;notgotkeepalivesignal;",sink->sipuri,sink->callid);
     //	}
     //	else
     	{
     	//    char *ptr=NULL;
		//	g_mutex_lock (&source_rcv_port_mutex);
		//	ptr = g_hash_table_lookup(Hashtbl_Udp_Source_rcv_port ,tmp->sipuri);
		//	g_mutex_unlock (&source_rcv_port_mutex);

     //		sprintf(tx_buf, "code=1001;sipuri=%s;callid=%s;",gst->sipuri,gst->callid);
     	}
    	send_packet(tx_buf,strlen(tx_buf),g_remote_ip,SND_PORT);


       // send bye message to local
     	bzero(tx_buf,sizeof(tx_buf));
    	sprintf(tx_buf, "self;callid=%s;sipuri=%s;bye=ok;",gst->callid,gst->sipuri);
    	printf("========================== \n");
      	send_packet(tx_buf,strlen(tx_buf),"0.0.0.0",LISTEN_PORT);  // comment for test
    	g_mutex_unlock (&snd_data_mutex);

    	return TRUE;


	}
    else if(len > 0)
    {

    	if(gst->sink.Nat_Traversal == 1 && gst->sink.Get_Nat_address_flag == 0)
    	{
			GInetSocketAddress *addr = G_INET_SOCKET_ADDRESS (src_address);
			gchar *ip = g_inet_address_to_string (g_inet_socket_address_get_address (addr));

			guint16 port = g_inet_socket_address_get_port (addr);
			printf ( "added new client ip %s:%u \n",ip, port);
			g_object_unref (addr);

			gst->sink.Get_Nat_address_flag = 1;

			// remove the sdp address, send to the NAT address
			 g_signal_emit_by_name (gst->sink.sink, "remove", gst->sink.dst_ip, gst->sink.dst_port, NULL);

			 g_signal_emit_by_name (gst->sink.sink, "add", ip, port, NULL);

			  g_free (ip);
    	}

    }

    if (src_address) {

       g_object_unref (src_address);
       src_address = NULL;
     }
      //g_object_unref (src_address);

      return TRUE;
}



 void create_keep_alive_socket_for_sink(GstCustom *gst)
 {
	 if((gst->sink.keep_alive_flag == FALSE && gst->sink.Nat_Traversal == FALSE) || gst->sink.type == TCPSERVER || gst->sink.type == TCPCLIENT)
	 {
		 //Create socket for receive data
		 return;
	 }


	 GError *err = NULL;
	 GSource *source;
	 gboolean flag = 0;
	  gst->sink.sndkeepalive_socket = g_socket_new(G_SOCKET_FAMILY_IPV4,
	 	 	                    G_SOCKET_TYPE_DATAGRAM,
	 	 	                    G_SOCKET_PROTOCOL_UDP,
	 	 	                    &err);

		g_assert(err == NULL);

	    printf("Create sink socket src_port %d \n ", gst->sink.src_port);
		flag= g_socket_bind( gst->sink.sndkeepalive_socket,
		 	              G_SOCKET_ADDRESS(g_inet_socket_address_new(g_inet_address_new_from_string(LOCAL_IP) ,gst->sink.src_port)),
		 	              TRUE,
		 	              &err);
		//printf("flag = %d\n", flag);
		if(flag == 0)
			return ;
	   // printf ("ERROR:  %s\n", err->message);
		g_assert(err == NULL);

		//g_socket_set_blocking( sink.sndkeepalive_socket, FALSE);
		 GError *opt_err = NULL;
		if (!g_socket_set_option (gst->sink.sndkeepalive_socket, SOL_SOCKET, SO_SNDBUF,
				212992, &opt_err)) {
			 printf("Could not create a buffer of requested %d bytes: %s",
					 40960, opt_err->message);
		        g_error_free (opt_err);
		        opt_err = NULL;
		}

		// Only keep alive flag is true need set timeout event
		if(gst->sink.keep_alive_flag == TRUE)
		{
			g_socket_set_timeout( gst->sink.sndkeepalive_socket, KEEP_ALIVE_TIME_OUT);
		}

		source =  g_socket_create_source(gst->sink.sndkeepalive_socket, G_IO_IN, NULL);

	 	g_source_set_callback (source, (GSourceFunc)keep_alive_timed_out_cb, gst, NULL);

	   gst->sink.sourceid = g_source_attach (source, NULL);

	   g_source_unref(source);

	  // return  g_source_id;
 }

 void rmv_keepalive_socket_for_sink( Sink *sink)
 {
	 if((sink->keep_alive_flag == FALSE && sink->Nat_Traversal == FALSE) || sink->type == TCPSERVER)
	 {
		 return;
	 }

//	printf("remove callid %s keep alive socket \n", sink->callid );
	g_socket_close( sink->sndkeepalive_socket, NULL);

	g_object_unref(sink->sndkeepalive_socket);
	g_source_remove( sink->sourceid);

	sink->keep_alive_flag = FALSE;
	sink->Nat_Traversal = FALSE;
 }

 void add_source(GMainContext *context)
 {
	  GSource *source;

	 	GError *err = NULL;

	 	g_socket = g_socket_new(G_SOCKET_FAMILY_IPV4,
	 	                    G_SOCKET_TYPE_DATAGRAM,
	 	                    G_SOCKET_PROTOCOL_UDP,
	 	                    &err);

	 	g_assert(err == NULL);
	 //	g_inet_socket_address_new
	 	g_socket_bind(g_socket,
	 	              G_SOCKET_ADDRESS(g_inet_socket_address_new(g_inet_address_new_from_string("0.0.0.0") ,LISTEN_PORT)),
	 	              TRUE,
	 	              &err);

	 	g_assert(err == NULL);

	 	fd = g_socket_get_fd(g_socket);
	    g_socket_set_blocking(g_socket, FALSE);
	 //	g_socket_set_timeout(sock, 10);
	 	g_channel = g_io_channel_unix_new(fd);

	 	source = g_io_create_watch(g_channel, G_IO_IN | G_IO_ERR);
	 	g_io_channel_set_encoding(g_channel,NULL  ,NULL);
	 	g_source_set_callback(source, (GSourceFunc)task_process_callback, g_channel, NULL);

	 	g_source_id = g_source_attach(source,NULL);
	    g_source_unref(source);

}


void remove_source()
{
	g_source_remove(g_source_id);
	g_io_channel_shutdown(g_channel, TRUE, NULL);
}

void Stop(int signo)
{
    printf("oops! stop!!!\n");
    exit_flag = TRUE;

	//close(gRcvSocket);
    g_main_loop_quit (loop);
	close(gSndSocket);
	//close(gSndSocket);

    _exit(0);
}


int time_ticket()
{
 	g_mutex_lock (&snd_data_mutex);
	send_packet("code=1004;keeplive=true;",sizeof("code=1004;keeplive=true;"),g_remote_ip,SND_PORT);
	g_mutex_unlock (&snd_data_mutex);
	return TRUE;
}


int  main (int argc, char **argv)
{
	//  GMainContext *context;

#ifdef LOGSAVE
	 fflush(stdout);
	 setvbuf(stdout,NULL,_IONBF,0);
	 printf("test stdout\n");
	 freopen("time.log","w",stdout); //注: 不要使用这类的代码 stdout = fopen("test1.txt","w");   这样的话输出很诡异的. 最好使用  freopen 这类的函数来替换它.
	 printf("test file\n");
	 fflush(stdout);
#endif
//	 freopen("/dev/tty","w",stdout);
//	 printf("test tty\n");

	FILE *fp = NULL;
	//GstCustom gstdata;

	//create a new time-out source
	 //   source = g_timeout_source_new(1000);
	 g_stpcpy(g_remote_ip, "192.168.128.151");

    printf("Mdeia_Distribute Version=%s",VERSION);

	fp = fopen("./md.cfg", "rb");

	if(fp == NULL)
	{
		printf("not find mediaproxy.cfg \n");
		return 0;
	}
	int size = fread(tx_buf,1,1500,fp);

	if(Nat_ip_parse(tx_buf, size, g_nat_ip) == 0)
	{
		printf(" not find ip \n");
		fclose(fp);
		return 1;
	}

	if(rcv_ip_parse(tx_buf, size, g_rcv_ip) == 0)
	{
		printf(" not find ip \n");
		fclose(fp);
		return 1;
	}

	if(control_ip_parse(tx_buf, size, g_remote_ip) == 0)
	{
		printf(" not find ip \n");
		fclose(fp);
		return 1;
	}

	g_audio_codec= audio_codec_parse(tx_buf, size);

	rcv_port_min = rcv_min_port_parse(tx_buf, size);
	Cur_Rcv_Udp_Port = rcv_port_min;
	Cur_Rcv_Tcp_Port = rcv_port_min;
	rcv_port_max = rcv_max_port_parse(tx_buf, size);
	snd_port_min = snd_min_port_parse(tx_buf, size);
	Cur_Snd_Udp_Port = snd_port_min;
	Cur_Snd_Tcp_Port = snd_port_min;
	snd_port_max = snd_max_port_parse(tx_buf, size);

	fclose(fp);

	gSndSocket = snd_socket_init();

//	pthread_t gst_rcv_tid;

	//int err;
	gst_init (&argc, &argv);
   // loop = g_main_loop_new (NULL, FALSE);

    gsthashtbale = g_hash_table_new_full (g_str_hash , g_str_equal ,free_sipuri_key,  free_sipuri_value);


    //Hashtbl_Udp_Source_rcv_port = g_hash_table_new_full (g_str_hash , g_str_equal ,free_udp_rcv_port_key,print_port_value);

    //Hashtbl_udp_sink_snd_port = g_hash_table_new_full (g_str_hash , g_str_equal ,free_udp_snd_port_key,print_port_value);

    //Hashtbl_Tcp_Source_rcv_port = g_hash_table_new_full (g_str_hash , g_str_equal ,free_tcp_rcv_port_key,print_port_value);
   // Hashtbl_Tcp_sink_snd_port = g_hash_table_new_full (g_str_hash , g_str_equal ,free_tcp_snd_port_key,print_port_value);


 	loop = g_main_loop_new (NULL, FALSE);
   // add_source(NULL);

 	 GThread *cmdthread =  g_thread_new("cmd_thread",cmd_thread,NULL);

    g_timeout_add_seconds(30,time_ticket ,NULL);

	 g_main_loop_run (loop);

	g_main_loop_unref (loop);
 	//remove_source();
 	g_socket_close (g_socket,NULL);

	return 0;
}
