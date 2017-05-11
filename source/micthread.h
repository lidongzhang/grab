#ifndef MICTHREAD_H
#define MICTHREAD_H

#include <QThread>
#include "miccapture.h"


class micThread : public QThread
{
    Q_OBJECT
public:
    micThread(MicCapture *micCapture);
private:
    MicCapture *m_micCapture;
protected:
    void run() Q_DECL_OVERRIDE;

};

#endif // MICTHREAD_H
