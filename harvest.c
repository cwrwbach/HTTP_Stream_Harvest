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

#define MAX_HEADER_LEN 8192

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

//Create a Socket for server communication
short SocketCreate(void)
{
short hSocket;
printf("Create the socket\n");
hSocket = socket(AF_INET, SOCK_STREAM, 0);
return hSocket;
}

//Connect with server
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

//receive the data from the server
int SocketReceive(int hSocket,char* Rsp,short RvcSize)
{
int shortRetval = -1;
struct timeval tv;
tv.tv_sec = 20;  /* 20 Secs Timeout */
tv.tv_usec = 0;
if(setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(tv)) < 0)
    {
    printf("Time Out Line: %d \n",__LINE__);
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
char SendToServer[100] = {0};
char server_reply[4096] = {0};
char getrequest[4096];


if (signal(SIGINT, sig_handler) == SIG_ERR)
  printf("\nCan't catch SIGINT\n");

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
	     "Icy-Metadata: 1\r\n"
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

printf("Lupin to rx stream \n");

while(1)
{
//Received the data from the server
read_size = SocketReceive(hSocket, server_reply, 2048);

//Write stream to a file
fwrite(server_reply,1,read_size,fd);
//if(read_size > 0) 
    //printf("Server Response : size: %d %s\n\n",read_size,server_reply);
}

close(hSocket);
shutdown(hSocket,0);
shutdown(hSocket,1);
shutdown(hSocket,2);
return 0;
}
 

 
