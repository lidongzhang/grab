#include "utils.h"
#include "QFile"
#include <QSettings>
#include <QThread>

Utils::Utils()
{
}

QString  Utils::getOsName(){
    #ifdef Q_OS_LINUX
        return "linux";
    #elif defined(Q_OS_WIN32)
        return "windows";
    #elif defined(Q_OS_MAC)
        return "mac";
    #endif
    return "";
}

QSize Utils::getDesktopSize(){
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry();
    qDebug() << "screen width:" << screenRect.width() << "height:" << screenRect.height();
    return QSize(screenRect.width(), screenRect.height() );
}

void Utils::setDefultGlobalParam(struct global_param_struct *global_param){
    if ( strcmp(global_param->os_name, "linux") == 0 ){
        strcpy(global_param->video_desktop_format_name, "x11grab");
        strcpy(global_param->video_desktop_input_name, ":0.0");
        strcpy(global_param->video_camera_format_name, "video4linux2");
        strcpy(global_param->video_camera_input_name, "/dev/video0");

        strcpy(global_param->audio_mic_format_name, "alsa");
        strcpy(global_param->audio_mic_input_name, "pulse");
        strcpy(global_param->audio_pc_format_name, "-");
        strcpy(global_param->audio_pc_input_name, "-");

        strcpy(global_param->video_capture_mode, "fullScreen");
        global_param->video_capture_rect.left = 0;
        global_param->video_capture_rect.top = 0;
        global_param->video_capture_rect.width = 640;
        global_param->video_capture_rect.height = 480;

        strcpy(global_param->audio_capture_mode, "mic");

        global_param->camera_isShowVideo = false;
        global_param->camera_show_video_rect.width = 200;
        global_param->camera_show_video_rect.height = 200;
        global_param->camera_show_video_rect.left = global_param->desktop_width - global_param->camera_show_video_rect.width;
        global_param->camera_show_video_rect.top = global_param->desktop_height - global_param->camera_show_video_rect.height;
    }

    if ( strcmp(global_param->os_name, "windows") == 0){
        strcpy(global_param->video_desktop_format_name, "dshow");
        strcpy(global_param->video_desktop_input_name, "video=screen-capture-recorder");
        strcpy(global_param->video_camera_format_name, "-");
        strcpy(global_param->video_camera_input_name, "-");

        strcpy(global_param->audio_mic_format_name, "-");
        strcpy(global_param->audio_mic_input_name, "-");
        strcpy(global_param->audio_pc_format_name, "dshow");
        strcpy(global_param->audio_pc_input_name, "audio=virtual-audio-capturer");

        strcpy(global_param->video_capture_mode, "fullScreen");
        global_param->video_capture_rect.left = 0;
        global_param->video_capture_rect.top = 0;
        global_param->video_capture_rect.width = 640;
        global_param->video_capture_rect.height = 480;

        if (strcmp(global_param->audio_mic_format_name, "-") != 0 )
            strcpy(global_param->audio_capture_mode, "mic");
        else if (strcmp(global_param->audio_pc_format_name, "-") != 0 )
            strcpy(global_param->audio_capture_mode, "pc");

        global_param->camera_isShowVideo = false;
        global_param->camera_show_video_rect.width = 200;
        global_param->camera_show_video_rect.height = 200;
        global_param->camera_show_video_rect.left = global_param->desktop_width - global_param->camera_show_video_rect.width;
        global_param->camera_show_video_rect.top = global_param->desktop_height - global_param->camera_show_video_rect.height;

    }

    if ( strcmp(global_param->os_name, "mac") == 0 ){
        //todo
    }
}

void Utils::setGlobalParamToConfFile(struct global_param_struct *global_param){
    QSettings *configIniWrite = new QSettings(global_param->ini_file_name, QSettings::IniFormat);
    configIniWrite->setValue("/os/os_name", global_param->os_name);
    configIniWrite->setValue("/video/desktop_format_name", global_param->video_desktop_format_name);
    configIniWrite->setValue("/video/desktop_input_name", global_param->video_desktop_input_name);
    configIniWrite->setValue("/video/camera_format_name", global_param->video_camera_format_name);
    configIniWrite->setValue("/video/camera_input_name", global_param->video_camera_input_name);
    configIniWrite->setValue("/video/capture_mode", global_param->video_capture_mode);

    configIniWrite->setValue("/video/capture_rect_left", global_param->video_capture_rect.left);
    configIniWrite->setValue("/video/capture_rect_top", global_param->video_capture_rect.top);
    configIniWrite->setValue("/video/capture_rect_width", global_param->video_capture_rect.width);
    configIniWrite->setValue("/video/capture_rect_height", global_param->video_capture_rect.height);

    configIniWrite->setValue("/audio/mic_format_name", global_param->audio_mic_format_name);
    configIniWrite->setValue("/audio/mic_input_name", global_param->audio_mic_input_name);
    configIniWrite->setValue("/audio/pc_format_name", global_param->audio_pc_format_name);
    configIniWrite->setValue("/audio/pc_input_name", global_param->audio_pc_input_name);
    configIniWrite->setValue("/audio/capture_mode", global_param->audio_capture_mode);


    configIniWrite->setValue("/camera/camera_isShowVideo", global_param->camera_isShowVideo);
    configIniWrite->setValue("/camera/show_video_rect_left", global_param->camera_show_video_rect.left);
    configIniWrite->setValue("/camera/show_video_rect_top", global_param->camera_show_video_rect.top);
    configIniWrite->setValue("/camera/show_video_rect_width", global_param->camera_show_video_rect.width);
    configIniWrite->setValue("/camera/show_video_rect_height", global_param->camera_show_video_rect.height);

    delete configIniWrite;
}

