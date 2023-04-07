#include "FileServer.h"
#include "MimeMapper.h"
#include "ArgParser.h"

using namespace std;
int main(int argc, char** argv) {
#ifdef _WIN32
  WSADATA wsaData;
  int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (result != 0) {
	std::cerr << "WSAStartup failed: " << result << std::endl;
	return 1;
  }
#endif
  int port = 3000;
  string dir = ".";
  try {
	ArgParser arg_parser;
	arg_parser.assign("-p", port);
	arg_parser.assign("--port", port);
	arg_parser.assign("-d", dir);
	arg_parser.assign("--dir", dir);
	arg_parser.parse(argc, argv);

	cout << "port: " << port << "; dir: " << dir << endl;
  }
  catch (ArgParser::parse_error e) {
	cerr << e.what() << endl;
	return 1;
  }
  FileServer server(port, dir);
  server.run();
#ifdef _WIN32
  WSACleanup();
#endif

  return 0;
}