#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <regex.h>

#define MYPORT "3500"
#define BACKLOG 10

void *get_in_addr(struct sockaddr *s)
{
  if(s->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)s)->sin_addr);
  }
  
  return &(((struct sockaddr_in6*)s)->sin6_addr);
}

int match(const char *string, char *pattern, char *substring)
{
  int    status;
  regex_t    re;
  regmatch_t pmatch[2];

  if (regcomp(&re, pattern, REG_EXTENDED) != 0) {
    return(0);
  }
  status = regexec(&re, string, (size_t) 2, pmatch, 0);
  regfree(&re);
  if (status != 0) {
    return(0);
  }
  else {
    int i = 0;
    int matchstart = (int)pmatch[1].rm_so;
    int matchend = (int)pmatch[1].rm_eo;

    for(i = 0; i < (matchend - matchstart); i++) {
      substring[i] = string[i + matchstart];
    }
  }

  return(1);
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

      char substr[64] = {'\0'};

      if(match(buf, "GET (.*) HTTP/1.1", substr)) {
        //open file and read contents into a string
        FILE *filestream;
        char fline[100];
        char filecontents[1024] = {'\0'};
      
        if(!strcmp(substr, "/") || !strcmp(substr, "/index.html")) {
          strcpy(substr, "/test.html");
        }

        char filename[64] = ".";
        strcat(filename, substr);

        if(filestream = fopen(filename, "r")) {
          while(fgets(fline, 100, filestream)){
            strcat(filecontents, fline);
          }
      
          //determine size of string
          int i = 0;
          while(filecontents[i]) {
            i++;
          }

          //send file contents
          send(newfd, "HTTP/1.0 200 OK\r\n", 17, 0);
          send(newfd, "\r\n", 2, 0);
          printf("%s", filecontents);
          send(newfd, filecontents, i, 0);
        }
        else {
          send(newfd, "HTTP/1.0 404 Not Found\r\n", 24, 0);
        }

      }
      close(newfd);
      exit(0); //exits just the child process, not the whole program
    }
    close(newfd);
  }

  return 0;
}
