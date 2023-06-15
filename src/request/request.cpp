#include "request.hpp"
#include <algorithm>

#define req_line 0
#define header 1
#define bad_request 404
#define ok 200
#define  parse_req_header 2
#define _body 3
parse_request::parse_request() :  read_request() , request_line() , header_fields() , body()
{}
request::request() : parse_request() ,  status_code()
{}
read_request::read_request() : level() , read_req_line() , read_header() , read_body()
{}

int parse_request::parse_request_line()
{
	std::size_t delimiter = 0;

	for (int component_index = 0; component_index < 2; component_index++)
	{
		delimiter = read_req_line.find(" ");
		if (delimiter != std::string::npos)
			request_line[component_index] = read_req_line.substr(0, delimiter);
		else
			return (bad_request);
		if (component_index != 1)
			read_req_line = read_req_line.substr(delimiter + 1);
		else
			request_line[component_index + 1] =  read_req_line.substr(delimiter);
		if (request_line[component_index].find(" ") != std::string::npos)
			return (bad_request);
	}
	if (request_line[2].find(" ") != std::string::npos)
		return (bad_request);
	return (ok);	
}

int parse_request::parse_header_fields()
{
	std::size_t delimiter = 0;
	// std::size_t error_delimiter = 0;

	std::pair<std::string, std::string> key_value;
	std::string header_field;

	while (1)
	{
		delimiter = read_header.find("\r\n");
		if (delimiter != std::string::npos)
		{
			header_field = read_header.substr(0, delimiter);
			read_header = read_header.substr(delimiter + 2);
			delimiter = header_field.find(":");
			if (delimiter != std::string::npos)
			{
				if (header_field.find(":", delimiter + 1) != std::string::npos)
					return (bad_request);
				key_value.first = header_field.substr(0, delimiter);
				if (key_value.first.find(" \t") != std::string::npos)
					return (bad_request);
				key_value.second = header_field.substr(delimiter + 1);
				header_fields.insert(key_value);
			}
			else
				return (bad_request);
		}
		else
			break;
	}
	// i dind't check the case if the header fields if empty
	return (ok);
}

int parse_request::parse_body()
{
	std::transform(request_line[0].begin(), request_line[0].end(), 
		request_line[0].begin(), static_cast<int (*)(int)>(std::toupper));

	if (request_line[0] != "post")
		return (ok);
	
}

int parse_request::parse_data()
{
	if (parse_request_line() == ok)
		if (parse_header_fields() != ok)
			return (bad_request);
	else
		return (bad_request);

}

void read_request::read_buffer(char *buffer, int buffer_len)
{
	std::size_t delimiter = 0;
	std::string new_buffer(buffer, buffer_len);

	// read request line
	if (level == req_line)
	{
		read_req_line.append(new_buffer);
		delimiter = read_req_line.find("\r\n");
		if (std::string::npos != delimiter)
		{
			if (read_req_line.size() > delimiter + 2)
				new_buffer = read_req_line.substr(delimiter + 2);
			else
				return ;
			read_req_line.resize(delimiter);
			level++;
		}
	}
	 // read header_fields
	if (level == header)
	{
		delimiter = 0;
		read_header.append(new_buffer);
		delimiter = read_header.find("\r\n\r\n");
		if (std::string::npos != delimiter)
		{
			if (read_header.size() > delimiter + 2)
				new_buffer = read_header.substr(delimiter + 2);
			else
				return ;
			read_header.resize(delimiter);
			level++;
		}
	}
	if (level = parse_req_header)
		read_body = new_buffer;
}


int request::process_request(char *buffer, int buffer_len)
{
	if (level >= parse_req_header)
		parse_data();
	read_buffer(buffer, buffer_len);
}