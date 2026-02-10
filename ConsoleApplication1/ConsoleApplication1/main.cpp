#include <opencv2/opencv.hpp>
#include <memory>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include "VideoSource.hpp"
#include "WindowButton.hpp"


int main() {
    bool cameraStatus = false;
    //testing windows camera
    std::unique_ptr<VideoSource> cap;
    cap = CameraFactory::createSource(CameraFactory::Platform(0));
    cameraStatus = cap->openCamera();
    cv::Mat frame;
    float fps = 15.00;  // TODO Make the fps measurement function to avoid hardcoding - inbuilt function from openCV isn't accurate

    bool startRec = false;

    int frameHeight = cap->getFrameHeight();
    int frameWidth = cap->getFrameWidth();


    //Storing the video - configuration
    int codec = cv::VideoWriter::fourcc('X', 'V', 'I', 'D');
    cv::String filename = "./output-video.avi";
    cv::VideoWriter writer;
  
   
    WindowHandler window("VideoInput");
    window.addButton((frameWidth/2 - 60), (frameHeight - 80), 40, cv::Scalar(40, 200, 40), [&]() {
        if (!startRec) {
            //create the writer
            writer.open(filename, codec, fps, cv::Size(frameWidth, frameHeight), true);
        }
        if (!writer.isOpened()) {
            std::cerr << "Error: VideoWriter failed to open\n";
            return -1;
        }
        startRec = true;
       });


    window.addButton((frameWidth / 2 + 60), (frameHeight - 80), 40, cv::Scalar(40, 40, 200), [&]() {
        if (startRec) {
            startRec = false;
            writer.release();
        }
        });

    while (1) {

        cap->getFrame(frame);

        window.display(frame, cameraStatus);


        if (startRec) {
            writer.write(frame);
        }

        char key = window.waitKey(1);
        if (key == 'q' || key == 27) {
            break;
        }
    }

    writer.release();

    return 0;
}