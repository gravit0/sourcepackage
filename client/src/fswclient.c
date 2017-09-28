#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <getopt.h>
#define SOCK_NAME sock_path
#define BUF_SIZE 256
static const char *optString = "c:u:i:r:sl:f:p:";
int main(int argc, char ** argv)
{
      int opt = getopt(argc,argv,optString);
    char sock_path[BUF_SIZE];
    int init_path = 0;
    char buf[BUF_SIZE];
    buf[0] = 0;
    while( opt != -1 ) {
            switch( opt ) {
                case 'i': {
                    strcat(buf,"install:");
                    strcat(buf,optarg);
                    break;
                    
                }
                case 'u': {
                    strcat(buf,"installu:");
                    strcat(buf,optarg);
                    break;
                    
                }
                case 'l': {
                    strcat(buf,"load:");
                    strcat(buf,optarg);
                    break;
                    
                }
                case 'c': {
                    strcat(buf,"setconfig:");
                    strcat(buf,optarg);
                    break;
                    
                }
                case 'r':{
                    strcat(buf,"remove:");
                    strcat(buf,optarg);
                    break;
                    
                }
                case 'p': {
                    strcat(buf,"setroot:");
                    strcat(buf,optarg);
                    break; 
                    
                }
                case 'f': {
                    strcpy(sock_path, optarg);
                    init_path = 1;
                    break; 
                    
                }
                case 's': {
                    strcpy(buf, "stop:");
                    break;
                }
                default:
                    /* сюда на самом деле попасть невозможно. */
                    break;
            }
            opt = getopt( argc, argv, optString );
        }
  int   sock;
  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  struct sockaddr srvr_name;
  if (sock < 0) 
  {
    perror("socket failed");
    return EXIT_FAILURE;
  }
  srvr_name.sa_family = AF_UNIX;
  if(init_path) strcpy(srvr_name.sa_data, SOCK_NAME);
  else strcpy(srvr_name.sa_data, "/run/sp");
  if(connect(sock, &srvr_name, sizeof(srvr_name)) < 0)
  {
     perror("connect failed");
     exit(2);
  }
  send(sock, buf, strlen(buf), 0);
  buf[0] = '\0';
  recv(sock,buf,sizeof(buf),0);
  if(buf[0] != '\0') printf("%s",buf);
  close(sock);
}

