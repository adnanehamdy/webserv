
#include<iostream>
#include<map>
#include<vector>
class read_request
{
	protected:
		std::string read_req_line;
		std::string read_header;
		std::string read_body;
		int level;

	void read_buffer(char *buffer, int buffer_len);
	read_request();
};

class parse_request : public read_request
{
	protected: 
		std::string request_line[3];
		std::map<std::string, std::string> header_fields;
		std::string body;
		int status_code;

	parse_request();

	int parse_data();
	int parse_request_line();
	int parse_header_fields();
	int parse_body();
};

class request : public parse_request
{
	int status_code;
	public :
		request();
		int process_request(char *buffer, int buffer_len);
	// std::pair <int, std::string> construct_request();
};