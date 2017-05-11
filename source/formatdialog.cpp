#include "formatdialog.h"

#include <QMessageBox>
#include <QThread>

FormatDialog::FormatDialog(struct global_param_struct *global_param, CameraCaptrue *cameraCaptrue)
{
    m_cameraCaptrue = cameraCaptrue;
    m_utils = new Utils();
    this->m_global_param = global_param;

    this->setWindowTitle("Set params");
    this->setGeometry(QRect(100,100, 520, 420));
    initWin();

}

void FormatDialog::showEvent(QShowEvent *event){
    qDebug() << "FormatDialog opened!";
    initData();
}

void FormatDialog::initData(){
    this->cmbVideoSourceDesktopFormat->clear();
    this->cmbVideoSourceDesktopFormat->addItem(m_global_param->video_desktop_format_name);
    this->cmbVideoSourceDesktopInput->clear();
    this->cmbVideoSourceDesktopInput->addItem(m_global_param->video_desktop_input_name);
    this->cmbVideoSourceCameraFormat->clear();
    this->cmbVideoSourceCameraFormat->addItem(m_global_param->video_camera_format_name);
    this->cmbVideoSourceCameraInput->clear();
    this->cmbVideoSourceCameraInput->addItem(m_global_param->video_camera_input_name);
    this->cmbAudioSourceMicFormat->clear();
    this->cmbAudioSourceMicFormat->addItem(m_global_param->audio_mic_format_name);
    this->cmbAudioSourceMicInput->clear();
    this->cmbAudioSourceMicInput->addItem(m_global_param->audio_mic_input_name);
    this->cmbAudioSourcePcFormat->clear();
    this->cmbAudioSourcePcFormat->addItem(m_global_param->audio_pc_format_name);
    if (strcmp(m_global_param->audio_pc_format_name, "-") == 0){
        this->cmbAudioSourcePcFormat->setEnabled(false);
        this->rdbPc->setEnabled(false);
        this->rdbMicAndPc->setEnabled(false);
        this->rdbMic->setChecked(true);
    }
    this->cmbAudioSourcePcInput->clear();
    this->cmbAudioSourcePcInput->addItem(m_global_param->audio_pc_input_name);
    if (strcmp(m_global_param->audio_pc_input_name, "-") == 0){
        this->cmbAudioSourcePcInput->setEnabled(false);
        this->rdbPc->setEnabled(false);
        this->rdbMicAndPc->setEnabled(false);
        this->rdbMic->setChecked(true);
    }
    if (strcmp(m_global_param->video_capture_mode, "fullScreen") == 0 )
        this->rdbFullScreen->setChecked(true);
    if (strcmp(m_global_param->video_capture_mode, "mouseCenter") == 0)
        this->rdbMouseScreen->setChecked(true);
    if (strcmp(m_global_param->video_capture_mode, "customRect") == 0)
        this->rdbCustomRect->setChecked(true);
    this->edtVideoRectLeft->setText(QString().number(m_global_param->video_capture_rect.left));
    this->edtVideoRectTop->setText(QString().number(m_global_param->video_capture_rect.top));
    this->edtVideoRectWidth->setText(QString().number(m_global_param->video_capture_rect.width));
    this->edtVideoRectHeight->setText(QString().number(m_global_param->video_capture_rect.height));
    if (strcmp(m_global_param->audio_capture_mode, "mic") == 0)
        this->rdbMic->setChecked(true);
    if (strcmp(m_global_param->audio_capture_mode, "pc") == 0)
        this->rdbPc->setChecked(true);
    if (strcmp(m_global_param->audio_capture_mode, "micAndPc") == 0)
        this->rdbMicAndPc->setChecked(true);

    //camera
    if (m_global_param->camera_isShowVideo)
        this->chkCameraIsShowVideo->setChecked(true);
    else
        this->chkCameraIsShowVideo->setChecked(false);

    this->edtCameraShowVideoRectLeft->setText(QString().number(m_global_param->camera_show_video_rect.left));
    this->edtCameraShowVideoRectTop->setText(QString().number(m_global_param->camera_show_video_rect.top));
    this->edtCameraShowVideoRectWidth->setText(QString().number(m_global_param->camera_show_video_rect.width));
    this->edtCameraShowVideoRectHeight->setText(QString().number(m_global_param->camera_show_video_rect.height));
}

