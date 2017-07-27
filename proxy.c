#include <glib.h>
#include <gst/gst.h>
#include <gst/gstbuffer.h>
//#include <gst/video/video.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include<stdio.h>
#include<pthread.h>

#define true  1
#define false 0
typedef struct  {
  GstElement *pipeline;
  GstElement *source;
  GstElement *depay;
  GstElement *tee;
  GstElement *queue;
  GstElement *sink;
  GstBus *bus;
  GstState current_state;
	
} gst_inputcapture;

gst_inputcapture gstdata_client;


/* This function is called when the pipeline changes states. We use it to
 * keep track of the current state. */
static void state_changed_cb (GstBus *bus, GstMessage *msg, gst_inputcapture *data)
{
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
    if (GST_MESSAGE_SRC (msg) == GST_OBJECT (data->pipeline))
    {
       //  data->current_state = new_state;
        g_print ("State set to %s\n", gst_element_state_get_name (new_state));
        if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED)
        {
          /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */

        }
    }
}

/* The appsink has received a buffer */
static  GstFlowReturn new_sample (GstElement *aaa, gst_inputcapture *data) {
  GstSample *sample ;
	printf("new sample --\n");
	//   GstMapInfo map;
	   //  GstBuffer *app_buffer, *buffer;
	       GstFlowReturn ret;


	    sample = gst_app_sink_pull_sample(GST_APP_SINK(aaa));
#if 0
	  //sample = gst_app_sink_pull_sample(sink);
  /* Retrieve the buffer */
//  g_signal_emit_by_name (sink, "pull-sample", &sample);
   if (sample) {
    /* The only thing we do in this example is print a * to indicate a received buffer */
    g_print ("*---\n");
    
     buffer = gst_sample_get_buffer (sample);
  
   /* make a copy */
 //   app_buffer = gst_buffer_copy (buffer);
   gst_buffer_map (buffer, &map, GST_MAP_READ);

 gst_buffer_unmap (buffer, &map);
 #endif
    gst_sample_unref (sample);
 // }
  
  return ret;

 
}


static GstFlowReturn
new_sample2 (GstElement * elt, gst_inputcapture * data)
{
static int i=0;
  GstSample *sample;
  GstBuffer *app_buffer, *buffer, *app_buffer2;
  GstElement *source;
 GstElement *source2;
  GstFlowReturn ret;
	printf("emit message \n");
  /* get the sample from appsink */
  sample = gst_app_sink_pull_sample (GST_APP_SINK (elt));

gst_sample_unref (sample);
  return ret;
}

static GstFlowReturn
on_new_sample_from_sink (GstElement * elt, gst_inputcapture * data)
{
static int i=0;
  GstSample *sample;
  GstBuffer *app_buffer, *buffer, *app_buffer2;
  GstElement *source;
 GstElement *source2;
  GstFlowReturn ret;
	printf("emit message \n");
  /* get the sample from appsink */
  sample = gst_app_sink_pull_sample (GST_APP_SINK (elt));
#if 0
  buffer = gst_sample_get_buffer (sample);

  /* make a copy */
  app_buffer = gst_buffer_copy (buffer);
 app_buffer2 = gst_buffer_copy (buffer);
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
 g_signal_emit_by_name (source2, "add", "192.168.128.153", 1025, NULL);
}
// ret = gst_app_src_push_buffer (GST_APP_SRC (source2), app_buffer2);
//  gst_object_unref (source);
// gst_object_unref (source2);
#endif
gst_sample_unref (sample);
  return ret;
}


static void cb_message (GstBus *bus, GstMessage *msg, gst_inputcapture *data) {

	printf("message = %d --- %s\n",
			GST_MESSAGE_TYPE (msg), GST_MESSAGE_TYPE_NAME(msg));

}


