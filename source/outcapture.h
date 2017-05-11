#ifndef OUTCAPTURE_H
#define OUTCAPTURE_H

#include <QObject>
#include <QString>

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

class OutCapture
{
private:
    AVFormatContext *m_formatContext;

public:
    OutCapture();

public:
    AVFormatContext *open(QString fileName);
    bool writeHeader();
    bool wirteTrailer();
    void close();
};

#endif // OUTCAPTURE_H
