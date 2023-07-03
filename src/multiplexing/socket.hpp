#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>

#include <ctime>
#include <iostream>
#include <cstring>
#include <vector>
#include "../request/request.hpp"

// #include "status_code.hpp"

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#define MAX_REQUEST_SIZE 2047

typedef struct	s_client_info 
{
	socklen_t address_length;
	struct sockaddr_storage address;
	SOCKET socket;
	request client_req;
	char data[MAX_REQUEST_SIZE + 1];

	int received;
}				t_client_info;

class client_info
{
	// private :

	public :
		std::vector<t_client_info *> client_vec;
		std::vector<t_client_info *> &get_client_vec(void) { return (client_vec); }
		const char	*file_type(const char *filename);
		SOCKET		create_socket(const char *hostname, const char *port);
		int			get_client(SOCKET s);
		void		drop_client(int client_index);
		const char	*get_client_address(int client_index);
		fd_set		wait_on_client(SOCKET server);
		// void		send_400(int client_index);
		// void		send_404(int client_index);
		void		serve_resource(int client_index, const char *path);
};

#endif