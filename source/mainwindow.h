#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QStatusBar>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QLabel>
#include <sys/time.h>
#include "miccapture.h"
#include "outcapture.h"
#include "desktopcapture.h"
#include "videowindow.h"
#include "cameracaptrue.h"
#include "camerathread.h"
#include "encoderthread.h"
#include "micthread.h"

extern "C"{
#include "type.h"
}

#include "formatdialog.h"
#include "utils.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Utils *m_utils;
    micThread *m_micThread;
    encoderThread *m_encoderThread;
    CameraCaptrue *m_cameraCapture;
    cameraThread *m_cameraThread;
    DesktopCapture *m_desktopCapture;
    OutCapture *m_outCapture;
    MicCapture *m_micCapture;
    FormatDialog *m_formatDialog;
    VideoWindow *m_videoWindow;
    AVFormatContext *m_outFormatContext;
    struct global_param_struct *m_formatParam;
    QString m_fileName;
    int m_timerID;
    //m_isCapture = false m_isPause = false , can open.  init status.
    //m_isCaptrue = true  m_isPause = false , can pase close.
    //m_isCaptrue = false m_isPause = true  , can none.
    //m_isCaptrue = true  m_isPause = true  , can open close
    //closed: set  m_isCaptrue = false, m_isPause = false.
    bool m_isCaputer;
    bool m_isPause;
    QPushButton *m_play;
    QPushButton *m_stop;
    QPushButton *m_pause;
    QLabel *m_statusMsg;


private:
    void initData();
    void createMenu();
    void initWin();
    QPushButton *play();
    QPushButton *stop();
    QPushButton *pause();
    QLabel *statusBarMsg();

private slots:
    void fileOpenActionSlot();
    void fileCloseActionSlot();
    void formatActionSlot();
    void playSlot();
    void stopSlot();
    void pauseSlot();
protected:
   void timerEvent( QTimerEvent *event );
   void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
