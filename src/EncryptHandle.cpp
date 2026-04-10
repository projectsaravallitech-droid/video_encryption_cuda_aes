#include<openssl/evp.h>
#include<openssl/rand.h>
#include <iostream>
#include <vector>
#include<fstream>
#include<memory>
#include "SessionHandle.hpp"
#include"KeyStore.hpp"
#include "EncryptHandle.hpp"


	std::vector<unsigned char> SymmetricEncrypt::acceptKey(KeyStore &k) {
		return k.key;
	} 

	void  AES_CBC256_Enc::initEncryption(KeyStore &k) {
		//TODO Error handling for the random bytes generation
		iv.resize(16);
		if (RAND_bytes(iv.data(), 16) != 1) {
			throw std::runtime_error("Error in random bytes generation");
		}

		ctx.reset(EVP_CIPHER_CTX_new());
		std::vector<unsigned char> aesKey = acceptKey(k);
		if(EVP_EncryptInit_ex(ctx.get(), EVP_aes_256_cbc(), NULL, aesKey.data(), iv.data()) != 1) {
			throw std::runtime_error("Error in CBC encryption init");
		}
	}

	std::vector<unsigned char> AES_CBC256_Enc::updateEncryption(std::vector<unsigned char> &input){
		std::vector<unsigned char> output;
		output.resize(input.size());
		int outLen = 0;
		if(EVP_EncryptUpdate(ctx.get(), output.data(), &outLen, input.data(), input.size()) != 1) {
			throw std::runtime_error("Error in CBC encryption update");
		}

		output.resize(outLen); //ensuring exact space, no extra bytes as we had allocated earier remain
		return output;
	}

	std::vector<unsigned char> AES_CBC256_Enc::finalEncryption() {
		std::vector<unsigned char> output;
		output.resize(EVP_MAX_BLOCK_LENGTH);
		int outLen = 0;
		if(EVP_EncryptFinal_ex(ctx.get(), output.data(), &outLen) != 1) {
			throw std::runtime_error("Error in CBC encryption finalization");
		}

		output.resize(outLen);
		return output;
	}

	std::vector<unsigned char> AES_CBC256_Enc::getPubInfo() {
		std::vector<unsigned char> pubInfo;

		static_assert(EVP_MAX_IV_LENGTH <= 255, "IV too large for 1-byte prefix");

		// build payload first so we know its size before writing the prefix
		std::vector<unsigned char> payload;
		payload.push_back(static_cast<unsigned char>(iv.size())); // ivLen: 1 byte
		payload.insert(payload.end(), iv.begin(), iv.end());      // iv bytes

		// total = 2 bytes (the prefix itself) + payload size
		uint16_t totalSize = static_cast<uint16_t>(2 + payload.size());

		// write 2 byte prefix using shifts
		pubInfo.push_back(static_cast<unsigned char>(totalSize >> 8));   // high byte
		pubInfo.push_back(static_cast<unsigned char>(totalSize & 0xFF)); // low byte

		// append the payload
		pubInfo.insert(pubInfo.end(), payload.begin(), payload.end());

		return pubInfo;
	}


	std::unique_ptr<SymmetricEncrypt> EncryptFactory::getEncryptInstance(SupportedAlgo algo) {
		if (algo == SupportedAlgo::AES_CBC256) {
			return std::make_unique<AES_CBC256_Enc>();
		}
		else {
			throw std::invalid_argument("Unsupported algorithm requested");
		}
	}