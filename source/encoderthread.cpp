#include "encoderthread.h"



encoderThread::encoderThread(MicCapture *micCapture, DesktopCapture *desktopCapture, AVFormatContext *outFormatContext )
{
    this->moveToThread(this);
    this->m_micCapture = micCapture;
    this->m_desktopCapture = desktopCapture;
    this->m_outFormatContext = outFormatContext;
}

void encoderThread::run(){
    while(true){
        bool isWriteVideo = true;
        do{
        if(av_compare_ts(m_desktopCapture->m_pts, m_outFormatContext->streams[m_desktopCapture->m_outStreamIndex]->time_base,
                    m_micCapture->m_pts, m_outFormatContext->streams[m_micCapture->m_outStreamIndex]->time_base) <= 0)
        {
            isWriteVideo = true;
            m_desktopCapture->encoder();
        }else{
            isWriteVideo = false;
            m_micCapture->encoder();
        }
        }while(isWriteVideo==false);
        QThread::msleep(5);
    }
}
