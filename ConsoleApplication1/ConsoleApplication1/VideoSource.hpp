#pragma once

#include <opencv2/opencv.hpp>
#include <memory>
#include <string>


class VideoSource {
public:
    virtual ~VideoSource() = default;

    virtual bool openCamera() = 0;
    virtual bool getFrame(cv::Mat& frame) = 0;
    virtual float getFrameHeight() = 0;
    virtual float getFrameWidth() = 0;
};


class WindowsCamera : public VideoSource {
private:
    std::unique_ptr<cv::VideoCapture> cap;
    int deviceID;

public:
    explicit WindowsCamera(int id);

    bool openCamera() override;
    bool getFrame(cv::Mat& frame) override;
    float getFrameHeight() override;
    float getFrameWidth() override;
};


class LinuxCamera : public VideoSource {
private:
    std::unique_ptr<cv::VideoCapture> cap;
    int deviceID;

public:
    explicit LinuxCamera(int id);

    bool openCamera() override;
    bool getFrame(cv::Mat& frame) override;
    float getFrameHeight() override;
    float getFrameWidth() override;
};


class VideoFile : public VideoSource {
private:
    std::unique_ptr<cv::VideoCapture> cap;
    std::string filepath;

public:
    explicit VideoFile(const std::string& fp);

    bool openCamera() override;
    bool getFrame(cv::Mat& frame) override;
    float getFrameHeight() override;
    float getFrameWidth() override;
};


class CameraFactory {
public:
    enum class Platform {
        Windows,
        Linux,
        File
    };

    static std::unique_ptr<VideoSource>
        createSource(Platform type, const std::string& path = "");
};
