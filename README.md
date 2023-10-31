# HTTP_Stream_Harvest
Simple non-curl HTTP stream manipulation client. 
This is an experimental Work-In-Progress for educational purposes only.

Opens Streams from chosen station by direct HTTP and GET Commans - no CURL.
Extracts Header first, then separates Audio chunks and MetaData strings.

Opens separate thead to play-out through VLC Library. Uses mkfifo to buffer between the two threads. The named Pipe is "Mable" who lives in RAM >>>S "/dev/shm/mable" 
