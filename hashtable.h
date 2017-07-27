#ifndef HASHTABLE
#define HASHTABLE

//string类型key的比较函数
extern gboolean g_str_equal (gconstpointer v1, gconstpointer v2);

//string类型key生成函数
extern guint g_str_hash (gconstpointer v);


extern void * free_value (gpointer data);

extern void * free_key (gpointer data);


#endif
