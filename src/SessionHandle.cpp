#include <iostream>
#include <chrono>
#include<thread>
#include "TransportHandle.hpp"
#include"platform_socket.hpp"
#include "TLSHandle.hpp"
#include"ECDHHandle.hpp"
#include "SessionHandle.hpp"
#include "KeyStore.hpp"
#include "DecryptHandle.hpp"

// Helper — converts string to byte vector
std::vector<unsigned char> toBytes(const std::string& s) {
	return std::vector<unsigned char>(s.begin(), s.end());
}

//Helper - converts byte vector to string
std::string toString(const std::vector<unsigned char>& v) {
	return std::string(v.begin(), v.end());
}



	void SessionHandler::generateStoreKey(std::vector<unsigned char> sharedKey, std::vector<unsigned char> salt) {
		auto prk =  ecdhObj.kdfExtract(sharedKey, salt);
		const std::string info = "key-for-AES";
		std::vector<unsigned char> info_vec = toBytes(info);
		std::vector<unsigned char> key = ecdhObj.kdfExpand(prk, info_vec, 32);
		keyObj.storeKey(key);

		std::string keyStr = toString(key);
		std::cout << "The key derrived is: " << keyStr << std::endl;
	}



	void SessionHandler::sendEncrypted(std::vector<unsigned char> &data) {
		encObj = EncryptFactory::getEncryptInstance(EncryptFactory::SupportedAlgo::AES_CBC256);
		encObj->initEncryption(keyObj);
		std::vector<unsigned char> cipher1 = encObj->updateEncryption(data);
		std::vector<unsigned char> cipher2 = encObj->finalEncryption();
		std::vector<unsigned char> pubInfo = encObj->getPubInfo();	//the IVlen length is 1 byte
		std::vector<unsigned char> cipherFinal;
		cipherFinal.insert(cipherFinal.end(), pubInfo.begin(), pubInfo.end());
		cipherFinal.insert(cipherFinal.end(), cipher1.begin(), cipher1.end());
		cipherFinal.insert(cipherFinal.end(), cipher2.begin(), cipher2.end());

		tlsObj->sendData(cipherFinal);
		
	}
	std::vector<unsigned char> SessionHandler::getDecrypted() {
		std::vector<unsigned char> rawData = tlsObj->receiveData();
		uint16_t pubInfoSize = (static_cast<uint16_t>(rawData[0]) << 8) | rawData[1];
		//to check
		if (rawData.size() < pubInfoSize) {
			throw std::runtime_error("Received data too short");
		}

		// seperate the pubInfo andd the ciphertext
		std::vector<unsigned char> pubInfo(rawData.begin(),
			rawData.begin() + pubInfoSize);

		std::vector<unsigned char> cipherText(rawData.begin() + pubInfoSize,
			rawData.end());
		if (cipherText.empty()) {
			throw std::runtime_error("No ciphertext found after pubInfo");
		}
		decObj = DecryptFactory::getDecryptInstance(DecryptFactory::supprtedAlgo::AES_CBC256);
		decObj->initDecryption(pubInfo, keyObj);
		std::vector<unsigned char> plainText1 = decObj->updateDecryption(cipherText);
		std::vector<unsigned char> plainText2 = decObj->finalDecryption();
		std::vector<unsigned char> plainTextFinal;
		plainTextFinal.insert(plainTextFinal.end(), plainText1.begin(), plainText1.end());
		plainTextFinal.insert(plainTextFinal.end(), plainText2.begin(), plainText2.end());

		return plainTextFinal;

	}


	SessionHandleServer::SessionHandleServer(uint16_t portNum, const char* certFile, const char* privKeyFile, const char* caFile) {
		tlsObj = std::make_unique<TLSServer>(certFile, privKeyFile, TLSRole::SERVER, caFile);
		this->portNum = portNum;
	}


	void SessionHandleServer::performHandshake(){
		static_cast<TLSServer*>(tlsObj.get())->performHandshakeServer(portNum);	
		ecdhObj.generateKeypair();
		std::vector<unsigned char> pubKey = ecdhObj.serializePubKey();

		tlsObj->sendData(pubKey);
		std::vector<unsigned char> peerKey = tlsObj->receiveData();

		std::vector<unsigned char> sharedKey = ecdhObj.generateSharedKey(peerKey);
		std::vector<unsigned char> salt = ECDHHandle::getSalt(32);//TODO Make this length dependant on the encryption used
		tlsObj->sendData(salt);
		generateStoreKey(sharedKey, salt);
	}


	//note that the session cannot be reset using this same pointer
	SessionHandleClient::SessionHandleClient(uint16_t portNum, const char* host, uint16_t serverPort, const char* certFile, const char* privKeyFile, const char* caFile) {
		tlsObj = std::make_unique<TLSClient>(certFile, privKeyFile, TLSRole::CLIENT, caFile);
		this->portNum = portNum;
		this->serverPort = serverPort;
		this->host = host;
	}

	void SessionHandleClient::performHandshake()  {
		/*TODO this should not call it before the server is listening, thats why its connecting to the old session and its all getting stuck after the 
		conncect in the SocketHandle as its not able to get the current session keys etc and hence its not able to re-do*/
		static_cast<TLSClient*>(tlsObj.get())->performHandshakeClient(portNum, host, serverPort);
		std::cout << "After TLS Handle client" << std::endl;
		ecdhObj.generateKeypair();
		std::vector<unsigned char> pubKey = ecdhObj.serializePubKey();
		std::cout << "Hi from performHanshake client side" << std::endl;
		//this send and recieve mechanism is made asymmetric so that both client and server do not wait for each other for the key at the same time
		std::vector<unsigned char> peerKey = tlsObj->receiveData();
		std::cout << "Recieved server data" << std::endl;
		tlsObj->sendData(pubKey);
		std::cout << "Sent my own data" << std::endl;
		
		std::vector<unsigned char> sharedKey = ecdhObj.generateSharedKey(peerKey);
		std::vector<unsigned char> salt = tlsObj->receiveData();
		generateStoreKey(sharedKey, salt);
	}