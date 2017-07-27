#include <string.h>
#include <gst/gst.h>

#include <sys/wait.h>
#include <sys/types.h>
#include "cmd_rcv.h"
#include <gio/gio.h>

#define LOCAL_IP   "192.168.168.152"
typedef struct
{
	gint source_id;  //check headbeat
	GIOChannel* io_channel;
}Heatbeat;


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
  Heatbeat  rcv_beatheart;
  int snd_port;
} Sink;

typedef struct
{
	GstElement *src;
	GstElement *tee;

    char ip[20];
    int port;
    int snd_beatheart;
}Source;


typedef struct
{
	Sink sink;
	Source source;
    GstElement *pipeline;
	GstBus *bus;
	char sipui[50];
} GstCustom;

typedef struct
{
	GstCustom  *GstCusom;
	Sink *sink;

} userpoint;

GMainLoop* loop;

GstCustom gstdata;
GHashTable *ghashtbale;

static gboolean
message_cb (GstBus * bus, GstMessage * message, gpointer user_data)
{

	const GstStructure *st = gst_message_get_structure (message);

  switch (GST_MESSAGE_TYPE (message)) {
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

      g_main_loop_quit (loop);
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


      /* We don't care for messages other than timeouts */
      if (!gst_structure_has_name (st, "GstUDPSrcTimeout"))
    break;
    g_print ("Timeout received from udpsrc \n");
 //   gst_element_set_state(GST_ELEMENT (pipeline),GST_STATE_NULL));
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


 int Create_udptoudp_Pipeline( GstCustom *gstcustom , char *inputuri, char*outputuri ,int bindport)
{
	 gstcustom->pipeline = gst_pipeline_new (NULL);

	 gstcustom->source.src = gst_element_factory_make ("udpsrc", "udpsrc");
	 gstcustom->source.tee = gst_element_factory_make ("tee", "tee");

	if (!gstcustom->pipeline || !gstcustom->source.src || !gstcustom->source.tee) {
		g_error ("Failed to create elements");
		return -1;
	}


	//g_object_set (src, "caps", gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING,"video","payload",G_TYPE_INT,33, NULL), NULL);


	 gst_bin_add_many (GST_BIN (gstcustom->pipeline), gstcustom->source.src, gstcustom->source.tee, gstcustom);
	  if (!gst_element_link_many (gstcustom->source.src, gstcustom->source.tee, NULL) )
	 {
		  g_error ("Failed to link elements");
		  return -2;
	 }

	  //g_signal_connect (dbin, "pad-added", G_CALLBACK (pad_added_cb), NULL);

	  //loop = g_main_loop_new (NULL, FALSE);

	  g_object_set (gstcustom->source.src, "timeout",5000000000);
	  g_object_set (gstcustom->source.src, "uri",inputuri, NULL);

	  gstcustom->bus = gst_pipeline_get_bus (GST_PIPELINE (gstcustom->pipeline));
	  gst_bus_add_signal_watch (gstcustom->bus);
	  g_signal_connect (G_OBJECT (gstcustom->bus), "message", G_CALLBACK (message_cb), NULL);
	  gst_object_unref (GST_OBJECT (gstcustom->bus));


	  GstPadTemplate *templ;

	 templ =
	        gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (gstcustom->source.tee),
	        "src_%u");
	    gstcustom->sink.teepad = gst_element_request_pad (gstcustom->source.tee, templ, NULL, NULL);
	    gstcustom->sink.queue = gst_element_factory_make ("queue", NULL);
	    gstcustom->sink.sink = gst_element_factory_make ("multiudpsink", "multiudpsink");
	    g_object_set (gstcustom->sink.sink, "clients", outputuri, NULL);
	   // g_object_set (gstcustom->sink.sink, "clients", outputuri, "bind-address","LOCAL_IP","bind-port",bindport, NULL);
	 //   gst_object_unref(templ);
	//    port += 1;
	//    g_signal_emit_by_name (sink, "add", "192.168.128.152", port, NULL);

	    //g_object_set (sink, "sync", TRUE, NULL);

	    gst_bin_add_many (GST_BIN (gstcustom->pipeline),gstcustom->sink.queue, gstcustom->sink.sink, NULL);
	    gst_element_link_many (gstcustom->sink.queue, gstcustom->sink.sink, NULL);

	    GstPad *sinkpad;

	    sinkpad = gst_element_get_static_pad (gstcustom->sink.queue, "sink");
	    gst_pad_link (gstcustom->sink.teepad, sinkpad);
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
	unsigned char rx_buf[80];
	unsigned char tx_buf[80];

	GstCustom gstdata;
	printf("cmd thread created \n");
	//gRcvSocket = rcv_socket_init();

	signal(SIGINT, Stop);

	while(1)
	{
		printf("receive data \n");
		memset(rx_buf, 0 ,80);
		//receive_packet(rx_buf);
        sleep(1);
		switch(rx_buf[1])
		{
		case 1:

		 	break;
		default:
			break;
		}
	}
	printf("exit \n");


}

 void *new_pipeline_thread( void *arg)
 {
//	 loop = g_main_loop_new (NULL, FALSE);
//	 Create_udptoudp_Pipeline( GstCustom *gstcustom , char *inputuri, char*outputuri);
//	 g_main_loop_run (loop);
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
     g_io_channel_read_chars(channel,buf, 100,&len, error);
    // g_io_channel_read (channel,buf, 100,&len);//, error);

     printf("str %s  len %d error= %d\n", buf, len ,error);
    // cmd= *(short*)(&buf[0]);

     printf("cmd %d \n", cmd);

     switch(buf[0])
     {
		 case 10:

			 switch(buf[1])
			 {
			 	 case 1:
			 		 printf("create pipeline \n");
			 		g_hash_table_insert (ghashtbale, strdup("sipui"), &gstdata);
			 		printf("add = %d \n", &gstdata);
			 		Create_udptoudp_Pipeline(&gstdata , "udp://0.0.0.0:1028", "192.168.128.151:1030",1024);
				 break;

				 case 2:

					 tmp = g_hash_table_lookup (ghashtbale, strdup("sipui"));
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
	 GSocket *sock;
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

	 	int fd = g_socket_get_fd(sock);
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
	  GMainContext *context;

	//GstCustom gstdata;

	//create a new time-out source
	 //   source = g_timeout_source_new(1000);

	pthread_t gst_rcv_tid;
	//int err;
	gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    ghashtbale = g_hash_table_new (g_str_hash , g_str_equal);

    add_source(NULL);
	//  pthread_create(&gst_rcv_tid, NULL, cmd_thread, &gstdata); //创建线程

//	loop = g_main_loop_new (NULL, FALSE);


  //g_source_set_callback (source,timeout_callback,loop,NULL);



	g_main_loop_run (loop);


	g_main_loop_unref (loop);
//	remove_source();
	return 0;
}
