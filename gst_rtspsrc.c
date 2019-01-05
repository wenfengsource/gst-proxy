#include <stdio.h> 
#include <unistd.h> 
#include <string.h> 
#include <gst/gst.h> 
#include <glib.h> 


GstElement *pipeline; 
GstElement *parser[5];
GstElement *depay[5];
GstElement *p_queue[5];

static gboolean 
bus_call (GstBus     *bus, 
          GstMessage *msg, 
          gpointer    data) 
{ 
  GMainLoop *loop = (GMainLoop *) data; 

//return ;
  switch (GST_MESSAGE_TYPE (msg)) { 

    case GST_MESSAGE_EOS: 
      g_print ("End of stream\n"); 
     // g_main_loop_quit (loop); 
      break; 

    case GST_MESSAGE_ERROR: { 
      gchar  *debug; 
      GError *error; 

      gst_message_parse_error (msg, &error, &debug); 
      g_free (debug); 

      g_printerr ("Error: %s\n", error->message); 
      g_error_free (error); 

     // g_main_loop_quit (loop); 
      break; 
    } 
/*    case GST_MESSAGE_ELEMENT: 
    { 
      const GstStructure *str; 

      str = gst_message_get_structure (msg); 
      const gchar *name = gst_structure_get_name (str); 

      printf("got message from %s\n", name); 
    } */ 
    default: 
      break; 
  } 

  return TRUE; 
} 


static void link_to_mux (GstPad *tolink_pad,  GstElement *mux)
{ 
  GstPad *pad; 
  gchar *srcname, *sinkname; 

  srcname = gst_pad_get_name (tolink_pad); 
  pad = gst_element_get_compatible_pad (mux, tolink_pad, NULL); 
  gst_pad_link (tolink_pad, pad); 
  sinkname = gst_pad_get_name (pad); 
  if (GST_PAD_IS_LINKED (pad)) g_print (" pad %s was linked to %s\n",
srcname, sinkname);
  else g_print("Link failed: %s to %s!\n",srcname, sinkname);
  gst_object_unref (GST_OBJECT (pad)); 

  
  g_free (sinkname); 
  g_free (srcname); 
  g_print(" Leaving link to mux\n");
} 


static GstPadProbeReturn
cb_probe (GstPad          *pad,
              GstPadProbeInfo *info,
              gpointer         user_data)
{
	printf("cb_probe === \n");
}


static GstPadProbeReturn
cb_have_data (GstPad          *pad,
              GstPadProbeInfo *info,
              gpointer         user_data)
{
  gint x, y;
  GstMapInfo map;
  guint16 *ptr, t;
  GstBuffer *buffer;


  buffer = GST_PAD_PROBE_INFO_BUFFER (info);

  buffer = gst_buffer_make_writable (buffer);

  gsize max,offset;
  gst_buffer_get_sizes(buffer,&offset,&max);

printf("pts = %lld  dts=%lld  offset =%lld  max=%lld \n"  ,GST_BUFFER_PTS(buffer),GST_BUFFER_DTS(buffer), offset, max) ;
#if 0
  /* Making a buffer writable can fail (for example if it
   * cannot be copied and is used more than once)
   */
  if (buffer == NULL)
    return GST_PAD_PROBE_OK;

  /* Mapping a buffer can fail (non-writable) */
  if (gst_buffer_map (buffer, &map, GST_MAP_WRITE)) {
    ptr = (guint16 *) map.data;
    /* invert data */
    for (y = 0; y < 288; y++) {
      for (x = 0; x < 384 / 2; x++) {
        t = ptr[384 - 1 - x];
        ptr[384 - 1 - x] = ptr[x];
        ptr[x] = t;
      }
      ptr += 384;
    }
    gst_buffer_unmap (buffer, &map);
  }

  GST_PAD_PROBE_INFO_DATA (info) = buffer;
#endif

  return GST_PAD_PROBE_OK;
}

