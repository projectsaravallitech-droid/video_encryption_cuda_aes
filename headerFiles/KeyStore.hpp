#ifndef KEYSTORE_HPP
#define KEYSTORE_HPP
#include<vector>

class SymmetricEncrypt;
class SymmetricDecrypt;

class KeyStore {
private:
	std::vector<unsigned char> key;
	friend class SymmetricEncrypt;
	friend class SymmetricDecrypt;
	std::vector<unsigned char> getKey();
public:
	void storeKey(std::vector<unsigned char>& sharedKey);
	~KeyStore();
};


#endif // !KEYSTORE_HPP
