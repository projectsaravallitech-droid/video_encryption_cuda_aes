#include<openssl/evp.h>
#include<openssl/rand.h>
#include <iostream>
#include <vector>
#include<fstream>
#include "KeyStore.hpp"
#include "SessionHandle.hpp"
#include "DecryptHandle.hpp"


	std::vector<unsigned char> SymmetricDecrypt::acceptKey(KeyStore &k) {
		return k.getKey();
	}

	void AES_CBC256_Dec::initDecryption(std::vector<unsigned char>& pubInfo, KeyStore &k){
		
		//logic for getting the IV of 1 byte
		if (pubInfo.size() < 2) {
			throw std::runtime_error("pubInfo too short to contain totalSize prefix");
		}

		// --- step 2: read and skip the 2 byte totalSize prefix ---
		// we already used this in getDecrypted() to slice, but it is still part of pubInfo
		uint16_t totalSize = (static_cast<uint16_t>(pubInfo[0]) << 8) | pubInfo[1];
		size_t offset = 2; // move past the 2 byte prefix

		// --- step 3: read ivLen (1 byte) ---
		if (pubInfo.size() < offset + 1) {
			throw std::runtime_error("pubInfo too short to contain ivLen");
		}
		uint8_t ivLen = pubInfo[offset];
		offset += 1; // move past ivLen

		// --- step 4: sanity check and read iv bytes ---
		if (pubInfo.size() < offset + ivLen) {
			throw std::runtime_error("pubInfo too short to contain IV of length " +
				std::to_string(ivLen));
		}
		std::vector<unsigned char> iv(pubInfo.begin() + offset,
			pubInfo.begin() + offset + ivLen);
		offset += ivLen; // move past iv

		std::vector<unsigned char> key = acceptKey(k);
		ctx.reset(EVP_CIPHER_CTX_new());

		EVP_DecryptInit_ex(ctx.get(), EVP_aes_256_cbc(), NULL, key.data(), iv.data());
	}

	std::vector<unsigned char>AES_CBC256_Dec::updateDecryption(std::vector<unsigned char>& input){
		std::vector<unsigned char> output;
		output.resize(input.size());
		int outLen = 0;
		EVP_DecryptUpdate(ctx.get(), output.data(), &outLen, input.data(), input.size());

		output.resize(outLen); //ensuring exact space, no extra bytes as we had allocated earier remain
		return output;
	}
	std::vector<unsigned char> AES_CBC256_Dec::finalDecryption(){
		std::vector<unsigned char> output;
		output.resize(EVP_MAX_BLOCK_LENGTH);
		int outLen = 0;
		EVP_DecryptFinal_ex(ctx.get(), output.data(), &outLen);

		output.resize(outLen);
		return output;
	}

 std::unique_ptr<SymmetricDecrypt> DecryptFactory::getDecryptInstance(supprtedAlgo algo) {

		if (algo == supprtedAlgo::AES_CBC256) {
			return std::make_unique<AES_CBC256_Dec>();
		}

	}
