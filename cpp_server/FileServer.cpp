#include "FileServer.h"
#include <unordered_map>

const int REQ_BUF_SIZE = 8192;

std::string getErrorMessage() {
    char buf[256];
#ifdef _WIN32
    // Use the Windows-specific version of strerror_s
    strerror_s(buf, sizeof(buf), WSAGetLastError());
#else
    // Use the POSIX version of strerror_s
    strerror_r(errno, buf, sizeof(buf));
#endif
    return std::string(buf);
}


FileServer::FileServer(int port, std::string root_dir) :
    port_(port), root_(std::move(root_dir))
{
}

SOCKET FileServer::create_socket()
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << getErrorMessage() << '\n';
        exit(1);
    }
    return sock;
}

void FileServer::send_ok_response(SOCKET client_socket, const std::string& mime_type)
{
    std::ostringstream oss;
    oss << "HTTP/1.0 200 OK\r\n"
        << "Content-Type: " << mime_type << "\r\n"
        << "Connection: close\r\n"
        << "\r\n";

    std::string response = oss.str();
    if (send(client_socket, response.c_str(), response.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Error sending response: " << getErrorMessage() << '\n';
    }
}

void FileServer::send_not_found_response(SOCKET client_socket)
{
    std::string response = "HTTP/1.0 404 Not Found\r\n\r\n";
    if (send(client_socket, response.c_str(), response.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Error sending response: " << getErrorMessage() << '\n';
    }
}

void FileServer::send_internal_server_error_response(SOCKET client_socket)
{
    std::string response = "HTTP/1.0 500 Internal Server Error\r\n\r\n";
    if (send(client_socket, response.c_str(), response.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Error sending response: " << getErrorMessage() << '\n';
    }
}

std::string FileServer::mime_type(const std::string& extension)
{
    static const std::unordered_map<std::string, std::string> mime_types {
        { ".html", "text/html" },
        { ".css", "text/css" },
        { ".js", "application/javascript" },
        { ".png", "image/png" },
        { ".jpg", "image/jpeg" },
        { ".jpeg", "image/jpeg" },
        { ".gif", "image/gif" },
        { ".svg", "image/svg+xml" },
        { ".ico", "image/x-icon" },
        { ".txt", "text/plain" },
        { ".pdf", "application/pdf" }
    };

    auto iter = mime_types.find(extension);
    if (iter != mime_types.end()) {
        return iter->second;
    }
    else {
        return "application/octet-stream";
    }
}

void FileServer::handle_request(SOCKET client_socket) {
    char buf[REQ_BUF_SIZE];
    int num_bytes = recv(client_socket, buf, REQ_BUF_SIZE, 0);
    if (num_bytes == SOCKET_ERROR) {
        std::cerr << "Error receiving request from client\n";
        return;
    }

    // Parse the request method, path, and HTTP version
    std::string request(buf, buf + num_bytes);
    std::string request_method;
    std::string request_path;
    std::string http_version;
    std::istringstream request_stream(request);
    request_stream >> request_method >> request_path >> http_version;

    // Ensure that the request method is GET and the HTTP version is 1.0 or 1.1
    if (request_method != "GET" || (http_version != "HTTP/1.0" && http_version != "HTTP/1.1")) {
        send_internal_server_error_response(client_socket);
        return;
    }

    // Append the root directory to the request path
    request_path = root_ + request_path;

    // If the request path is a directory, append "index.html"
    if (request_path.empty() || request_path.back() == '/') {
        request_path += "index.html";
    }

    // Open the file
    std::ifstream file(request_path, std::ios::in | std::ios::binary);
    if (!file) {
        send_not_found_response(client_socket);
        return;
    }

    // Determine the file extension and corresponding MIME type
    size_t extension_index = request_path.rfind('.');
    if (extension_index == std::string::npos) {
        send_internal_server_error_response(client_socket);
        return;
    }
    std::string extension = request_path.substr(extension_index);
    std::string mime_type_str = mime_type(extension);

    // Send the OK response and file contents
    send_ok_response(client_socket, mime_type_str);
    std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    send(client_socket, file_contents.c_str(), file_contents.size(), 0);

    // Close the file and socket
    file.close();
    closesocket(client_socket);
}

void FileServer::run() {
    SOCKET server_socket = create_socket();
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // Bind to any available interface
    addr.sin_port = htons(port_); // Bind to the specified port

    if (bind(server_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "Error binding server socket: " << getErrorMessage() << std::endl;
        closesocket(server_socket);
        return;
    }

    // Listen for incoming connections
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error listening for incoming connections: " << getErrorMessage() << std::endl;
        closesocket(server_socket);
    }

    // Wait for incoming connections
    std::cout << "Server listening on port " << port_ << std::endl;

    while (true) {
        // Accept a client connection
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Error accepting client connection: " << getErrorMessage() << std::endl;
            continue;
        }

        // Handle the client request
        handle_request(client_socket);

        // Close the client socket
        closesocket(client_socket);
    }

    // Close the server socket
    closesocket(server_socket);
}

