#ifndef DECRYPTHANDLE_HPP
#define DECRYPTHANDLE_HPP

#include<openssl/evp.h>
#include<openssl/rand.h>
#include <iostream>
#include <vector>
#include "KeyStore.hpp"

class SymmetricDecrypt {
protected:
	std::vector<unsigned char> acceptKey(KeyStore& k);
public:
	virtual void initDecryption(std::vector<unsigned char>& pubInfo, KeyStore &k) = 0;
	virtual std::vector<unsigned char>updateDecryption(std::vector<unsigned char>&) = 0;
	virtual std::vector<unsigned char> finalDecryption() = 0;

};

class AES_CBC256_Dec :public SymmetricDecrypt {
private:
	std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx{ nullptr, EVP_CIPHER_CTX_free };

public:
	void initDecryption(std::vector<unsigned char>& pubInfo, KeyStore &k)override;
	std::vector<unsigned char>updateDecryption(std::vector<unsigned char>& input);
	std::vector<unsigned char> finalDecryption() override;
};

class DecryptFactory {
public:
	enum class supprtedAlgo {
		AES_CBC256
	};
	static std::unique_ptr<SymmetricDecrypt> getDecryptInstance(supprtedAlgo algo);
};

#endif // !DECRYPTHANDLE_HPP
