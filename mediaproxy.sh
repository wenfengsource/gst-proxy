#!/bin/sh  


export LD_LIBRARY_PATH=/opt/gstreamer/out/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=/opt/gstreamer/out/lib/pkgconfig:$PKG_CONFIG_PATH
export GST_PLUGIN_PATH=/opt/gstreamer/out/lib
 
ARG=$1 

case   $ARG   in    


start):
   #pkill media_distri  
   kill -9 `netstat -anp|grep 50001|awk '{printf $6}'|cut -d/ -f1`
   rm /var/run/mediaproxy.pid 
   cd /opt/gst-proxy/  
   nohup ./media_distribute > /opt/media.log &  
   echo `netstat -anp|grep 50001|awk '{printf $6}'|cut -d/ -f1` >> /var/run/mediaproxy.pid 
;;  
stop):  
#   pkill   media_distri 
   kill -9 `netstat -anp|grep 50001|awk '{printf $6}'|cut -d/ -f1` 
   rm /var/run/mediaproxy.pid 
;;  
restart):  
   kill -9 `netstat -anp|grep 50001|awk '{printf $6}'|cut -d/ -f1`
   rm /var/run/mediaproxy.pid 
   cd /opt/gst-proxy/ 
   nohup ./media_distribute > /opt/media.log &
   echo `netstat -anp|grep 50001|awk '{printf $6}'|cut -d/ -f1` >> /var/run/mediaproxy.pid 
;;  
esac
