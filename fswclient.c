#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <getopt.h>
#define SOCK_NAME sock_path
#define BUF_SIZE 256
static const char *optString = "u:i:r:sl:f:p:";
int main(int argc, char ** argv)
{
      int opt = getopt(argc,argv,optString);
    char sock_path[BUF_SIZE];
    int init_path = 0;
    char buf[BUF_SIZE];
    while( opt != -1 ) {
            switch( opt ) {
                case 'i': {
                    char newstr[BUF_SIZE];
                    strcat(newstr,"install:");
                    strcat(newstr,optarg);
                    strcpy(buf, newstr);
                    break;
                    
                }
                case 'u': {
                    char newstr[BUF_SIZE];
                    strcat(newstr,"installu:");
                    strcat(newstr,optarg);
                    strcpy(buf, newstr);
                    break;
                    
                }
                case 'l': {
                    char newstr[BUF_SIZE];
                    strcat(newstr,"load:");
                    strcat(newstr,optarg);
                    strcpy(buf, newstr);
                    break;
                    
                }
                case 'r':{
                    char newstr[BUF_SIZE];
                    strcat(newstr,"remove:");
                    strcat(newstr,optarg);
                    strcpy(buf, newstr);
                    break;
                    
                }
                case 'p': {
                    char newstr[BUF_SIZE];
                    strcat(newstr,"setroot:");
                    strcat(newstr,optarg);
                    strcpy(buf, newstr);
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
  sock = socket(AF_UNIX, SOCK_DGRAM, 0);
  struct sockaddr srvr_name;
  if (sock < 0) 
  {
    perror("socket failed");
    return EXIT_FAILURE;
  }
  srvr_name.sa_family = AF_UNIX;
  if(init_path) strcpy(srvr_name.sa_data, SOCK_NAME);
  else strcpy(srvr_name.sa_data, "/run/sp");
  sendto(sock, buf, strlen(buf), 0, &srvr_name,
    strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family));
}

