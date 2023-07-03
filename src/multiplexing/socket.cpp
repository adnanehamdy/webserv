/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adnane <adnane@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/18 16:58:11 by anass_elaou       #+#    #+#             */
/*   Updated: 2023/06/22 09:46:33 by adnane           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"

const char	*client_info::file_type(const char *filename)
{
	const char *last_dot = strrchr(filename, '.');
	if (last_dot)
	{
		if (strcmp(last_dot, ".ico") == 0) return "image/x-icon";
		if (strcmp(last_dot, ".jpeg") == 0) return "image/jpeg";
		if (strcmp(last_dot, ".jpg") == 0) return "image/jpeg";
		if (strcmp(last_dot, ".js") == 0) return "application/javascript";
		if (strcmp(last_dot, ".json") == 0) return "application/json";
		if (strcmp(last_dot, ".png") == 0) return "image/png";
		if (strcmp(last_dot, ".pdf") == 0) return "application/pdf";
		if (strcmp(last_dot, ".svg") == 0) return "image/svg+xml";
		if (strcmp(last_dot, ".txt") == 0) return "text/plain";
	}
		return "application/octet-stream";
}

SOCKET	client_info::create_socket(const char *hostname, const char *port)
{
	std::cout << "Configurating local address\n";
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE;
	struct addrinfo *bind_address;
	getaddrinfo(hostname, port, &hints, &bind_address);
	std::cout << "Creating socket..." << "\n";
	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	int yes = 1;
	if(setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
	{
		std::cerr << "setsockopt failed " << std::endl;
		exit(-1);
	}
	if (!ISVALIDSOCKET(socket_listen))
	{
		std::cerr << "socket() failed." << GETSOCKETERRNO() << "\n";
		exit (-1);
	}
	std::cout << "Binding to local address..." << "\n";
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
	{
		std::cerr << "bind() failed." << GETSOCKETERRNO() << "\n";
		exit (-1);
	}
	freeaddrinfo(bind_address);
	std::cout << "Listening..." << "\n";
	if (listen(socket_listen, 10) < 0)
	{
		std::cerr << "listen() failed." << GETSOCKETERRNO() << "\n";
		exit (-1);
	}
	return (socket_listen);
}

int		client_info::get_client(SOCKET s)
{
	size_t i = 0;
	std::vector<t_client_info *> &client_vec = get_client_vec();
	if (client_vec.size())
	{
		for (i = 0; i < client_vec.size(); i++)
		{
			if (client_vec[i]->socket == s)
				break ;
		}
		if (i < client_vec.size())
			return (i);
	}
	t_client_info *clnt = new t_client_info;
	clnt->address_length = sizeof(clnt->address);
	client_vec.push_back(clnt);
	return (i);
}

void	client_info::drop_client(int client_index)
{
	CLOSESOCKET(client_vec[client_index]->socket);
	client_vec.erase(client_vec.begin() + client_index - 1);
}

const char	*client_info::get_client_address(int client_index)
{
	char *address_buffer = static_cast<char*>(malloc(100));
	getnameinfo((struct sockaddr*)&client_vec[client_index]->address, client_vec[client_index]->address_length,
		address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
	return(address_buffer);
}

fd_set	client_info::wait_on_client(SOCKET server)
{
	fd_set reads;
	// fd_set tmpReads;
	FD_ZERO(&reads);
	FD_SET(server, &reads);
	SOCKET max_socket = server;
	std::vector<t_client_info *> &client_vec = get_client_vec();
	
	for (size_t i = 0; i < client_vec.size(); i++)
	{
		FD_SET(client_vec[i]->socket, &reads);
		if (client_vec[i]->socket > max_socket)
			max_socket = client_vec[i]->socket;
	}
	if (select(max_socket + 1, &reads, 0, 0, 0) < 0)
	{
		std::cerr << "select() failed." << GETSOCKETERRNO() << "\n";
		exit(0);
	}
	return (reads);
}

void		client_info::serve_resource(int client_index, const char *path)
{
	std::cout << "serve_resource " << get_client_address(client_index) << path << "\n";
	FILE *fp = fopen("./index.html", "r");
	
	if (!fp)
	{
		std::cerr << "fopen failed " << std::endl;
		exit(-1);
	}
	fseek(fp, 0L, SEEK_END);
	size_t cl = ftell(fp);
	rewind(fp);
	char buffer[1024];
	sprintf(buffer, "HTTP/1.1 200 OK\r\n");
	send(client_vec[client_index]->socket, buffer, strlen(buffer), 0);
	sprintf(buffer, "Connection: close\r\n");
	send(client_vec[client_index]->socket, buffer, strlen(buffer), 0);
	sprintf(buffer, "Content-Length: %lu\r\n", cl);
	send(client_vec[client_index]->socket, buffer, strlen(buffer), 0);
	sprintf(buffer, "\r\n");
	send(client_vec[client_index]->socket, buffer, strlen(buffer), 0);
	int r = fread(buffer, 1, 1024, fp);
	send(client_vec[client_index]->socket, buffer, r, 0);
	while (r)
	{
		send(client_vec[client_index]->socket, buffer, r, 0);
		r = fread(buffer, 1, 1024, fp);
	}
	fclose(fp);
	drop_client(client_index);
}