#ifndef TYPE_H
#define TYPE_H


struct rect_struct{
    int left;
    int top;
    int width;
    int height;
};

struct global_param_struct{
    char os_name[50];

    char video_desktop_format_name[50];
    char video_desktop_input_name[50];
    char video_camera_format_name[50];
    char video_camera_input_name[50];

    char audio_mic_format_name[50];
    char audio_mic_input_name[50];
    char audio_pc_format_name[50];
    char audio_pc_input_name[50];

    char ini_file_name[2048];
    char video_capture_mode[50]; //fullScreen, mouseCenter, customRect;
    struct rect_struct video_capture_rect;
    char audio_capture_mode[50]; //mic, pc, micAndPc;

    int desktop_width;
    int desktop_height;

    bool camera_isShowVideo;
    struct rect_struct camera_show_video_rect;
};

#ifdef GLOBALS
    struct global_param_struct global_param;
#else
    extern struct global_param_struct global_param;
#endif

#endif // TYPE_H
