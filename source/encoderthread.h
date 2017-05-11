#ifndef ENCODERTHREAD_H
#define ENCODERTHREAD_H

#include <QThread>

#include "miccapture.h"
#include "desktopcapture.h"


class encoderThread : public QThread
{
    Q_OBJECT
public:
    encoderThread(MicCapture *micCapture, DesktopCapture *desktopCapture,AVFormatContext *outFormatContext);
private:
    MicCapture *m_micCapture;
    DesktopCapture *m_desktopCapture;
    AVFormatContext *m_outFormatContext;
protected:
    void run() Q_DECL_OVERRIDE;
};

#endif // ENCODERTHREAD_H