static void on_pad_added (GstElement *element,
              GstPad     *pad, 
              gpointer    data) 
{ 
  GstElement *mux = (GstElement *) data; 
  GstCaps *caps; 
  gchar *caps_string; 
  gchar *name; 
  
  GstPad *sink_pad, *src_pad;
  printf("on_pad_added \n") ;
  name = gst_pad_get_name (pad); 
  printf("A new pad %s was created\n", name); 

  caps = gst_pad_query_caps (pad, NULL); 
  caps_string = gst_caps_to_string (caps); 
  g_print (" Capability:  %s\n", caps_string); 

  if ((strstr(caps_string, "video/mpeg")) && (!parser[0]))
  {
    parser[0] = gst_element_factory_make ("mpegvideoparse",  NULL);
    gst_bin_add (GST_BIN (pipeline), parser[0]);
    p_queue[0] = gst_element_factory_make ("queue",  NULL);
    gst_bin_add (GST_BIN (pipeline), p_queue[0]);

    gst_element_set_state (parser[0], GST_STATE_PLAYING);
    gst_element_set_state (p_queue[0], GST_STATE_PLAYING);

    gst_element_link (p_queue[0], parser[0]);

    sink_pad = gst_element_get_static_pad (p_queue[0], "sink");
    src_pad = gst_element_get_static_pad (parser[0], "src");

    gst_pad_link (pad, sink_pad);
    link_to_mux(src_pad, mux);
    gst_object_unref (GST_OBJECT (sink_pad));
    gst_object_unref (GST_OBJECT (src_pad));
  }
  else if ((strstr(caps_string, "video")) && (!parser[0]))
  {
    printf("h264 pad link \n");
    depay[0] = gst_element_factory_make ("rtph264depay",  NULL);
    parser[0] = gst_element_factory_make ("h264parse",  NULL);
    p_queue[0] = gst_element_factory_make ("queue",  NULL);
    gst_bin_add_many (GST_BIN (pipeline), depay[0], parser[0], p_queue[0], NULL);
   
   // gst_bin_add (GST_BIN (pipeline), p_queue[0]);

    gst_element_set_state (parser[0], GST_STATE_PLAYING);
    gst_element_set_state (p_queue[0], GST_STATE_PLAYING);
   gst_element_set_state (depay[0], GST_STATE_PLAYING);
    gst_element_link_many (depay[0],p_queue[0], parser[0],NULL);

    sink_pad = gst_element_get_static_pad (depay[0], "sink");
    src_pad = gst_element_get_static_pad (parser[0], "src");

//gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BUFFER,
 //     (GstPadProbeCallback) cb_have_data, NULL, NULL);

GstPad *sinkpad;

sinkpad = gst_element_get_static_pad (parser[0], "src");

gst_pad_add_probe (sinkpad, GST_PAD_PROBE_TYPE_PULL,
      (GstPadProbeCallback) cb_probe, NULL, NULL);

    gst_pad_link (pad, sink_pad);
    link_to_mux(src_pad, mux);
    gst_object_unref (GST_OBJECT (sink_pad));
    gst_object_unref (GST_OBJECT (src_pad));
  }

  else if ((strstr(caps_string, "MPEG4-GENERIC")) && (!parser[1]))
  { 
   printf("aac link \n") ;
   depay[1] = gst_element_factory_make ("rtpmp4gdepay",  NULL);
    parser[1] = gst_element_factory_make ("aacparse",  NULL);
    p_queue[1] = gst_element_factory_make ("queue",  NULL);
    gst_bin_add_many (GST_BIN (pipeline), depay[1], parser[1], p_queue[1], NULL);
   
   // gst_bin_add (GST_BIN (pipeline), p_queue[0]);

    gst_element_set_state (parser[1], GST_STATE_PLAYING);
    gst_element_set_state (p_queue[1], GST_STATE_PLAYING);
   gst_element_set_state (depay[1], GST_STATE_PLAYING);
    gst_element_link_many (depay[1],p_queue[1], parser[1],NULL);

    sink_pad = gst_element_get_static_pad (depay[1], "sink");
    src_pad = gst_element_get_static_pad (parser[1], "src");

    gst_pad_link (pad, sink_pad);
    link_to_mux(src_pad, mux);
    gst_object_unref (GST_OBJECT (sink_pad));
    gst_object_unref (GST_OBJECT (src_pad));
  } 
#if 0
  else if ((strstr(caps_string, "audio/x-ac3")) && (!parser[2]))
  { 
    parser[2] = gst_element_factory_make ("ac3parse",  NULL);
    gst_bin_add (GST_BIN (pipeline), parser[2]);
    p_queue[2] = gst_element_factory_make ("queue",  NULL);
    gst_bin_add (GST_BIN (pipeline), p_queue[2]);

    gst_element_set_state (parser[2], GST_STATE_PLAYING);
    gst_element_set_state (p_queue[2], GST_STATE_PLAYING);

    gst_element_link (p_queue[2], parser[2]);

    sink_pad = gst_element_get_static_pad (p_queue[2], "sink");
    src_pad = gst_element_get_static_pad (parser[2], "src");

    gst_pad_link (pad, sink_pad);
    link_to_mux(src_pad, mux);
    gst_object_unref (GST_OBJECT (sink_pad));
    gst_object_unref (GST_OBJECT (src_pad));
  } 

#endif
  gst_caps_unref (caps); 
  g_free (name); 
  g_free (caps_string);   
  g_print("Leaving Pad-Adder\n");
} 


