
#include "CameraManager.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <QDebug>

CameraManager::CameraManager(int cameraIndex, QObject *parent)
    : QObject(parent), cameraIndex(cameraIndex) {}

CameraManager::~CameraManager()
{
    closeCamera();
}

/**
 * @brief 打开摄像头
 * @return bool
 */
bool CameraManager::openCamera()
{
    if (!camera.open(cameraIndex))
    {
        qWarning() << "Failed to open camera with index:" << cameraIndex;
        return false;
    }

    qDebug() << "Camera opened successfully.";
    return true;
}
/**
 * @brief 关闭摄像头
 * @return void
 */
void CameraManager::closeCamera()
{
    if (camera.isOpened())
    {
        camera.release();
        qDebug() << "Camera closed.";
    }
}
/**
 * @brief 拍照
 * @param filename 文件名
 * @return bool 是否拍照成功
 */
bool CameraManager::captureImage(const std::string &filename)
{
    if (!camera.isOpened())
    {
        qWarning() << "Camera is not opened.";
        return false;
    }

    cv::Mat frame;
    camera >> frame;

    if (frame.empty())
    {
        qWarning() << "Failed to capture image.";
        return false;
    }

    if (!cv::imwrite(filename, frame))
    {
        qWarning() << "Failed to save image to" << QString::fromStdString(filename);
        return false;
    }

    qDebug() << "Image saved to" << QString::fromStdString(filename);
    return true;
}

/**
 * @brief 判断摄像头是否打开
 * @return bool 是否打开
 */
bool CameraManager::isCameraOpen() const
{
    return camera.isOpened();
}
/**
 * @brief 设置摄像头索引
 * @param cameraIndex 摄像头索引
 */
void CameraManager::setCameraIndex(int cameraIndex)
{
    this->cameraIndex = cameraIndex;
}
