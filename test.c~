/* GStreamer
 *
 * appsink-src.c: example for using appsink and appsrc.
 *
 * Copyright (C) 2008 Wim Taymans <wim.taymans@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <gst/gst.h>

#include <string.h>

#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

/* these are the caps we are going to pass through the appsink and appsrc */
const gchar *audio_caps =
    "audio/x-raw,format=S16LE,channels=1,rate=8000, layout=interleaved";

typedef struct
{
  GMainLoop *loop;
  GstElement *source;
GstElement *sink2;
  GstElement *sink;
} ProgramData;

/* called when the appsink notifies us that there is a new buffer ready for
 * processing */
static GstFlowReturn
on_new_sample_from_sink (GstElement * elt, ProgramData * data)
{
static int i=0;
  GstSample *sample;
  GstBuffer *app_buffer, *buffer, *app_buffer2;
  GstElement *source;
 GstElement *source2;
  GstFlowReturn ret;
//	printf("emit message \n");
  /* get the sample from appsink */
  sample = gst_app_sink_pull_sample (GST_APP_SINK (elt));

  buffer = gst_sample_get_buffer (sample);

  /* make a copy */
  app_buffer = gst_buffer_copy (buffer);
 //app_buffer2 = gst_buffer_copy (buffer);
  /* we don't need the appsink sample anymore */
  

//  GstMapInfo map;
//  gst_buffer_map (buffer, &map, GST_MAP_READ);
//  printf("size = %d \n", map.size);
// gst_buffer_unmap (buffer, &map);
  /* get source an push new buffer */
  source = gst_bin_get_by_name (GST_BIN (data->sink), "testsource");
  source2 = gst_bin_get_by_name (GST_BIN (data->sink), "multiudpsink");
  ret = gst_app_src_push_buffer (GST_APP_SRC (source), app_buffer);

if(i==0)
{
 i =1;
 g_signal_emit_by_name (source2, "add", "192.168.128.153", 1024, NULL);
g_signal_emit_by_name (source2, "add", "192.168.128.152", 1026, NULL);
g_signal_emit_by_name (source2, "add", "192.168.128.154", 1028, NULL);
g_signal_emit_by_name (source2, "add", "192.168.128.154", 1030, NULL);
g_signal_emit_by_name (source2, "add", "192.168.128.154", 1032, NULL);
g_signal_emit_by_name (source2, "add", "192.168.128.154", 1034, NULL);
g_signal_emit_by_name (source2, "add", "192.168.128.154", 1036, NULL);
g_signal_emit_by_name (source2, "add", "192.168.128.154", 1038, NULL);
g_signal_emit_by_name (source2, "add", "192.168.128.154", 1040, NULL);
g_signal_emit_by_name (source2, "add", "192.168.128.154", 1044, NULL);
}
// ret = gst_app_src_push_buffer (GST_APP_SRC (source2), app_buffer2);
  gst_object_unref (source);
 gst_object_unref (source2);

gst_sample_unref (sample);
  return ret;
}

/* called when we get a GstMessage from the source pipeline when we get EOS, we
 * notify the appsrc of it. */
static gboolean
on_source_message (GstBus * bus, GstMessage * message, ProgramData * data)
{
  GstElement *source;
  	printf("message = %d --- %s\n",
			GST_MESSAGE_TYPE (message), GST_MESSAGE_TYPE_NAME(message));


  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_EOS:
      g_print ("The source got dry\n");
      source = gst_bin_get_by_name (GST_BIN (data->sink), "testsource");
      gst_app_src_end_of_stream (GST_APP_SRC (source));
      gst_object_unref (source);
      break;
    case GST_MESSAGE_ERROR:
      g_print ("Received error\n");
      g_main_loop_quit (data->loop);
      break;
    default:
      break;
  }
  return TRUE;
}

/* called when we get a GstMessage from the sink pipeline when we get EOS, we
 * exit the mainloop and this testapp. */
static gboolean
on_sink_message (GstBus * bus, GstMessage * message, ProgramData * data)
{
  /* nil */
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_EOS:
      g_print ("Finished playback\n");
      g_main_loop_quit (data->loop);
      break;
    case GST_MESSAGE_ERROR:
      g_print ("Received error\n");
      g_main_loop_quit (data->loop);
      break;
    default:
      break;
  }
  return TRUE;
}