void FormatDialog::initWin(){
    QTabWidget* tab = this->CreateTab();

    //ok
    QPushButton *btn_ok = new QPushButton(this);

    btn_ok->setText("ok");
    connect(btn_ok, SIGNAL(clicked()), this, SLOT(okSlot()));
    //cancel
    QPushButton *btn_cancel = new QPushButton(this);

    btn_cancel->setText("cancel");
    connect(btn_cancel, SIGNAL(clicked()), this, SLOT(cancelSlot()));

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->addWidget(tab, 0, 0,1,5);
    mainLayout->addWidget(btn_ok, 1, 1);
    mainLayout->addWidget(btn_cancel, 1, 3);

    this->setLayout(mainLayout);
}

QTabWidget* FormatDialog::CreateTab(){
    QTabWidget *tab = new QTabWidget(this);

    QWidget *tabWidget = new QWidget();
    QGridLayout *mainLayout = new QGridLayout();

    //video desktop
    QLabel *labelVideoSourceDesktopFormat = new QLabel("Video desktop format:");
    cmbVideoSourceDesktopFormat = new QComboBox();
    cmbVideoSourceDesktopFormat->setEditable(true);
    mainLayout->addWidget(labelVideoSourceDesktopFormat, 0, 0);
    mainLayout->setAlignment(labelVideoSourceDesktopFormat, Qt::AlignRight);
    mainLayout->addWidget(cmbVideoSourceDesktopFormat, 0, 1);

    QLabel *labelVideoSourceDesktopInput = new QLabel(" input:");
    cmbVideoSourceDesktopInput = new QComboBox();
    cmbVideoSourceDesktopInput->setEditable(true);
    mainLayout->addWidget(labelVideoSourceDesktopInput, 0, 2);
    mainLayout->setAlignment(labelVideoSourceDesktopInput, Qt::AlignRight);
    mainLayout->addWidget(cmbVideoSourceDesktopInput, 0, 3);
    labelVideoSourceDesktopTest = new QLabel();
    mainLayout->addWidget(labelVideoSourceDesktopTest, 0, 4);

    //video camera
    QLabel *labelVideoSourceCameraFormat = new QLabel("Audio camera format:");
    cmbVideoSourceCameraFormat = new QComboBox();
    cmbVideoSourceCameraFormat->setEditable(true);
    mainLayout->addWidget(labelVideoSourceCameraFormat, 1, 0);
    mainLayout->setAlignment(labelVideoSourceCameraFormat, Qt::AlignRight);
    mainLayout->addWidget(cmbVideoSourceCameraFormat, 1, 1);

    QLabel *labelVideoSourceCameraInput = new QLabel(" input:");
    cmbVideoSourceCameraInput = new QComboBox();
    cmbVideoSourceCameraInput->setEditable(true);
    mainLayout->addWidget(labelVideoSourceCameraInput, 1, 2);
    mainLayout->setAlignment(labelVideoSourceCameraInput, Qt::AlignRight);
    mainLayout->addWidget(cmbVideoSourceCameraInput, 1, 3);

    labelVideoSourceCameraTest = new QLabel();
    mainLayout->addWidget(labelVideoSourceCameraTest, 1, 4);
    //audio mic
    QLabel *labelAudioSourceMicFormat = new QLabel("Video mic format:");
    cmbAudioSourceMicFormat = new QComboBox();
    cmbAudioSourceMicFormat->setEditable(true);
    mainLayout->addWidget(labelAudioSourceMicFormat, 2, 0);
    mainLayout->setAlignment(labelAudioSourceMicFormat, Qt::AlignRight);
    mainLayout->addWidget(cmbAudioSourceMicFormat, 2, 1);

    QLabel *labelAudioSourceMicInput = new QLabel(" input:");
    cmbAudioSourceMicInput = new QComboBox();
    cmbAudioSourceMicInput->setEditable(true);
    mainLayout->addWidget(labelAudioSourceMicInput, 2, 2);
    mainLayout->setAlignment(labelAudioSourceMicInput, Qt::AlignRight);
    mainLayout->addWidget(cmbAudioSourceMicInput, 2, 3);

    labelAudioSourceMicTest = new QLabel();
    mainLayout->addWidget(labelAudioSourceMicTest, 2, 4);
    //audio pc
    QLabel *labelAudioSourcePcFormat = new QLabel("Audio pc format:");
    cmbAudioSourcePcFormat = new QComboBox();
    cmbAudioSourcePcFormat->setEditable(true);
    mainLayout->addWidget(labelAudioSourcePcFormat, 3, 0);
    mainLayout->setAlignment(labelAudioSourcePcFormat, Qt::AlignRight);
    mainLayout->addWidget(cmbAudioSourcePcFormat, 3, 1);

    QLabel *labelAudioSourcePcInput = new QLabel(" input:");
    cmbAudioSourcePcInput = new QComboBox();
    cmbAudioSourcePcInput->setEditable(true);
    mainLayout->addWidget(labelAudioSourcePcInput, 3, 2);
    mainLayout->setAlignment(labelAudioSourcePcInput, Qt::AlignRight);
    mainLayout->addWidget(cmbAudioSourcePcInput, 3, 3);

    labelAudioSourcePcTest = new QLabel();
    mainLayout->addWidget(labelAudioSourcePcTest, 3, 4);
    //video mode
    QLabel *labelVideoModel = new QLabel("Video mode:");
    QButtonGroup *bgrVideo = new QButtonGroup();
    rdbFullScreen = new QRadioButton();
    rdbFullScreen->setText("full screen");
    rdbMouseScreen = new QRadioButton();
    rdbMouseScreen->setText("mouse center");
    rdbCustomRect = new QRadioButton();
    rdbCustomRect->setText("custom rect");

    bgrVideo->addButton(rdbFullScreen);
    bgrVideo->addButton(rdbMouseScreen);
    bgrVideo->addButton(rdbCustomRect);
    mainLayout->addWidget(labelVideoModel, 4, 0);
    mainLayout->setAlignment(labelVideoModel, Qt::AlignRight);
    QHBoxLayout *videoModeLayout = new QHBoxLayout();

    videoModeLayout->addWidget(rdbFullScreen);
    videoModeLayout->addWidget(rdbMouseScreen);
    videoModeLayout->addWidget(rdbCustomRect);
    mainLayout->addLayout(videoModeLayout, 4,1, 1,3 );

    //video rect
    QSize minCaptureSize;
    minCaptureSize.setWidth(10);
    minCaptureSize.setHeight(10);
    QHBoxLayout *videoRectLayout = new QHBoxLayout();
    QLabel *labelVideoRectLeft = new QLabel("left:");
    edtVideoRectLeft = new QLineEdit();
    edtVideoRectLeft->setValidator(new QIntValidator(0, m_global_param->desktop_width - 1 - minCaptureSize.width(), edtVideoRectLeft));
    videoRectLayout->addWidget(labelVideoRectLeft);
    videoRectLayout->addWidget(edtVideoRectLeft);

    QLabel *labelVideoRectTop = new QLabel("top:");
    edtVideoRectTop = new QLineEdit();
    edtVideoRectTop->setValidator(new QIntValidator(0, m_global_param->desktop_height - 1 - minCaptureSize.height(), edtVideoRectTop));
    videoRectLayout->addWidget(labelVideoRectTop);
    videoRectLayout->addWidget(edtVideoRectTop);

    QLabel *labelVideoRectWidth = new QLabel("width:");
    edtVideoRectWidth = new QLineEdit();
    edtVideoRectWidth->setValidator(new QIntValidator(minCaptureSize.width(), m_global_param->desktop_width, edtVideoRectWidth));
    videoRectLayout->addWidget(labelVideoRectWidth);
    videoRectLayout->addWidget(edtVideoRectWidth);

    QLabel *labelVideoRectHeight = new QLabel("height:");
    edtVideoRectHeight = new QLineEdit();
    edtVideoRectHeight->setValidator(new QIntValidator(minCaptureSize.height(), m_global_param->desktop_height, edtVideoRectHeight));
    videoRectLayout->addWidget(labelVideoRectHeight);
    videoRectLayout->addWidget(edtVideoRectHeight);

    mainLayout->addLayout(videoRectLayout, 5,1,1,3);

    //audio mode
    QLabel *labelAudioModel = new QLabel("Audio mode:");
    QButtonGroup *bgrAudio = new QButtonGroup();
    rdbMic = new QRadioButton();
    rdbMic->setText("mic");
    rdbPc = new QRadioButton();
    rdbPc->setText("pc");
    rdbMicAndPc = new QRadioButton();
    rdbMicAndPc->setText("mic and pc");

    bgrAudio->addButton(rdbMic);
    bgrAudio->addButton(rdbPc);
    bgrAudio->addButton(rdbMicAndPc);
    mainLayout->addWidget(labelAudioModel, 6, 0);
    mainLayout->setAlignment(labelAudioModel, Qt::AlignRight);

    QHBoxLayout *audioModeLayout = new QHBoxLayout();
    audioModeLayout->addWidget(rdbMic);
    audioModeLayout->addWidget(rdbPc);
    audioModeLayout->addWidget(rdbMicAndPc);
    mainLayout->addLayout(audioModeLayout, 6,1, 1,3 );

    //camera
    QLabel *labelCameraIsShowVideo = new QLabel("If show camera video:");
    mainLayout->addWidget(labelCameraIsShowVideo, 7, 0);
    chkCameraIsShowVideo = new QCheckBox();
    mainLayout->addWidget(chkCameraIsShowVideo, 7, 1);
    //rect
    QHBoxLayout *cameraShowVideoRectLayout = new QHBoxLayout();
    QLabel *labelCameraShowVideoRectLeft = new QLabel("left:");
    edtCameraShowVideoRectLeft = new QLineEdit();
    edtCameraShowVideoRectLeft->setValidator(new QIntValidator(0, m_global_param->desktop_width - 1, edtCameraShowVideoRectLeft));
    cameraShowVideoRectLayout->addWidget(labelCameraShowVideoRectLeft);
    cameraShowVideoRectLayout->addWidget(edtCameraShowVideoRectLeft);

    QLabel *labelCameraShowVideoRectTop = new QLabel("top:");
    edtCameraShowVideoRectTop = new QLineEdit();
    edtCameraShowVideoRectTop->setValidator(new QIntValidator(0, m_global_param->desktop_height - 1, edtCameraShowVideoRectTop));
    cameraShowVideoRectLayout->addWidget(labelCameraShowVideoRectTop);
    cameraShowVideoRectLayout->addWidget(edtCameraShowVideoRectTop);

    QLabel *labelCameraShowVideoRectWidth = new QLabel("width:");
    edtCameraShowVideoRectWidth = new QLineEdit();
    edtCameraShowVideoRectWidth->setValidator(new QIntValidator(minCaptureSize.width(), m_global_param->desktop_width, edtCameraShowVideoRectWidth));
    cameraShowVideoRectLayout->addWidget(labelCameraShowVideoRectWidth);
    cameraShowVideoRectLayout->addWidget(edtCameraShowVideoRectWidth);

    QLabel *labelCameraShowVideoRectHeight = new QLabel("height:");
    edtCameraShowVideoRectHeight = new QLineEdit();
    edtCameraShowVideoRectHeight->setValidator(new QIntValidator(minCaptureSize.height(), m_global_param->desktop_height, edtCameraShowVideoRectHeight));
    cameraShowVideoRectLayout->addWidget(labelCameraShowVideoRectHeight);
    cameraShowVideoRectLayout->addWidget(edtCameraShowVideoRectHeight);

    mainLayout->addLayout(cameraShowVideoRectLayout, 8,1,1,3);


    //
    tabWidget->setLayout(mainLayout);

    tab->addTab(tabWidget,"linux");

    return tab;
}

