/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multiplex.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adnane <adnane@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/05 15:17:45 by anass_elaou       #+#    #+#             */
/*   Updated: 2023/07/02 15:28:51 by adnane           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "socket.hpp"

int main()
{
	
	int res = 0;
	client_info client_inf;
	SOCKET server = client_inf.create_socket("127.0.0.1", "6969");
	std::vector<t_client_info *> &vec = client_inf.get_client_vec();
	int client_index;
	int i = 0;
	while (1)
	{
		fd_set reads;
		reads = client_inf.wait_on_client(server);
		if (FD_ISSET(server, &reads))
		{
			client_index = client_inf.get_client(-1);
			vec[client_index]->socket = accept(server, (struct sockaddr*) &(vec[client_index]->address), &(vec[client_index]->address_length));
			if (!ISVALIDSOCKET(vec[client_index]->socket))
			{
				std::cerr << "accept() failed." << GETSOCKETERRNO() << "\n";
				return (-1);
			}
			std::cout << "New connection from " << client_inf.get_client_address(client_index) << "\n";
		}
		for (size_t i = 0; i < vec.size(); i++)
		{
			if (FD_ISSET(vec[i]->socket, &reads))
			{
				int r = recv(vec[i]->socket, vec[i]->data , 1024, 0);
				if (r < 1)
				{
					std::cerr << "Unexpected disconnect from " << client_inf.get_client_address(client_index) << "\n";
					client_inf.drop_client(client_index);
				}
				res = vec[i]->client_req.process_request(vec[i]->data, r);
				if (res == 777)
				{
					std::cout << vec[i]->client_req.request_line[0] <<  "   "<<
						vec[i]->client_req.request_line[1] << "  " << vec[i]->client_req.request_line[2] << std::endl;
					std::cout << "heADERS\n";
					for (std::map<std::string, std::string>::iterator it = vec[i]->client_req.header_fields.begin()
						; it != vec[i]->client_req.header_fields.end(); it++)
						std::cout << it->first << " : " << it->second << std::endl;
					std::cout << "request_done \n";
					exit(0);
					// client_inf.drop_client(client_index);
				}
				else if (res == 404)
				{
					std::cout << "bad_request\n";
					exit(0);
				}
			}
		}
	}
	std::cout << "\nClosing socket...\n";
	CLOSESOCKET(server);
	std::cout << "Finished.\n";
	return (0);
}