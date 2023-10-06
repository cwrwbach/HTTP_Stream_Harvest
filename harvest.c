#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#include <mpg123.h>
#include <ao/ao.h>


//https://aticleworld.com/socket-programming-in-c-using-tcpip/
//"23.111.178.66" 11392 //Costa Rica

//https://curl.se/libcurl/c/curl_easy_setopt.html
//https://thecodeartist.blogspot.com/2013/02/shoutcast-internet-radio-protocol.html
// The Curl API: https://curl.se/libcurl/c/

//Some URLS:
//curl_easy_setopt(curl_handle, CURLOPT_URL, "http://media-ice.musicradio.com:80/ClassicFMMP3");
//curl_easy_setopt(curl_handle, CURLOPT_URL, "http://media-the.musicradio.com/ClassicFM-M-Relax");
//curl_easy_setopt(curl_handle, CURLOPT_URL, "http://s1.voscast.com:11392/stream");
//curl_easy_setopt(curl_handle, CURLOPT_URL, "http://cast1.torontocast.com:1950/stream");

//"http://icecast.thisisdax.com/SmoothUKMP3"

//https://gist.github.com/niko/2a1d7b2d109ebe7f7ca2f860c3505ef0

//./streamripper "http://s1.voscast.com:11392/stream" //works

//===========================================================

FILE * fd;
struct hostent *host;
int hSocket;
char host_url[256];

mpg123_handle *mh = NULL;
ao_device *dev = NULL;



char audio_buffer[16384];





#define MAX_HEADER_LEN 8192
#define BITS 8
//---

void sig_handler(int signo)
{
if (signo == SIGINT)
    printf("received SIGINT\n");
close(hSocket);
shutdown(hSocket,0);
shutdown(hSocket,1);
shutdown(hSocket,2);
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
int ServerPort = 80 ; //90190; //FIXME FIXI GOOF AND PROPPER
struct sockaddr_in remote= {0};

//remote.sin_addr.s_addr = inet_addr("23.111.178.66"); //Costa Rica
//remote.sin_addr.s_addr = inet_addr("81.20.48.165"); //Classic

remote.sin_addr.s_addr = inet_addr(inet_ntoa (*(struct in_addr*)host->h_addr)); //Costa Rica

/*
//just me being silly - don't work yet anyway"
int temp  = (host->h_addr[0] <<24) | (host->h_addr[1] <<16)
                           | (host->h_addr[2] <<8) | (host->h_addr[3]);

printf (">> temp %d \n",temp);
remote.sin_addr.s_addr = temp;
*/

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

//receive the data from the server
int SocketReceive(int hSocket,char* Rsp,short RvcSize)
{
int shortRetval = -1;
struct timeval tv;
tv.tv_sec = 20;  /* 20 Secs Timeout */
tv.tv_usec = 0;
if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
    printf("Time Out. Line: %d \n",__LINE__);
    sleep(2);
    exit -1;
    }
shortRetval = recv(hSocket, Rsp, MAX_HEADER_LEN, 0);
return shortRetval;
}

//---


#if 1
size_t play_stream(void *buffer, size_t size, size_t nmemb, void *userp)
{
    int err;
    off_t frame_offset;
    unsigned char *audio;
    size_t done;
    ao_sample_format format;
    int channels, encoding;
    long rate;

    mpg123_feed(mh, (const unsigned char*) buffer, size * nmemb);
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

    return size * nmemb;
}

#endif



//---
#if 0
void setup_audio()
{
    ao_initialize();  //sound card access
    
    mpg123_init();
    mh = mpg123_new(NULL, NULL);
    mpg123_open_feed(mh);

//    CURL *curl = curl_easy_init();
//    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, play_stream); //specifies the Callback function

   // curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
//    curl_easy_setopt(curl, CURLOPT_URL, carsick);

//curl_easy_setopt(curl,CURLOPT_HEADER,1);
//curl_easy_setopt(curl,CURLOPT_READDATA,"FRED");
//curl_easy_setopt(curl,CURLOPT_VERBOSE,1);


//main loop in by-yer
//    curl_easy_perform(curl);
//    curl_easy_cleanup(curl);

    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    ao_close(dev);
    ao_shutdown();

}
#endif

//---