void no_more_pads (GstElement* object, gpointer user_data) 
{ 
  g_print("No more pads event from %s\n", gst_element_get_name(object)); 
} 

void on_pad_removed (GstElement* object, GstPad* pad, gpointer user_data)
{

  g_print("pad %s was removed\n", gst_pad_get_name (pad));
}

int 
main () 
{ 
  GMainLoop *loop; 

  GstElement *source, *queue2, *demux, *mux, *queue, *sink;
  GstBus *bus; 

  gst_init (0, NULL); 

  loop = g_main_loop_new (NULL, FALSE); 

  pipeline = gst_pipeline_new ("DVB-Filesave"); 
  source   = gst_element_factory_make ("rtspsrc",  "dvb-source"); 
  //queue2   = gst_element_factory_make ("queue2",      "dvb-queue2");
 // demux    = gst_element_factory_make ("tsdemux",     "dvb-demux"); 
  mux	  = gst_element_factory_make ("mpegtsmux",   "dvb-mux"); 
  queue    = gst_element_factory_make ("queue",       "dvb-queue");
  sink     = gst_element_factory_make ("filesink",    "file-output");

  if (!pipeline || !source || !mux || !queue || !sink) {
    g_printerr ("One element could not be created. Exiting.\n"); 
    if(!pipeline) g_printerr("Pipeline not created\n"); 
    else if(!source) g_printerr("Source not created\n"); 
    else if(!demux) g_printerr("Demux not created\n"); 
    else if(!mux) g_printerr("Muxer not created\n"); 
    else if(!queue) g_printerr("Queue not created\n");
    else if(!sink) g_printerr("Sink not created\n"); 
    return -1; 
  } 

//  g_object_set (G_OBJECT (source), "adapter", 3, NULL);
//  g_object_set (G_OBJECT (source), "frequency", 12544000, NULL);
//  g_object_set (G_OBJECT (source), "program-numbers", "17501", NULL);
//  g_object_set (G_OBJECT (source), "polarity", "h", NULL);
//  g_object_set (G_OBJECT (source), "symbol-rate", 22000, NULL);

 // g_object_set (G_OBJECT (source), "async-handling", TRUE, NULL);
  g_object_set (G_OBJECT (source), "location", "rtsp://127.0.0.1:8554/stream0","protocols", 0x00000004,NULL);
  g_object_set (G_OBJECT (sink), "location", "remux_test.ts", NULL); 
  g_object_set (G_OBJECT (sink), "sync", FALSE, NULL);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline)); 
  gst_bus_add_watch (bus, bus_call, loop); 
  gst_object_unref (bus); 

  gst_bin_add_many (GST_BIN (pipeline), source, mux, queue,
sink, NULL);

  //gst_element_link_many (source, queue2, demux, NULL);
  gst_element_link_many (mux, queue, sink, NULL);

  g_signal_connect (source, "pad-added", G_CALLBACK (on_pad_added), mux); 
  g_signal_connect (source, "no-more-pads", G_CALLBACK (no_more_pads), NULL);
  g_signal_connect (source, "pad-removed", G_CALLBACK (on_pad_removed),
NULL);


  g_print ("Now playing: "); 
  gst_element_set_state (pipeline, GST_STATE_PLAYING); 

  g_print ("Running...\n"); 
  g_main_loop_run (loop); 

  return 0; 
}
