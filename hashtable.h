#ifndef HASHTABLE
#define HASHTABLE

//string类型key的比较函数
//extern gboolean g_str_equal (gconstpointer v1, gconstpointer v2);

//string类型key生成函数
//extern guint g_str_hash (gconstpointer v);


extern void * free_value (gpointer data);

extern void * free_key (gpointer data);

extern void * free_sink_key (gpointer data);
extern void * free_sink_value (gpointer data);

extern void * free_sipuri_key (gpointer data);
extern void * free_sipuri_value (gpointer data);

extern void * free_tcpclient_key (gpointer data);
extern void * free_tcpclient_value (gpointer data);

extern void * free_udp_rcv_port_key (gpointer data);
extern void * free_udp_snd_port_key (gpointer data);
extern void * free_tcp_rcv_port_key (gpointer data);
extern void * free_tcp_snd_port_key (gpointer data);

#endif
