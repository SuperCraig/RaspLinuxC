#include "PracticalSocket.h"      // For UDPSocket and SocketException
#include <iostream>               // For cout and cerr
#include <cstdlib>                // For atoi()

using namespace std;

#include "opencv2/opencv.hpp"
using namespace cv;

#include "config.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>


unsigned char M8CmdID[8] = {'M','B','I','-','N','e','t',0xFF};
unsigned char VsyncFlag = 0x01;
unsigned char ImageFlag = 0x02;
unsigned char SerialNo = 0;
unsigned char PacketNo_1 = 0;
unsigned char PacketNo_2 = 0;
unsigned char PixelLen_1 = (PACKET_PIXELS >> 8);
unsigned char PixelLen_2 = (PACKET_PIXELS & 0xFF);
unsigned char Reserved_1 = 0;
unsigned char Reserved_2 = 0;
unsigned char WidthPixel_1 = (FRAME_WIDTH >> 8);
unsigned char WidthPixel_2 = (FRAME_WIDTH & 0xFF);
unsigned char HeightPixel_1 = (FRAME_HEIGHT >> 8);
unsigned char HeightPixel_2 = (FRAME_HEIGHT & 0xFF);
unsigned char RESERVED[28] = {0};

vector<unsigned char> preamble;
vector<unsigned char> pixels_v;

void ComposePreamble(){
 preamble.resize(48);
 preamble.assign(M8CmdID, M8CmdID+8);
 preamble.push_back(ImageFlag);
 preamble.push_back(SerialNo);
 preamble.push_back(PacketNo_1);
 preamble.push_back(PacketNo_2);
 preamble.push_back(PixelLen_1);
 preamble.push_back(PixelLen_2);
 preamble.push_back(Reserved_1);
 preamble.push_back(Reserved_2);
 preamble.push_back(WidthPixel_1);
 preamble.push_back(WidthPixel_2);
 preamble.push_back(HeightPixel_1);
 preamble.push_back(HeightPixel_2);
 preamble.insert(preamble.end(), RESERVED, RESERVED+28);
}

int count_p = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;

void *even(void *arg){
 while(1){
	pthread_mutex_lock(&mutex);
	while(count_p % 2 !=0){
		pthread_cond_wait(&cond, &mutex);
	}
	cout<< count_p <<endl;
	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&cond);
 }
 pthread_exit(0);
}

void *odd(void *arg){
 while(1){
	pthread_mutex_lock(&mutex);
	while(count_p % 2 != 1){
		pthread_cond_wait(&cond, &mutex);
	}
	cout<< count_p <<endl;
	pthread_mutex_unlock(&mutex);
	pthread_cond_signal(&cond);
 }
 pthread_exit(0);
}


