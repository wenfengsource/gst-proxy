#include <stdio.h> 
#include <unistd.h> 
#include <string.h> 
#include <gst/gst.h> 
#include <glib.h> 


GstElement *pipeline; 
GstElement *parser[5];
GstElement *p_queue[5];

static gboolean 
bus_call (GstBus     *bus, 
          GstMessage *msg, 
          gpointer    data) 
{ 
  GMainLoop *loop = (GMainLoop *) data; 

  switch (GST_MESSAGE_TYPE (msg)) { 

    case GST_MESSAGE_EOS: 
      g_print ("End of stream\n"); 
      g_main_loop_quit (loop); 
      break; 

    case GST_MESSAGE_ERROR: { 
      gchar  *debug; 
      GError *error; 

      gst_message_parse_error (msg, &error, &debug); 
      g_free (debug); 

      g_printerr ("Error: %s\n", error->message); 
      g_error_free (error); 

      g_main_loop_quit (loop); 
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

static void on_pad_added (GstElement *element,
              GstPad     *pad, 
              gpointer    data) 
{ 
  GstElement *mux = (GstElement *) data; 
  GstCaps *caps; 
  gchar *caps_string; 
  gchar *name; 
  
  GstPad *sink_pad, *src_pad;

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
  else if ((strstr(caps_string, "video/x-h264")) && (!parser[0]))
  {
    parser[0] = gst_element_factory_make ("h264parse",  NULL);
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
  else if ((strstr(caps_string, "audio/mpeg")) && (!parser[1]))
  { 
    parser[1] = gst_element_factory_make ("mpegaudioparse",  NULL);
    gst_bin_add (GST_BIN (pipeline), parser[1]);
    p_queue[1] = gst_element_factory_make ("queue",  NULL);
    gst_bin_add (GST_BIN (pipeline), p_queue[1]);

    gst_element_set_state (parser[1], GST_STATE_PLAYING);
    gst_element_set_state (p_queue[1], GST_STATE_PLAYING);

    gst_element_link (p_queue[1], parser[1]);

    sink_pad = gst_element_get_static_pad (p_queue[1], "sink");
    src_pad = gst_element_get_static_pad (parser[1], "src");

    gst_pad_link (pad, sink_pad);
    link_to_mux(src_pad, mux);
    gst_object_unref (GST_OBJECT (sink_pad));
    gst_object_unref (GST_OBJECT (src_pad));
  } 
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
  source   = gst_element_factory_make ("filesrc",  "dvb-source"); 
  queue2   = gst_element_factory_make ("queue2",      "dvb-queue2");
  demux    = gst_element_factory_make ("tsdemux",     "dvb-demux"); 
  mux	  = gst_element_factory_make ("mpegtsmux",   "dvb-mux"); 
  queue    = gst_element_factory_make ("queue",       "dvb-queue");
  sink     = gst_element_factory_make ("filesink",    "file-output");

  if (!pipeline || !source || !demux || !mux || !queue || !sink) {
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
  g_object_set (G_OBJECT (source), "location", "./test.ts", NULL);
  g_object_set (G_OBJECT (sink), "location", "remux_test.mpg", NULL); 
  g_object_set (G_OBJECT (sink), "sync", FALSE, NULL);

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline)); 
  gst_bus_add_watch (bus, bus_call, loop); 
  gst_object_unref (bus); 

  gst_bin_add_many (GST_BIN (pipeline), source, queue2, demux, mux, queue,
sink, NULL);

  gst_element_link_many (source, queue2, demux, NULL);
  gst_element_link_many (mux, queue, sink, NULL);

  g_signal_connect (demux, "pad-added", G_CALLBACK (on_pad_added), mux); 
  g_signal_connect (demux, "no-more-pads", G_CALLBACK (no_more_pads), NULL);
  g_signal_connect (demux, "pad-removed", G_CALLBACK (on_pad_removed),
NULL);


  g_print ("Now playing: "); 
  gst_element_set_state (pipeline, GST_STATE_PLAYING); 

  g_print ("Running...\n"); 
  g_main_loop_run (loop); 

  return 0; 
}
