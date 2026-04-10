#include <openssl/pem.h>
#include<openssl/ssl.h>
#include <openssl/err.h>
#include <vector> 
#include <stdexcept>
#include <iostream>
#include "TransportHandle.hpp"
#include "platform_socket.hpp"
#include <chrono>
#include <thread>
#include "TLSHandle.hpp"


	void TLSHandle::readExact(void* buf, size_t totalBytes) {
		size_t recieved = 0;
		unsigned char* ptr = static_cast<unsigned char*>(buf);

		while (recieved < totalBytes) {
			int result = SSL_read(sslObj.get(), ptr + recieved, totalBytes - recieved);

			if (result > 0) {
				recieved = recieved + result;
				continue;
			}

			int err = SSL_get_error(sslObj.get(), result);

			if (err == SSL_ERROR_WANT_READ) {
				continue;   // non-blocking — retry
			}
			if (err == SSL_ERROR_ZERO_RETURN) {
				throw std::runtime_error("Connection closed by peer");
			}

			char errBuf[256];
			ERR_error_string_n(ERR_get_error(), errBuf, sizeof(errBuf));
			throw std::runtime_error(std::string("SSL_read failed: ") + errBuf);

		}
	
	}


	TLSHandle::TLSHandle(const char* certFile, const char* privKeyFile, TLSRole roleObj, const char* caFile) {
		//here we do the setup of the ctx for the server and client
		this->role = roleObj;
		const SSL_METHOD* method = TLS_method();
		ctx.reset(SSL_CTX_new(method));

		SSL_CTX_set_min_proto_version(ctx.get(), TLS1_3_VERSION);
		SSL_CTX_set_max_proto_version(ctx.get(), TLS1_3_VERSION); // Force 1.3 for testing

		// Load certs
		if (SSL_CTX_use_certificate_file(ctx.get(), certFile, SSL_FILETYPE_PEM) <= 0) {
			throw std::runtime_error("Error in loading the certificate");
		}
		if(SSL_CTX_use_PrivateKey_file(ctx.get(), privKeyFile, SSL_FILETYPE_PEM) <= 0) {
			throw std::runtime_error("Error in loading the private key");
		}

		//verify if private key matches the cerificate
		if (SSL_CTX_check_private_key(ctx.get()) <= 0) {
			throw std::runtime_error("Loaded private key does not match the certificate");
		}

		//load the CA bundle
		//TODO Load the actual filepaths of the CA bundle here
		if (SSL_CTX_load_verify_locations(ctx.get(), caFile, nullptr) <= 0) {
			throw std::runtime_error("Error in loading the CA bundle");
		}

		//set the mTLS verification mode
		if (this->role == TLSRole::CLIENT) {
			SSL_CTX_set_verify(ctx.get(), SSL_VERIFY_PEER, nullptr);
		}
		else {
			SSL_CTX_set_verify(ctx.get(), SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
		}
	}

	bool TLSHandle::sendData(std::vector<unsigned char> &msg) {

		uint32_t msgLen = htonl(msg.size());	//converting the number conventions for transmission(littile endian and big endian)
		if (SSL_write(sslObj.get(), &msgLen, sizeof(msgLen)) <= 0) {
			throw std::runtime_error("Error in writing data to the port");
		}

		if (SSL_write(sslObj.get(), msg.data(), msg.size()) <= 0) {
			throw std::runtime_error("Error in writing data to the port");
		}
		return true;
	}

	std::vector<unsigned char> TLSHandle::receiveData() {
		
		uint32_t msgLen = 0;
		readExact(&msgLen, 4);
		uint32_t payloadLen = ntohl(msgLen);

		static constexpr uint32_t MAX_MESSAGE_SIZE = 1048576;

		if (payloadLen == 0) {
			return {};   // valid empty message
		}
		if (payloadLen > MAX_MESSAGE_SIZE) {
			throw std::runtime_error("Message length exceeds maximum allowed size");
		}

		std::vector<unsigned char> buffer(payloadLen);
		readExact(buffer.data(), payloadLen);   
		return buffer;
	}


	TLSServer::TLSServer(const char* certFile, const char* privKeyFile, TLSRole roleObj, const char* caFile) : TLSHandle(certFile, privKeyFile, roleObj, caFile) {}


	void TLSServer::performHandshakeServer(uint16_t portNum) {

		//do the binding, handshakes, certificate verification here, and estabilish a secure communication channel at the end of this function
		//TCP Layer operations

		if (!serverSocket) {
			serverSocket = std::make_unique<SocketHandle>(true, portNum, 1);
			serverSocket->bindSocket();
			serverSocket->listenSocket(5);
		}

		connectedSocket = std::make_unique<SocketHandle>(
			std::move(serverSocket->acceptClient())
		);
		
		//TLS operations for handshake
		sslObj.reset(SSL_new(ctx.get()));
		SSL_set_fd(sslObj.get(), static_cast<int>(connectedSocket->getRawFd()));
		if (SSL_accept(sslObj.get()) <= 0) {
			throw std::runtime_error("Failed to estabilish connection with the client");
		}

		long verifyResult = SSL_get_verify_result(sslObj.get());
		if (verifyResult != X509_V_OK) {
			throw std::runtime_error(
				std::string("Client certificate verification failed: ") +
				X509_verify_cert_error_string(verifyResult)
			);
		}
		
	}


	TLSClient::TLSClient(const char* certFile, const char* privKeyFile, TLSRole roleObj, const char* caFile) : TLSHandle(certFile, privKeyFile, roleObj, caFile){	}

	void TLSClient::performHandshakeClient(uint16_t portNum, const char* host, uint16_t serverPort) {

		SocketHandle clientSocket(false, portNum, 1);
		clientSocket.connectSocket(host, serverPort);
		connectedSocket = std::make_unique<SocketHandle>(std::move(clientSocket));
		sslObj.reset(SSL_new(ctx.get()));
		SSL_set_fd(sslObj.get(), static_cast<int>(connectedSocket->getRawFd()));
		
		// Must be set BEFORE SSL_connect
		// SNI — tells server which certificate to present (matters if server hosts multiple)
		SSL_set_tlsext_host_name(sslObj.get(), host);

		// Hostname verification — certificate must match this hostname
		SSL_set1_host(sslObj.get(), host);



		if (SSL_connect(sslObj.get()) <= 0) {
			throw std::runtime_error("Unable to conncet to the server socket");
		}

		long verifyResult = SSL_get_verify_result(sslObj.get());
		if (verifyResult != X509_V_OK) {
			throw std::runtime_error(
				std::string("Server certificate verification failed: ") +
				X509_verify_cert_error_string(verifyResult)
			);
		}

	}



