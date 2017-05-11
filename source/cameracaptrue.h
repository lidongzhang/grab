#ifndef CAMERACAPTRUE_H
#define CAMERACAPTRUE_H

#include <QString>
#include <QRect>
#include <QDebug>
#include <qwindowdefs.h>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavdevice/avdevice.h>
#include <libavutil/audio_fifo.h>
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libswresample/swresample.h"
#include <stdio.h>
}

class CameraCaptrue
{
public:
    CameraCaptrue();

private:
    AVFormatContext *m_formatContext;
    AVCodecContext *m_codecContext;
    AVFrame *m_frame;

    AVPacket *m_packet;
    SwsContext *m_swsContext;
    WId m_winId;
    QRect m_cameraShowVideoRect;
    int m_streamIndex;
public:
    bool open(QString formatName, QString inputName, WId winId, QRect cameraShowVideoRect);
    char* getCameraVideoBuffer();
    AVFrame *m_frameYUV;
    bool m_opened;
    void close();
};

#endif // CAMERACAPTRUE_H
