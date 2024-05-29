
//A module that I made for managing ssl resources and sockets needed for this bot

#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include "exceptUtils.h"

#include <string>
#include <stdexcept>
#include <openssl/ssl.h>
#include <openssl/err.h>

#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

#define SOCKET_UTILS_MAX_SHUTDOWN_ATTEMPTS 2

#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Automatically link a library - only works in VS

using socketFD = SOCKET; //type definition for the file descriptor type on windows

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> //for the close() function
#include <netdb.h>
#include <fcntl.h>

using socketFD = int; //type definition for the file descriptor type on non-windows
const int INVALID_SOCKET = -1; //already defined on windows

#endif

/*
Instructions to configure openssl in VS 2022:
1) get openssl directly from this page - https://slproweb.com/products/Win32OpenSSL.html
2) Open the 'Project' tab at the top left corner
	Open 'project name' Properties
3) Under 'Configuration Properties' navigate to 'VC++ Directories'
	In the 'Include Directories' field, add C:\\Program Files\\OpenSSL-Win64\\include
4) Under 'Configuration Properties' navigate to 'VC++ Directories'
	In the 'Library Directories' field, add C:\\Program Files\\OpenSSL-Win64\\lib
5) Under 'Linker' > 'Input' > 'Additional Dependencies, add 'libssl.lib' and 'libcrypto.lib'
6) Copy 'libssl-3-x64.dll' and 'libcrypto-3-x64.dll' to the same directory as the executable for this bot.
*/

class SSLNoReturn : public std::exception //throw upon receiving SSL_ERROR_ZERO_RETURN
{
public:
	SSLNoReturn(const char* Message) : message(Message) {}
	SSLNoReturn(const std::string Message) : message(Message) {}

	virtual const char* what() const noexcept override { return message.c_str(); }

private:
	std::string message;
};

#ifdef _WIN32

class WSAWrapper //when using sockets on Windows this needs to be used
{
public:
	WSAWrapper(const WSAWrapper&); //I have this to make sure that I am not using the copy constructor

	WSAWrapper();
	~WSAWrapper();

	WSAWrapper& operator=(const WSAWrapper&); //I have this to make sure that I am not using item assignment

private:
	WSADATA wsaData;
};

#endif

void socketCleanup(SSL*&, socketFD);

class SSLContextWrapper //you need to make sure that this object outlives all sockets in your program
{
public:
	SSLContextWrapper(const SSLContextWrapper&); //I have this to make sure that I am not using the copy constructor

	SSLContextWrapper();
	~SSLContextWrapper();

	SSLContextWrapper& operator=(const SSLContextWrapper&); //I have this to make sure that I am not using item assignment

	SSL_CTX* get_context() const noexcept;

private:
	SSL_CTX* ssl_context;
};

class SSLSocket
{
public:
	SSLSocket(const SSLSocket&);
	SSLSocket(const SSLContextWrapper&, const std::string, bool);
	~SSLSocket();

	SSLSocket& operator=(const SSLSocket&); //I have this to make sure that I am only using the copy constructor

	void reInit(); //initialize or reinitialize the socket
	void writeIpAddrToString(std::string&); //write the recorded ip address to a string

	SSL* get_struct() const noexcept;

	int read(void* buffer, const int buffer_size); //could be virtual
	int write(const std::string& message); //could be virtual

protected:
	SSL* ssl_struct;
	socketFD ssl_socket;

	std::string host;

private:
	friend void socketInit(SSLSocket&); //this shouldn't be accessable outside the class

	int bytes_read = 0;
	int bytes_write = 0;

	int error_read = 0; //ssl error code on read
	int error_write = 0; //ssl error code on write

	bool blocking; //true if the socket is a blocking socket
	
	char ip_address[INET6_ADDRSTRLEN] = ""; //record ip address for debugging

	SSLContextWrapper& ssl_context_wrapper;
};

#endif
