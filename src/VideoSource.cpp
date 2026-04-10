#include <opencv2/opencv.hpp>
#include <memory>
#include <string>
#include "VideoSource.hpp"

    WindowsCamera:: WindowsCamera(int id) {
            deviceID = id;
            cap = std::make_unique<cv::VideoCapture>();
        }

    bool WindowsCamera:: openCamera() {
            return cap->open(deviceID, cv::CAP_DSHOW);
    }

    bool WindowsCamera:: getFrame(cv::Mat& frame) {
        if (!cap || !cap->isOpened()) {
            return false;
        }

        return cap->read(frame);
    }
    float WindowsCamera:: getFrameHeight() {
        return cap->get(cv::CAP_PROP_FRAME_HEIGHT);
    }
    float WindowsCamera :: getFrameWidth() {
        return cap->get(cv::CAP_PROP_FRAME_WIDTH);
    }




    LinuxCamera :: LinuxCamera(int id) {
        deviceID = id;
        cap = std::make_unique<cv::VideoCapture>();
    }

    bool LinuxCamera::openCamera() {
        return cap->open(deviceID, cv::CAP_GSTREAMER);
    }

    bool LinuxCamera::getFrame(cv::Mat& frame) {
        if (!cap || !cap->isOpened()) {
            return false;
        }

        return cap->read(frame);
    }
    float LinuxCamera::getFrameHeight() {
        return cap->get(cv::CAP_PROP_FRAME_HEIGHT);
    }
    float LinuxCamera::getFrameWidth() {
        return cap->get(cv::CAP_PROP_FRAME_WIDTH);
    }


    VideoFile:: VideoFile(const std::string& fp) {
        filepath = fp;
        cap = std::make_unique<cv::VideoCapture>();
    }

    bool VideoFile::openCamera() {
        return cap->open(filepath);
    }

    bool VideoFile:: getFrame(cv::Mat& frame) {
        if (!cap || !cap->isOpened()) {
            return false;
        }
        return cap->read(frame);
    }
    float VideoFile::getFrameHeight() {
        return cap->get(cv::CAP_PROP_FRAME_HEIGHT);
    }
    float VideoFile::getFrameWidth() {
        return cap->get(cv::CAP_PROP_FRAME_WIDTH);
    }


    std::unique_ptr<VideoSource> CameraFactory:: createSource(int id, CameraFactory::Platform type, const std::string& path) {
        switch (type) {
        case Platform::Windows:
            return std::make_unique<WindowsCamera>(id);
        case Platform::Linux:
            return std::make_unique<LinuxCamera>(id);
        case Platform::File:
            return std::make_unique<VideoFile>(path);
        default:
            return nullptr;
        }
    }
