//#define FRAME_HEIGHT 720
#define FRAME_HEIGHT 256
#define FRAME_WIDTH 512
//#define FRAME_WIDTH 1280
//#define FRAME_INTERVAL (1000/30)
#define FRAME_INTERVAL (1000/60)
//#define PACK_SIZE 4096 //udp pack size; note that OSX limits < 8100 bytes
//#define PACK_SIZE 1424 //Maximum packet size is 1514-14(eth header)-28 = 1472
#define PACK_SIZE 1392 // 464 (16 multiplies) * 3 (RGB)
#define ENCODE_QUALITY 80

#define PACKET_PIXELS 464 //packet per pixels 464 is 16 mutiplies
