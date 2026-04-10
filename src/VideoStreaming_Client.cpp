//Actual Video reception cod
#include <iostream>
#include <opencv2/opencv.hpp>
#include<chrono>
#include "TransportHandle.hpp"
#include"platform_socket.hpp"
#include "TLSHandle.hpp"
#include"ECDHHandle.hpp"
#include "SessionHandle.hpp"
#include"KeyStore.hpp"

static const char* SERVER_CERT = "D:\\Internship_AravalliTech\\certs\\server.crt";
static const char* SERVER_KEY = "D:\\Internship_AravalliTech\\certs\\server.key";
static const char* CLIENT_CERT = "D:\\Internship_AravalliTech\\certs\\client.crt";
static const char* CLIENT_KEY = "D:\\Internship_AravalliTech\\certs\\client.key";
static const char* CA_FILE = "D:\\Internship_AravalliTech\\certs\\ca.crt";
static const uint16_t PORT = 8443;
std::vector<unsigned char> EOS_MARKER = { 0xFF, 0xFE, 0xFD, 0xFC };

int main() {
	std::vector<unsigned char> rawData;
	cv::Mat frame;
	double time_elapsed;
	int frame_count = 0;
	double fps = 0;
	
	while(1){
		std::cout << "Waiting for the session to begin" << std::endl;
		std::unique_ptr<SessionHandler> client = std::make_unique<SessionHandleClient>(0, "127.0.0.1", PORT, CLIENT_CERT, CLIENT_KEY, CA_FILE);
		try {
			client->performHandshake();

			auto start = std::chrono::steady_clock::now();
			while (1) {
				rawData = client->getDecrypted();
				
				if (rawData == EOS_MARKER) {
					std::cout << "Stream Ended" << std::endl;
					break;
				}
				frame = cv::imdecode(rawData, cv::IMREAD_COLOR);

				if (frame.empty()) {
					std::cerr << "Warning: failed to decode frame, skipping\n";
					continue;
				}
				frame_count++;
				auto end = std::chrono::steady_clock::now();
				time_elapsed = std::chrono::duration<double>(end - start).count();
				if (time_elapsed >= 1) {
					std::cout << frame_count;
					fps = frame_count / time_elapsed;
					frame_count = 0;
					time_elapsed = 0;
					start = std::chrono::steady_clock::now();
				}

				//Here the FPS measured is with the decryption delay, as observed. 15 FPS is obtained as around 8 FPS due to decrypt delay
				std::string text = "FPS: " + std::to_string(float(fps));
				cv::putText(frame, text, cv::Point(10, 40), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0), 2);
				cv::imshow("Obtained frames", frame);

				if (cv::waitKey(1) == 27) {  // ESC to manually exit
					cv::destroyAllWindows();
					return 0;
				}

			}
		}
		catch (std::exception& e) {
			std::cerr << "error in the session" << e.what();
		}
		
		cv::destroyAllWindows();
		client.reset();
	}

	return 0;
}



////Benchmarking
//#include <iostream>
//#include <opencv2/opencv.hpp>
//#include "TransportHandle.hpp"
//#include"platform_socket.hpp"
//#include "TLSHandle.hpp"
//#include"ECDHHandle.hpp"
//#include "SessionHandle.hpp"
//#include"KeyStore.hpp"
//
//static const char* SERVER_CERT = "D:\\Internship_AravalliTech\\certs\\server.crt";
//static const char* SERVER_KEY = "D:\\Internship_AravalliTech\\certs\\server.key";
//static const char* CLIENT_CERT = "D:\\Internship_AravalliTech\\certs\\client.crt";
//static const char* CLIENT_KEY = "D:\\Internship_AravalliTech\\certs\\client.key";
//static const char* CA_FILE = "D:\\Internship_AravalliTech\\certs\\ca.crt";
//static const uint16_t PORT = 8443;
//std::vector<unsigned char> EOS_MARKER = { 0xFF, 0xFE, 0xFD, 0xFC };
//
//int main() {
//	std::unique_ptr<SessionHandler> client = std::make_unique<SessionHandleClient>(0, "127.0.0.1", PORT, CLIENT_CERT, CLIENT_KEY, CA_FILE);
//	client->performHandshake();
//	std::vector<unsigned char> rawData;
//	while (1) {
//		rawData = client->getDecrypted();
//		if (rawData == EOS_MARKER) {
//			std::cout << "End of stream" << std::endl;
//			break;
//		}
//
//	}
//}