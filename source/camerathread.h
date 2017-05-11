#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include "cameracaptrue.h"
#include "videowindow.h"
#include "type.h"

class cameraThread :public QThread
{
    Q_OBJECT
public:
    cameraThread(CameraCaptrue *cameraCaptrue, VideoWindow *m_videoWindow);
private:
    CameraCaptrue *m_cameraCaptrue;
    VideoWindow *m_videoWindow;
protected:
    void run() Q_DECL_OVERRIDE;

};

#endif // CAMERATHREAD_H
