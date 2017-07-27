#include <string.h>
#include <gst/gst.h>

#include <sys/wait.h>
#include <sys/types.h>
#include "cmd_rcv.h"
#include "hashtable.h"
#include <gio/gio.h>

#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <sys/wait.h>
#include <sys/types.h>

#define LOCAL_IP   "192.168.168.152"
#define UDP    1
#define RTP    2
#define TCP    3

typedef struct
{
	gint source_id;  //check headbeat
	GIOChannel* io_channel;
}Heatbeat;



typedef struct
{
	char src_ip[20];
	char dst_ip[20];
	int src_port;
	int dst_port;
	int keep_alive;
	int time_cnt;
	int sock_fd;
	char callid[50];
}SinkAddress;


typedef struct
{
  GstPad *teepad;
  GstElement *queue;
  GstElement *conv;
  GstElement *depay;
  GstElement *sink;
  gboolean removing;
  char dst_ip[20];
  int dst_port;
  char *dst_uri;

  SinkAddress *sinkaddress;

  Heatbeat  rcv_beatheart;
  int snd_port;
  int type;  // UDP/RTP/TCP

  //GHashTable Multi_Address; //  address as key, keep_live as value

  //GList *Address_list;
  GHashTable *hashtb_address;   // call id as key SinkAddress as value
} Sink;

typedef struct
{
	GstElement *src;
	GstElement *tee;

    char ip[20];
    int port;
    char src_uri[50];
    int snd_beatheart;
    int type;   // TCP/RTP/UDP
}Source;


typedef struct
{
	Sink *sink;  //getting from  sink_hashtable
	Source source;
    GstElement *pipeline;
	GstBus *bus;
	char session_id[50];  //  = sip_uri
	GMainLoop* loop;
	GThread * gthread;
	GHashTable *sink_hashtable; //sink type, udp/rtp/tcp

} GstCustom;

typedef struct
{
	GstCustom  *GstCusom;
	Sink *sink;

}userpoint;

GMainLoop* loop;

GstCustom gstdata;
GHashTable *gsthashtbale;

GSocket *sock;
int fd;

void *new_pipeline_thread( gpointer *arg);
void add_source(GMainContext *context);

static gboolean
message_cb (GstBus * bus, GstMessage * message, gpointer user_data)
{
	GstCustom *gst_ptr;
	gst_ptr = (GstCustom*)user_data;
	//printf("message cb %s \n", ptr->sipui);

	const GstStructure *st = gst_message_get_structure (message);

  switch (GST_MESSAGE_TYPE (message)) {
  	case GST_MESSAGE_STATE_CHANGED:
  	{
  		GstState old_state, new_state, pending_state;
  		 gst_message_parse_state_changed (message, &old_state, &new_state, &pending_state);
  		     if (GST_MESSAGE_SRC (message) == GST_OBJECT (gst_ptr->sink->sink))
  		    {
  		       //  data->current_state = new_state;
  		        g_print ("State set to %s\n", gst_element_state_get_name (new_state));
  		       if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED)
  		       {
  		          /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
  		    	   printf("received data from %s \n", gst_ptr->source.src_uri);
  		       }
  		    }
  	}
  		break;
    case GST_MESSAGE_ERROR:{
      GError *err = NULL;
      gchar *name, *debug = NULL;

      name = gst_object_get_path_string (message->src);
      gst_message_parse_error (message, &err, &debug);

      g_printerr ("ERROR: from element %s: %s\n", name, err->message);
      if (debug != NULL)
        g_printerr ("Additional debug info:\n%s\n", debug);

      g_error_free (err);
      g_free (debug);
      g_free (name);

      g_main_loop_quit (gst_ptr->loop);
      break;
    }
    case GST_MESSAGE_WARNING:{
      GError *err = NULL;
      gchar *name, *debug = NULL;

      name = gst_object_get_path_string (message->src);
      gst_message_parse_warning (message, &err, &debug);

      g_printerr ("ERROR: from element %s: %s\n", name, err->message);
      if (debug != NULL)
        g_printerr ("Additional debug info:\n%s\n", debug);

      g_error_free (err);
      g_free (debug);
      g_free (name);
      break;
    }
    case GST_MESSAGE_ELEMENT:

    	 // g_print ("Timeout \n");
      /* We don't care for messages other than timeouts */
      if (!gst_structure_has_name (st, "GstUDPSrcTimeout"))
    break;
    g_print ("Timeout received from udpsrc %s\n", gst_ptr->session_id);
    // call thread exit
//     gst_element_set_state(GST_ELEMENT (gst_ptr->pipeline),GST_STATE_NULL);
//     g_thread_join(gst_ptr->gthread);
//
//	if(g_hash_table_remove(gsthashtbale,gst_ptr->session_id))
//	{
//		printf("remove session successful2 \n");
//	}

     break;

    case GST_MESSAGE_EOS:
      g_print ("Got EOS\n");
      g_main_loop_quit (loop);
      break;
    default:
      break;
  }

  return TRUE;
}

