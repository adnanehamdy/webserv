#include <iostream>
#include <string>
#include <algorithm>

int main() {
    std::string request_line[3] = {"get", "path", "http/1.1"};

    // Transform request_line[0] to uppercase
    std::transform(request_line[0].begin(), request_line[0].end(), request_line[0].begin(), ::toupper);

    // Print the transformed value
    std::cout << request_line[0] << std::endl;

    return 0;
}
