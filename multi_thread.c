#include <string.h>
#include <gst/gst.h>

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

#define LOCAL_IP   "192.168.168.152"
#define UDP    1
#define RTP    2
#define TCP    3


#define  RCV_PORT_MIN   60000
#define  RCV_PORT_MAX   61000
#define  SND_PORT_MIN   60000
#define  SND_PORT_MAX   61000
#define  PORT_STEP      2

static int Cur_Rcv_Port=60000;
static int Cur_Snd_Port=60000;


GHashTable *Hashtbl_Cur_Rcv_Port; // sipuri and port
GHashTable *Hashtbl_Cur_Snd_Port; // callid and port

GList *sink_bind_port_list = NULL;
GList *source_rcv_port_list = NULL;

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
    g_print ("Timeout received from udpsrc %s\n", gst_ptr->sip_uri);
    // call thread exit
//     gst_element_set_state(GST_ELEMENT (gst_ptr->pipeline),GST_STATE_NULL);
//     g_thread_join(gst_ptr->gthread);
//
//	if(g_hash_table_remove(gsthashtbale,gst_ptr->sip_uri))
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
	  printf("gstcustom->sipuri = %s \n",gstcustom->sip_uri);
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


	 //   g_object_set (gstcustom->sink->sink, "bind-address",gstcustom->sink->src_ip,"bind-port",gstcustom->sink->src_port, NULL);

	 //   gst_object_unref(templ);
	//    port += 1;


	    g_object_set (gstcustom->sink->sink, "send-duplicates", FALSE, NULL);

	    g_signal_connect (gstcustom->sink->sink, "client-added",G_CALLBACK (cb_udp_client_add), gstcustom);
	    g_signal_connect (gstcustom->sink->sink, "client-removed",G_CALLBACK (cb_udp_client_remove), gstcustom);

	    g_signal_emit_by_name (gstcustom->sink->sink, "add", gstcustom->sink->dst_ip, gstcustom->sink->dst_port, NULL);

        printf("gstcustom->sink->dst_ip %s gstcustom->sink->dst_port = %d \n",  gstcustom->sink->dst_ip, gstcustom->sink->dst_port);

        gst_bin_add_many (GST_BIN (gstcustom->pipeline), gst_object_ref (gstcustom->sink->queue),gst_object_ref (gstcustom->sink->sink), NULL);
	   // gst_bin_add_many (GST_BIN (gstcustom->pipeline),gstcustom->sink->queue, gstcustom->sink->sink, NULL);
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

int Add_udpsink_to_udpudp_pipeline(GstCustom *gstcustom, Sink *sink)
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

	//	g_object_set (sink->sink, "bind-address",sink->src_ip,"bind-port",sink->src_port, NULL);

		//   gst_object_unref(templ);
		//    port += 1;

		g_object_set (sink->sink, "send-duplicates", FALSE, NULL);

		g_signal_connect (sink->sink, "client-added",G_CALLBACK (cb_udp_client_add), gstcustom);
		g_signal_connect (sink->sink, "client-removed",G_CALLBACK (cb_udp_client_remove), gstcustom);

		g_signal_emit_by_name (sink->sink, "add", sink->dst_ip, sink->dst_port, NULL);

		printf("gstcustom->sink->dst_ip %s gstcustom->sink->dst_port = %d \n", sink->dst_ip, sink->dst_port);


	    sink->removing = FALSE;

	    gst_bin_add_many (GST_BIN (gstcustom->pipeline), gst_object_ref (sink->queue),gst_object_ref (sink->sink), NULL);
	    gst_element_link_many (sink->queue, sink->sink, NULL);

	    gst_element_sync_state_with_parent (sink->queue);

	    gst_element_sync_state_with_parent (sink->sink);

	    sinkpad = gst_element_get_static_pad (sink->queue, "sink");
	    gst_pad_link (sink->teepad, sinkpad);
	    gst_object_unref (sinkpad);

	    g_print ("added\n");

	   // create_keep_alive_socket_rcv_source();
}





