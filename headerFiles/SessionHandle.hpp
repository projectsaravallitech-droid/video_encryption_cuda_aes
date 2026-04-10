#ifndef SESSIONHANDLE_HPP
#define SESSIONHANDLE_HPP

#include <vector>
#include "TransportHandle.hpp"
#include"platform_socket.hpp"
#include "TLSHandle.hpp"
#include"ECDHHandle.hpp"
#include "EncryptHandle.hpp"
#include"KeyStore.hpp"
#include "DecryptHandle.hpp"

// Helper — converts string to byte vector
std::vector<unsigned char> toBytes(const std::string& s);

//Helper - converts byte vector to string
std::string toString(const std::vector<unsigned char>& v);

class SessionHandler {
private:

protected:
	std::unique_ptr<TLSHandle> tlsObj;
	ECDHHandle ecdhObj;
	KeyStore keyObj;
	std::unique_ptr<SymmetricEncrypt> encObj;
	std::unique_ptr<SymmetricDecrypt> decObj;
	void generateStoreKey(std::vector<unsigned char> sharedKey, std::vector<unsigned char> salt);

public:
	virtual void performHandshake() = 0;
	void sendEncrypted(std::vector<unsigned char>& data);
	std::vector<unsigned char> getDecrypted();

};

class SessionHandleServer : public SessionHandler {
private:
	uint16_t portNum;

public:
	SessionHandleServer(uint16_t portNum, const char* certFile, const char* privKeyFile, const char* caFile);
	void performHandshake();

};

class SessionHandleClient : public SessionHandler {
private:
	uint16_t portNum;
	uint16_t serverPort;
	const char* host;

public:
	SessionHandleClient(uint16_t portNum, const char* host, uint16_t serverPort, const char* certFile, const char* privKeyFile, const char* caFile);
	void performHandshake();
};


#endif // !SESSIONHANDLE_HPP