void cb_udp_client_add  (GstElement* object, gchararray arg0, gint arg1, gpointer user_data)

{
	printf("arg0= %s\n");
	printf("arg1 = %d \n", arg1);
	//add_source(NULL);
}

void cb_udp_client_remove  (GstElement* object, gchararray arg0, gint arg1, gpointer user_data)
{
	printf("arg0= %s\n");
	printf("arg1 = %d \n", arg1);
}


 int Create_udptoudp_Pipeline( GstCustom *gstcustom)
{
	 gstcustom->pipeline = gst_pipeline_new (NULL);

	 gstcustom->source.src = gst_element_factory_make ("udpsrc", "udpsrc");
	 gstcustom->source.tee = gst_element_factory_make ("tee", "tee");

	if (!gstcustom->pipeline || !gstcustom->source.src || !gstcustom->source.tee) {
		g_error ("Failed to create elements");
		return -1;
	}


	//g_object_set (src, "caps", gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING,"video","payload",G_TYPE_INT,33, NULL), NULL);


	 gst_bin_add_many (GST_BIN (gstcustom->pipeline), gstcustom->source.src, gstcustom->source.tee,NULL);
	  if (!gst_element_link_many (gstcustom->source.src, gstcustom->source.tee, NULL) )
	 {
		  g_error ("Failed to link elements");
		  return -2;
	 }

	  //g_signal_connect (dbin, "pad-added", G_CALLBACK (pad_added_cb), NULL);

	  //loop = g_main_loop_new (NULL, FALSE);

	  g_object_set (gstcustom->source.src, "timeout",5000000000);
	  printf("gstcustom->session_id = %s \n",gstcustom->session_id);
	  g_object_set (gstcustom->source.src, "uri",gstcustom->source.src_uri, NULL);
	//  g_object_set (gstcustom->source.src, "port", gstcustom->source.port,NULL);
	 // g_object_set (gstcustom->source.src, "address", "0.0.0.0",NULL);
	  gstcustom->bus = gst_pipeline_get_bus (GST_PIPELINE (gstcustom->pipeline));
	  gst_bus_add_signal_watch (gstcustom->bus);
	  g_signal_connect (G_OBJECT (gstcustom->bus), "message", G_CALLBACK (message_cb), gstcustom);
	  gst_object_unref (GST_OBJECT (gstcustom->bus));


	  GstPadTemplate *templ;

	 templ =
	        gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (gstcustom->source.tee),
	        "src_%u");
	    gstcustom->sink->teepad = gst_element_request_pad (gstcustom->source.tee, templ, NULL, NULL);
	    gstcustom->sink->queue = gst_element_factory_make ("queue", NULL);
	    gstcustom->sink->sink = gst_element_factory_make ("multiudpsink", "multiudpsink");
	 //   g_object_set (gstcustom->sink.sink, "host", gstcustom->sink.dst_ip,"port",gstcustom->sink.dst_port, NULL);
	   // g_object_set (gstcustom->sink.sink, "clients", outputuri, "bind-address","LOCAL_IP","bind-port",bindport, NULL);
	 //   gst_object_unref(templ);
	//    port += 1;


	    g_object_set (gstcustom->sink->sink, "send-duplicates", FALSE, NULL);

	    g_signal_connect (gstcustom->sink->sink, "client-added",G_CALLBACK (cb_udp_client_add), gstcustom);
	    g_signal_connect (gstcustom->sink->sink, "client-removed",G_CALLBACK (cb_udp_client_remove), gstcustom);

	    g_signal_emit_by_name (gstcustom->sink->sink, "add", gstcustom->sink->sinkaddress->dst_ip, gstcustom->sink->sinkaddress->dst_port, NULL);

        printf("gstcustom->sink->dst_ip %s gstcustom->sink->dst_port = %d \n",  gstcustom->sink->sinkaddress->dst_ip, gstcustom->sink->sinkaddress->dst_port);

	    gst_bin_add_many (GST_BIN (gstcustom->pipeline),gstcustom->sink->queue, gstcustom->sink->sink, NULL);
	    gst_element_link_many (gstcustom->sink->queue, gstcustom->sink->sink, NULL);

	    GstPad *sinkpad;

	    sinkpad = gst_element_get_static_pad (gstcustom->sink->queue, "sink");
	    gst_pad_link (gstcustom->sink->teepad, sinkpad);
	    gst_object_unref (sinkpad);

	  // GstPad *srcpad;
	 // srcpad = gst_element_get_static_pad (src, "src");

	  //gst_pad_add_probe (srcpad, GST_PAD_PROBE_TYPE_BUFFER , cb_have_data, NULL, NULL);
	 // gst_object_unref (srcpad);
	   printf("start player \n");
	  gst_element_set_state (gstcustom->pipeline, GST_STATE_PLAYING);
	  return TRUE;

}

 int Release_udptoudp_Pipeline( GstCustom *gstcustom)
 {
	 gst_element_set_state (gstcustom->pipeline, GST_STATE_NULL);
 }