static GstPadProbeReturn
unlink_cb (GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
  GstCustom *gstptr = (GstCustom*)user_data;
  Sink *sink = gstptr->sink;
  GstElement *tee = gstptr->source.tee;

  GstPad *sinkpad;

  if (!g_atomic_int_compare_and_exchange (&sink->removing, FALSE, TRUE))
    return GST_PAD_PROBE_OK;

  sinkpad = gst_element_get_static_pad (sink->queue, "sink");
  gst_pad_unlink (sink->teepad, sinkpad);


  gst_object_unref (sinkpad);

  gst_bin_remove (GST_BIN (gstptr->pipeline), sink->queue);
 
  gst_bin_remove (GST_BIN(gstptr->pipeline), sink->sink);

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
	 int rcv_size;
	 GstCustom *gst_ptr;
	signal(SIGINT, Stop);

	printf("cmd thread created \n");
	gRcvSocket = rcv_socket_init();
	while(1)
	{

		unsigned char rx_buf[1500];
		unsigned char tx_buf[1500];


		int src_type = 0, sink_type = 0, invite_flag =0, bye_flag = 0, sink_dst_port= 0, sink_src_port = 0, sink_keep_alive_flag=0;
		char src_uri[30], sink_dst_uri[30], sink_dst_ip[30];
		char gst_hashtable_key[50], sipuri[50], callid[50];


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

		if(request_address()  == 1)
		{
			//snd address to request
			char *ptr;
			char str[255];


			while(1)
			{
				ptr = NULL;
				sprintf(str, "%d", Cur_Rcv_Port);
				ptr = g_hash_table_lookup(Hashtbl_Cur_Rcv_Port ,str);
				if(ptr == NULL)
				{
					// this port is available
					printf("not find port %d in Hashtbl_Cur_Rcv_Port  \n", Cur_Rcv_Port);
					break;
				}
				else
				{

					printf(" find port= %d in Hashtbl_Cur_Rcv_Port  sipuri = %s \n", Cur_Rcv_Port, ptr);  // this port is available
					if(g_strcmp0(ptr,sipuri) == 0)
					{
						printf("the same sipuri \n");  // request the same sipuri streaming
						break;
					}
					else
					{
						   //assign new port
						   if(Cur_Rcv_Port >= RCV_PORT_MAX)
						   {
							   Cur_Rcv_Port = RCV_PORT_MIN;
						   }
						   Cur_Rcv_Port +=2;
					}

				}
			}
		    g_hash_table_insert ( Hashtbl_Cur_Rcv_Port, g_strdup(str), g_strdup(sipuri));


			while(1)
			{
				ptr = NULL;
				sprintf(str, "%d", Cur_Snd_Port);
				ptr = g_hash_table_lookup(Hashtbl_Cur_Snd_Port ,str);
				if(ptr == NULL)
				{
					// this port is available
					printf("not find port %d in Hashtbl_Cur_Snd_Port  \n", Cur_Snd_Port);
					break;
				}
				else
				{
					if(g_strcmp0(ptr,callid) == 0)
					{
						printf("error : the same callid \n");  // request the same sipuri streaming
						break;
					}
					//assign new port
					if(Cur_Snd_Port >= SND_PORT_MAX)
					{
					   Cur_Snd_Port = SND_PORT_MIN;
					}
					Cur_Snd_Port +=2;
				}
			}
		    g_hash_table_insert ( Hashtbl_Cur_Snd_Port, g_strdup(str), g_strdup(callid));

			//if()
			continue;
		}
		//string  parse
		invite_flag = invite_parse(rx_buf, rcv_size);
		bye_flag    =  bye_parse(rx_buf, rcv_size);
		src_type = src_type_parse(rx_buf, rcv_size);
		sink_type =  sink_type_parse(rx_buf, rcv_size);
		sink_src_port =  sink_src_port_parse(rx_buf, rcv_size);
		sink_keep_alive_flag = sink_keep_alive_parse(rx_buf, rcv_size);

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
			GstCustom *tmp = g_hash_table_lookup (gsthashtbale, sipuri);

			switch(src_type)
			{
			case UDP:  //udp type src
			{
				//sprintf(gst_hashtable_key,"udp://%s",src_uri);


				if(tmp != 0)  // src session id is esstibition
				{
					printf("find %s \n", sipuri);
					if(sink_type == 1)
					{

						//sink = g_hash_table_lookup (tmp->sink_hashtable, "UDP");
						//if(sink != NULL)
						{
							printf("find the same sipuri, attach the new sink to sipuri %s \n", sipuri);

							Sink *sink;

							sink = g_hash_table_lookup (tmp->sink_hashtable, callid);
							if(sink != NULL)
							{
								printf("error the same callid and the same sipuri \n");
								break;
							}
							printf("create new sink \n");

							 sink  = g_new0(Sink, 1);

							 g_stpcpy(sink->callid, callid);
							 g_stpcpy(sink->sipuri, sipuri);

							 sink->snd_port = sink_src_port;
							 sink->keep_alive_flag = sink_keep_alive_flag;

							 create_keep_alive_socket_rcv_source(sink);

							 g_hash_table_insert (tmp->sink_hashtable, g_strdup(callid), sink);

							 printf("sink_hashtable  %d \n", g_hash_table_size(tmp->sink_hashtable));

							 g_stpcpy(sink->dst_ip, sink_dst_ip);
							 sink->dst_port = sink_dst_port;
							 tmp->sink = sink;


							Add_udpsink_to_udpudp_pipeline(tmp, sink);

//							 char dst_uri[50];
//							 sprintf(dst_uri,"%s:%d",sink_dst_ip,sink_dst_port);
//
//
//							 SinkAddress *sinkaddress = g_new0(SinkAddress, 1);
//
//							 g_hash_table_insert (sink->hashtb_address, g_strdup(callid), sinkaddress);
//							 g_stpcpy(sinkaddress->dst_ip, sink_dst_ip);
//							 sinkaddress->dst_port = sink_dst_port;
//
//							 printf("hashtb_address size  %d \n", g_hash_table_size(sink->hashtb_address));
							// sink->Address_list = g_list_append(sink->Address_list, dst_uri);
						}
					}
				}
				else     // src session is not build, create new pipeline
				{
				   // check udp type src pipeline if is created
					 printf("create pipeline \n");
					 GstCustom *gst_ptr = g_new0(GstCustom, 1);
					// sprintf(gst_ptr->sip_uri,"udp://%s",src_uri);
					// gst_ptr->source.src_uri = g_strdup_printf("udp://%s",src_uri);
					 g_stpcpy(gst_ptr->sip_uri, sipuri);
					 sprintf(gst_ptr->source.src_uri,"udp://%s",src_uri);
					 g_hash_table_insert (gsthashtbale,  g_strdup(gst_ptr->sip_uri), gst_ptr);

					 printf("Create sink \n");
					 gst_ptr->sink = g_new0(Sink, 1);
					 g_stpcpy(gst_ptr->sink->callid, callid);
					 g_stpcpy(gst_ptr->sink->sipuri, sipuri);
					 gst_ptr->sink->snd_port = sink_src_port;
					 gst_ptr->sink->keep_alive_flag = sink_keep_alive_flag;

					 create_keep_alive_socket_rcv_source(gst_ptr->sink);

					 gst_ptr->sink_hashtable = g_hash_table_new_full (g_str_hash , g_str_equal,free_key,  free_value );
					 g_hash_table_insert (gst_ptr->sink_hashtable, g_strdup(callid), gst_ptr->sink);

					 printf("sink_hashtable  %d \n", g_hash_table_size(gst_ptr->sink_hashtable));

					 g_stpcpy(gst_ptr->sink->dst_ip, sink_dst_ip);
					 gst_ptr->sink->dst_port = sink_dst_port;

					 // create address table
//					 gst_ptr->sink->hashtb_address = g_hash_table_new_full (g_str_hash , g_str_equal,free_key,  free_value);
//					 gst_ptr->sink->sinkaddress = g_new0(SinkAddress, 1);
//					 g_hash_table_insert (gst_ptr->sink->hashtb_address, g_strdup(callid),  gst_ptr->sink->sinkaddress);
//
//					 g_stpcpy(gst_ptr->sink->sinkaddress->dst_ip, sink_dst_ip);
//					 gst_ptr->sink->sinkaddress->dst_port = sink_dst_port;

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
		else if(bye_flag == TRUE)
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
						sink = g_hash_table_lookup (tmp->sink_hashtable, callid);

						tmp->sink = sink;
						if(sink != NULL)
						{
							// char dst_uri[50];
							//sprintf(dst_uri,"%s:%d",sink_dst_ip,sink_dst_port);
							{

								// Only remove this udp sink
								if(g_hash_table_size(tmp->sink_hashtable) > 1)
								{
									printf(" remove udp sink callid = %s \n", callid);
									// g_hash_table_destroy(sink->hashtb_address);
									 //g_hash_table_unref(sink->hashtb_address);

									// gst_pad_add_probe (sink->teepad, GST_PAD_PROBE_TYPE_IDLE, unlink_cb, tmp, (GDestroyNotify) g_free);

									 gst_pad_add_probe (sink->teepad, GST_PAD_PROBE_TYPE_IDLE, unlink_cb, tmp,NULL);

									 rmv_keepalive_socket_rcv_source(sink);

									if(g_hash_table_remove(tmp->sink_hashtable, callid))
									{
										printf("remove udps sink successful \n");
									}
								}

								else if((g_hash_table_size(tmp->sink_hashtable) == 1))
								{
									// only left one sink client, close this pipeline
									printf("quit this pipeline \n");

									// if need check ref count need check ,
//									printf("queue value =%d \n" ,GST_OBJECT_REFCOUNT_VALUE(sink->queue));
//								    gst_object_unref (sink->queue);
//									gst_object_unref (sink->sink);
//								    printf("queue value =%d \n" ,GST_OBJECT_REFCOUNT_VALUE(sink->queue));

									gst_element_set_state(GST_ELEMENT (tmp->pipeline),GST_STATE_NULL);
									printf("set status to NUll \n");
									//sleep(1);

									rmv_keepalive_socket_rcv_source(sink);

								//	printf("queue value =%d \n" ,GST_OBJECT_REFCOUNT_VALUE(sink->queue));
									g_main_loop_quit (tmp->loop);

									printf(" waitting thread exit \n");
									g_thread_join(tmp->gthread);

								//	printf("queue value =%d \n" ,GST_OBJECT_REFCOUNT_VALUE(sink->queue));

									if(g_hash_table_remove(gsthashtbale,tmp->sip_uri))
									{
										printf("remove sipuri %s \n", tmp->sip_uri);
									}

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
//							else
//							{
//								printf("not find callid %s \n",callid);
//							}
						}
						else
						{
							printf("not find UDP sink \n");
						}
					}
				}
				else   // src session is not build, create new pipeline
				{
//				   // check udp type src pipeline if is created
//					 printf("create pipeline \n");
//					 GstCustom *gst_ptr = g_new0(GstCustom, 1);
//				//	 gst_ptr->sip_uri = g_strdup_printf("udp://%s",src_uri);
//					// g_stpcpy(gst_ptr->source.src_uri, "192.168.128.152:60000");
//					 g_hash_table_insert (gsthashtbale,  gst_ptr->sip_uri, gst_ptr);
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


    printf("exit %s \n", gstdata->sip_uri);

	 //end thread, destory hash table

	 g_hash_table_destroy(gstdata->sink_hashtable);

	// remove_source();
//	if(g_hash_table_remove(gsthashtbale,gstdata->sip_uri))
//	{
//		printf("remove session successful \n");
//	}


 }


 // listen keep alive packet
void *keep_alive_thread( gpointer *arg)
{


	///////////GstCustom *tmp = g_hash_table_lookup (gsthashtbale, sipuri);
   GstCustom *gstdata;
   Sink *sink;
   SinkAddress *sinkaddress;
 //gstdata = (GstCustom *)arg;
#if 0
	while(1)
	{
		GList *sipuri_key;
		sipuri_key = g_hash_table_get_keys (gsthashtbale);

		printf("sipuri len %d \n", g_list_length(sipuri_key));
		GList *it = NULL, *sink_list = NULL, *sinkaddress_list = NULL;
		// for each sipuri
		for (it = sipuri_key; it != NULL; it = it->next)
		{

			printf("sip uri %s \n", it->data);
			gstdata = g_hash_table_lookup(gsthashtbale , it->data);

			GList *sink_key;
			sink_key = g_hash_table_get_keys (gstdata->sink_hashtable);

			printf("each sipuri sink len %d \n", g_list_length(sink_key));
			//gstdata->sink_hashtable;
			// for each sink
			for(sink_list = sink_key; sink_list !=NULL;sink_list=sink_list->next)
			{
				printf("sink type %s \n", sink_list->data);
				sink = g_hash_table_lookup(gstdata->sink_hashtable , sink_list->data);
				GList *callid_list;  //callid
				callid_list = g_hash_table_get_keys (sink->hashtb_address);
				printf("each sink of total callid len %d \n", g_list_length(callid_list));

				// only one multiudpsink for send to multi client

				receive_data_from_address();  // first read data and store client address;

				for(sinkaddress_list = callid_list; sinkaddress_list !=NULL;sinkaddress_list=sinkaddress_list->next)
				{
					printf("sink address list callid %s \n", sinkaddress_list->data);
					sinkaddress = g_hash_table_lookup(sink->hashtb_address , sinkaddress_list->data);
					printf("sink address alive %d \n",sinkaddress->keep_alive);

					// keep alive detect process
					if(sinkaddress->keep_alive == 1)
					{
						//read data;
						// if read data return false
						//
						rcv_keep_alive_socket_init(sinkaddress);
						address = receive_data_from_address();
					}

				}
				g_list_free(sinkaddress_list);

			}

			 g_list_free(sink_key);

		}
		printf("sink type 555 \n");
		g_list_free(sipuri_key);
		sleep(5);

	}
#endif

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

     printf("task callback \n");
   //  g_io_channel_read_chars(channel,buf, 100,&len, error);
   //   g_io_channel_read (channel,buf, 100,&len);//, error);

     struct sockaddr_in src_Addr;
   //  int rec_size=recvfrom(fd,  buf, 100, 0, (struct sockaddr *)&src_Addr, sizeof(struct sockaddr_in));

  //   printf("rec %s \n", buf);
  //   printf("rec_size= %d , ip= %s \n",rec_size, inet_ntoa(src_Addr.sin_addr));
     GSocketAddress *src_address;

//   //  sockaddr_in *addr;
     len= g_socket_receive_from (sock,
    		 &src_address,
                             buf,
                            100,
                            NULL,
                            &error);

     GInetAddress *addr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(src_address));
       guint16 port = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(src_address));

       g_print("New Connection from %s:%d\n", g_inet_address_to_string(addr), port);

    if (len < 0)
  	{
  	  g_printerr ("Error receiving from socket: %s\n",
  		      error->message);

  	}

//
//     g_print (" from %s  \n", socket_address_to_string (src_address));

//     g_socket_address_to_native(src_address,addr, g_socket_address_get_native_size(GSocketAddress), NULL);
//
//     printf(" address %s ",inet_ntoa(*addr.sin_addr.s_addr));
//
      printf("str %s  len %d error= %d\n", buf, len ,error);
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


static gboolean
keep_alive_timed_out_cb (GSocket      *client,
			 GIOCondition  cond,
			 gpointer      user_data)
{
	 Sink *sink = (Sink *) user_data;

	 if(sink->keep_alive_flag == FALSE)
	  return;

    GSocketAddress *src_address;
    GError *error = NULL;
    int len;
    char buf[100];
//   //  sockaddr_in *addr;
    len= g_socket_receive_from (client,
   		 &src_address,
                            buf,
                           100,
                           NULL,
                           &error);

    if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT))
	{
	// Assume that this is EPRINTERONFIRE
    	printf("time out \n");

    	g_printerr ("Error receiving from socket: %s\n",
    				  error->message);

     	printf(" sipuri %s, callid %s not receive keeplive \n", sink->sipuri,sink->callid);


     	GstCustom *tmp = g_hash_table_lookup (gsthashtbale, sink->sipuri);

		if(tmp != 0)  // src session is builded
		{
			//printf("finded %s \n", sipuri);
			//if(sink_type == UDP)
			{
			//	Sink *sink;
			//	printf("remove sink type ==1 \n");
			//	sink = g_hash_table_lookup (tmp->sink_hashtable, sink->callid);

				tmp->sink = sink;
				if(sink != NULL)
				{
					// char dst_uri[50];
					//sprintf(dst_uri,"%s:%d",sink_dst_ip,sink_dst_port);
					{
						// Only remove this udp sink
						if(g_hash_table_size(tmp->sink_hashtable) > 1)
						{
							printf(" remove udp sink callid = %s \n", sink->callid);
							// g_hash_table_destroy(sink->hashtb_address);
							 //g_hash_table_unref(sink->hashtb_address);

							// gst_pad_add_probe (sink->teepad, GST_PAD_PROBE_TYPE_IDLE, unlink_cb, tmp, (GDestroyNotify) g_free);

							 gst_pad_add_probe (sink->teepad, GST_PAD_PROBE_TYPE_IDLE, unlink_cb, tmp,NULL);

							 rmv_keepalive_socket_rcv_source(sink);

							if(g_hash_table_remove(tmp->sink_hashtable, sink->callid))
							{
								printf("remove udps sink successful \n");
							}
						}

						// only left one sink client, close this pipeline
						else if((g_hash_table_size(tmp->sink_hashtable) == 1))
						{

							printf("quit this pipeline \n");

							// if need check ref count need check ,
//									printf("queue value =%d \n" ,GST_OBJECT_REFCOUNT_VALUE(sink->queue));
//								    gst_object_unref (sink->queue);
//									gst_object_unref (sink->sink);
//								    printf("queue value =%d \n" ,GST_OBJECT_REFCOUNT_VALUE(sink->queue));

							gst_element_set_state(GST_ELEMENT (tmp->pipeline),GST_STATE_NULL);
							printf("set status to NUll \n");
							//sleep(1);

							rmv_keepalive_socket_rcv_source(sink);

						//	printf("queue value =%d \n" ,GST_OBJECT_REFCOUNT_VALUE(sink->queue));
							g_main_loop_quit (tmp->loop);

							printf(" waitting thread exit \n");
							g_thread_join(tmp->gthread);

						//	printf("queue value =%d \n" ,GST_OBJECT_REFCOUNT_VALUE(sink->queue));

							if(g_hash_table_remove(gsthashtbale,tmp->sip_uri))
							{
								printf("remove sipuri session %s \n", tmp->sip_uri);
							}

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
//							else
//							{
//								printf("not find callid %s \n",callid);
//							}
				}
				else
				{
					printf("not find UDP sink \n");
				}
			}
		}

//		 gst_pad_add_probe (sink->teepad, GST_PAD_PROBE_TYPE_IDLE, unlink_cb, tmp,NULL);
//
//		if(g_hash_table_remove(tmp->sink_hashtable, callid))
//		{
//			printf("remove udps sink successful \n");
//		}

    	return TRUE;
	}
    else if(len > 0)
    {
    	printf(" len=%d, buf=%s \n", len, buf);

		GInetAddress *addr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(src_address));
		guint16 port = g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(src_address));

		g_print("New Connection from %s:%d\n", g_inet_address_to_string(addr), port);

		// create NAT sink in here

		g_object_unref (src_address);
		g_object_unref (addr);

    }

      return TRUE;
}



 void create_keep_alive_socket_rcv_source(Sink *sink)
 {
	 if(sink->keep_alive_flag == FALSE)
	 return;

	 GError *err = NULL;
	 GSource *source;
	 gboolean flag;
	  sink->sndkeepalive_socket = g_socket_new(G_SOCKET_FAMILY_IPV4,
	 	 	                    G_SOCKET_TYPE_DATAGRAM,
	 	 	                    G_SOCKET_PROTOCOL_UDP,
	 	 	                    &err);

		g_assert(err == NULL);

	    printf("sink snd_port %d \n ", sink->snd_port);
		flag= g_socket_bind( sink->sndkeepalive_socket,
		 	              G_SOCKET_ADDRESS(g_inet_socket_address_new(g_inet_address_new_from_string("0.0.0.0") ,sink->snd_port)),
		 	              TRUE,
		 	              &err);
		printf("flag = %d\n", flag);

	    g_printerr ("ERROR:  %s\n", err->message);
		g_assert(err == NULL);

		g_socket_set_blocking( sink->sndkeepalive_socket, FALSE);
		g_socket_set_timeout( sink->sndkeepalive_socket, 30);

		source =  g_socket_create_source(sink->sndkeepalive_socket, G_IO_IN, NULL);


		g_source_set_callback (source, (GSourceFunc)keep_alive_timed_out_cb,
				sink, NULL);

	   sink->sourceid = g_source_attach (source, NULL);

	   g_source_unref(source);

	  // return  g_source_id;
 }

 void rmv_keepalive_socket_rcv_source( Sink *sink)
 {
	 if(sink->keep_alive_flag == FALSE)
		 return;

	printf("remove callid keep alive %s \n", sink->callid );
	g_socket_close( sink->sndkeepalive_socket, NULL);
	g_source_remove( sink->sourceid);

 }


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
	 	              G_SOCKET_ADDRESS(g_inet_socket_address_new(g_inet_address_new_from_string("0.0.0.0") ,2000)),
	 	              TRUE,
	 	              &err);
	 	printf("dddd\n");
	 	g_assert(err == NULL);

	 	  fd = g_socket_get_fd(sock);
	 	 g_socket_set_blocking(sock, FALSE);
	 	g_socket_set_timeout(sock, 10);
	 	g_channel = g_io_channel_unix_new(fd);
	 	source = g_io_create_watch(g_channel, G_IO_IN | G_IO_ERR);
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


    Hashtbl_Cur_Rcv_Port = g_hash_table_new_full (g_str_hash , g_str_equal ,free_key,  free_value);
    Hashtbl_Cur_Snd_Port = g_hash_table_new_full (g_str_hash , g_str_equal ,free_key,  free_value);



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


 	loop = g_main_loop_new (NULL, FALSE);
 //   add_source(NULL);

 	//create_keep_alive_socket_rcv_source();
   pthread_create(&gst_rcv_tid, NULL, cmd_thread,NULL); //创建线程
   pthread_create(&keep_alive_tid, NULL, keep_alive_thread,NULL); //创建线程

//    while(1)
//    {
//    	usleep(100000);
//    }





  //g_source_set_callback (source,timeout_callback,loop,NULL);



	 g_main_loop_run (loop);


	g_main_loop_unref (loop);
//	remove_source();
	return 0;
}