void cb_udp_client_add (GstElement* object,
                                        gchararray arg0,
                                        gint arg1,
                                        gpointer user_data)
{
printf("arg0 = %s \n", arg0);
printf("arg1 = %d \n", arg1);
}


/* This function is called when the pipeline changes states. We use it to
 * keep track of the current state. */
static void state_changed_cb (GstBus *bus, GstMessage *msg, ProgramData *data)
{
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
    if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->source))
    {
       //  data->current_state = new_state;
        g_print ("State set to %s\n", gst_element_state_get_name (new_state));
        if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED)
        {
          /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */

        }
    }
}


int
main (int argc, char *argv[])
{
  gchar *filename = NULL;
  ProgramData *data = NULL;
  gchar *string = NULL;
  GstBus *bus = NULL;
  GstElement *testsink = NULL;
  GstElement *testsource = NULL;
 GstElement *testsource2 = NULL;
  gst_init (&argc, &argv);

  

  data = g_new0 (ProgramData, 1);

  data->loop = g_main_loop_new (NULL, FALSE);

  /* setting up source pipeline, we read from a file and convert to our desired
   * caps. */
  string =
      g_strdup_printf
      ("gst-launch-1.0 -v udpsrc uri=udp://0.0.0.0:1234 ! appsink name=testsink");
  //g_free (filename);

  data->source = gst_parse_launch (string, NULL);
  g_free (string);

  if (data->source == NULL) {
    g_print ("Bad source\n");
    return -1;
  }

  /* to be notified of messages from this pipeline, mostly EOS */
  bus = gst_element_get_bus (data->source);
  gst_bus_add_watch (bus, (GstBusFunc) on_source_message, data);

  g_signal_connect (G_OBJECT (bus), "message::state-changed", (GCallback)state_changed_cb, data);

  gst_object_unref (bus);

  /* we use appsink in push mode, it sends us a signal when data is available
   * and we pull out the data in the signal callback. We want the appsink to
   * push as fast as it can, hence the sync=false */
  testsink = gst_bin_get_by_name (GST_BIN (data->source), "testsink");
  g_object_set (G_OBJECT (testsink), "emit-signals", TRUE, "sync", FALSE, NULL);
  g_signal_connect (testsink, "new-sample",
      G_CALLBACK (on_new_sample_from_sink), data);
  gst_object_unref (testsink);

  /* setting up sink pipeline, we push audio data into this pipeline that will
   * then play it back using the default audio sink. We have no blocking
   * behaviour on the src which means that we will push the entire file into
   * memory. */
  string =
      g_strdup_printf ("appsrc name=testsource ! multiudpsink bind-port=1048 name=multiudpsink clients=192.168.128.151:1048");
  data->sink = gst_parse_launch (string, NULL);
  g_free (string);

  if (data->sink == NULL) {
    g_print ("Bad sink\n");
    return -1;
  }



  testsource = gst_bin_get_by_name (GST_BIN (data->sink), "testsource");
  /* configure for time-based format */
  g_object_set (testsource, "format", GST_FORMAT_TIME, NULL);
  /* uncomment the next line to block when appsrc has buffered enough */
  /* g_object_set (testsource, "block", TRUE, NULL); */
  gst_object_unref (testsource);


  testsource = gst_bin_get_by_name (GST_BIN (data->sink), "multiudpsink");
   
  g_signal_connect (testsource, "client-added",
      G_CALLBACK (cb_udp_client_add), data);

  gst_object_unref (testsource);


 
  bus = gst_element_get_bus (data->sink);
  gst_bus_add_watch (bus, (GstBusFunc) on_sink_message, data);
  gst_object_unref (bus);

  /* launching things */
  gst_element_set_state (data->sink, GST_STATE_PLAYING);
  gst_element_set_state (data->sink2, GST_STATE_PLAYING);
  gst_element_set_state (data->source, GST_STATE_PLAYING);

  /* let's run !, this loop will quit when the sink pipeline goes EOS or when an
   * error occurs in the source or sink pipelines. */
  g_print ("Let's run!\n");
  g_main_loop_run (data->loop);
  g_print ("Going out\n");

  gst_element_set_state (data->source, GST_STATE_NULL);
  gst_element_set_state (data->sink, GST_STATE_NULL);

  gst_object_unref (data->source);
  gst_object_unref (data->sink);
  g_main_loop_unref (data->loop);
  g_free (data);

  return 0;
}
