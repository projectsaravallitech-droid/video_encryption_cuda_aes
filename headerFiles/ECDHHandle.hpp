#ifndef ECDHHANDLE_HPP
#define ECDHHANDLE_HPP

#include <openssl/evp.h>
#include <vector>
#include <memory>


class ECDHHandle {
private:
	std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)> ctx{ nullptr, EVP_PKEY_CTX_free };
	EVP_PKEY* keypair = nullptr;
public:

	ECDHHandle();
	~ECDHHandle();

	void generateKeypair();

	//TODO there will be changes here, in the arguments of the below two functions 
	std::vector<unsigned char> generateSharedKey(const std::vector<unsigned char> &peerPubKey);
	std::vector<unsigned char> serializePubKey();
	std::vector<unsigned char> kdfExtract(std::vector<unsigned char>& sharedSecret, std::vector<unsigned char>& salt);
	std::vector<unsigned char> kdfExpand(std::vector<unsigned char>& prk, std::vector<unsigned char>& info, int outLen);
	static std::vector<unsigned char> getSalt(int len);

};


#endif // !ECDHHANDLE_HPP
