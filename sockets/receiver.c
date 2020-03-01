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

	int connect_out = connect(sockfd, &addr, sizeof(addr));

	int valsend = read(sockfd, &message_data, sizeof(message_data));

	printf("%i %i", message_data.number1, message_data.number2);
}
