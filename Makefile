CC            = gcc
CXX           = g++
DEFINES       = -DQT_NO_DEBUG -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB
CFLAGS        = -pipe -O2 -pthread -pthread -Wall -W -D_REENTRANT -fPIE $(DEFINES)
CXXFLAGS      = -pipe -O2 -pthread -pthread -Wall -W -D_REENTRANT -fPIE $(DEFINES)
INCPATH       = -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include -I/usr/include/opencv -I/usr/include/qt5 -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtGui -I/usr/include/qt5/QtCore -I. -I.
LINK          = g++
LFLAGS        = -Wl,-O1
LIBS          = $(SUBLIBS) -lgstapp-1.0 -lgstvideo-1.0 -lgstbase-1.0 -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0 -lX11 -L/usr/local/cuda-6.5/lib -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab -lopencv_detection_based_tracker -lopencv_esm_panorama -lopencv_facedetect -lopencv_imuvstab -lopencv_tegra -lopencv_vstab -lcufft -lnpps -lnppi -lnppc -lcudart -ltbb -lrt -lm -ldl -lQt5Widgets -L/usr/lib/arm-linux-gnueabihf -lQt5Gui -lQt5Core -lGLESv2 -lpthread 


multi_thread:
	gcc multi_thread.c cmd_rcv.c hashtable.c -o media_distribute `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gio-2.0`


multi_test:
	gcc multi_test.c cmd_rcv.c hashtable.c -o multi_test `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gio-2.0`

gst-proxy:
	gcc proxy.c -o gst-proxy `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-mpegts-1.0 `


test:
	gcc test.c -o test `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-mpegts-1.0 `


testtcpserver:
	gcc test-tcpserver.c -o testtcpserver `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-mpegts-1.0 `



probe:
	gcc probe.c -o probe `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-mpegts-1.0 `

multi-pipeline:
	gcc multi-pipeline.c cmd_rcv.c hashtable.c -o multi-pipeline `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-mpegts-1.0 gio-2.0`


gdbmulti_thread:
	gcc -g multi_thread.c cmd_rcv.c hashtable.c -o gdbmulti_thread `pkg-config --cflags --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-mpegts-1.0 gio-2.0`

clean:
	rm -f gst-proxy test testtcpserver probe multi-pipeline multi_thread gdbmulti_thread
