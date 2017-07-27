#include <string.h>
#include <gst/gst.h>

static GMainLoop *loop;
static GstElement *pipeline;
static GstElement *src, *dbin, *conv, *tee;
static gboolean linked = FALSE;
static GList *sinks;

typedef struct
{
  GstPad *teepad;
  GstElement *queue;
  GstElement *conv;
 GstElement *depay;
  GstElement *sink;
  gboolean removing;
} Sink;

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
     g_print ("Got timeout\n");
     
      /* We don't care for messages other than timeouts */
      if (!gst_structure_has_name (st, "GstUDPSrcTimeout"))
    break;
    g_print ("Timeout received from udpsrc");
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

static GstPadProbeReturn
unlink_cb (GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
  Sink *sink = user_data;
  GstPad *sinkpad;

  if (!g_atomic_int_compare_and_exchange (&sink->removing, FALSE, TRUE))
    return GST_PAD_PROBE_OK;

  sinkpad = gst_element_get_static_pad (sink->queue, "sink");
  gst_pad_unlink (sink->teepad, sinkpad);
  gst_object_unref (sinkpad);

  gst_bin_remove (GST_BIN (pipeline), sink->queue);
 
  gst_bin_remove (GST_BIN (pipeline), sink->sink);

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


static gboolean
tick_cb (gpointer data)
{
	  static int port=21000;
  static int flag=1;
  
  if (!sinks  || flag++ < 15) {
    Sink *sink = g_new0 (Sink, 1);
    GstPad *sinkpad;
    GstPadTemplate *templ;

    templ =
        gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (tee),
        "src_%u");

    g_print ("add\n");


    sink->teepad = gst_element_request_pad (tee, templ, NULL, NULL);

    sink->queue = gst_element_factory_make ("queue", NULL);
	//sink->jitter = gst_element_factory_make ("rtpjitter", NULL);
 	sink->depay = gst_element_factory_make ("rtpmp2tdepay", NULL);

  //  sink->conv = gst_element_factory_make ("videoconvert", NULL);
    sink->sink = gst_element_factory_make ("udpsink", NULL);
    
    

      g_print ("port = %d \n", port);
	static int j = 0;   
     if(j ==0)
 		{
 			j= 1;
		 g_object_set (sink->sink , "host", "192.168.128.151","port", port, NULL);
		}
   else
		g_object_set (sink->sink , "host", "192.168.128.161","port", port, NULL);

     port+=1;
	 
    sink->removing = FALSE;

    gst_bin_add_many (GST_BIN (pipeline), gst_object_ref (sink->queue), 
        gst_object_ref(sink->depay), gst_object_ref (sink->sink), NULL);
    gst_element_link_many (sink->queue, sink->depay, sink->sink, NULL);

    gst_element_sync_state_with_parent (sink->queue);
     gst_element_sync_state_with_parent (sink->depay);
    gst_element_sync_state_with_parent (sink->sink);

    sinkpad = gst_element_get_static_pad (sink->queue, "sink");
    gst_pad_link (sink->teepad, sinkpad);
    gst_object_unref (sinkpad);

    g_print ("added\n");

    sinks = g_list_append (sinks, sink);
  } else {
    Sink *sink;

    g_print ("remove\n");

    sink = sinks->data;
   // sinks = g_list_delete_link (sinks, sinks);
    
  // static int i =0;
 //  if(i ==0)
 //  {
   // gst_pad_add_probe (sink->teepad, GST_PAD_PROBE_TYPE_IDLE, unlink_cb, sink,
    //    (GDestroyNotify) g_free);
        
	//}
  }

  return TRUE;
}

static void
pad_added_cb (GstElement * element, GstPad * pad, gpointer user_data)
{
  GstCaps *caps;
  GstStructure *s;
  const gchar *name;

  if (linked)
    return;



    GstPad *sinkpad, *teepad;
    GstElement *queue, *sink;
    GstPadTemplate *templ;



  caps = gst_pad_get_current_caps (pad);
  s = gst_caps_get_structure (caps, 0);
  name = gst_structure_get_name (s);
  printf("name %s\n", name );
/*
  if (strcmp (name, "video/x-raw") == 0) {
    GstPad *sinkpad, *teepad;
    GstElement *queue, *sink;
    GstPadTemplate *templ;

    sinkpad = gst_element_get_static_pad (conv, "sink");
    if (gst_pad_link (pad, sinkpad) != GST_PAD_LINK_OK) {
      g_printerr ("Failed to link dbin with conv\n");
      gst_object_unref (sinkpad);
      g_main_loop_quit (loop);
      return;
    }
    gst_object_unref (sinkpad);
*/
   
 // }

  gst_caps_unref (caps);
}

int
main (int argc, char **argv)
{
  GstBus *bus;


    GstPad *sinkpad, *teepad;
    GstElement *queue, *sink;
    GstPadTemplate *templ;


 // if (argc != 2) {
   // g_error ("Usage: %s filename", argv[0]);
 //   return 0;
 // }

  gst_init (&argc, &argv);

  pipeline = gst_pipeline_new (NULL);

  src = gst_element_factory_make ("udpsrc", "udpsrc");
//  dbin = gst_element_factory_make ("decodebin", NULL);
//  conv = gst_element_factory_make ("videoconvert", NULL);
  tee = gst_element_factory_make ("tee", NULL);

  if (!pipeline || !src || !tee) {
    g_error ("Failed to create elements");
    return -1;
  }

  g_object_set (src, "timeout",2000000000);
 g_object_set (src, "uri", "udp://0.0.0.0:60004");
g_object_set (src, "caps", gst_caps_new_simple("application/x-rtp", "media", G_TYPE_STRING,"video","payload",G_TYPE_INT,33, NULL), NULL);


  gst_bin_add_many (GST_BIN (pipeline), src, tee, NULL);
  if (!gst_element_link_many (src, tee, NULL) )
     {
    g_error ("Failed to link elements");
    return -2;
  }

  //g_signal_connect (dbin, "pad-added", G_CALLBACK (pad_added_cb), NULL);

  loop = g_main_loop_new (NULL, FALSE);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_signal_watch (bus);
  g_signal_connect (G_OBJECT (bus), "message", G_CALLBACK (message_cb), NULL);
  gst_object_unref (GST_OBJECT (bus));


 templ =
        gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (tee),
        "src_%u");
    teepad = gst_element_request_pad (tee, templ, NULL, NULL);
    queue = gst_element_factory_make ("queue", NULL);
    sink = gst_element_factory_make ("fakesink", "filesink");

 //g_object_set (sink, "location", "./test.ts", NULL);
  
    g_object_set (sink, "sync", TRUE, NULL);

    gst_bin_add_many (GST_BIN (pipeline), queue, sink, NULL);
    gst_element_link_many (queue, sink, NULL);

    sinkpad = gst_element_get_static_pad (queue, "sink");
    gst_pad_link (teepad, sinkpad);
    gst_object_unref (sinkpad);

     g_timeout_add_seconds (3, tick_cb, NULL);
    linked = TRUE;

    GstPad *srcpad;
  srcpad = gst_element_get_static_pad (src, "src");

//gst_pad_add_probe (srcpad, GST_PAD_PROBE_TYPE_IDLE , cb_have_data, NULL, NULL);
  gst_object_unref (srcpad);

  
  

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  g_main_loop_run (loop);

  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_main_loop_unref (loop);

  gst_object_unref (pipeline);

  return 0;
}
