#ifndef FILE_SERVER_H
#define FILE_SERVER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <regex>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#ifndef _WIN32
#define closesocket close
#endif
typedef int socklen_t;
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
typedef int SOCKET;
#endif

namespace fs = std::filesystem;

class FileServer {
public:
    FileServer(int port, std::string root_dir);

    void run();

private:
    SOCKET create_socket();
    void send_ok_response(SOCKET client_socket, const std::string& mime_type);
    void send_not_found_response(SOCKET client_socket);
    void send_internal_server_error_response(SOCKET client_socket);
    std::string mime_type(const std::string& extension);
    void handle_request(SOCKET client_socket);

    int port_;
    std::string root_;
};

#endif // FILE_SERVER_H
