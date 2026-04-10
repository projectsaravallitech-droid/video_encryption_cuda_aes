//Actual video streaming code
#include <opencv2/opencv.hpp>
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include<chrono>
#include <functional>
#include "VideoSource.hpp"
#include "WindowButton.hpp"
#include "SessionHandle.hpp"

//change this path for cert files in the "cert" folder in the git repo
static const char* SERVER_CERT = "D:\\Internship_AravalliTech\\certs\\server.crt";
static const char* SERVER_KEY = "D:\\Internship_AravalliTech\\certs\\server.key";
static const char* CLIENT_CERT = "D:\\Internship_AravalliTech\\certs\\client.crt";
static const char* CLIENT_KEY = "D:\\Internship_AravalliTech\\certs\\client.key";
static const char* CA_FILE = "D:\\Internship_AravalliTech\\certs\\ca.crt";
static const uint16_t PORT = 8443;
std::vector<unsigned char> EOS_MARKER = { 0xFF, 0xFE, 0xFD, 0xFC };

int main() {
    
    bool cameraStatus = false;
    //testing windows camera
    std::unique_ptr<VideoSource> cap;
    cap = CameraFactory::createSource(0, CameraFactory::Platform::Windows); //0 for the inbuilt camera
    cameraStatus = cap->openCamera();
    cv::Mat frame;
    bool startRec = false;
    std::unique_ptr<SessionHandler> server;

    int frameHeight = cap->getFrameHeight();
    int frameWidth = cap->getFrameWidth();


    WindowHandler window("VideoInput");

    window.addButton((frameWidth / 2 - 60), (frameHeight - 80), 40, cv::Scalar(40, 200, 40), [&]() {
        if (!startRec) {
            server = std::make_unique<SessionHandleServer>(PORT, SERVER_CERT, SERVER_KEY, CA_FILE);
            std::cout << "Before handshake" << std::endl;
            server->performHandshake();
            std::cout << "After handshake" << std::endl;
        }
        startRec = true;
        });


    window.addButton((frameWidth / 2 + 60), (frameHeight - 80), 40, cv::Scalar(40, 40, 200), [&]() {
        
        if (startRec) {
            startRec = false;
            server->sendEncrypted(EOS_MARKER);
            server.reset();
            std::cout << "Stop button pressed, status: " << startRec << std::endl;
        }
        });
    auto start = std::chrono::steady_clock::now();
    double time_elapsed = 0;
    int frame_count = 0;
    float fps = 0;
    while (1) {

        cap->getFrame(frame);
        frame_count++;
        auto end = std::chrono::steady_clock::now();
        time_elapsed = std::chrono::duration<double>(end - start).count();
        if (time_elapsed >= 1.0) {
            fps = frame_count / time_elapsed;
            frame_count = 0;
            time_elapsed = 0;
            start = std::chrono::steady_clock::now();
        }

        std::string text = "FPS: " + std::to_string(fps);

        window.display(frame, cameraStatus, text);

        if (startRec) {
            std::vector<unsigned char> dataBuffer;
            cv::imencode(".jpg", frame, dataBuffer);
            server->sendEncrypted(dataBuffer);  
        }

        char key = window.waitKey(1);
        if (key == 'q' || key == 27) {
            if (startRec && server) {
                server->sendEncrypted(EOS_MARKER);
                server.reset();
            }
            break;
        }
    }
    return 0;
}


//Benchmarking the encrypt function
//#include <opencv2/opencv.hpp>
//#include <memory>
//#include <string>
//#include <iostream>
//#include <vector>
//#include <functional>
//#include "VideoSource.hpp"
//#include "SessionHandle.hpp"
//#include "Benchmarking.hpp"
//
//static const char* SERVER_CERT = "D:\\Internship_AravalliTech\\certs\\server.crt";
//static const char* SERVER_KEY = "D:\\Internship_AravalliTech\\certs\\server.key";
//static const char* CLIENT_CERT = "D:\\Internship_AravalliTech\\certs\\client.crt";
//static const char* CLIENT_KEY = "D:\\Internship_AravalliTech\\certs\\client.key";
//static const char* CA_FILE = "D:\\Internship_AravalliTech\\certs\\ca.crt";
//static const uint16_t PORT = 8443;
//std::vector<unsigned char> EOS_MARKER = { 0xFF, 0xFE, 0xFD, 0xFC };
//
//
//
//int main() {
//
//	std::unique_ptr<VideoSource> camera = CameraFactory::createSource(CameraFactory::Platform::Windows);
//	std::unique_ptr<SessionHandler> server = std::make_unique<SessionHandleServer>(PORT, SERVER_CERT, SERVER_KEY, CA_FILE);
//	server->performHandshake();
//	
//	camera->openCamera();
//	cv::Mat frame, frame4K;
//	camera->getFrame(frame);
//
//	cv::resize(frame, frame4K, cv::Size(3840, 2160));
//
//	std::cout << "Camera opened and handshake done" << std::endl;
//	std::vector<unsigned char> buffer;
//	cv::imencode(".jpg", frame4K, buffer);	//Frame is resized to simulate the 4K resolution image that will be there in future
//
//	auto funct = [&]() {server->sendEncrypted(buffer); };
//	double latency = Benchmarking::getLatency(funct, 100);
//	server->sendEncrypted(EOS_MARKER);
//	std::cout << "Latency of sendEncrypted is : " << latency << " ms" << std::endl;	//its around 1.5ms
//
//	return 0;
//}