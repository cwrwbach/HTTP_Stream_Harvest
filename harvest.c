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

#define MAX_HEADER_LEN 2048

pthread_t go_play;
int fifo_d;
unsigned char file_buf[32768];
char header_string[MAX_HEADER_LEN];
unsigned int meta_interval;
int mytime,secs,mins;

void sig_handler(int signo)
{
if (signo == SIGINT)
    printf("received SIGINT\n");

printf("Closing socket \n");
close(hSocket);
shutdown(hSocket,0);
shutdown(hSocket,1);
shutdown(hSocket,2);
printf("Closing \n");

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


void parse_icy_header(char * icy_head)
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

//This the separate thread for audio play-out
void * play_id(void *go_play)
{
libvlc_instance_t *inst;
libvlc_media_player_t *mp;
libvlc_media_t *m;

// load the vlc engine
inst = libvlc_new(0, NULL);

// create a new item
m = libvlc_media_new_path(inst, "/dev/shm/mable");

// create a media play playing environment
mp = libvlc_media_player_new_from_media(m);

// no need to keep the media now
libvlc_media_release(m);

libvlc_media_player_play(mp); // play the media_player

//I want this to go on for evah and evah
while(1)
    sleep(10);
    
libvlc_media_player_stop(mp);// stop playing
libvlc_media_player_release(mp); // free the media_player

libvlc_release(inst);
return 0;
}

//================================================

int main(int argc, char *argv[])
{
char show_header[2048] = {0};
char getrequest[2048];

unsigned int read_size,nx,header_len;
unsigned char meta_char;
unsigned short meta_len;

int size;
int mp3_int;

pthread_create(&go_play, NULL, play_id, NULL);

printf("\n   --- ICY STREAM HARVESTER ---\n");

//fifo setup
char * myfifo ="/dev/shm/mable";
mkfifo(myfifo,0666);
fifo_d = open(myfifo,O_WRONLY);

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
 //   end_of_header=nx;
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
        write(fifo_d,stream_buffer,size);
        }
    while (mp3_int > 0);

    //recv one byte for meta length
    size = recv(hSocket,file_buf,1, 0);
    meta_char = file_buf[0]; 
    meta_len = meta_char *16;
  
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
        printf("META-DATA>>> %s \n",file_buf);

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

