#include <string.h>
#include <gst/gst.h>

#include <sys/wait.h>
#include <sys/types.h>
#include "cmd_rcv.h"
#include <gio/gio.h>

#define UDP 1
#define RTP 2
#define TCP 3

//string类型key的比较函数
//gboolean g_str_equal (gconstpointer v1, gconstpointer v2)
//{
//const gchar *string1 = v1;
//const gchar *string2 = v2;
//
//return strcmp (string1, string2) == 0;
//}
//
////string类型key生成函数
//guint g_str_hash (gconstpointer v)
//{
///* 31 bit hash function */
//const signed char *p = v;
//guint32 h = *p;
//
//if (h)
//for (p += 1; *p != '\0'; p++)
//h = (h << 5) - h + *p;
//
//return h;
//}

void * free_value (gpointer data)
{
	printf ("freeing value: %s %p\n", (char *) data, data);
  g_free (data);
 // data = NULL;
 // printf ("freeing VALUE %p\n", data);
}

void * free_key (gpointer data)
{
  printf ("freeing key: %s %p\n", (char *) data, data);
  g_free (data);
  //data = NULL;
 // printf ("freeing KEY %p\n", data);
}

void * free_udp_rcv_port_key (gpointer data)
{
  printf ("free_udp_rcv_port_key: %s %p\n", (char *) data, data);
  g_free (data);
  //data = NULL;
 // printf ("freeing KEY %p\n", data);
}


void * print_port_value (gpointer data)
{
	 printf("port = %d \n",GPOINTER_TO_INT(data));
 // printf ("free_udp_rcv_port_key: %s %p\n", (char *) data, data);
  //data = NULL;
 // printf ("freeing KEY %p\n", data);
}

void * free_udp_snd_port_key (gpointer data)
{
  printf ("free_udp_snd_port_key: %s %p\n", (char *) data, data);
  g_free (data);
  //data = NULL;
 // printf ("freeing KEY %p\n", data);
}
void * free_tcp_rcv_port_key (gpointer data)
{
  printf ("free_tcp_rcv_port_key: %s %p\n", (char *) data, data);
  g_free (data);
  //data = NULL;
 // printf ("freeing KEY %p\n", data);
}

void * free_tcp_snd_port_key (gpointer data)
{
  printf ("free_tcp_snd_port_key: %s %p\n", (char *) data, data);
  g_free (data);
  //data = NULL;
 // printf ("freeing KEY %p\n", data);
}

void * free_sipuri_value (gpointer data)
{
	printf ("free_sipuri_value: %p\n", data);
	g_free (data);
 // data = NULL;
 // printf ("freeing VALUE %p\n", data);
}

void * free_sipuri_key (gpointer data)
{
  printf ("free_sipuri_key:%s  %p\n", (char *) data, data);
  g_free (data);
  //data = NULL;
 // printf ("freeing KEY %p\n", data);
}




void * free_sink_key (gpointer data)
{
	printf ("freeing sink key: %s %p\n", (char *) data, data);
  g_free (data);
 // data = NULL;
 // printf ("freeing VALUE %p\n", data);
}
void * free_sink_value (gpointer data)
{
	//Sink *sink= (Sink *)data;

  printf ("freeing sink value:  %p\n"   , data);

  g_free (data);
  //data = NULL;
 // printf ("freeing KEY %p\n", data);
}


void * free_tcpclient_key (gpointer data)
{
	printf ("freeing tcpclient key: %s %p\n", (char *) data, data);
  g_free (data);
 // data = NULL;
 // printf ("freeing VALUE %p\n", data);
}

void * free_tcpclient_value (gpointer data)
{
	//Sink *sink= (Sink *)data;

  printf ("free_tcpclient_value:  %p\n"   , data);

  g_free (data);
  //data = NULL;
 // printf ("freeing KEY %p\n", data);
}

