#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "message.h"
#include "sys/socket.h"
#include "netinet/in.h"

#define MY_SOCK_PATH "test.socket"

void main()
{
	struct MESSAGE_T message_data;

	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr addr;
	addr.sa_family = AF_UNIX;
	memset(&addr, 0, sizeof(addr));
	strncpy(addr.sa_data, MY_SOCK_PATH, sizeof(addr.sa_data) - 1);

	int bind_out = bind(sockfd, &addr, sizeof(addr));

	int listen_out = listen(sockfd, 1000);

	struct sockaddr client_addr;

	int addrlen = sizeof(client_addr);
	int new_socket = accept(sockfd, &client_addr, (socklen_t*)&addrlen);

	message_data.number1 = 48;
	message_data.number2 = 19;

	int valsend = send(new_socket, &message_data, sizeof(message_data), 0);

}
