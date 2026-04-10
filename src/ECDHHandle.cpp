#include <openssl/evp.h>
#include <vector>
#include<openssl/rand.h>
#include <iostream>
#include <openssl/kdf.h>
#include <openssl/core_names.h>
#include <stdexcept>
#include "ECDHHandle.hpp"

ECDHHandle::ECDHHandle() {
		ctx.reset(EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, NULL));
}

ECDHHandle::~ECDHHandle() {
		if (keypair) {
			EVP_PKEY_free(keypair);
		}
}

void ECDHHandle:: generateKeypair() {
		if (EVP_PKEY_keygen_init(ctx.get()) <= 0) {
			throw std::runtime_error("Failure in initialization of ECDH keypair generation process");
		}

		if (EVP_PKEY_keygen(ctx.get(), &keypair) <= 0) {
			throw std::runtime_error("Failure in generating ECDH keypair");
		}
}


std::vector<unsigned char> ECDHHandle:: generateSharedKey(const std::vector<unsigned char> &peerPubKey) {
		if (peerPubKey.size() != 32) {
			throw std::runtime_error("ECDH key size is not 32 bytes, hence terminating");
		}
		//this pubkey is an unsigned char I need that in form of EVP_PKEY
		EVP_PKEY* peer_key =
			EVP_PKEY_new_raw_public_key(
				EVP_PKEY_X25519,
				NULL,
				peerPubKey.data(),
				32
			);
		if (!peer_key) {
			throw std::runtime_error("Failed to create peer key object");
		}

		
		EVP_PKEY_CTX* derive_ctx = EVP_PKEY_CTX_new(keypair, NULL);
		if (!derive_ctx) {
			throw std::runtime_error("Failed to create context in shared secret derivation");
		}
		if (EVP_PKEY_derive_init(derive_ctx) <= 0 || EVP_PKEY_derive_set_peer(derive_ctx, peer_key) <= 0) {
			EVP_PKEY_CTX_free(derive_ctx);
			EVP_PKEY_free(peer_key);
			throw std::runtime_error("Failed to initilize derived key context or set peer for derived key");
		}

		std::vector<unsigned char> secret;
		size_t secret_len;

		if (EVP_PKEY_derive(derive_ctx, NULL, &secret_len) <= 0) {
			EVP_PKEY_CTX_free(derive_ctx);
			EVP_PKEY_free(peer_key);
			throw std::runtime_error("Failed to get the length of secret key");
		}

		secret.resize(secret_len);


		if (EVP_PKEY_derive(derive_ctx, secret.data(), &secret_len) <= 0) {
			EVP_PKEY_CTX_free(derive_ctx);
			EVP_PKEY_free(peer_key);
			throw std::runtime_error("Failed to get the shared secret");
		}

		EVP_PKEY_CTX_free(derive_ctx);
		EVP_PKEY_free(peer_key);

		return secret;
}

std::vector<unsigned char> ECDHHandle::serializePubKey() {
		size_t pubKeyLen = 32;   //as we are using the curve2219, the key size is 32 bytes
		std::vector<unsigned char> pubKey(pubKeyLen);

		if (EVP_PKEY_get_raw_public_key(keypair, pubKey.data(), &pubKeyLen) <= 0) {
			throw std::runtime_error("Failed to serialize the public key of ECDH");
		}
		pubKey.resize(pubKeyLen);
		return pubKey;
	
}

std::vector<unsigned char> ECDHHandle:: kdfExtract(std::vector<unsigned char>& sharedSecret, std::vector<unsigned char>& salt){
		std::vector<unsigned char> prk(32);  //Pseudo random number obtained after extraaction - as SHA256 is being used, its 32 bytes output
		EVP_KDF* kdf = EVP_KDF_fetch(NULL, "HKDF", NULL);//fetch the algorithm = HKDF
		if (!kdf) throw std::runtime_error("HKDF fetch failed");

		EVP_KDF_CTX* ctx = EVP_KDF_CTX_new(kdf);//create context for the KDF 
		EVP_KDF_free(kdf);
		if (!ctx) throw std::runtime_error("HKDF ctx failed");

		int mode = EVP_KDF_HKDF_MODE_EXTRACT_ONLY;//default is EXTRACT_AND_EXPAND

		//now we need a parameter list for the kdf implementation
		OSSL_PARAM params[] = {
			OSSL_PARAM_construct_utf8_string(OSSL_KDF_PARAM_DIGEST, (char*)"SHA256", 0),
			OSSL_PARAM_construct_int(OSSL_KDF_PARAM_MODE, &mode),
			OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, (void*)salt.data(), salt.size()),
			OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_KEY, (void*)sharedSecret.data(), sharedSecret.size()),
			OSSL_PARAM_construct_end() };
		

		if (EVP_KDF_derive(ctx, prk.data(), prk.size(), params) <= 0) {
			EVP_KDF_CTX_free(ctx);
			throw std::runtime_error("HKDF extract failed");
		}

		EVP_KDF_CTX_free(ctx);
		return prk;
}



std::vector<unsigned char> ECDHHandle:: kdfExpand(std::vector<unsigned char>& prk, std::vector<unsigned char>& info, int outLen) {
		//This function is seperated as we may need to derive IV, AES-KEY from the single shared secret, hence using different info, different keys can be derived

		std::vector<unsigned char> outKey(outLen);
		EVP_KDF* kdf = EVP_KDF_fetch(NULL, "HKDF", NULL);
		if (!kdf) {
			throw std::runtime_error("failed to fetch the kdf instance");
		}

		EVP_KDF_CTX* ctx = EVP_KDF_CTX_new(kdf);
		EVP_KDF_free(kdf);
		if (!ctx) {
			throw std::runtime_error("failed to load the context for kdf");
		}

		int mode = EVP_KDF_HKDF_MODE_EXPAND_ONLY;

		//now parameters for key expansion
		OSSL_PARAM params[]{
			OSSL_PARAM_construct_utf8_string(OSSL_KDF_PARAM_DIGEST, (char*)"SHA256", 0),
			OSSL_PARAM_construct_int(OSSL_KDF_PARAM_MODE, &mode),
			OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_KEY, (void*)prk.data(), prk.size()),
			OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_INFO, (void*)info.data(), info.size()),
			OSSL_PARAM_construct_end()
		};

		//now derive the output key
		if (EVP_KDF_derive(ctx, outKey.data(), outKey.size(), params) <= 0) {
			EVP_KDF_CTX_free(ctx);
			throw std::runtime_error("Failed to expand the prk");
		}
		EVP_KDF_CTX_free(ctx);
		return outKey;

}

std::vector<unsigned char>ECDHHandle::getSalt(int len) {
	std::vector<unsigned char> buffer(len);
	if(RAND_bytes(buffer.data(), len) != 1) {
		throw std::runtime_error("Error in salt generation");
	}
	return buffer;

}
