
#include "mainwindow.h"
#include <QWindow>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->m_utils = new Utils();
    this->setWindowIcon(QIcon(":/app/capture.jpg"));
    m_utils->ffmpegReg();
    this->initData();
    initWin();

    m_outCapture = new OutCapture();
    m_micCapture = new MicCapture();
    m_desktopCapture = new DesktopCapture();
    m_cameraCapture = new CameraCaptrue();

    if(global_param.camera_isShowVideo){
        m_cameraCapture->open(global_param.video_camera_format_name, global_param.video_camera_input_name, NULL,
                          QRect(global_param.camera_show_video_rect.left,
                                global_param.camera_show_video_rect.top,
                                global_param.camera_show_video_rect.width,
                                global_param.camera_show_video_rect.height));
        m_videoWindow->setVisible(true);
    }
    m_cameraThread = new cameraThread(m_cameraCapture , m_videoWindow);
    m_cameraThread->start();

    m_micThread = new micThread(m_micCapture);

    m_encoderThread = NULL;
}

MainWindow::~MainWindow()
{

}

void MainWindow::initData(){

    m_timerID = 0;
    m_isCaputer = false;
    m_isPause = false;
    m_fileName = "";
    m_utils->initGlobalParam(&global_param);

    qDebug() << global_param.os_name;

}

void MainWindow::initWin(){
    this->setWindowTitle("Acaptuer");

    m_videoWindow = NULL;
    m_videoWindow = new VideoWindow();
    m_videoWindow->setPostionRect(QRect(global_param.camera_show_video_rect.left,
                                        global_param.camera_show_video_rect.top,
                                        global_param.camera_show_video_rect.width,
                                        global_param.camera_show_video_rect.height));

    m_play = NULL;
    m_play = play();
    m_stop = NULL;
    m_stop = stop();
    m_pause = NULL;
    m_pause = pause();
    m_statusMsg = NULL;
    m_statusMsg = statusBarMsg();
    m_formatDialog = new FormatDialog(&global_param, m_cameraCapture);

    this->setGeometry(QRect(100,100,100,60));
    play()->setGeometry(QRect(0,20,play()->size().width(), play()->size().height()));
    pause()->setGeometry(QRect(play()->size().width(), 20, pause()->size().width(), pause()->size().height()));
    stop()->setGeometry(QRect(play()->size().width() + pause()->size().width(), 20,
                              stop()->size().width(), stop()->size().height()));
    statusBarMsg()->setText("Waiting...");

    this->createMenu();
}

void MainWindow::createMenu(){

    //创建打开文件动作
    QAction * fileOpenAction = new QAction(QIcon(tr("images/open.ico")), tr("打开文件"), this);
    //设置打开文件动作的快捷键
    fileOpenAction->setShortcut(tr("Ctrl + O"));
    //设置打开文件动作的提示信息
    fileOpenAction->setStatusTip(tr("打开一个文件"));
    //关联打开文件动作的信号和槽
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpenActionSlot()));

    QAction *formatAction = new QAction("Format", this);
    formatAction->setShortcut(tr("Ctrl + F"));
    formatAction->setStatusTip("set file format");
    connect(formatAction, SIGNAL(triggered()), this, SLOT(formatActionSlot()));

    QAction *fileCloseAction = new QAction(QIcon(tr("images/close.ico")), tr("关闭文件"), this);
    fileCloseAction->setShortcut(tr("Ctrl + Q"));
    fileCloseAction->setStatusTip(tr("关闭一个文件"));
    connect(fileCloseAction, SIGNAL(triggered()), this, SLOT(fileCloseActionSlot()));

    QMenu *menu = this->menuBar()->addMenu(tr("文件"));
    menu->addAction(fileOpenAction);
    menu->addAction(formatAction);
    menu->addAction(fileCloseAction);
}

void MainWindow::timerEvent( QTimerEvent *event ){
    struct timeval tpstart,tpend;
    float timeuse;
    gettimeofday(&tpstart,NULL);

    m_desktopCapture->capture();

    gettimeofday(&tpend,NULL);
    timeuse=(1000000*(tpend.tv_sec-tpstart.tv_sec) + tpend.tv_usec-tpstart.tv_usec)/1000.0;
    qDebug()<< "avcodec one time ms: " << timeuse << "ms";
    //qDebug()<< "grab_video:" << grab_video << "grab_audio:" << grab_audio;
}

void MainWindow::fileOpenActionSlot(){
   QString fileName = QFileDialog::getSaveFileName(this, tr("open file"), " ",  tr("mp4file(*.mp4);;Allfile(*.*)"));
   if ( fileName != ""){
     this->m_fileName = fileName;
   }
   qDebug() << "m_fileName:" << m_fileName;
}

