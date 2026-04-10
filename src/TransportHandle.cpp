#include <vector> 
#include <stdexcept>
#include "TransportHandle.hpp"
#include <iostream>
#include "platform_socket.hpp"
//TODO Implement a factory like structure as we want to handle files, sockets etc
FileHandle:: FileHandle(const std::string& filename, const std::string& mode) {
		//make the bio pointer and open the file
		bio.reset(BIO_new_file(filename.c_str(), mode.c_str()));
		if (!bio) {
			throw std::runtime_error("Error in opening the file");
		}
	}

	void FileHandle:: sendData(std::vector<unsigned char> &bytestream){
		if (BIO_write(bio.get(), bytestream.data(), bytestream.size()) <= 0) {
			throw std::runtime_error("Error in writing to file");
		}
	}

	std::vector<unsigned char> FileHandle::getData(size_t len){
		std::vector<unsigned char> buffer(len);
		if (BIO_read(bio.get(), buffer.data(), len) <= 0) {
			throw std::runtime_error("Error in reading from file");
		}
		buffer.resize(len);
		return buffer;
	}




	SocketHandle::SocketHandle(SocketFd existingFd, bool isServer, uint16_t portNum) {
			socketFd = existingFd;
			this->isServer = isServer;
			port = portNum;
		}


	SocketHandle::SocketHandle(bool isServer, uint16_t portNum, int opt) {//TODO Take the server port from the certificate file, and then pass it here for socket in case of client			
			this->isServer = isServer;
			port = portNum;
			socketFd = socket(AF_INET, SOCK_STREAM, 0);
			if (socketFd == INVALID_FD) {
				throw std::runtime_error("Socket creation failed");
			}
			//This provision is for development as it allows port re-use for server, by default its gonna be 0, so in case of deployment, do not set this opt parameter
			if (opt == 1) {
				setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
			}
			
		}

	const bool SocketHandle::bindSocket() {
			if (isServer) {
				sockaddr_in addr{};
				addr.sin_family = AF_INET;
				addr.sin_port = htons(port);
				addr.sin_addr.s_addr = INADDR_ANY;
				if (bind(socketFd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
					throw std::runtime_error("Failed to bind the server socket");
				}
				else {
					return true;
				}
			}
			else {
				throw std::runtime_error("Cannot bind client socket");
			}
		}

	bool SocketHandle::listenSocket(int backlogSize){
			if (isServer) {
				if (listen(socketFd, backlogSize) != 0) {
					throw std::runtime_error("Unable to set server socket to listen state");
				}
				else {
					return true;
				}
			}
			else {
				throw std::runtime_error("Cannot set client socket to listen");
			}
				
		}

		SocketHandle SocketHandle:: acceptClient() {
			//TODO Implement timeout logic for this as the thread waits indefinately
			if (isServer) {
				//the function returns a socket instance, but we need to encapsulate that in the SocketHandle object
				sockaddr_in clientAddr{};
				SockLen addrLen = sizeof(clientAddr);
				SocketFd clientFd = accept(socketFd, (struct sockaddr*)&clientAddr, &addrLen);

				if (clientFd == INVALID_FD) {
					throw std::runtime_error("Accept failed");
				}

				// construct a pre-configured client SocketHandle
				return SocketHandle(clientFd, false, clientAddr.sin_port);

			}
			else {
				throw std::runtime_error("Cannot set client socket to accept");
				
			}
		}



	bool SocketHandle::connectSocket(const char* host, uint16_t serverPort) {
			//TODO Implement timeout logic for this as the thread waits indefinately
			if (isServer) {
				throw std::runtime_error("Cannot call connect method on sevrer");
			}
			//basically right now its gonna be local host
			//the client needs the server port here, so maybe use the port from the certificate, and pass it here
			//in the conncet, the client needs the address of the server and its info, so that must be passed
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(serverPort);

			if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
				throw std::runtime_error("Invalid server IP address");
			}
			std::cout << "Before connect" << std::endl;
			if (connect(socketFd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
				throw std::runtime_error("Connection to server failed");
			}
			std::cout << "After connect" << std::endl;
			return true;
		}


	SocketFd SocketHandle:: getRawFd() {
		return this->socketFd;
	}



	SocketHandle::~SocketHandle() {//As I need the destructor, I also need to refer the rule of 3 and rule of 5
			if (socketFd != INVALID_FD) {
				closeSocket(socketFd);
			}
			
		}


	// implement move
	SocketHandle::SocketHandle(SocketHandle&& other) noexcept
			: socketFd(other.socketFd), isServer(other.isServer), port(other.port) {
			other.socketFd = INVALID_FD;  // prevent double close
		}

	SocketHandle& SocketHandle::operator=(SocketHandle&& other) noexcept {
			if (this != &other) {
				if (socketFd != INVALID_FD)  
					closeSocket(socketFd);
				socketFd = other.socketFd;
				other.socketFd = INVALID_FD;
			}
			return *this;
		}

PlatformInit SocketHandle::platformInit;

