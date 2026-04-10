#ifndef TLSHANDLE_HPP
#define TLSHANDLE_HPP
#include <memory>
#include <openssl/ssl.h>
#include <vector>
#include "TransportHandle.hpp"

enum class TLSRole {
	SERVER, CLIENT
};


class TLSHandle {
protected:
	std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> ctx{ nullptr, SSL_CTX_free };
	std::unique_ptr<SSL, decltype(&SSL_free)> sslObj{ nullptr, SSL_free };
	TLSRole role;
	std::unique_ptr<SocketHandle> connectedSocket{ nullptr };

	void readExact(void* buf, size_t totalBytes);

public:

	TLSHandle(const char* certFile, const char* privKeyFile, TLSRole roleObj, const char* caFile);
	bool sendData(std::vector<unsigned char>& msg);
	std::vector<unsigned char> receiveData();
};


class TLSServer : public TLSHandle {
private:
	std::unique_ptr<SocketHandle> serverSocket{ nullptr };
public:

	TLSServer(const char* certFile, const char* privKeyFile, TLSRole roleObj, const char* caFile);
	void performHandshakeServer(uint16_t portNum);
};


class TLSClient : public TLSHandle {
public:

	TLSClient(const char* certFile, const char* privKeyFile, TLSRole roleObj, const char* caFile);

	void performHandshakeClient(uint16_t portNum, const char* host, uint16_t serverPort);
};


#endif
