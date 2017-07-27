#ifndef UDP_REC
#define UDP_REC

#include <string.h>
#include <gst/gst.h>

#define TOTAL_DISTRIBUTION    10

typedef struct
{
  GstPad *teepad;
  GstElement *queue;
  GstElement *depay;
  GstElement *sink;
  gboolean removing;
  int dst_port;
  char *dst_ip;
} DstSink;


typedef struct
{
  GstElement *udpsrc;
  GstElement *tee;
  GstElement *pipeline;
  GstBus     *bus;
} Src;

typedef struct
{
	DstSink dstsink;
	int usedflag;
} cmbsink;

class udp_rcv
{
public:

	void udp_rcv( char *sipuri , int type);
	void create_pipeline();
	virtual ~udp_rcv();
	void del_vtl_destsink();

private:
	DstSink vtl_dstsink;
	int udprcv_port;
	char* udprcv_add;
	Src custom_src;
	cmbsink  cmbsink[TOTAL_DISTRIBUTION];
	char sipuri[50];
	static GMainLoop *loop;

};

#endif