void  create_capture_pipeline(void )
{


	gstdata_client.pipeline = gst_pipeline_new ("rtsp-pipeline");
	gstdata_client.source = gst_element_factory_make ("udpsrc", "source");
	gstdata_client.depay = gst_element_factory_make ("rtpmp2tdepay", "rtpmp2tdepay");
	//gstdata_client.queue = gst_element_factory_make ("queue", "queue");
 
	gstdata_client.sink = gst_element_factory_make("appsink","clientsink");

	    if (!gstdata_client.pipeline || !gstdata_client.source  || !gstdata_client.sink )
	    {
	       g_printerr ("Not all elements could be created.\n");
	    }
	    gst_bin_add_many (GST_BIN (gstdata_client.pipeline), gstdata_client.source, /*gstdata_client.depay, gstdata_client.queue,*/ gstdata_client.sink,NULL);

	    if (!(gst_element_link (gstdata_client.source, /*,gstdata_client.depay ,gstdata_client.queue, */gstdata_client.sink)))

	    {
	        printf ("Elements could not be linked \n");
	    }


	    gstdata_client.bus = gst_element_get_bus (gstdata_client.pipeline);

	    /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
	    gst_bus_add_signal_watch (gstdata_client.bus);
	 //   g_signal_connect (G_OBJECT (gstdata_client.bus), "message::error", (GCallback)error_cb, gstdata.pipeline);
	 //   g_signal_connect (G_OBJECT (gstdata_client.bus), "message::eos", (GCallback)eos_cb, gstdata.pipeline);
	    g_signal_connect (G_OBJECT (gstdata_client.bus), "message::state-changed", (GCallback)state_changed_cb, &gstdata_client);
	    g_signal_connect (G_OBJECT (gstdata_client.bus), "message", (GCallback)cb_message, &gstdata_client);


	    // appsink property configuration
	 //   g_object_set (gstdata_client.sink, "drop", true, NULL);
	 //   g_object_set (gstdata_client.sink , "max-buffers", 1, NULL);
	 //   g_object_set (gstdata_client.sink , "max-lateness", 0, NULL);
	     g_object_set (gstdata_client.sink , "sync", false, NULL);
	 //   g_object_set (gstdata_client.sink , "render-delay", 0, NULL);
	    g_object_set (gstdata_client.sink , "emit-signals", true, NULL);

	    g_object_set (gstdata_client.source, "port", 60004, NULL);
 	// g_object_set (gstdata_client.sink, "location", "./dd.mts", NULL);
 //g_object_set (gstdata_client.source, "port",1024, NULL);
  g_signal_connect (gstdata_client.sink, "new-sample", G_CALLBACK (new_sample), NULL);
	 //  g_object_set (gstdata_client.source, "caps", gst_caps_new_simple("application/x-rtp, media=(string)video, clock-rate=(int)90000", NULL), NULL);
}



  
 
int  start_play_capture_data()
{
GstBus *bus;
GstMessage *msg;
  GstStateChangeReturn ret;
  gboolean terminate = FALSE;


	printf("start \n");
	//gst_element_set_state (gstdata_client.pipeline,GST_STATE_PLAYING);
  //  int ret ;
	ret = gst_element_set_state (gstdata_client.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    //gst_object_unref (data.playbin);
    return -1;
  }

  printf("success \n");
}




void stop_play_capture_data()
{
	gst_element_set_state (gstdata_client.pipeline,GST_STATE_NULL);
}

void* thread_loop(void *arg)
{
	char aa[10];
 	GMainLoop *main_loop;

	gst_init (NULL, NULL);
	create_capture_pipeline();
	//scanf("%s",aa);
    printf("start 1\n");
	start_play_capture_data();


	main_loop = g_main_loop_new (NULL, FALSE);
	// g_main_loop_run (main_loop);
	 
}

void main()
{
	pthread_t gst_rcv_tid;
   int err;  
    err = pthread_create(&gst_rcv_tid, NULL, thread_loop, NULL); //创建线程  
    if(err != 0){  
        printf("create thread error: %s/n",strerror(err));  
        return 1;  
    }  

    g_main_loop_run (main_loop);
    while(1)
    {
		usleep(10000); 
	}
}
