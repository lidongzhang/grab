#ifndef MICCAPTURE_H
#define MICCAPTURE_H

#include <QObject>
#include <QString>
#include <QMutex>

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

#define OUTPUT_BIT_RATE 96000
#define OUTPUT_CHANNELS 2

class MicCapture : public QObject
{
    Q_OBJECT
public:
    explicit MicCapture(QObject *parent = 0);

signals:

private:

    AVFormatContext *m_formatContext;
    AVCodecContext *m_codecContext;
    AVFormatContext *m_outFormatContext;
    AVCodecContext *m_outCodecContext;
    SwrContext *m_resampleContextl;

    QMutex m_mute;

private:

    int p_encode_audio_frame(AVFrame *frame,
                                  AVFormatContext *output_format_context,
                                  AVCodecContext *output_codec_context,
                                  int *data_present);
    int p_init_output_frame(AVFrame **frame,
                                 AVCodecContext *output_codec_context,
                                 int frame_size);
    int p_load_encode_and_write(AVAudioFifo *fifo,
                                     AVFormatContext *output_format_context,
                                     AVCodecContext *output_codec_context);
    void p_init_packet(AVPacket *packet);
    int p_read_decode_convert_and_store(AVAudioFifo *fifo,
                                             AVFormatContext *input_format_context,
                                             AVCodecContext *input_codec_context,
                                             AVCodecContext *output_codec_context,
                                             SwrContext *resampler_context,
                                             int *finished);
    int p_init_input_frame(AVFrame **frame);
    int p_decode_audio_frame(AVFrame *frame,
                           AVFormatContext *input_format_context,
                           AVCodecContext *input_codec_context,
                           int *data_present, int *finished);
    int p_init_converted_samples(uint8_t ***converted_input_samples,
                                 AVCodecContext *output_codec_context,
                                 int frame_size);
    int p_convert_samples(const uint8_t **input_data,
                        uint8_t **converted_data, const int frame_size,
                        SwrContext *resample_context);
    int p_add_samples_to_fifo(AVAudioFifo *fifo,
                            uint8_t **converted_input_samples,
                            const int frame_size);

public:
    AVAudioFifo *m_fifo;
    int m_outStreamIndex;
    int64_t m_pts;
    int m_streamIndex;
    bool open(QString formatName, QString inputName, AVFormatContext *outFormatContext);
    bool capture();
    bool encoder();
    bool flushEncoder();
    bool close();

public slots:
};

#endif // MICCAPTURE_H
