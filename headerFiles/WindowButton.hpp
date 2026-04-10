#pragma once

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <string>
#include <vector>
#include <functional>
#include <iostream>


class Button {
private:
    int x, y;
    int rad;

public:
    // Visual properties
    cv::Scalar colorNormal;
    cv::Scalar colorPressed;

    // State
    bool isPressed;

    // Action callback
    std::function<void()> onClick;

    Button(int xin, int yin, int radin, cv::Scalar color);

    // Check if point is inside circular button
    bool contains(int px, int py) const;

    // Draw the button on an image
    void draw(cv::Mat& image) const;

    // Getters (needed for mouse callback)
    int getX() const;
    int getY() const;
    int getRadius() const;
};


class WindowHandler {
private:
    std::string m_windowName;
    int windowHeight = 480;
    int windowWidth = 640;
    bool windowCreated;
    std::vector<Button> buttons;

    // Static callback for OpenCV
    static void mouseCallbackStatic(int event, int x, int y,
        int flags, void* userdata);

    // Instance method to handle mouse
    void handleMouse(int event, int x, int y, int flags);

public:
    explicit WindowHandler(std::string name);
    ~WindowHandler();

    // Add button and store it
    void addButton(int x, int y, int radius,
        cv::Scalar color,
        std::function<void()> callback);

    void display(cv::Mat frame, bool cameraStatus);
    int waitKey(int ms);
};
