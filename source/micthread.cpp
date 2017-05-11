#include "micthread.h"

micThread::micThread(MicCapture *micCapture)
{
    this->moveToThread(this);
    this->m_micCapture = micCapture;
}

void micThread::run(){
    while(true){
        this->m_micCapture->capture();
        QThread::msleep(5);
    }
}
