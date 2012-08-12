#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MYPORT "3500"
#define BACKLOG 10

void *get_in_addr(struct sockaddr *s)
{
  if(s->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)s)->sin_addr);
  }
  
  return &(((struct sockaddr_in6*)s)->sin6_addr);
}

int main(void)
{
  struct sockaddr_storage their_addr;
  struct addrinfo hints, *res;
  socklen_t addr_size;
  int sockfd, newfd;
  char ipstr[INET6_ADDRSTRLEN], buf[512];
  int yes = 1;
  int byte_count;

  // first, load up address structs with getaddrinfo():

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

  getaddrinfo(NULL, MYPORT, &hints, &res);
  
  //create a new socket
  
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  //set socket options
  
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

  //bind to that socket

  bind(sockfd, res->ai_addr, res->ai_addrlen);

  //listen on that port

  listen(sockfd, BACKLOG);

  //accept a connection and create new socket
 
  while(1) {
    addr_size = sizeof(their_addr);
    newfd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    if (!fork()) {
      close(sockfd);

      //received data
      byte_count = recv(newfd, buf, sizeof buf, 0);
      printf("++listener: got packet from %s\n",
             inet_ntop(their_addr.ss_family,
                       get_in_addr((struct sockaddr *)&their_addr),
                       ipstr, sizeof ipstr));
      printf("++recv()'d %d bytes of data\n", byte_count);
      printf("%s\n", buf);

      //open file and read contents into a string
      FILE *filestream;
      char fline[100], filecontents[1024];
      
      filestream = fopen("test.html", "r");
      while(fgets(fline, 100, filestream)){
        strcat(filecontents, fline);
      }
      
      //determine size of string
      int i = 0;
      while(filecontents[i]) {
        i++;
      }

      //send file contents
      send(newfd, filecontents, i + 1, 0);

      close(newfd);
      exit(0); //exits just the child process, not the whole program
    }
    close(newfd);
  }

  return 0;
}