void Utils::getGlobalParamFromConfFile(struct global_param_struct *global_param){

    QSettings *configIniRead = new QSettings(global_param->ini_file_name, QSettings::IniFormat);
    QString osName = configIniRead->value("/os/os_name").toString();

    if (strcmp(global_param->os_name, osName.toLocal8Bit().data()) !=0 ){
        setDefultGlobalParam(global_param);
        return;
    }

    strcpy(global_param->video_desktop_format_name, configIniRead->value("/video/desktop_format_name").toString().toLocal8Bit().data());
    strcpy(global_param->video_desktop_input_name, configIniRead->value("/video/desktop_input_name").toString().toLocal8Bit().data());
    strcpy(global_param->video_camera_format_name, configIniRead->value("/video/camera_format_name").toString().toLocal8Bit().data());
    strcpy(global_param->video_camera_input_name, configIniRead->value("/video/camera_input_name").toString().toLocal8Bit().data());
    strcpy(global_param->video_capture_mode, configIniRead->value("/video/capture_mode").toString().toLocal8Bit().data());
    int left =  configIniRead->value("/video/capture_rect_left").toInt();
    int top =  configIniRead->value("/video/capture_rect_top").toInt();
    int width =  configIniRead->value("/video/capture_rect_width").toInt();
    int height =  configIniRead->value("/video/capture_rect_height").toInt();
    global_param->video_capture_rect.left = left;
    global_param->video_capture_rect.top = top;
    global_param->video_capture_rect.width = width;
    global_param->video_capture_rect.height = height;

    strcpy(global_param->audio_mic_format_name, configIniRead->value("/audio/mic_format_name").toString().toLocal8Bit().data());
    strcpy(global_param->audio_mic_input_name, configIniRead->value("/audio/mic_input_name").toString().toLocal8Bit().data());
    strcpy(global_param->audio_pc_format_name, configIniRead->value("/audio/pc_format_name").toString().toLocal8Bit().data());
    strcpy(global_param->audio_pc_input_name, configIniRead->value("/audio/pc_input_name").toString().toLocal8Bit().data());
    strcpy(global_param->audio_capture_mode, configIniRead->value("/audio/capture_mode").toString().toLocal8Bit().data());

    global_param->camera_isShowVideo = configIniRead->value("/camera/camera_isShowVideo").toBool();
    global_param->camera_show_video_rect.left = configIniRead->value("/camera/show_video_rect_left").toInt();
    global_param->camera_show_video_rect.top = configIniRead->value("/camera/show_video_rect_top").toInt();
    global_param->camera_show_video_rect.width = configIniRead->value("/camera/show_video_rect_width").toInt();
    global_param->camera_show_video_rect.height = configIniRead->value("/camera/show_video_rect_height").toInt();

    delete configIniRead;
}

void Utils::initGlobalParam(struct global_param_struct *global_param){
    QString confile = QCoreApplication::applicationDirPath() + "/config.ini" ;
    strcpy(global_param->os_name, getOsName().toLocal8Bit().data());
    strcpy(global_param->ini_file_name, confile.toLocal8Bit().data());
    QSize screenSize = Utils::getDesktopSize();
    global_param->desktop_width = screenSize.width();
    global_param->desktop_height = screenSize.height();
    QFile file(confile);
    if (file.exists()){
        getGlobalParamFromConfFile(global_param);
    }else{
        setDefultGlobalParam(global_param);
    }
}


bool Utils::openInputTest(char* format, char* input){
    int ret = 0;
    qDebug() << "test start";
    AVFormatContext* input_video_format_context = avformat_alloc_context();

    AVInputFormat *ifmt=av_find_input_format(format);

    if(avformat_open_input(&input_video_format_context, input, ifmt, NULL)!=0){
        printf("Couldn't open input stream.\n");
        fflush(stdout);
        ret = -1;
    }
    QThread::sleep(1);
    avformat_close_input(&input_video_format_context);
    avformat_free_context(input_video_format_context);
    qDebug() << "test finished";
    return ret;
}

void Utils::ffmpegReg(){
    av_register_all();
    avdevice_register_all();
}

