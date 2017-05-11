#include "cameracaptrue.h"

CameraCaptrue::CameraCaptrue()
{
    m_codecContext = NULL;
    m_formatContext = NULL;
    m_opened = false;

}

bool CameraCaptrue::open(QString formatName, QString inputName, WId winId, QRect cameraShowVideoRect){
    this->m_winId = winId;
    this->m_cameraShowVideoRect = cameraShowVideoRect;

    m_formatContext = avformat_alloc_context();

    AVInputFormat *ifmt=av_find_input_format(formatName.toLocal8Bit().data());
    AVDictionary* options = NULL;
    //Set some options
    //grabbing frame rate
    av_dict_set(&options,"framerate","25",0);
    if(avformat_open_input(&m_formatContext, inputName.toLocal8Bit().data(), ifmt, &options)!=0){
        qDebug() << "Couldn't open input stream.";
        return false;
    }

    if(avformat_find_stream_info(m_formatContext,NULL)<0){
        qDebug() << "Couldn't find stream information.";
        return false;
    }

    if ( m_formatContext->nb_streams != 1){
        qDebug() << "input_video number streams is not 1. ";
        avformat_close_input(&m_formatContext);
        return false;
    }
    m_opened = true;
    m_streamIndex = 0;
    m_codecContext = m_formatContext->streams[m_streamIndex]->codec;
    AVCodec*
    pCodec = avcodec_find_decoder(m_codecContext->codec_id);
    if (pCodec == NULL)
    {
       qDebug() << ("video codec not found. \n");
       return false;
    }
    if(avcodec_open2(m_codecContext, pCodec, NULL) < 0){
        qDebug() << "Could not open video codec. ";
        return false;
    }
    m_frame = av_frame_alloc();
    m_frameYUV = av_frame_alloc();

    uint8_t *
    out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, m_cameraShowVideoRect.width(), m_cameraShowVideoRect.height()));
    avpicture_fill((AVPicture *)(m_frameYUV), out_buffer, AV_PIX_FMT_YUV420P, m_cameraShowVideoRect.width(), m_cameraShowVideoRect.height());

    m_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    m_swsContext = sws_getContext(m_codecContext->width,
                                  m_codecContext->height,
                                  m_codecContext->pix_fmt,
                                  m_cameraShowVideoRect.width(),
                                  m_cameraShowVideoRect.height(),
                                  AV_PIX_FMT_YUV420P,
                                  SWS_BICUBIC,
                                  NULL,NULL,NULL);

    return true;
}

char* CameraCaptrue::getCameraVideoBuffer(){
    if(av_read_frame(m_formatContext, m_packet)>=0){
        if(m_packet->stream_index==m_streamIndex){
          int got_picture=0;
          int ret = avcodec_decode_video2(m_codecContext, m_frame, &got_picture, m_packet);
          if(ret < 0){
            printf("Decode Error.\n");
            return NULL;
          }
          if(got_picture){

            /*
            //split v
            m_frame->data[0] += m_frame->linesize[0] * (m_codecContext->height - 1);
            m_frame->linesize[0] *= -1;
            m_frame->data[1] += m_frame->linesize[1] * (m_codecContext->height / 2 - 1);
            m_frame->linesize[1] *= -1;
            m_frame->data[2] += m_frame->linesize[2] * (m_codecContext->height / 2 - 1);
            m_frame->linesize[2] *= -1;
            */

            sws_scale(m_swsContext, (const unsigned char* const*)m_frame->data, m_frame->linesize, 0,  m_codecContext->height,
                      m_frameYUV->data,
                      m_frameYUV->linesize);

        }

     }
        av_free_packet(m_packet);
    }
    return (char *) m_frameYUV->data[0];
}

void CameraCaptrue::close(){
    if (m_codecContext){
        avcodec_close(m_codecContext);
        m_codecContext = NULL;
    }
    if (m_formatContext){
        avformat_close_input(&m_formatContext);
        m_formatContext = NULL;
    }
}
