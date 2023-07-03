
#include<iostream>
#include<map>
#include<vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include<ctime>
// #include "helper_func.cpp"
#define req_line 0
#define header 1
#define bad_request 404
#define ok 200
#define  parse_req_header 2
#define _body 3
#define chunked 1
#define n_length 2
#define none -777
#define done 777
#define fstream_error 8
#define empty 90

class read_request
{
	protected:
		int level;
		std::string read_req_line;
		std::string read_header;
		std::string read_body;

	void read_buffer(const char * buffer, int buffer_len);
	read_request();
};

class parse_request : public read_request
{
	public :
			std::string request_line[3];
			std::map<std::string, std::string> header_fields;
			std::fstream body;
	protected :
		int body_type;
		int body_length;
		size_t chunk_size;
		int max_length;
		std::string remain;
		std::string size_remain;
		std::string filename;
		int body_remain;

	parse_request();

	int parse_data();
	int parse_request_line();
	int parse_header_fields();
	int is_there_a_body();
	int parse_body(const char * buffer, int buffer_len);
	int n_length_body();
};

class request : public parse_request
{
	int status_code;
	public :

		int process_request(const char * buffer, int buffer_len);
};