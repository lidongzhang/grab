#include "desktopcapture.h"

DesktopCapture::DesktopCapture()
{
}

bool DesktopCapture::open(QString formatName, QString inputName, AVFormatContext *outFormatContext,
                          QString captrueMode, QRect captureRect, QSize screenSize){
    m_outFormatContext = outFormatContext;
    m_captureRect.setLeft(0);
    m_captureRect.setTop(0);
    m_captureRect.setWidth(0);
    m_captureRect.setHeight(0);
    m_formatContext = avformat_alloc_context();
    AVDictionary* options = NULL;
    //Set some options
    //grabbing frame rate
    av_dict_set(&options,"framerate","25",0);//This param can set is what number, need depend camera ability , 10 is ok.
    if ( captrueMode == "fullScreen" ){
        m_captureRect.setLeft(0);
        m_captureRect.setTop(0);
        m_captureRect.setWidth(screenSize.width());
        m_captureRect.setHeight(screenSize.height());
        QString t = QString("%1x%2").arg(m_captureRect.width()).arg(m_captureRect.height());//"640x480"
        av_dict_set(&options,"video_size", t.toLocal8Bit().data(),0);
    }
    else if(captrueMode == "mouseCenter"){
        m_captureRect.setLeft(0);
        m_captureRect.setTop(0);
        //Make the grabbed area follow the mouse
        av_dict_set(&options,"follow_mouse","centered",0);
        //Video frame size. The default is to capture the full screen
        m_captureRect.setWidth(captureRect.width());
        m_captureRect.setHeight(captureRect.height());
        QString t = QString("%1x%2").arg(m_captureRect.width()).arg(m_captureRect.height());//"640x480"
        av_dict_set(&options,"video_size", t.toLocal8Bit().data(),0);
    }else if(captrueMode == "customRect"){
        m_captureRect.setLeft(captureRect.left());
        m_captureRect.setTop(captureRect.top());
        m_captureRect.setWidth(captureRect.width());
        m_captureRect.setHeight(captureRect.height());
        //Video frame size. The default is to capture the full screen
        QString t = QString("%1x%2").arg(m_captureRect.width()).arg(m_captureRect.height());//"640x480"
        av_dict_set(&options,"video_size", t.toLocal8Bit().data(),0);
    }

    AVInputFormat *ifmt=av_find_input_format(formatName.toLocal8Bit().data());//"x11grab"
    //Grab at position 10,20
    QString lefAndTop = QString("+%1,%2").arg(m_captureRect.left()).arg(m_captureRect.top());
    //if ( m_captureRect.left() != 0 && m_captureRect.top() != 0)
    if ( inputName.startsWith(":"))
        inputName += lefAndTop;

    if(avformat_open_input(&m_formatContext, inputName.toLocal8Bit().data(), ifmt, &options)!=0){//":0.0+10,20"
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
    out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, m_captureRect.width(), m_captureRect.height()));
    avpicture_fill((AVPicture *)(m_frameYUV), out_buffer, AV_PIX_FMT_YUV420P, m_captureRect.width(), m_captureRect.height());

    m_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    m_swsContext = sws_getContext(m_codecContext->width,
                                  m_codecContext->height,
                                  m_codecContext->pix_fmt,
                                  m_codecContext->width,
                                  m_codecContext->height,
                                  AV_PIX_FMT_YUV420P,
                                  SWS_BICUBIC,
                                  NULL,NULL,NULL);

    //set out stream
    AVCodec* output_video_codec = NULL;
    output_video_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    AVStream*
    video_st = avformat_new_stream(m_outFormatContext, output_video_codec);
    if(video_st == NULL){
        qDebug() << "avformat_new_stream add video is error!";
        return false;
    }

    m_outStreamIndex = video_st->index;
    m_outCodecContext = video_st->codec;
    m_outCodecContext->codec_id = AV_CODEC_ID_H264;
    m_outCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    m_outCodecContext->pix_fmt =  AV_PIX_FMT_YUV420P;
    m_outCodecContext->width = m_captureRect.width();
    m_outCodecContext->height = m_captureRect.height();
    //m_outCodecContext->bit_rate = 400000; //编码输出的比特率  this param is 严重的影响了编码后的视频质量
    //m_outCodecContext->gop_size = 250; //关键帧的最大间隔帧数

    m_outCodecContext->time_base.num = 1;
    m_outCodecContext->time_base.den = 25;

    m_outCodecContext->qmin = 10;//最小的量化因子。取值范围1-51。建议在10-30之间。
    m_outCodecContext->qmax = 51; //最大的量化因子。取值范围1-51。建议在10-30之间。

    //m_outCodecContext->max_b_frames = 3;//最大B帧数.

    AVDictionary *param = 0;
    // H.264
    //鉴于x264的参数众多，各种参数的配合复杂，为了使用者方便，x264建议如无特别需要可使用preset和tune设置。这套开发者推荐的参数较为合理，可在此基础上在调整一些具体参数以符合自己需要，手动设定的参数会覆盖preset和tune里的参数。
    // --preset的参数主要调节编码速度和质量的平衡，有ultrafast、superfast、veryfast、faster、fast、medium、slow、slower、veryslow、placebo这10个选项，从快到慢。
    //av_dict_set(&param, "preset", "slow", 0);
    //av_dict_set(&param, "preset", "superfast", 0);
    /* --tune的参数主要配合视频类型和视觉优化的参数，或特别的情况。如果视频的内容符合其中一个可用的调整值又或者有其中需要，则可以使用此选项，否则建议不使用（如tune grain是为高比特率的编码而设计的）。
    tune的值有： film：  电影、真人类型；
    animation：  动画；
    grain：      需要保留大量的grain时用；
    stillimage：  静态图像编码时使用；
    psnr：      为提高psnr做了优化的参数；
    ssim：      为提高ssim做了优化的参数；
    fastdecode： 可以快速解码的参数；
    zerolatency：零延迟，用在需要非常低的延迟的情况下，比如电视电话会议的编码
    */
    //av_dict_set(&param, "tune", "zerolatency", 0);  //压缩0延时，但会影响视频质量

    //Show some Information
    //av_dump_format(output_format_context, 0, "/tmp/out.mp4" , 1);

    AVCodec* outPCodec = avcodec_find_encoder(m_outCodecContext->codec_id);
    if (!outPCodec){
        qDebug() << ("Can not find encoder! \n");
        return false;
    }

    if (m_outFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        m_outCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    //This &param not set  also is ok .
    if (avcodec_open2(m_outCodecContext, outPCodec,&param) < 0){
        qDebug() << "Failed to open encoder! \n";
        return false;
    }
    m_outFrame = av_frame_alloc();
    int picture_size = avpicture_get_size(m_outCodecContext->pix_fmt,
                                          m_outCodecContext->width,
                                          m_outCodecContext->height);

    uint8_t* m_outFrameBuf = (uint8_t *)av_malloc(picture_size);
    avpicture_fill((AVPicture *)(m_outFrame), m_outFrameBuf,
                   m_outCodecContext->pix_fmt,
                   m_outCodecContext->width,
                   m_outCodecContext->height);

    m_outPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_new_packet(m_outPacket,picture_size);

    //fifo
    //申请30帧缓存
    m_fifo = av_fifo_alloc(30 * avpicture_get_size(AV_PIX_FMT_YUV420P, m_codecContext->width, m_codecContext->height));

    m_frameIndex = 0;
    return true;
}