int main(int argc, char * argv[]) {
 if ((argc < 3) || (argc > 3)) { // Test for correct number of arguments
	cerr << "Usage: " << argv[0] << " <Server> <Server Port>\n";
	exit(1);
 }

 string servAddress = argv[1]; // First arg: server address
 unsigned short servPort = Socket::resolveService(argv[2], "udp");

 try {
	UDPSocket sock;

	//Set up desktop capture object
	gint x,y;
	GdkScreen* cur_screen = NULL;
	GdkWindow* window = NULL;
	GdkPixbuf* pixbuf_screenshot = NULL;
	GdkRectangle rect;
	GdkRectangle screen_rect;


	//Set up video capture object
	vector <uchar> temp;	//Added by Craig for temp bytes
	namedWindow("send", CV_WINDOW_AUTOSIZE);

	ComposePreamble();	//Added by Craig

	int count = 0;
	//clock_t last_cycle = clock();
	while (1) {
		clock_t last_cycle = clock();

		if(cur_screen == NULL)
			cur_screen = gdk_screen_get_default(); //get screen

		screen_rect.x = 0;
		screen_rect.y = 0;
		screen_rect.width = gdk_screen_get_width(cur_screen); //get screen width
		screen_rect.height = gdk_screen_get_height(cur_screen);	//get screen height
		printf("screen_rect: x=%d, y=%d, w=%d, h=%d\n", screen_rect.x, screen_rect.y, screen_rect.width, screen_rect.height);

		window = gdk_screen_get_root_window(cur_screen); //get window by screen
		gdk_window_get_origin(window, &x, &y); //get origin point
		rect.x = x;
		rect.y = y;
		gdk_drawable_get_size(GDK_DRAWABLE (window), &rect.width, &rect.height);  //get drawable size
		rect.width = FRAME_WIDTH;
		rect.height = FRAME_HEIGHT;
		printf("rect: x=%d, y=%d, w=%d, h=%d\n", rect.x, rect.y, rect.width, rect.height);

		clock_t getScreen1 = clock();
		pixbuf_screenshot = gdk_pixbuf_get_from_drawable(NULL, window,
			NULL, rect.x - x, rect.y - y, 0, 0,
			rect.width, rect.height);  //get pixbuf from drawable widget
		clock_t getScreen2 = clock();
		printf("shot screen time: %d\n", getScreen2-getScreen1);

		clock_t buf1 = clock();
		int n_channels = gdk_pixbuf_get_n_channels(pixbuf_screenshot);
		int rowstride = gdk_pixbuf_get_rowstride(pixbuf_screenshot);
		guchar *pixels = gdk_pixbuf_get_pixels(pixbuf_screenshot);
	 	pixels_v.assign(pixels, pixels + (rect.width * rect.height * n_channels));
		clock_t buf2 = clock();
		printf("buffer to vector time: %d\n", buf2-buf1);

		int total_pack = 1 + (pixels_v.size() - 1) / PACK_SIZE;
		int ibuf[1];
		ibuf[0] = total_pack;
		sock.sendTo(ibuf, sizeof(int), servAddress, servPort);

		printf("PackSize: %d\n", PACK_SIZE + preamble.size());

		clock_t send_p1 = clock();
		for (int i = 0; i < total_pack; i++){
			temp.assign(pixels_v.begin()+(i*PACK_SIZE), pixels_v.begin()+((i+1)*PACK_SIZE));
			temp.insert(temp.begin(), preamble.begin(), preamble.end());
			sock.sendTo( & temp[0], PACK_SIZE + preamble.size(), servAddress, servPort);

			preamble[9] = ++SerialNo; //packet no. adding
			count ++;
			preamble[10] = (count >> 8);
			preamble[11] = (count & 0xFF);
		}
		clock_t send_p2 = clock();
		printf("Send packet out time: %d\n", send_p2-send_p1);
		//waitKey(FRAME_INTERVAL);
		//waitKey(1);

		clock_t next_cycle = clock();
		double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
		cout << "effective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8);

		cout <<"\tOperation time: "<< next_cycle - last_cycle <<endl;
		//last_cycle = next_cycle;

		/*
		char *file = NULL;
		file = (char*)malloc(10);
		strcat(file, "Hello");
		strcat(file, ".jpg");
		gdk_pixbuf_save(pixbuf_screenshot, file, "jpeg", NULL, "quality", "100", NULL);
		*/
		g_object_unref(pixbuf_screenshot);

		//pthread_t t1;
		//pthread_t t2;

		//pthread_mutex_init(&mutex, 0);
		//pthread_cond_init(&cond, 0);

		//pthread_create(&t1, 0, &even, NULL);
		//pthread_create(&t2, 0, &odd, NULL);

		//pthread_join(t1, 0);
		//pthread_join(t2, 0);

		//pthread_mutex_destroy(&mutex);
		//pthread_cond_destroy(&cond);
	}
	// Destructor closes the socket
 } catch (SocketException & e) {
	cerr << e.what() << endl;
	exit(1);
 }

 return 0;
}
