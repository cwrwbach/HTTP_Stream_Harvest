#include "harvest.h"
//=================
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
#include <pthread.h>
#include <vlc/vlc.h>
//#define MPG

pthread_t go_play;

FILE *logfd;
#define MAX_HEADER_LEN 2048

unsigned char file_buf[65536];
char dummy[16384];
char header_string[MAX_HEADER_LEN];

FILE * f_raw;
FILE * f_mp3;

mpg123_handle *mh = NULL;
ao_device *dev = NULL;

unsigned int meta_interval;


unsigned int int_len;
unsigned mp3_base;
int mytime,secs,mins;

int fifo_d;

void sig_handler(int signo)
{
if (signo == SIGINT)
    printf("received SIGINT\n");

printf("Closing socket \n");
close(hSocket);
shutdown(hSocket,0);
shutdown(hSocket,1);
shutdown(hSocket,2);
printf("Closing mpg123 \n");

printf(" QUAT ! \n");
exit(-1);
}

//Create TCP socket
short SocketCreate(void)
{
short hSocket;
printf("Create the socket\n");
hSocket = socket(AF_INET, SOCK_STREAM, 0);
return hSocket;
}

//Connect to server
int SocketConnect(int hSocket)
{
int iRetval=-1;
int ServerPort = 80 ; //90190; //FIXME CHANSEL
//int ServerPort = 11392; //80 ; //90190; //FIXME CHANSEL
struct sockaddr_in remote= {0};

remote.sin_addr.s_addr = inet_addr(inet_ntoa (*(struct in_addr*)host->h_addr)); //Costa Rica
remote.sin_family = AF_INET;
remote.sin_port = htons(ServerPort);
iRetval = connect(hSocket,(struct sockaddr *)&remote,sizeof(struct sockaddr_in));
return iRetval;
}