int Add_udpsink_to_udpudp_pipeline(GstCustom *gstcustom, Sink *sink, char *ip, int port)
{
	 GstPad *sinkpad;
	    GstPadTemplate *templ;

	 templ =
	        gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (gstcustom->source.tee),
	        "src_%u");

	    g_print ("add\n");

	    sink->teepad = gst_element_request_pad (gstcustom->source.tee, templ, NULL, NULL);

	    sink->queue = gst_element_factory_make ("queue", NULL);

	    sink->sink = gst_element_factory_make ("udpsink", NULL);
	    sink->removing = FALSE;

	    gst_bin_add_many (GST_BIN (gstcustom->pipeline), gst_object_ref (sink->queue),gst_object_ref (sink->sink), NULL);
	    gst_element_link_many (sink->queue, sink->sink, NULL);

	    gst_element_sync_state_with_parent (sink->queue);

	    gst_element_sync_state_with_parent (sink->sink);

	    sinkpad = gst_element_get_static_pad (sink->queue, "sink");
	    gst_pad_link (sink->teepad, sinkpad);
	    gst_object_unref (sinkpad);

	    g_print ("added\n");
}





static GstPadProbeReturn
unlink_cb (GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{

  Sink *sink = ((userpoint *)user_data)->sink;
  GstElement *tee = ((userpoint *)user_data)->GstCusom->source.tee;
  GstPad *sinkpad;

  if (!g_atomic_int_compare_and_exchange (&sink->removing, FALSE, TRUE))
    return GST_PAD_PROBE_OK;

  sinkpad = gst_element_get_static_pad (sink->queue, "sink");
  gst_pad_unlink (sink->teepad, sinkpad);
  gst_object_unref (sinkpad);

  gst_bin_remove (GST_BIN (((userpoint *)user_data)->GstCusom->pipeline), sink->queue);
 
  gst_bin_remove (GST_BIN (((userpoint *)user_data)->GstCusom->pipeline), sink->sink);

  gst_element_set_state (sink->sink, GST_STATE_NULL);
 
  gst_element_set_state (sink->queue, GST_STATE_NULL);

  gst_object_unref (sink->queue);
 
  gst_object_unref (sink->sink);

  gst_element_release_request_pad (tee, sink->teepad);
  gst_object_unref (sink->teepad);

  g_print ("removed\n");

  return GST_PAD_PROBE_REMOVE;
}


static gboolean
cb_have_data (GstPad    *pad,
      GstBuffer *buffer,
      gpointer   u_data)
{
   
  printf("have data \n");
  return TRUE;
}


 static void *cmd_thread(void *arg)
{
	unsigned char rx_buf[1500];
	unsigned char tx_buf[1500];
    int rcv_size;
 	GstCustom *gst_ptr;
	printf("cmd thread created \n");
	gRcvSocket = rcv_socket_init();
    int src_type, sink_type, invite_flag, bye_flag, sink_dst_port;
    char src_uri[30], sink_dst_uri[30], sink_dst_ip[30];
    char gst_hashtable_key[50], sipuri[50], callid[50];
	signal(SIGINT, Stop);


	while(1)
	{
		printf("receive data \n");
		memset(rx_buf, 0 ,1500);
		rcv_size = receive_packet(rx_buf);

		memset(sipuri, 0 ,50);
		if(sipuri_parse(rx_buf,rcv_size,sipuri) == 0)
		{
			printf("not find sipuri \n");
			continue;
		}

		memset(callid, 0 ,50);
		if(callid_parse(rx_buf,rcv_size,callid) == 0)
		{
			printf("not find callid \n");
			continue;
		}

		//string  parse
		invite_flag = invite_parse(rx_buf, rcv_size);
		bye_flag    =  bye_parse(rx_buf, rcv_size);
		src_type = src_type_parse(rx_buf, rcv_size);
		sink_type =  sink_type_parse(rx_buf, rcv_size);

		if(src_uri_parse(rx_buf, rcv_size, src_uri) == 0)
		{
			printf("not find src_uri \n");
			continue;
		}

		if(sink_dst_uri_parse(rx_buf, rcv_size,sink_dst_ip, &sink_dst_port) == 0)
		{
			printf("not find sink_dst_uri \n");
			continue;
		}

       // sleep(1);
		if(invite_flag == TRUE)
		{
			switch(src_type)
			{
			case UDP:  //udp type src
			{
				//sprintf(gst_hashtable_key,"udp://%s",src_uri);
				GstCustom *tmp = g_hash_table_lookup (gsthashtbale, sipuri);

				if(tmp != 0)  // src session id is esstibition
				{
					printf("find %s \n", src_uri);
					if(sink_type == 1)
					{
						Sink *sink;
						sink = g_hash_table_lookup (tmp->sink_hashtable, "UDP");
						if(sink != NULL)
						{
							printf("find udp sink attached this dstaddress to sink\n");
							g_signal_emit_by_name (sink->sink, "add", sink_dst_ip, sink_dst_port, NULL);
							 char dst_uri[50];
							 sprintf(dst_uri,"%s:%d",sink_dst_ip,sink_dst_port);


							 SinkAddress *sinkaddress = g_new0(SinkAddress, 1);

							 g_hash_table_insert (sink->hashtb_address, g_strdup(callid), sinkaddress);
							 g_stpcpy(sinkaddress->dst_ip, sink_dst_ip);
							 sinkaddress->dst_port = sink_dst_port;

							 printf("hashtb_address size  %d \n", g_hash_table_size(sink->hashtb_address));
							// sink->Address_list = g_list_append(sink->Address_list, dst_uri);
						}
					}
				}
				else     // src session is not build, create new pipeline
				{
				   // check udp type src pipeline if is created
					 printf("create pipeline \n");
					 GstCustom *gst_ptr = g_new0(GstCustom, 1);
					// sprintf(gst_ptr->session_id,"udp://%s",src_uri);
					// gst_ptr->source.src_uri = g_strdup_printf("udp://%s",src_uri);
					 g_stpcpy(gst_ptr->session_id, sipuri);
					 sprintf(gst_ptr->source.src_uri,"udp://%s",src_uri);
					 g_hash_table_insert (gsthashtbale,  g_strdup(gst_ptr->session_id), gst_ptr);

					 printf("Create sink \n");
					 gst_ptr->sink = g_new0(Sink, 1);
					 gst_ptr->sink_hashtable = g_hash_table_new_full (g_str_hash , g_str_equal,free_key,  free_value );
					 g_hash_table_insert (gst_ptr->sink_hashtable, g_strdup("UDP"), gst_ptr->sink);

					 printf("sink_hashtable  %d \n", g_hash_table_size(gst_ptr->sink_hashtable));

					 // create address table
					 gst_ptr->sink->hashtb_address = g_hash_table_new_full (g_str_hash , g_str_equal,free_key,  free_value);
					 gst_ptr->sink->sinkaddress = g_new0(SinkAddress, 1);
					 g_hash_table_insert (gst_ptr->sink->hashtb_address, g_strdup(callid),  gst_ptr->sink->sinkaddress);

					 g_stpcpy(gst_ptr->sink->sinkaddress->dst_ip, sink_dst_ip);
					 gst_ptr->sink->sinkaddress->dst_port = sink_dst_port;

					 // store dst address to glist
					 //gst_ptr->sink->dst_ip = g_strdup_printf("udp://%s",sink_dst_uri);
					// g_stpcpy(gst_ptr->sink->dst_ip, sink_dst_ip);
					// gst_ptr->sink->dst_port = sink_dst_port;
					// char dst_uri[50];
					// sprintf(dst_uri,"%s:%d",sink_dst_ip,sink_dst_port);
					 //gst_ptr->sink->Address_list = g_list_append(gst_ptr->sink->Address_list, dst_uri);

					 gst_ptr->gthread  = g_thread_new("jieshouip:port", new_pipeline_thread , gst_ptr);
				}
//
//				gst_ptr = g_hash_table_lookup (ghashtbale, strdup("sipui"));
//				if(gst_ptr != NULL)
//				{
//					g_print("find sipui \n");
//					g_signal_emit_by_name (gst_ptr->sink->sink, "add", "192.168.2.2", 5004, NULL);
//				}
//				else
//					printf("not find sipui \n");
			}
 				break;
			default:
				break;
			}
		}
		else if(bye_flag ==TRUE)
		{
			switch(src_type)
			{
			case UDP:  //udp type src
				{
				//sprintf(gst_hashtable_key,"udp://%s",src_uri);
				GstCustom *tmp = g_hash_table_lookup (gsthashtbale, sipuri);

				if(tmp != 0)  // src session is builded
				{
					printf("finded %s \n", sipuri);
					if(sink_type == UDP)
					{
						Sink *sink;
					//	printf("remove sink type ==1 \n");
						sink = g_hash_table_lookup (tmp->sink_hashtable, "UDP");
						if(sink != NULL)
						{
							 char dst_uri[50];
							sprintf(dst_uri,"%s:%d",sink_dst_ip,sink_dst_port);
						//	GList *it = NULL;
						//	it = g_list_find(sink->Address_list, dst_uri);
							//SinkAddress *sinkaddress;
							SinkAddress *sinkaddress = g_hash_table_lookup (sink->hashtb_address, callid);

							if(sinkaddress != NULL)
							{

								// Only remove this udp sink
								if((g_hash_table_size(tmp->sink_hashtable) > 1) && g_hash_table_size(sink->hashtb_address) == 1)
								{

									 g_hash_table_destroy(sink->hashtb_address);
									 //g_hash_table_unref(sink->hashtb_address);
									if(g_hash_table_remove(tmp->sink_hashtable,"UDP"))
									{
										printf("remove udps sink successful \n");
									}
								}

								else if((g_hash_table_size(tmp->sink_hashtable) == 1) && g_hash_table_size(sink->hashtb_address) > 1)
								{
									if(g_hash_table_remove(sink->hashtb_address,callid))
									{
										printf("removed call id = %s \n", callid);
									}
									g_signal_emit_by_name (sink->sink, "remove", sink_dst_ip, sink_dst_port, NULL);
								}

								// Both address list and sink is one, quit this pipeline
								else if(g_hash_table_size(sink->hashtb_address) == 1  && g_hash_table_size(tmp->sink_hashtable) == 1)
								{
									// only left one sink client, close this pipeline
									printf("quit this pipeline \n");
									gst_element_set_state(GST_ELEMENT (tmp->pipeline),GST_STATE_NULL);
									g_main_loop_quit (tmp->loop);

								//	g_list_free(sink->Address_list);

								 // 	g_hash_table_destroy(tmp->sink_hashtable);


									 g_hash_table_destroy(sink->hashtb_address);
									 g_hash_table_unref(sink->hashtb_address);

								 	// waitting thread exit
								 	printf(" waitting thread exit \n");
								 	g_thread_join(tmp->gthread);

								 	if(g_hash_table_remove(gsthashtbale,tmp->session_id))
									{
										printf("remove sipuri session %s \n", tmp->session_id);
									}

									printf("exit thread \n");
								}
								else
								{
									//printf("remove dst_uri %s \n" ,dst_uri);
									//g_signal_emit_by_name (sink->sink, "remove", sink_dst_ip, sink_dst_port, NULL);
									//sink->Address_list = g_list_remove(sink->Address_list, dst_uri);
									//int i = g_list_length(sink->Address_list);
									//printf("list length = %d \n", i);
								}
							}
							else
							{
								printf("not find callid %s \n",callid);
							}
						}
						else
						{
							printf("not find UDP sink \n");
						}
					}
				}
				else     // src session is not build, create new pipeline
				{
//				   // check udp type src pipeline if is created
//					 printf("create pipeline \n");
//					 GstCustom *gst_ptr = g_new0(GstCustom, 1);
//				//	 gst_ptr->session_id = g_strdup_printf("udp://%s",src_uri);
//					// g_stpcpy(gst_ptr->source.src_uri, "192.168.128.152:60000");
//					 g_hash_table_insert (gsthashtbale,  gst_ptr->session_id, gst_ptr);
//
//					 printf("Create sink \n");
//					 gst_ptr->sink = g_new0(Sink, 1);
//					 gst_ptr->sink_hashtable = g_hash_table_new_full (g_str_hash , g_str_equal,free_key,  free_value );
//					 g_hash_table_insert (gst_ptr->sink_hashtable, g_strdup("UDP"), gst_ptr->sink);
//
//					 // store dst address to glist
//					 //gst_ptr->sink->dst_ip = g_strdup_printf("udp://%s",sink_dst_uri);
//					 g_stpcpy(gst_ptr->sink->dst_ip, sink_dst_ip);
//					 gst_ptr->sink->dst_port = sink_dst_port;
//					 char dst_uri[50];
//					 sprintf(dst_uri,"%s:%d",sink_dst_ip,sink_dst_port);
//					 gst_ptr->sink->Address_list = g_list_append(gst_ptr->sink->Address_list, dst_uri);
//
//					 gst_ptr->gthread  = g_thread_new("jieshouip:port", new_pipeline_thread , gst_ptr);
				}
//
//				gst_ptr = g_hash_table_lookup (ghashtbale, strdup("sipui"));
//				if(gst_ptr != NULL)
//				{
//					g_print("find sipui \n");
//					g_signal_emit_by_name (gst_ptr->sink->sink, "add", "192.168.2.2", 5004, NULL);
//				}
//				else
//					printf("not find sipui \n");
				}
				break;
			default:
				break;
			}
		}
	}
	printf("exit \n");


}

 void *new_pipeline_thread( gpointer *arg)
 {
	GstCustom *gstdata;
	gstdata = (GstCustom *)arg;
	// gst_init (NULL, NULL);
	gstdata->loop =  g_main_loop_new (NULL, FALSE);
	printf("create new thread for new pipeline\n");


	Create_udptoudp_Pipeline(gstdata);

	g_main_loop_run (gstdata->loop);

	//gst_element_set_state (gstdata->pipeline, GST_STATE_NULL);
	//g_main_loop_quit (gstdata->loop);

	g_main_loop_unref (gstdata->loop);
	gst_object_unref (gstdata->pipeline);
    printf("exit %s \n", gstdata->session_id);

	 //end thread, destory hash table

	 g_hash_table_destroy(gstdata->sink_hashtable);

	// remove_source();
//	if(g_hash_table_remove(gsthashtbale,gstdata->session_id))
//	{
//		printf("remove session successful \n");
//	}


 }


 // listen keep alive packet
void *keep_alive_thread( gpointer *arg)
{


	///////////GstCustom *tmp = g_hash_table_lookup (gsthashtbale, sipuri);
 //GstCustom *gstdata;
 //gstdata = (GstCustom *)arg;

	while(1)
	{
		GList *sipuri_key;
		sipuri_key = g_hash_table_get_keys (gsthashtbale);

		printf("sipuri len %d \n", g_list_length(sipuri_key));
		GList *it = NULL;
		for (it = sipuri_key; it != NULL; it = it->next)
		{
		 // do something with l->data
			printf("it.data %s \n", it->data);
		}
		g_list_free(sipuri_key);
		sleep(25);

	}


}

 gboolean task_process_callback(GIOChannel *channel)
 {
     static int i = 0;
     static gchar buf[100];
      gsize len;
     short cmd;
     GstCustom *tmp=NULL;

     GError *error = NULL;
     memset(buf,0,100);
   //  g_io_channel_read_chars(channel,buf, 100,&len, error);
    // g_io_channel_read (channel,buf, 100,&len);//, error);

     struct sockaddr_in src_Addr;
     int rec_size=recvfrom(fd,  buf, 100, 0, (struct sockaddr *)&src_Addr, sizeof(struct sockaddr_in));

     printf("rec %s \n", buf);
     printf("rec_size= %d , ip= %s \n",rec_size, inet_ntoa(src_Addr.sin_addr));
//     GSocketAddress *src_address;
//   //  sockaddr_in *addr;
//     g_socket_receive_from (sock,
//    		 &src_address,
//                             buf,
//                            100,
//                            NULL,
//                            NULL);
//
//     g_print (" from %s  \n", socket_address_to_string (src_address));

//     g_socket_address_to_native(src_address,addr, g_socket_address_get_native_size(GSocketAddress), NULL);
//
//     printf(" address %s ",inet_ntoa(*addr.sin_addr.s_addr));
//
//     printf("str %s  len %d error= %d\n", buf, len ,error);
//    // cmd= *(short*)(&buf[0]);

     printf("cmd %d \n", cmd);

     switch(buf[0])
     {
		 case 10:

			 switch(buf[1])
			 {
			 	 case 1:
//			 		 printf("create pipeline \n");
//			 		GstCustom *gst_ptr = g_new0(GstCustom, 1);
//			 		g_hash_table_insert (gsthashtbale, g_strdup("sipui"), gst_ptr);
//			 		printf("add = %d \n", &gstdata);
			 	//	Create_udptoudp_Pipeline(&gstdata , "udp://0.0.0.0:1028", "192.168.128.151:1030",1024);
				 break;

				 case 2:

					 tmp = g_hash_table_lookup (gsthashtbale, strdup("sipui"));
					  printf("add = %d \n", tmp);

					 // delete pipeline ;
					 break;
				 default:
					 break;
			 }

			 break;
		 default:
			 break;



     }
    // g_print("timeout_callback called %d times\n", len);

     //    g_main_loop_quit( (GMainLoop*)data );
     //    return FALSE;

    //  g_free(str);
     return TRUE;
 }

static guint g_source_id;
GIOChannel* g_channel ;

 void add_source(GMainContext *context)
 {
	  GSource *source;

	 	GError *err = NULL;

	 	sock = g_socket_new(G_SOCKET_FAMILY_IPV4,
	 	                    G_SOCKET_TYPE_DATAGRAM,
	 	                    G_SOCKET_PROTOCOL_UDP,
	 	                    &err);
	 	printf("cccc\n");
	 	g_assert(err == NULL);
	 //	g_inet_socket_address_new
	 	g_socket_bind(sock,
	 	              G_SOCKET_ADDRESS(g_inet_socket_address_new(g_inet_address_new_from_string("0.0.0.0") ,10000)),
	 	              TRUE,
	 	              &err);
	 	printf("dddd\n");
	 	g_assert(err == NULL);

	 	  fd = g_socket_get_fd(sock);
	 	g_channel = g_io_channel_unix_new(fd);
	 	source = g_io_create_watch(g_channel, G_IO_IN);
	 	g_io_channel_set_encoding(g_channel,NULL  ,NULL);
	 	g_source_set_callback(source, (GSourceFunc)task_process_callback, g_channel, NULL);

	 	  printf("aaaa\n");
	 	//g_io_channel_unref(channel);
	 	printf("bbbbb\n");

	 	g_source_id = g_source_attach(source,NULL);
	    g_source_unref(source);

}


void remove_source()
{
	g_source_remove(g_source_id);
	g_io_channel_shutdown(g_channel, TRUE, NULL);
}

int
main (int argc, char **argv)
{
	//  GMainContext *context;

	//GstCustom gstdata;

	//create a new time-out source
	 //   source = g_timeout_source_new(1000);

	pthread_t gst_rcv_tid;
	pthread_t keep_alive_tid;
	//int err;
	gst_init (&argc, &argv);
   // loop = g_main_loop_new (NULL, FALSE);

    gsthashtbale = g_hash_table_new_full (g_str_hash , g_str_equal ,free_key,  free_value);

  // add_source(NULL);
   // memcpy(gstdata.sipui,"suzhou", 6);
  //  memcpy(gstdata.sink->dst_ip,"192.168.2.22", 12);
  // gstdata.sink.dst_ip="192.168.2.22";

//    gstdata.source.port = 1026;
//	// Create sink hash table
//	gstdata.sink_hashtable = g_hash_table_new_full (g_str_hash , g_str_equal,free_key,  free_value );
//
//	gstdata.sink = g_new0 (Sink, 1);
//    gstdata.sink->dst_port = 1024;
//    g_hash_table_insert (gstdata.sink_hashtable, strdup("siptobeijing"), gstdata.sink);
//
//    gstdata.gthread  = g_thread_new("hello word", new_pipeline_thread , &gstdata);
//
// //   g_hash_table_insert (ghashtbale, strdup("siptobeijing"), &gstdata);
//  //  g_hash_table_insert (ghashtbale, strdup("sipui"), &gstdata);


   pthread_create(&gst_rcv_tid, NULL, cmd_thread,NULL); //创建线程
   pthread_create(&keep_alive_tid, NULL, keep_alive_thread,NULL); //创建线程

    while(1)
    {
    	usleep(100000);
    }


//	loop = g_main_loop_new (NULL, FALSE);


  //g_source_set_callback (source,timeout_callback,loop,NULL);



	//g_main_loop_run (loop);


	g_main_loop_unref (loop);
//	remove_source();
	return 0;
}