bool DesktopCapture::capture(){
    if(av_read_frame(m_formatContext, m_packet)>=0){

        if(m_packet->stream_index== m_streamIndex){

           int got_picture;
           int ret = avcodec_decode_video2(m_codecContext,
                                           m_frame, &got_picture,
                                           m_packet);
            if(ret < 0){
                qDebug() << ("Decode Error.\n");
                return false;
            }
            if(got_picture){
               if ( sws_scale(m_swsContext,
                          (const unsigned char* const*)(m_frame->data),
                          m_frame->linesize, 0,
                          m_codecContext->height,
                          m_frameYUV->data,
                          m_frameYUV->linesize) != m_codecContext->height ){
                   qDebug() << ("sws_scale is error !");
               }

            }
            //write to fifo
            m_mute.lock();
            int height = m_codecContext->height;
            int width = m_codecContext->width;
            int y_size=height*width;

            av_fifo_generic_write(m_fifo, m_frameYUV->data[0], y_size * 1.5, NULL);
            //av_fifo_generic_write(m_fifo, m_frameYUV->data[1], y_size/4, NULL);
            //av_fifo_generic_write(m_fifo, m_frameYUV->data[2], y_size/4, NULL);
            m_mute.unlock();

        }

        av_free_packet(m_packet);
    }

    return true;
}

