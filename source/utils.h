#ifndef UTILS_H
#define UTILS_H

#define GLOBALS

#include <QString>
#include <QObject>
#include <QDesktopWidget>
#include <QDebug>
#include <QApplication>

extern "C" {
    #include "type.h"
}

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


class Utils : public QObject
{
public:
    Utils();
public:
     QString getOsName();
     QSize getDesktopSize();
     void initGlobalParam(struct global_param_struct *global_param);
     void setDefultGlobalParam(struct global_param_struct *global_param);
     void getGlobalParamFromConfFile(struct global_param_struct *global_param);
     void setGlobalParamToConfFile(struct global_param_struct *global_param);
     bool openInputTest(char* format, char* input);
     void ffmpegReg();
};

#endif // UTILS_H
