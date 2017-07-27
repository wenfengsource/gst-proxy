#include "udp_rcv.hh"


void udp_rcv( char *sipuri , int type)
{

}

void ~udp_rcv()
{

	   gst_element_set_state (pipeline, GST_STATE_NULL);

	   g_main_loop_unref (loop);

	   gst_object_unref (custom_src.pipeline);

}

void *threadFunction(GMainLoop *loop)
{
    printf("This is a thread");
    g_main_loop_run (loop);
}

 void create_pipeline()
{
    GstPad *sinkpad, *teepad;
    GstPadTemplate *templ;

	 gst_init (NULL,NULL);

	 custom_src.pipeline = gst_pipeline_new (NULL);

	 custom_src.udpsrc = gst_element_factory_make ("udpsrc", "udpsrc");

	 custom_src.tee = gst_element_factory_make ("tee", "tee");

	 vtl_dstsink.queue = gst_element_factory_make("queue","queue");


	  if (!custom_src.pipeline || !custom_src.udpsrc || !custom_src.tee) {
	    g_error ("Failed to create elements");
	   // return -1;
	  }


	  gst_bin_add_many (GST_BIN (pipeline), src, tee, NULL);
	   if (!gst_element_link_many (src, tee, NULL) )
	      {
	     g_error ("Failed to link elements");
	     //return -2;
	   }

	   //g_signal_connect (dbin, "pad-added", G_CALLBACK (pad_added_cb), NULL);

	   loop = g_main_loop_new (NULL, FALSE);

	   custom_src.bus = gst_pipeline_get_bus (GST_PIPELINE (custom_src.pipeline));
	   gst_bus_add_signal_watch (custom_src.bus);
	   g_signal_connect (G_OBJECT (custom_src.bus), "message", G_CALLBACK (message_cb), NULL);
	   gst_object_unref (GST_OBJECT (custom_src.bus));


	  templ =
	         gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (custom_src.tee),
	         "src_%u");
	     teepad = gst_element_request_pad (custom_src.tee, templ, NULL, NULL);
	     vtl_dstsink.queue = gst_element_factory_make ("queue", NULL);
	     vtl_dstsink.sink = gst_element_factory_make ("fakesink", "filesink");

	  //g_object_set (sink, "location", "./test.ts", NULL);

	   //  g_object_set (sink, "sync", TRUE, NULL);

	     gst_bin_add_many (GST_BIN (custom_src.pipeline), vtl_dstsink.queue, vtl_dstsink.sink, NULL);
	     gst_element_link_many (vtl_dstsink.queue, vtl_dstsink.sink, NULL);

	     sinkpad = gst_element_get_static_pad (queue, "sink");
	     gst_pad_link (teepad, sinkpad);
	     gst_object_unref (sinkpad);

	  //    g_timeout_add_seconds (3, tick_cb, NULL);


	//   GstPad *srcpad;
	//   srcpad = gst_element_get_static_pad (src, "src");

	 //gst_pad_add_probe (srcpad, GST_PAD_PROBE_TYPE_IDLE , cb_have_data, NULL, NULL);
	//   gst_object_unref (srcpad);

	   gst_element_set_state (custom_src.pipeline, GST_STATE_PLAYING);

	   pthread_t threadID;
	   pthread_create(&threadID, NULL, threadFunction, loop);

}

void  del_vtl_destsink()
{

}
