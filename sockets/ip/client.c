#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char * argv[])
{
  int socket_desc;
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc == -1)
    printf("Could not initialize socket.");

  struct sockaddr_in server;
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons(8800);

  if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)))
  {
    printf("connect error");
    return 1;
  }
  printf("connected");


  char * msg = "Hello There.";

  if (send(socket_desc, msg, strlen(msg), 0) < 0)
  {
    printf("Send message failed.");
    return 1;
  }
  printf("success");
  return 0;
}