int main(int argc, char *argv[])
{
int read_size;
struct sockaddr_in server;
char SendToServer[100] = {0};
char server_reply[16384] = {0};
char show_header[2048] = {0};
char show_meta[2048] = {0};
char save_mp3[2048] = {0};

char getrequest[4096];
int nx;
unsigned int meta_interval;
unsigned int mp;


//audio vars

  int err;
    off_t frame_offset;
    unsigned char *audio;
    size_t done;
    ao_sample_format format;
    int channels, encoding;
    long rate;

//endaudio vard




if (signal(SIGINT, sig_handler) == SIG_ERR)
  printf("\nCan't catch SIGINT\n");

   ao_initialize();  //sound card access
    
mpg123_init();
mh = mpg123_new(NULL, NULL);
mpg123_open_feed(mh);




for(int n=0;n<4096;n++) getrequest[n]=0;

fd = fopen("fred.mp3","w");
printf(" fopen = %d \n",fd);

//home crafted HTTP GET with forced variables FIXME TODO etc

//strcpy(host_url,"s1.voscast.com");
strcpy(host_url,"media-ice.musicradio.com");

host = gethostbyname(host_url);

printf("\nIP address of %s is: ", host->h_name );
printf("%s\n\n",inet_ntoa (*(struct in_addr*)host->h_addr));

char * myurl = "/ClassicFMMP3";
char * useragent= "Streamripper/1.x";
char * myhost = "media-ice.musicradio.com";
int myport = 80 ;

meta_interval = 8000; //FIXME - should come from header

//char * myurl = "/stream";
//char * useragent= "Streamripper/1.x";
//char * myhost = "s1.voscast.com";
//int myport = 11392;

// This is the header suggested Florian Stoehr 
sprintf(
        getrequest,
	     "GET %s HTTP/1.1\r\n"
	     "Accept: */*\r\n"
	     "Cache-Control: no-cache\r\n"
	     "User-Agent: %s\r\n"
	     "Icy-Metadata: 0\r\n" //make this a 1 to accept meta-data
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

//Send data to the server
printf("sendin HTTP GET Packet \n");
SocketSend(hSocket,getrequest , strlen(getrequest));


printf(" Fetching Header \n");

//read_size = SocketReceive(hSocket, server_reply, 4096);
read_size = recv(hSocket,server_reply,2048,MSG_PEEK);

for(nx = 0;nx<read_size;nx++)
if((server_reply[nx  ] == 0x0d ) && (server_reply[nx+1] == 0x0a ) && 
    (server_reply[nx+2] == 0x0d ) && (server_reply[nx+3] == 0x0a ))    
    {
    nx+=4;
    strncpy(show_header,server_reply,nx);
    printf("\nHeader:\n%s h-sz %d \n\n",show_header,nx);
    }
printf(" nx: %d read_sz %d \n",nx,read_size);

//read_size = recv(hSocket,server_reply,2048,0);


//fwrite(server_reply+nx,1,read_size-nx,fd); //start after header, save remainder

//fwrite(server_reply,1,read_size,fd); //start after header, save remainder



printf("Lupin to rx stream \n");

//----


nx+=8000;
mp = 0;
int qq = 42;

int size;
int nmemb;

while(1)
{

/*
    read_size = recv(hSocket,server_reply, nx, 0);
    nx -=read_size;
    if(read_size <1)
        {
        printf("done 8k %d \n",nx);
        nx=8000;
        }
    printf(" rsz = %d nx = %d \n",nx);   
fwrite(server_reply,1,read_size,audio_buffer);
*/

nmemb=1;
size = recv(hSocket,audio_buffer, nx, 0);

   mpg123_feed(mh, (const unsigned char*) audio_buffer, size * nmemb);
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

}



while(0)
    {
    read_size = recv(hSocket,server_reply, nx, 0);
    nx -=read_size;
    if(read_size <1)
        {
        printf("done 8k %d \n",nx);
        nx=8000;
        }
    printf(" rsz = %d nx = %d \n",nx);    
fwrite(server_reply,1,read_size,stdout);
  //  fwrite(server_reply,1,read_size,fd);
    }

//fwrite(server_reply,1,8000,fd);
printf(" ALL DONE 8k\n");


 mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    ao_close(dev);
    ao_shutdown();

close(hSocket);
shutdown(hSocket,0);
shutdown(hSocket,1);
shutdown(hSocket,2);
return 0;
}
 

 
