#ifndef DESKTOPCAPTURE_H
#define DESKTOPCAPTURE_H

#include <QString>
#include <QRect>
#include <QDebug>
#include <QMutex>

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

class DesktopCapture
{
public:
    DesktopCapture();

private:
    AVFormatContext *m_formatContext;
    AVCodecContext *m_codecContext;
    AVFormatContext *m_outFormatContext;
    AVCodecContext *m_outCodecContext;
    AVFrame *m_frame;
    AVFrame *m_frameYUV;
    AVFrame *m_outFrame;
    uint8_t* m_outFrameBuf;
    AVPacket *m_packet;
    AVPacket *m_outPacket;
    SwsContext *m_swsContext;
    QRect m_captureRect;

    QMutex m_mute;
    long m_frameIndex;
public:
    AVFifoBuffer *m_fifo;
    int64_t m_pts;
    int m_streamIndex;
    int m_outStreamIndex;
    bool open(QString formatName, QString inputName, AVFormatContext *outFormatContext,
              QString captrueMode, QRect captrueRect, QSize screenSize);
    bool capture();
    bool encoder();
    bool flushEncoder();
    bool close();
};

#endif // DESKTOPCAPTURE_H
