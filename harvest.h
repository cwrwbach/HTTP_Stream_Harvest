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










mpg123_handle *mh;
ao_device *dev;

char stream_buffer[32768];
char audio_buffer[32768];
char meta_buffer[2048];
char test_buffer[2048];

#define MAX_HEADER_LEN 8192
#define BITS 8













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
