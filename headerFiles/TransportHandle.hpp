#ifndef TRANSPORTHANDLE_HPP
#define TRANSPORTHANDLE_HPP
#include <openssl/pem.h>
#include <vector>
#include <string>
#include <memory>
#include "platform_socket.hpp"
class TransportHandle {
public:
	virtual void sendData(std::vector<unsigned char>& bytestream) = 0;
	virtual std::vector<unsigned char> getData(size_t len) = 0;

};

class FileHandle : public TransportHandle {
private:
	std::unique_ptr<BIO, decltype(&BIO_free)> bio{ nullptr, BIO_free };
public:
	FileHandle(const std::string& filename, const std::string& mode);
		void sendData(std::vector<unsigned char>& bytestream)override;
		std::vector<unsigned char> getData(size_t len)override;
};


class SocketHandle {
private:
	static PlatformInit platformInit;  //for the platform init of windows
	SocketFd socketFd = INVALID_FD;
	bool isServer;
	uint16_t port;

	//Private constructor for making the socket objects in the class
	SocketHandle(SocketFd existingFd, bool isServer, uint16_t portNum);

public:
	SocketHandle(bool isServer, uint16_t portNum, int opt = 0);

	const bool bindSocket();
	bool listenSocket(int backlogSize);
	SocketHandle acceptClient();
	bool connectSocket(const char* host, uint16_t serverPort);
	SocketFd getRawFd();

	~SocketHandle();
	// delete copy
	SocketHandle(const SocketHandle&) = delete;
	SocketHandle& operator=(const SocketHandle&) = delete;

	// implement move
	SocketHandle(SocketHandle&& other) noexcept;
	SocketHandle& operator=(SocketHandle&& other) noexcept;

};




#endif
