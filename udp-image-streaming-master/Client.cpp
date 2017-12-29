/*
 *   C++ UDP socket client for live image upstreaming
 *   Modified from http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/practical/UDPEchoClient.cpp
 *   Copyright (C) 2015
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "PracticalSocket.h"      // For UDPSocket and SocketException
#include <iostream>               // For cout and cerr
#include <cstdlib>                // For atoi()

using namespace std;

#include "opencv2/opencv.hpp"
using namespace cv;
#include "config.h"

#include <pthread.h>


unsigned char M8CmdID[8] = {'M','B','I','-','N','e','t',0xFF};
unsigned char VsyncFlag = 0x01;
unsigned char ImageFlag = 0x02;
unsigned char SerialNo = 0;
unsigned char PacketNo_1 = 0;
unsigned char PacketNo_2 = 0;
unsigned char PixelLen_1 = 0;
unsigned char PixelLen_2 = 0;
unsigned char Reserved_1 = 0;
unsigned char Reserved_2 = 0;
unsigned char WidthPixel_1 = (FRAME_WIDTH >> 8);
unsigned char WidthPixel_2 = (FRAME_WIDTH & 0xFF);
unsigned char HeightPixel_1 = (FRAME_HEIGHT >> 8);
unsigned char HeightPixel_2 = (FRAME_HEIGHT & 0xFF);
unsigned char RESERVED[28] = {0};

vector<unsigned char> preamble;

void ComposePreamble(){
	preamble.resize(48);
	preamble.assign(M8CmdID, M8CmdID+8);
	preamble.push_back(VsyncFlag);
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
        int jpegqual =  ENCODE_QUALITY; // Compression Parameter

        Mat frame, send;
        vector < uchar > encoded;
	vector <uchar> temp;	//Added by Craig for temp bytes

        //VideoCapture cap(0); // Grab the camera
	VideoCapture cap("WildCreatures.wmv");
        namedWindow("send", CV_WINDOW_AUTOSIZE);
        if (!cap.isOpened()) {
            cerr << "OpenCV Failed to open camera";
            exit(1);
        }

	ComposePreamble();	//Added by Craig
        clock_t last_cycle = clock();
        while (1) {

            cap >> frame;

            if(frame.size().width==0)continue;//simple integrity check; skip erroneous data...

            resize(frame, send, Size(FRAME_WIDTH, FRAME_HEIGHT), 0, 0, INTER_LINEAR);

            //vector < int > compression_params;
            //compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
            //compression_params.push_back(jpegqual);

            //imencode(".jpg", send, encoded, compression_params);	//Copress pic file

	    if(send.isContinuous()){	//Mat format to vector
		encoded.assign(send.datastart, send.dataend);
	    }else{
		for(int i=0; i<send.rows; i++){
			encoded.insert(encoded.begin(), send.ptr<uchar>(i), send.ptr<uchar>(i)+send.cols);
		}
	    }

            imshow("send", send);

            int total_pack = 1 + (encoded.size() - 1) / PACK_SIZE;
            int ibuf[1];
            ibuf[0] = total_pack;
            sock.sendTo(ibuf, sizeof(int), servAddress, servPort);

	    printf("PackSize1: %d", PACK_SIZE + preamble.size());

            for (int i = 0; i < total_pack; i++){
		temp.assign(encoded.begin()+(i*PACK_SIZE), encoded.begin()+((i+1)*PACK_SIZE));
		temp.insert(temp.begin(), preamble.begin(), preamble.end());
		sock.sendTo( & temp[0], PACK_SIZE + preamble.size(), servAddress, servPort);
                //sock.sendTo( & encoded[i * PACK_SIZE], PACK_SIZE, servAddress, servPort);
	    }
            //waitKey(FRAME_INTERVAL);
	    waitKey(1);

            clock_t next_cycle = clock();
            double duration = (next_cycle - last_cycle) / (double) CLOCKS_PER_SEC;
            cout << "\teffective FPS:" << (1 / duration) << " \tkbps:" << (PACK_SIZE * total_pack / duration / 1024 * 8) << endl;

            cout <<"Operation time: "<< next_cycle - last_cycle;
            last_cycle = next_cycle;

	    //break;	//For test

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
