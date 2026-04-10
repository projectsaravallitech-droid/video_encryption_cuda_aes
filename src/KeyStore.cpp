#include<iostream>
#include"KeyStore.hpp"


void KeyStore::storeKey(std::vector<unsigned char>& sharedKey) {
	this->key = sharedKey;
	//TODO CRITIAL Uncomment the below line after demostrantion of the keys, as the key zeroization is important
	std::fill(sharedKey.begin(), sharedKey.end(), 0);
}

std::vector<unsigned char> KeyStore::getKey() {
	return key;
}

KeyStore::~KeyStore() {
	//performing zeroisation of the key here, according to the protocol defined , industry standard
	std::fill(key.begin(), key.end(), 0);


}