
#include "CameraManager.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <QDebug>

CameraManager::CameraManager(int cameraIndex, QObject *parent)
    : QObject(parent), cameraIndex(cameraIndex) {}

CameraManager::~CameraManager() {
    closeCamera();
}

bool CameraManager::openCamera() {
    if (!camera.open(cameraIndex)) {
        qWarning() << "Failed to open camera with index:" << cameraIndex;
        return false;
    }

    qDebug() << "Camera opened successfully.";
    return true;
}

void CameraManager::closeCamera() {
    if (camera.isOpened()) {
        camera.release();
        qDebug() << "Camera closed.";
    }
}

bool CameraManager::captureImage(const std::string &filename) {
    if (!camera.isOpened()) {
        qWarning() << "Camera is not opened.";
        return false;
    }

    cv::Mat frame;
    camera >> frame;

    if (frame.empty()) {
        qWarning() << "Failed to capture image.";
        return false;
    }

    if (!cv::imwrite(filename, frame)) {
        qWarning() << "Failed to save image to" << QString::fromStdString(filename);
        return false;
    }

    qDebug() << "Image saved to" << QString::fromStdString(filename);
    return true;
}

bool CameraManager::isCameraOpen() const {
    return camera.isOpened();
}


void CameraManager::setCameraIndex(int cameraIndex) {
    this->cameraIndex = cameraIndex;
}
