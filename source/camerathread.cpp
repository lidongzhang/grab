#include "camerathread.h"

cameraThread::cameraThread(CameraCaptrue *cameraCaptrue, VideoWindow *videoWindow)
{
    this->moveToThread(this);
    this->m_cameraCaptrue = cameraCaptrue;
    this->m_videoWindow = videoWindow;
}

void cameraThread::run(){
while(true){
    if ( global_param.camera_isShowVideo){
        m_videoWindow->displayCameraVideo(m_cameraCaptrue->getCameraVideoBuffer());
        QThread::msleep(5);
    }else{
        QThread::msleep(1000);
    }
}
}
