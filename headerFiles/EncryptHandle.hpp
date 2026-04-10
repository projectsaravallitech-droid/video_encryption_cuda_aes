#ifndef ENCRYPTHANDLE_HPP
#define ENCRYPTHANDLE_HPP
#include<openssl/evp.h>
#include <iostream>
#include<memory>
#include <vector>
#include"KeyStore.hpp"

class SymmetricEncrypt {
	//this will act as the abstract base class with pure virtual function
protected:
	std::vector<unsigned char> acceptKey(KeyStore& k);
public:
	virtual void initEncryption(KeyStore& k) = 0;
	virtual std::vector<unsigned char>updateEncryption(std::vector<unsigned char>&) = 0;
	virtual std::vector<unsigned char> finalEncryption() = 0;
	virtual std::vector<unsigned char> getPubInfo() = 0;
};


class AES_CBC256_Enc : public SymmetricEncrypt {
private:
	std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx{ nullptr, EVP_CIPHER_CTX_free };
	std::vector<unsigned char> iv;

public:
	void initEncryption(KeyStore& k) override;
	std::vector<unsigned char> updateEncryption(std::vector<unsigned char>& input)override;
	std::vector<unsigned char> finalEncryption()override;
	std::vector<unsigned char> getPubInfo()override;
};

class EncryptFactory {
public:
	enum class SupportedAlgo {
		AES_CBC256
	};
	static std::unique_ptr<SymmetricEncrypt> getEncryptInstance(SupportedAlgo algo);
};



#endif // !ENCRYPTHANDLE_HPP