bool DesktopCapture::encoder()
{
    int vret = 0;
    //encode
    m_mute.lock();
    int size = avpicture_get_size(m_outCodecContext->pix_fmt,
                                  m_captureRect.width(),
                                  m_captureRect.height());
    if(av_fifo_size(m_fifo) >= size ){
        av_fifo_generic_read(m_fifo, m_outFrame->data[0], size, NULL); //this must use m_outFrame->data[0], can't use m_outFrameBuf

    }else
    {
         vret = 1;
    }
    m_mute.unlock();
    if ( vret == 1) return true;

    /*
    m_outFrame->data[0] = m_frameYUV->data[0];
    m_outFrame->data[1] = m_frameYUV->data[1];
    m_outFrame->data[2] = m_frameYUV->data[2];
    m_outFrame->linesize[0] = m_frameYUV->linesize[0];
    m_outFrame->linesize[1] = m_frameYUV->linesize[1];
    m_outFrame->linesize[2] = m_frameYUV->linesize[2];
    */
    m_outFrame->width = m_captureRect.width();
    m_outFrame->height = m_captureRect.height();
    m_outFrame->format = AV_PIX_FMT_YUV420P;

    AVRational r;
    r.den = 25;
    r.num = 1;
    m_outFrame->pts = m_frameIndex; //(av_gettime() - start_av_gettime) * 25 ;
    //qDebug() << "video frame pts:" << m_outFrame->pts;
    int got_picture=0;
    //Encode
    int ret = avcodec_encode_video2(m_outCodecContext,
                                m_outPacket,
                                m_outFrame, &got_picture);
    if(ret < 0){
        qDebug() << ("Video failed to encode! \n");
        return false;
    }
    if (got_picture==1){
        //qDebug() << "packet pts:" <<  m_outPacket->pts;
        //qDebug() << "Succeed to encode frame: " << frame_index << ", tsize:" << m_outPacket->size;
        //next code is very import , it can use video time is right.
        if (m_outPacket->pts != AV_NOPTS_VALUE)
            m_outPacket->pts = av_rescale_q(m_outPacket->pts, m_outCodecContext->time_base,
                                                    m_outFormatContext->streams[m_outStreamIndex]->time_base);

        if (m_outPacket->dts != AV_NOPTS_VALUE)
             m_outPacket->dts = av_rescale_q(m_outPacket->dts, m_outCodecContext->time_base,
                                                    m_outFormatContext->streams[m_outStreamIndex]->time_base);

        m_outPacket->stream_index = m_outStreamIndex;

        m_pts = m_outPacket->pts;

        ret = av_write_frame(m_outFormatContext, m_outPacket);
        av_free_packet(m_outPacket);
    }
    m_frameIndex++;

    return true;
}

bool DesktopCapture::flushEncoder(){
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(m_outFormatContext->streams[m_outStreamIndex]->codec->codec->capabilities &
        CODEC_CAP_DELAY))
        return 0;
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2 (m_outFormatContext->streams[m_outStreamIndex]->codec, &enc_pkt,
            NULL, &got_frame);

        //next code is very import, it can use video time is right;
        if (enc_pkt.pts != AV_NOPTS_VALUE)
            enc_pkt.pts = av_rescale_q(enc_pkt.pts, m_outCodecContext->time_base,
                                       m_outFormatContext->streams[m_outStreamIndex]->time_base);

        if (enc_pkt.dts != AV_NOPTS_VALUE)
             enc_pkt.dts = av_rescale_q(enc_pkt.dts, m_outCodecContext->time_base,
                                        m_outFormatContext->streams[m_outStreamIndex]->time_base);

        enc_pkt.stream_index = m_outStreamIndex;
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame){
            ret=0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",enc_pkt.size);
        fflush(stdout);
        /* mux encoded frame */
        ret = av_write_frame(m_outFormatContext, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret == 0 ? true : false ;
}

bool DesktopCapture::close(){

    av_fifo_free(m_fifo);

    if (m_outCodecContext)
        avcodec_close(m_outCodecContext);
    if (m_codecContext)
        avcodec_close(m_codecContext);
    if (m_formatContext)
        avformat_close_input(&m_formatContext);
    return true;
}
