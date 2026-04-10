#include <opencv2/opencv.hpp>
#include <memory>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <cmath>
#include "WindowButton.hpp"


//Button class implementation

Button::Button(int xin, int yin, int radin, cv::Scalar color)
        : x(xin), y(yin), rad(radin)
        , colorNormal(color)
        , colorPressed(cv::Scalar(80, 80, 80))
        , isPressed(false)
        , onClick(nullptr)
    {
    }

    // Check if point is inside circular button
    bool Button:: contains(int px, int py) const {
        // Distance formula: sqrt((px - x)² + (py - y)²)
        int dx = px - x;
        int dy = py - y;
        int distanceSquared = dx * dx + dy * dy;
        int radiusSquared = rad * rad;

        return (distanceSquared <= radiusSquared);
    }

    // Draw the button on an image
    void Button::draw(cv::Mat& image) const {
        // Choose color based on state
        cv::Scalar color = colorNormal;
        if (isPressed) {
            color = colorPressed;
        }

        // Draw filled circle
        cv::circle(image, cv::Point(x, y), rad, color, -1);  // -1 = filled

        // Draw border
        cv::circle(image, cv::Point(x, y), rad, cv::Scalar(0, 0, 0), 2);  // 2 = thickness
    }

    // Getters (needed for mouse callback)
    int Button::getX() const { return x; }
    int Button::getY() const { return y; }
    int Button::getRadius() const { return rad; }





//WindowHandler class implementation

     void WindowHandler:: mouseCallbackStatic(int event, int x, int y, int flags, void* userdata) {
        // Cast userdata back to WindowHandler*
        WindowHandler* handler = static_cast<WindowHandler*>(userdata);
        handler->handleMouse(event, x, y, flags);
    }

    // Instance method to handle mouse (can access buttons vector)
    void WindowHandler::handleMouse(int event, int x, int y, int flags) {
        // Handle mouse button down
        if (event == cv::EVENT_LBUTTONDOWN) {
            for (auto& button : buttons) {
                if (button.contains(x, y)) {
                    button.isPressed = true;
                }
            }
        }

        // Handle mouse button up (complete click)
        if (event == cv::EVENT_LBUTTONUP) {
            for (auto& button : buttons) {
                if (button.isPressed && button.contains(x, y)) {
                    // Execute button action
                    if (button.onClick) {
                        button.onClick();
                    }
                }
                button.isPressed = false;
            }
        }
    }


    WindowHandler :: WindowHandler(std::string name) {
        try {
            m_windowName = name;
            cv::namedWindow(m_windowName);
            windowCreated = true;

            // Register callback with 'this' pointer
            cv::setMouseCallback(m_windowName, mouseCallbackStatic, this);
        }
        catch (...) {
            windowCreated = false;
            std::cerr << "Window not created successfully" << std::endl;
        }
    }

    WindowHandler:: ~WindowHandler() {
        if (windowCreated) {
            cv::destroyWindow(m_windowName);
        }
    }

    // Add button and store it
    void WindowHandler::addButton(int x, int y, int radius, cv::Scalar color, std::function<void()> callback) {
        Button button(x, y, radius, color);
        button.onClick = callback;
        buttons.push_back(button);
    }

    void WindowHandler::display(cv::Mat frame, bool cameraStatus) {
        /* TODO This should display the signal not found message when the frames are empty or the camera is not initialized, but due to internal drivers, I am getting some garbage value
       in the frame buffer when my camera is beign used by other app, so it misses both these checks and the buttons are drwan on a mostly black image with certain sharp RGB noise like patterns
        So, need to implement logic to detect these error frames and correctly display the error message on screen*/
        if (frame.empty() || !cameraStatus) {
            cv::Mat emptyMessage(windowHeight, windowWidth, CV_8UC3, cv::Scalar(0, 0, 0));
            std::string message = "Not Found";
            cv::Point textOrg(windowHeight / 2, 200);
            int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            cv::Scalar textColor(255, 0, 0);

            cv::putText(emptyMessage, message, textOrg, fontFace, 1, textColor, 2);
            cv::imshow(m_windowName, emptyMessage);
            return;
        }
        if (!windowCreated) {
            return;
        }

        // Clone frame and draw buttons on it
        cv::Mat displayFrame = frame.clone();

        // Draw all buttons
        for (const auto& button : buttons) {
            button.draw(displayFrame);
        }

        cv::imshow(m_windowName, displayFrame);
    }

    int WindowHandler::waitKey(int ms) {
        return cv::waitKey(ms);
    }








