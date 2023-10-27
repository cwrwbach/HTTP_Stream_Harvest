#pragma once

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


FILE * fd;
struct hostent *host;
int hSocket;


//station select
char host_url[256];
char myurl[256];
char useragent[256]; 
char myhost[256];
int myport;

char stream_buffer[32768];
//char audio_buffer[32768];
//char meta_buffer[2048];
//char test_buffer[2048];
char header_buffer[2048];


#define BITS 8


void set_station()
{
#define CFM
//#define RELAX
//#define VOS
//#define TORONTO
//#define SMOOTH
//#define VIRGIN
//#define SWISS

#ifdef CFM
//"http://media-ice.musicradio.com:80/ClassicFMMP3"); >>WORks
strcpy(host_url,"media-ice.musicradio.com"); 
strcpy(myurl,"/ClassicFMMP3"); 
strcpy(useragent,"Streamripper/1.x");
strcpy(myhost,"media-ice.musicradio.com");
myport = 80 ;
//#define METINT 8000
#endif


#ifdef RELAX
//"http://media-the.musicradio.com/ClassicFM-M-Relax");

strcpy(host_url,"media-the.musicradio.com"); 
strcpy(myurl,"/ClassicFM-M-Relax"); 
strcpy(useragent,"Streamripper/1.x");
strcpy(myhost,"media-the.musicradio.com");
myport = 80 ;
//#define METINT 8000
#endif


#ifdef VIRGIN

//https://radio.virginradio.co.uk/stream-chilled
//WORKS --- NO Metatdata in stream !
strcpy(host_url,"radio.virginradio.co.uk"); 
strcpy(myurl,"/stream-chilled"); 
strcpy(useragent,"Streamripper/1.x");
strcpy(myhost,"radio.virginradio.co.uk");
myport = 80 ;
//#define METINT 1024
#endif


#ifdef VOS
//"http://s1.voscast.com:11392/stream";
//???

strcpy(host_url,"s1.voscast.com"); 
strcpy (myurl,"/stream"); 
strcpy(useragent,"Streamripper/1.x");
strcpy(myhost,"s1.voscast.com");
myport = 11392;

//#define METINT 6000
#endif

#ifdef TORONTO
//"http://cast1.torontocast.com:1950/stream"  //DOES NOT WORK YET
strcpy(host_url,"cast1.torontocast.com"); 
strcpy (myurl,"/stream"); 
strcpy(useragent,"Streamripper/1.x");
strcpy(myhost,"cast1.torontocast.com");
myport = 1950;

//#define METINT 8000
#endif



#ifdef SMOOTH
//"http://icecast.thisisdax.com/SmoothUKMP3" >>WORKS
strcpy(host_url,"icecast.thisisdax.com"); 
strcpy (myurl,"/SmoothUKMP3"); 
strcpy(useragent,"Streamripper/1.x");
strcpy(myhost,"icecast.thisisdax.com");
myport = 80;
//#define METINT 8000
#endif


#ifdef SWISS
https://stream.srg-ssr.ch/m/rsc_de/mp3_128
strcpy(host_url,"stream.srg-ssr.ch"); 
strcpy (myurl,"/m/rsc_de/mp3_128"); 
strcpy(useragent,"Streamripper/1.x");
strcpy(myhost,"stream.srg-ssr.ch");
myport = 80;
//#define METINT 16000
#endif
}









//--------------------------------------------
//Inflamations

//https://en.wikipedia.org/wiki/FAAC


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

//https://radio.virginradio.co.uk/stream-chilled




//"http://icecast.thisisdax.com/SmoothUKMP3"

//https://gist.github.com/niko/2a1d7b2d109ebe7f7ca2f860c3505ef0

//./streamripper "http://s1.voscast.com:11392/stream" //works

//LIB-ao information

//LIB mpg123 information
//maybe this does not realy need lib0ao?
//https://www.mpg123.de/index.shtml
