#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <QObject>

class CameraManager : public QObject {
    Q_OBJECT

public:
    explicit CameraManager(int cameraIndex = 0, QObject *parent = nullptr);
    ~CameraManager();

    bool openCamera();
    void closeCamera();
    bool captureImage(const std::string &filename);
    bool isCameraOpen() const;
    void setCameraIndex(int cameraIndex);

private:
    int cameraIndex;
    cv::VideoCapture camera;
};

#endif // CAMERAMANAGER_H