void MainWindow::fileCloseActionSlot()
{
     if ( QMessageBox::warning(this, tr("提示"), tr("Are you sure exit?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){

         this->close();
     }
}

void MainWindow::formatActionSlot(){
    m_utils->initGlobalParam(&global_param);
    int r = m_formatDialog->exec();
    qDebug() << r;
    if ( r == 1){ //
        m_utils->setGlobalParamToConfFile(&global_param);
    }
}

QPushButton *MainWindow::play(){
    if (m_play == NULL ){
        m_play = new QPushButton(this);
        m_play->setText("▶");
        m_play->resize(20, 20);
        connect(m_play, SIGNAL(clicked()), this, SLOT(playSlot()));
    }
    return m_play;
}

void MainWindow::playSlot(){
    if ( m_isCaputer == true && m_isPause == false ) return;
    if ( m_isCaputer == true && m_isPause == true ){
        //start timer
        m_timerID = startTimer( 40 );
        m_isPause = false;
    }
    if ( m_isCaputer == false ){
        //jugde m_fileName  Whether exist.
        if (m_fileName == ""){
            QMessageBox::about(this,"Warning", "Please select a file.");
            return;
        }
        QFile file(m_fileName);
        if (file.exists()){
          if (QMessageBox::question(this,"Warning","File has exist, continue will cover it ?", QMessageBox::Yes|QMessageBox::No)
                  == QMessageBox::No)
              return;
        }
        /*
        if(global_param.camera_isShowVideo)
            m_cameraCapture->open(global_param.video_camera_format_name, global_param.video_camera_input_name, NULL,
                              QRect(global_param.camera_show_video_rect.left,
                                    global_param.camera_show_video_rect.top,
                                    global_param.camera_show_video_rect.width,
                                    global_param.camera_show_video_rect.height));
        */
        m_outFormatContext = m_outCapture->open(m_fileName.toLocal8Bit().data());
        if ( strcmp(global_param.audio_mic_format_name, "-") != 0 )
            m_micCapture->open(global_param.audio_mic_format_name, global_param.audio_mic_input_name, m_outFormatContext);
        else if (( strcmp(global_param.audio_pc_format_name, "-") != 0 ))
            m_micCapture->open(global_param.audio_pc_format_name, global_param.audio_pc_input_name, m_outFormatContext);

        m_desktopCapture->open(QString(global_param.video_desktop_format_name),
                               QString(global_param.video_desktop_input_name),
                               m_outFormatContext, QString(global_param.video_capture_mode),
                               QRect(global_param.video_capture_rect.left,
                                     global_param.video_capture_rect.top,
                                     global_param.video_capture_rect.width,
                                     global_param.video_capture_rect.height),
                               QSize(global_param.desktop_width, global_param.desktop_height));
        m_outCapture->writeHeader();
        //start timer
        m_isCaputer = true;
        m_isPause = false;

        m_timerID = startTimer( 40 );
        m_micThread->start();
        if (m_encoderThread == NULL)
             m_encoderThread = new encoderThread(m_micCapture, m_desktopCapture, m_outFormatContext);
        m_encoderThread->start();
    }
   if ( m_videoWindow && global_param.camera_isShowVideo )
       m_videoWindow->show();
   statusBarMsg()->setText("Capturing...");
}

QPushButton *MainWindow::stop(){
    if (m_stop == NULL){
        m_stop = new QPushButton(this);
        m_stop->setText("■");
        m_stop->resize(20, 20);
        connect(m_stop, SIGNAL(clicked()), this, SLOT(stopSlot()));
    }
    if(m_videoWindow)
        m_videoWindow->setVisible(false);
    return m_stop;
}

void MainWindow::stopSlot(){
    if ( m_isCaputer == false ) return;
    m_isCaputer = false;
    m_isPause = false;
    killTimer(m_timerID);
    QThread::msleep(500);
    m_micThread->terminate();
    QThread::msleep(500);
    m_encoderThread->terminate();

    //mic outfile close .
    m_desktopCapture->flushEncoder();
    m_micCapture->flushEncoder();
    m_outCapture->wirteTrailer();
    m_micCapture->close();
    m_desktopCapture->close();
    m_outCapture->close();
    //if(m_videoWindow)
    //    m_videoWindow->setVisible(false);
    // m_cameraCapture->close();
    statusBarMsg()->setText("Closed");
}

QPushButton *MainWindow::pause(){
    if (m_pause == NULL){
        m_pause = new QPushButton(this);
        m_pause->setText("||");
        m_pause->resize(20, 20);
        connect(m_pause, SIGNAL(clicked()), this, SLOT(pauseSlot()));
    }
    return m_pause;
}

void MainWindow::pauseSlot(){
    if ( m_isCaputer == false ) return;
    killTimer(m_timerID);
    m_isPause = true;
    statusBarMsg()->setText("Paused");
}

QLabel* MainWindow::statusBarMsg(){
    if ( m_statusMsg == NULL){
        m_statusMsg = new QLabel(this);
        m_statusMsg->setMinimumSize(m_statusMsg->sizeHint());
        m_statusMsg->setAlignment(Qt::AlignHCenter);
        m_statusMsg->setText("test");

        statusBar()->addWidget(m_statusMsg);
        statusBar()->setStyleSheet(QString("QStatusBar::item{border: 2opx}"));
        statusBar()->setSizeGripEnabled(false);//QStatusBar右下角的大小控制点cancel
    }
    return m_statusMsg;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_cameraThread->terminate();
    m_cameraCapture->close();
    m_videoWindow->close();

}