// Send the data to the server and set the timeout of 20 seconds
int SocketSend(int hSocket,char* Rqst,short lenRqst)
{
int shortRetval = -1;
struct timeval tv;
tv.tv_sec = 20;  /* 20 Secs Timeout */
tv.tv_usec = 0;

if(setsockopt(hSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
    printf("Time Out\n");
    sleep(1);
    return -1;
    }
shortRetval = send(hSocket, Rqst, lenRqst, 0);
return shortRetval;
}

//================================================


int parse_icy_header(char * icy_head)
{
char * pos;
char heading[256];
printf("\n SCANNING HEADER \n");

strcpy(heading,"icy-metaint:");
pos = strstr(icy_head,heading);

meta_interval = 0;
if(pos > 0 ) 
    {
    meta_interval = atoi(pos+strlen(heading));
    printf("Meta interval: %d\n ",meta_interval);
    }


/*
pos = strstr(icy_head,"content-type:");
if(pos > 0 ) 
    {
    printf("* %s \n",pos);
    }

pos = strstr(icy_head,"icy-br:");
if(pos > 0 ) 
    {
    printf("* %s \n",pos);
    }

pos = strstr(icy_head,"ice-audio-info:");
if(pos > 0 ) 
    {
    printf("+++ %s \n",pos);
    }
*/

}                    


void * playit(void *go_play)
{
FILE * fred;


//int main//(int argc, char **argv)
{
    libvlc_instance_t *inst;
    libvlc_media_player_t *mp;
    libvlc_media_t *m;

    // load the vlc engine
    inst = libvlc_new(0, NULL);

fred = fopen("my_log,txt","w");

libvlc_log_set_file(inst,fred);

    // create a new item
  m = libvlc_media_new_path(inst, "/dev/shm/mable");

 // m=libvlc_media_new_location(inst,"http://media-the.musicradio.com/ClassicFM-M-Relax");
//    m=libvlc_media_new_location(inst,"https://allclassical.streamguys1.com/ac96k");
    // create a media play playing environment
    mp = libvlc_media_player_new_from_media(m);

    // no need to keep the media now
    libvlc_media_release(m);

    // play the media_player
    libvlc_media_player_play(mp);

        //I want this to go on for evah and evah
    while(1)
        sleep(10);

    // stop playing
    libvlc_media_player_stop(mp);

    // free the media_player
    libvlc_media_player_release(mp);

    libvlc_release(inst);


    return 0;
}


}



//================================================

int main(int argc, char *argv[])
{
//int read_size;
struct sockaddr_in server;
//char server_reply[16384] = {0};
char show_header[2048] = {0};
char getrequest[4096];


unsigned int fsize,read_size,end_of_header,nx,header_len;
unsigned char meta_char;
unsigned short meta_len;

int debug;
int count;
int size;
int mp3_int;


pthread_create(&go_play, NULL, playit, NULL);

//audio vars

int err;
    off_t frame_offset;
    unsigned char *audio;
    size_t done;
    ao_sample_format format;
    int channels, encoding;
    long rate;
//end audio vars


printf("\n   --- ICY STREAM HARVESTER ---\n");
execl("ls -l","");
//execl("../VLC/libvlc/play","");

#ifdef MPG
ao_initialize();  //sound card access
mpg123_init();
mh = mpg123_new(NULL, NULL);
mpg123_open_feed(mh);
#endif


count = 0;

//fifo setup
char * myfifo ="/dev/shm/mable";
mkfifo(myfifo,0666);
fifo_d = open(myfifo,O_WRONLY);

//logging
logfd = fopen("classicfm.mp3", "wb");

if (signal(SIGINT, sig_handler) == SIG_ERR)
  printf("\nCan't catch SIGINT\n");

//Select parameters of required stream  
set_station();

//home crafted HTTP GET with forced variables FIXME TODO etc
host = gethostbyname(host_url);

printf("\nIP address of %s is: ", host->h_name );
printf("%s\n\n",inet_ntoa (*(struct in_addr*)host->h_addr));

sprintf(
        getrequest,
	     "GET %s HTTP/1.1\r\n"
	     "Accept: */*\r\n"
	     "Cache-Control: no-cache\r\n"
	     "User-Agent: %s\r\n"
	     "Icy-Metadata: 1\r\n" //make this a 1 to accept meta-data, or 0 to not accept :-)
	     "Connection: close\r\n"
	     "Host: %s:%d\r\n\r\n",
	     myurl,
	     useragent[0] ? useragent: "Streamripper/1.x",
	     myhost,
	     myport
        );

//Create socket
hSocket = SocketCreate();
if(hSocket == -1)
    {
    printf("Could not create socket\n");
    return 1;
    }
printf("Socket is created\n");

//Connect to remote server
if (SocketConnect(hSocket) < 0)
    {
    perror("connect failed.\n");
    return 1;
    }
printf("Sucessfully conected with server\n");

//Send GET request to the server
printf("Sending HTTP GET Packet \n");
SocketSend(hSocket,getrequest , strlen(getrequest));

printf("Fetching Header \n");

//First PEEK to find length of header
read_size = recv(hSocket,header_buffer,MAX_HEADER_LEN,MSG_PEEK); 

for(nx = 0;nx<read_size;nx++)
if((header_buffer[nx  ] == 0x0d ) && (header_buffer[nx+1] == 0x0a ) && 
    (header_buffer[nx+2] == 0x0d ) && (header_buffer[nx+3] == 0x0a ))    
    {
    nx+=4;
    strncpy(show_header,header_buffer,nx);
    end_of_header=nx;
    header_len = nx;
    printf("Header length: %d\n",nx);
    }

//clean up the buf
memset(header_buffer,0,MAX_HEADER_LEN);

//Now read the correct length header and remove from stream
read_size = recv(hSocket,header_buffer,header_len,0); 

printf("\nHEADER RESPONSE:\n%s",header_buffer); 


parse_icy_header(header_buffer);


printf("NOW go Lupin to rx stream \n");


//----

while(1) //Main loop
    {
    mp3_int = meta_interval;

    do 
        {
        size = recv(hSocket,stream_buffer, mp3_int, 0);
        mp3_int -=size;
#ifdef MPG
 mpg123_feed(mh, (const unsigned char*) stream_buffer, size * 1);



    do 
        {
        err = mpg123_decode_frame(mh, &frame_offset, &audio, &done);
        switch(err) 
            {
            case MPG123_NEW_FORMAT:
            mpg123_getformat(mh, &rate, &channels, &encoding);
            format.bits = mpg123_encsize(encoding) * BITS;
            format.rate = rate;
            format.channels = channels;
            format.byte_format = AO_FMT_NATIVE;
            format.matrix = 0;
            dev = ao_open_live(ao_default_driver_id(), &format, NULL);
            break;

            case MPG123_OK:
            ao_play(dev, audio, done);
            break;

            case MPG123_NEED_MORE:
            break;

            default:
            break;
            }
        } while(done > 0);




#else

        write(fifo_d,stream_buffer,size);
#endif
       // fwrite(stream_buffer, size, 1, logfd); //logging only
        }
    while (mp3_int > 0);

    //recv one byte for meta length
    size = recv(hSocket,file_buf,1, 0);
    meta_char = file_buf[0]; 
    meta_len = meta_char *16;
    //printf(" m char: 0x%2.2x m length %d \n",meta_char,meta_len);
   
    //get the meta-data, if any
    if(meta_interval !=0)
        {
        do
            {
            size = recv(hSocket,file_buf,meta_len, 0);
            meta_len -= size;
            }
        while (meta_len >0);
    
        //just print it for now
        printf("META-DATA>>>\n %s \n",file_buf);

        mytime++;
        secs = mytime/2;
        mins = secs/60;
        secs = secs - (mins*60);
        //printf(" %d:%d t: %d\n",mins,secs,time);
        }
    } //main loop

printf(" ALL DONE 8k\n");

close(hSocket);
shutdown(hSocket,0);
shutdown(hSocket,1);
shutdown(hSocket,2);
return 0;

} 

