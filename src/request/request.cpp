#include "request.hpp"
#include "helper_func.cpp"


parse_request::parse_request() : read_request()  , body_remain(0), chunk_size(-1) , header_fields()  , body_type(none) , max_length(-1)
{}

read_request::read_request() : level() , read_req_line() , read_header() , read_body()
{}



int parse_request::parse_request_line()
{
	std::stringstream ss(read_req_line);

	ss >> request_line[0] >> request_line[1] >> request_line[2];	
	return (ok);
}

int parse_request::parse_header_fields()
{
	std::size_t delimiter = 0;

	std::pair<std::string, std::string> key_value;
	std::string header_field;
	while (1)
	{
		delimiter = read_header.find("\r\n");
		if (delimiter 
		!= std::string::npos)
		{
			header_field = read_header.substr(0, delimiter);
			read_header = read_header.substr(delimiter + 2);
			delimiter = header_field.find(":");
			if (delimiter != std::string::npos)
			{
				if (header_field.find(":", delimiter + 1) != std::string::npos)
				{
					key_value.first = header_field.substr(0, delimiter);
					std::transform(key_value.first.begin(), key_value.first.end(),
					 key_value.first.begin(), static_cast<int (*) (int)>(std::tolower));
					if (key_value.first != "host")
						return (bad_request);
				}
				key_value.first = header_field.substr(0, delimiter);;
				std::transform(key_value.first.begin(), key_value.first.end(),
					 key_value.first.begin(), static_cast<int (*) (int)>(std::tolower));
				if (key_value.first.find(" \t") != std::string::npos)
					return (bad_request);
				key_value.second = trim(header_field.substr(delimiter + 1));
				if ((key_value.first == "content-length" || key_value.first == "transfer-coding"))
				{
					if (header_fields.find(key_value.first) != header_fields.end())
						return (bad_request);
				}
				header_fields.insert(key_value);
			}
			else
				return (bad_request);
		}
		else
			break;
	}
	return (is_there_a_body());
}

int parse_request::is_there_a_body()
{
	std::stringstream ss;
	std::fstream local;

	std::transform(request_line[0].begin(), request_line[0].end(), 
		request_line[0].begin(), static_cast<int (*)(int)>(std::toupper));

	if (request_line[0] != "POST")
	{
		level = done;
		return (done);
	}
	if	(header_fields.find("transfer-encoding") != header_fields.end())
	{
		 if (header_fields.find("transfer-encoding")->second == "chunked")
			body_type = chunked;
		else
			return (bad_request);
	}
	if (header_fields.find("content-length") != header_fields.end())
	{
		if (body_type != none)
			return (bad_request);
		std::cout << "content-length = " << header_fields.find("content-length")->second << std::endl;
		ss << header_fields.find("content-length")->second;
		ss >> max_length;
		if (max_length < 0)
			return (bad_request);
		body_type = n_length;
	}
	filename = "local" + gen_name();
	// filename = "/tmp/" + gen_name();
	local.open(filename, std::fstream::out | std::fstream::app);
	if (!local.is_open())
		return (fstream_error);
	if (body_type == n_length)
	{
		body << read_body;
		body_length += read_body.size();
	}
	local.close();
	level++;
	return (ok);
}

int parse_request::parse_data()
{
	int value;
	if (parse_request_line() == ok)
	{
		value = parse_header_fields();
		if (value != ok && value != done)
			return (bad_request);
	}
	else
		return (bad_request);
	return (ok);
}

void read_request::read_buffer(const char *buffer, int buffer_len)
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

			// std::cout << "here\n";
			if (read_header.size() > delimiter + 4)
				new_buffer = read_header.substr(delimiter + 4);
			else
				return ;
			read_header.resize(delimiter + 2);
			level++;
		}
	}
	if (level == parse_req_header)
		read_body = new_buffer;
}

int parse_request::parse_body(const char *buffer, int buffer_len)
{
	std::string body_data(buffer, buffer_len);
	std::stringstream ss;
	std::string data;
		std::fstream file; 
	file.open(filename, std::fstream::out | std::fstream::app);

	if (read_body.size())
	{
		body_data = read_body;
		read_body = std::string();
	}
	if (body_type == n_length)
	{
		// if (body_length + buffer_len > max_length)
		// {
		// 	body_data.resize(max_length - body_length);
		// 	std::cout << max_length - body_length << std::endl;
		// }
		file << body_data;
		body_length += body_data.size();
		file.close();
		if (body_length >= max_length)
		{
			std::cout << "max_length: "  << max_length << std::endl;
			return (done);
		}
		return ok;
	}
	else if (body_type == chunked)
	{
		try
		{
			if (body_remain)
			{
				if (remain.size())
				{
					body_data = remain + body_data;
					remain = std::string();
				}
				if (body_remain - 2 > body_data.size())
				{
					body_remain -= body_data.size();
					file << body_data;
					return (ok);
				}
				else if (body_remain + 2 <= body_data.size())
				{
					file << body_data.substr(0, body_remain);
					body_data = body_data.substr(body_remain + 2);
					body_remain = 0;
					chunk_size = -1;
				}
				else
				{
					remain = body_data;
					return (ok);
				}
			}
			while (body_data.size())
			{
				if (chunk_size == -1)
				{
				if (size_remain.size())
				{
					body_data = size_remain + body_data;
					size_remain = std::string();
				}
				delimiter = body_data.find("\r\n");
				if (delimiter == std::string::npos)
				{
					size_remain = body_data;
					return (ok);
				}
				ss << body_data.substr(0, delimiter);
				ss >> std::hex >> chunk_size;
				if (chunk_size == 0)
					return (done);
				body_data.erase(0, delimiter + 2);
				}
				if (body_data.size()  < chunk_size)
				{
					body_remain = chunk_size - body_data.size();
					file << body_data;
					body_length += body_data.size();
					return (ok);
				}
				else
				{
					file << body_data.substr(0, chunk_size);
					if (body_data.size() > chunk_size + 2)
						body_data = body_data.substr(chunk_size + 2);
					else
					{
						remain = body_data;
						body_remain = body_data.size();
					}
				}
				chunk_size = -1;
			}
			file.close();
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			exit(0);
		}
	}
	return (ok);
}

int request::process_request(const char * buffer, int buffer_len)
{
	if (level < parse_req_header)
		read_buffer(buffer, buffer_len);
	if (level == parse_req_header)
	{
		if (parse_data() != ok)
			return (bad_request);
	}
	if  (level == _body)
		return (parse_body(buffer, buffer_len));
	if (level == done)
		return (done);
	return (done);
}
