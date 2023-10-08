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

//LIB-ao information

//LIB mpg123 information
//maybe this does not realy need lib0ao?
//https://www.mpg123.de/index.shtml

//===========================================================

FILE * fd;
struct hostent *host;
int hSocket;
char host_url[256];

mpg123_handle *mh = NULL;
ao_device *dev = NULL;

char stream_buffer[32768];
char audio_buffer[32768];
char meta_buffer[2048];
char test_buffer[2048];

#define MAX_HEADER_LEN 8192
#define BITS 8
//---

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

mpg123_close(mh);
mpg123_delete(mh);
mpg123_exit();

printf("NOT Closing AO - doesnt like it ? \n");
//ao_close(dev);
//ao_shutdown();

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

//receive the data from the server this AINT USED !!!
int xxSocketReceive(int hSocket,char* Rsp,short RvcSize)
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

int main(int argc, char *argv[])
{
int read_size;
struct sockaddr_in server;
char server_reply[16384] = {0};
char show_header[2048] = {0};
char getrequest[4096];
int nx;
unsigned int meta_interval;

//audio vars
int err;
off_t frame_offset;
unsigned char *audio;
size_t done;
ao_sample_format format;
int channels, encoding;
long rate;

if (signal(SIGINT, sig_handler) == SIG_ERR)
  printf("\nCan't catch SIGINT\n");

//sound card access
ao_initialize();  
    
mpg123_init();
mh = mpg123_new(NULL, NULL);
mpg123_open_feed(mh);

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
int end_of_header;
meta_interval = 8000; //FIXME - should come from header

//char * myurl = "/stream";
//char * useragent= "Streamripper/1.x";
//char * myhost = "s1.voscast.com";
//int myport = 11392;

sprintf(
        getrequest,
	     "GET %s HTTP/1.1\r\n"
	     "Accept: */*\r\n"
	     "Cache-Control: no-cache\r\n"
	     "User-Agent: %s\r\n"
	     "Icy-Metadata: 1\r\n" //make this a 1 to accept meta-data
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

//First find length of header
read_size = recv(hSocket,server_reply,2048,MSG_PEEK);

for(nx = 0;nx<read_size;nx++)
if((server_reply[nx  ] == 0x0d ) && (server_reply[nx+1] == 0x0a ) && 
    (server_reply[nx+2] == 0x0d ) && (server_reply[nx+3] == 0x0a ))    
    {
    nx+=4;
    strncpy(show_header,server_reply,nx);
    printf("\nHeader:\n%s h-sz %d \n\n",show_header,nx);
    end_of_header=nx;
    }

//read the header to remove from stream
read_size = recv(hSocket,test_buffer,end_of_header,0); ///nx
printf(" nx: %d read_sz %d \n",end_of_header,read_size);

printf("\n TEST \n %s",test_buffer); 

printf("NOW Lupin to rx stream \n");

//----

int size;
int nmemb;

unsigned char aaa;
int spin;
int mp3_int;

while(1) //for evah and evah
    {
    nmemb=1; //what meaneth this?
    mp3_int = 8000;

//Get complete MP3 frame  
    do 
    {
    size = recv(hSocket,stream_buffer, mp3_int, 0);
    mp3_int -=size;
    printf("mp3int %d: \n",mp3_int);
    memcpy(audio_buffer,stream_buffer,size);
    }
    while (mp3_int > 0);

//now get meta-data
    recv(hSocket,&aaa,1, 0); //one byte for metadata length (/16)
    printf("Size of meta(\16) %d\n",aaa);   
    spin = (int) (aaa*16) ;
    printf("Length of metadata: %d\n",spin);  
    size = recv(hSocket,meta_buffer,spin, 0);
    printf(">%s ",meta_buffer); //print the headers

// send to decoder
    mpg123_feed(mh, (const unsigned char*) audio_buffer, 8000 * nmemb);

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
    } //while for evah

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

 