void FormatDialog:: okSlot(){
    qDebug() << cmbVideoSourceDesktopFormat->lineEdit()->text();
    //video desktop
    int ret_VideoSourceDesktop = m_utils->openInputTest(cmbVideoSourceDesktopFormat->lineEdit()->text().toLocal8Bit().data(),
                              cmbVideoSourceDesktopInput->lineEdit()->text().toLocal8Bit().data());
    if ( ret_VideoSourceDesktop == 0 )
        labelVideoSourceDesktopTest->setText("√");
    else
        labelVideoSourceDesktopTest->setText("×");
    qDebug() << "desktop test finished";
    //video camera
    int ret_VideoSourceCamera = m_utils->openInputTest(cmbVideoSourceCameraFormat->lineEdit()->text().toLocal8Bit().data(),
                              cmbVideoSourceCameraInput->lineEdit()->text().toLocal8Bit().data());
    if ( ret_VideoSourceCamera == 0 )
        labelVideoSourceCameraTest->setText("√");
    else
        labelVideoSourceCameraTest->setText("×");
    if (m_cameraCaptrue->m_opened){
        labelVideoSourceCameraTest->setText("√");
        ret_VideoSourceCamera = 0;
    }
    //QThread::sleep(1);
    qDebug() << "camera test finished";
    //audio mic
    int ret_AudioSourceMic = m_utils->openInputTest(cmbAudioSourceMicFormat->lineEdit()->text().toLocal8Bit().data(),
                              cmbAudioSourceMicInput->lineEdit()->text().toLocal8Bit().data());
    if ( ret_AudioSourceMic == 0 )
        labelAudioSourceMicTest->setText("√");
    else
        labelAudioSourceMicTest->setText("×");
    //QThread::sleep(1);
    qDebug() << "mic test finished";
    //audio pc
    int ret_AudioSourcePc = 0;
    if ( strcmp(m_global_param->audio_pc_format_name, "-") != 0 &&
         strcmp(m_global_param->audio_pc_input_name, "-") != 0){
        ret_AudioSourcePc = m_utils->openInputTest(cmbAudioSourcePcFormat->lineEdit()->text().toLocal8Bit().data(),
                                      cmbAudioSourcePcInput->lineEdit()->text().toLocal8Bit().data());
        if ( ret_AudioSourcePc == 0 )
            labelAudioSourcePcTest->setText("√");
        else
            labelAudioSourcePcTest->setText("×");
    }
    qDebug() << "AUDIO PC test finished";
    qDebug() << "msgbox";
    if (ret_VideoSourceDesktop != 0 ||  ret_VideoSourceCamera != 0 ||
        ret_AudioSourceMic !=0 ||  ret_AudioSourcePc != 0  ){
        QMessageBox::about(this, "Warring", "Test is error, can't save.");
        return;
    }
    //set value
    strcpy(m_global_param->video_desktop_format_name, cmbVideoSourceDesktopFormat->lineEdit()->text().toLocal8Bit().data());
    strcpy(m_global_param->video_desktop_input_name, cmbVideoSourceDesktopInput->lineEdit()->text().toLocal8Bit().data());
    strcpy(m_global_param->video_camera_format_name, cmbVideoSourceCameraFormat->lineEdit()->text().toLocal8Bit().data());
    strcpy(m_global_param->video_camera_input_name, cmbVideoSourceCameraInput->lineEdit()->text().toLocal8Bit().data());
    strcpy(m_global_param->audio_mic_format_name, cmbAudioSourceMicFormat->lineEdit()->text().toLocal8Bit().data());
    strcpy(m_global_param->audio_mic_input_name, cmbAudioSourceMicInput->lineEdit()->text().toLocal8Bit().data());
    strcpy(m_global_param->audio_pc_format_name, cmbAudioSourcePcFormat->lineEdit()->text().toLocal8Bit().data());
    strcpy(m_global_param->audio_pc_input_name, cmbAudioSourcePcInput->lineEdit()->text().toLocal8Bit().data());

    if ( rdbFullScreen->isChecked() )
        strcpy(m_global_param->video_capture_mode, "fullScreen");
    if ( rdbMouseScreen->isChecked())
        strcpy(m_global_param->video_capture_mode, "mouseCenter");
    if ( rdbCustomRect->isChecked())
        strcpy(m_global_param->video_capture_mode, "customRect");

    m_global_param->video_capture_rect.left = edtVideoRectLeft->text().toInt();
    m_global_param->video_capture_rect.top = edtVideoRectTop->text().toInt();
    m_global_param->video_capture_rect.width = edtVideoRectWidth->text().toInt();
    m_global_param->video_capture_rect.height = edtVideoRectHeight->text().toInt();

    if (rdbMic->isChecked())
        strcpy(m_global_param->audio_capture_mode, "mic");
    if (rdbPc->isChecked())
        strcpy(m_global_param->audio_capture_mode, "pc");
    if (rdbMicAndPc->isChecked())
        strcpy(m_global_param->audio_capture_mode, "micAndPc");

    //camera
    if(this->chkCameraIsShowVideo->checkState())
        m_global_param->camera_isShowVideo = true;
    else
        m_global_param->camera_isShowVideo = false;
    m_global_param->camera_show_video_rect.left = edtCameraShowVideoRectLeft->text().toInt();
    m_global_param->camera_show_video_rect.top = edtCameraShowVideoRectTop->text().toInt();
    m_global_param->camera_show_video_rect.width = edtCameraShowVideoRectWidth->text().toInt();
    m_global_param->camera_show_video_rect.height = edtCameraShowVideoRectHeight->text().toInt();

    done(1);
}

void FormatDialog::cancelSlot(){

    done(0);
}
