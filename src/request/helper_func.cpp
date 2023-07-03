
std::string trim(std::string str)
{
    // Remove space from the beginning 
    size_t start = str.find_first_not_of(" \t");
    if (start != std::string::npos)
        str = str.substr(start);

    // Remove space from the end
    size_t end = str.find_last_not_of(" \t");
    if (end != std::string::npos)
        str = str.substr(0, end + 1);

    return str;
}

std::string gen_name()
{
   std::string elements = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::string name;
		 std::srand(static_cast<unsigned int>(std::time(nullptr)));
        for(int i = 0; i < 13; ++i)
            name += elements[rand() % 60];
        return name;
}