#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>

#define MAX_CLIENT_NUM 10

int main(int argc, char * argv[])
{
  int socket_desc, client_list[10];
  int i;
  
  for (i = 0; i < MAX_CLIENT_NUM; i++)
  {
    client_list[i] = 0;
  }
  
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc == -1)
    printf("Could not initialize socket.");

  int opt = 1;
  setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

  struct sockaddr_in server;
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons(8800);

  if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)))
  {
    printf("bind error");
    return 1;
  }

  listen(socket_desc, 3);
  
  fd_set readfds;
  while (1)
  {
    FD_ZERO(&readfds);
    
    FD_SET(socket_desc, &readfds);
    int max_sd = socket_desc;
    for (i = 0; i < MAX_CLIENT_NUM; i++)
    {
      if (client_list[i] > 0)
        FD_SET(client_list[i], &readfds);
      if (client_list[i] > max_sd)
        max_sd = client_list[i];
    }
    
    select(max_sd + 1, &readfds, NULL, NULL, NULL);
    printf("ping\n");
    if (FD_ISSET(socket_desc, &readfds))
    {
      struct sockaddr_in client_addr;
      int client_addr_len = sizeof(client_addr);
      int new_socket = accept(socket_desc, (struct sockaddr *)&client_addr, (socklen_t*)&client_addr_len);
      
      for (i = 0; MAX_CLIENT_NUM; i++)
      {
        if (client_list[i] == 0)
        {
          client_list[i] = new_socket;
          break;
        }
      }
      write(new_socket, "Hello new client!\n", 19);
      printf("We got a client!\n");
    }
    else
    {
      for (i = 0; i < MAX_CLIENT_NUM; i++)
      {
        if (FD_ISSET(client_list[i], &readfds))
        {
          char buffer[100];
          int len = 0;
          len = read(client_list[i], buffer, 100);
          if (len > 0)
          {
            int j;
            for (j = 0; j < MAX_CLIENT_NUM; j++)
            {
              if (client_list[j] > 0 && i != j)
              {
                write(client_list[j], buffer, len);
              }
            }
          }
          else
          {
            close(client_list[i]);
            client_list[i] = 0;
            printf("Good bye client!\n");
          }
        }
      }
    }
  }
}


