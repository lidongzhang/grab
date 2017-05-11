#include "miccapture.h"
#include <QDebug>

/** Add converted input audio samples to the FIFO buffer for later processing. */
int MicCapture::p_add_samples_to_fifo(AVAudioFifo *fifo,
                               uint8_t **converted_input_samples,
                               const int frame_size)
{
    int error;
    /**
     * Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples.
     */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }
    /** Store the new samples in the FIFO buffer. */
    m_mute.lock();
    if (av_audio_fifo_write(fifo, (void **)converted_input_samples,
                            frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    m_mute.unlock();
    return 0;
}

/**
 * Convert the input audio samples into the output sample format.
 * The conversion happens on a per-frame basis, the size of which is specified
 * by frame_size.
 */
int MicCapture::p_convert_samples(const uint8_t **input_data,
                           uint8_t **converted_data, const int frame_size,
                           SwrContext *resample_context)
{
    int error;
    /** Convert the samples using the resampler. */
    if ((error = swr_convert(resample_context,
                             converted_data, frame_size,
                             input_data    , frame_size)) < 0) {
        qDebug() <<  "Could not convert input samples (error '" << error << "')\n";
        return error;
    }
    return 0;
}

/**
 * Initialize a temporary storage for the specified number of audio samples.
 * The conversion requires temporary storage due to the different format.
 * The number of audio samples to be allocated is specified in frame_size.
 */
int MicCapture::p_init_converted_samples(uint8_t ***converted_input_samples,
                                  AVCodecContext *output_codec_context,
                                  int frame_size)
{
    int error;
    /**
     * Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = (uint8_t **)calloc(output_codec_context->channels,
                                            sizeof(**converted_input_samples)))) {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }
    /**
     * Allocate memory for the samples of all channels in one consecutive
     * block for convenience.
     */
    if ((error = av_samples_alloc(*converted_input_samples, NULL,
                                  output_codec_context->channels,
                                  frame_size,
                                  output_codec_context->sample_fmt, 0)) < 0) {
        qDebug() <<  "Could not allocate converted input samples (error '" << error <<"')\n";
        av_freep(&(*converted_input_samples)[0]);
        free(*converted_input_samples);
        return error;
    }
    return 0;
}

/** Decode one audio frame from the input file. */
int MicCapture::p_decode_audio_frame(AVFrame *frame,
                              AVFormatContext *input_format_context,
                              AVCodecContext *input_codec_context,
                              int *data_present, int *finished)
{
    /** Packet used for temporary storage. */
    AVPacket input_packet;
    int error;
    p_init_packet(&input_packet);
    /** Read one audio frame from the input file into a temporary packet. */
    if ((error = av_read_frame(input_format_context, &input_packet)) < 0) {
        /** If we are at the end of the file, flush the decoder below. */
        if (error == AVERROR_EOF)
            *finished = 1;
        else {
            qDebug() << "Could not read frame (error '" << error << "')";
            return error;
        }
    }
    /**
     * Decode the audio frame stored in the temporary packet.
     * The input audio stream decoder is used to do this.
     * If we are at the end of the file, pass an empty packet to the decoder
     * to flush it.
     */
    if ((error = avcodec_decode_audio4(input_codec_context, frame,
                                       data_present, &input_packet)) < 0) {
        qDebug() << "Could not decode frame (error '" << error <<  "')\n";
        av_packet_unref(&input_packet);
        return error;
    }
    /**
     * If the decoder has not been flushed completely, we are not finished,
     * so that this function has to be called again.
     */
    if (*finished && *data_present)
        *finished = 0;
    av_packet_unref(&input_packet);
    return 0;
}

/** Initialize one audio frame for reading from the input file */
int MicCapture::p_init_input_frame(AVFrame **frame)
{
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate input frame\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

/**
 * Read one audio frame from the input file, decodes, converts and stores
 * it in the FIFO buffer.
 */
int MicCapture::p_read_decode_convert_and_store(AVAudioFifo *fifo,
                                         AVFormatContext *input_format_context,
                                         AVCodecContext *input_codec_context,
                                         AVCodecContext *output_codec_context,
                                         SwrContext *resampler_context,
                                         int *finished)
{
    /** Temporary storage of the input samples of the frame read from the file. */
    AVFrame *input_frame = NULL;
    /** Temporary storage for the converted input samples. */
    uint8_t **converted_input_samples = NULL;
    int data_present;
    int ret = AVERROR_EXIT;
    /** Initialize temporary storage for one input frame. */
    if (p_init_input_frame(&input_frame))
        goto cleanup;
    /** Decode one frame worth of audio samples. */
    if (p_decode_audio_frame(input_frame, input_format_context,
                           input_codec_context, &data_present, finished))
        goto cleanup;
    //printf("audio frame nb_sample:%d", input_frame->nb_samples);
    //fflush(stdout);
    /**
     * If we are at the end of the file and there are no more samples
     * in the decoder which are delayed, we are actually finished.
     * This must not be treated as an error.
     */
    if (*finished && !data_present) {
        ret = 0;
        goto cleanup;
    }
    /** If there is decoded data, convert and store it */
    if (data_present) {
        /** Initialize the temporary storage for the converted input samples. */
        if (p_init_converted_samples(&converted_input_samples, output_codec_context,
                                   input_frame->nb_samples))
            goto cleanup;
        /**
         * Convert the input samples to the desired output sample format.
         * This requires a temporary storage provided by converted_input_samples.
         */
        if (p_convert_samples((const uint8_t**)input_frame->extended_data, converted_input_samples,
                            input_frame->nb_samples, resampler_context))
            goto cleanup;
        /** Add the converted input samples to the FIFO buffer for later processing. */
        if (p_add_samples_to_fifo(fifo, converted_input_samples,
                                input_frame->nb_samples))
            goto cleanup;
        ret = 0;
    }
    ret = 0;
cleanup:
    if (converted_input_samples) {
        av_freep(&converted_input_samples[0]);
        free(converted_input_samples);
    }
    av_frame_free(&input_frame);
    return ret;
}

/** Initialize one data packet for reading or writing. */
void MicCapture::p_init_packet(AVPacket *packet)
{
    av_init_packet(packet);
    /** Set the packet data and size so that it is recognized as being empty. */
    packet->data = NULL;
    packet->size = 0;
}

/** Encode one frame worth of audio to the output file. */
int MicCapture::p_encode_audio_frame(AVFrame *frame,
                              AVFormatContext *output_format_context,
                              AVCodecContext *output_codec_context,
                              int *data_present)
{
    /** Packet used for temporary storage. */
    AVPacket output_packet;
    int error;
    p_init_packet(&output_packet);
    /** Set a timestamp based on the sample rate for the container. */
    if (frame) {
        frame->pts = m_pts;
        m_pts += frame->nb_samples;
    }else
    {
        int t =1;
    }
    /**
     * Encode the audio frame and store it in the temporary packet.
     * The output audio stream encoder is used to do this.
     */
    if ((error = avcodec_encode_audio2(output_codec_context, &output_packet,
                                       frame, data_present)) < 0) {
        qDebug() << "Could not encode frame (error '" << error << "')";
        av_packet_unref(&output_packet);
        return error;
    }
    /** Write one audio frame from the temporary packet to the output file. */
    if (*data_present) {
        if ((error = av_write_frame(output_format_context, &output_packet)) < 0) {
            qDebug() <<  "Could not write frame (error '" << error << "')\n";
            av_packet_unref(&output_packet);
            return error;
        }
        av_packet_unref(&output_packet);
    }
    return 0;
}

/**
 * Initialize one input frame for writing to the output file.
 * The frame will be exactly frame_size samples large.
 */
int MicCapture::p_init_output_frame(AVFrame **frame,
                             AVCodecContext *output_codec_context,
                             int frame_size)
{
    int error;
    /** Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate output frame\n");
        return AVERROR_EXIT;
    }
    /**
     * Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity.
     */
    (*frame)->nb_samples     = frame_size;
    (*frame)->channel_layout = output_codec_context->channel_layout;
    (*frame)->format         = output_codec_context->sample_fmt;
    (*frame)->sample_rate    = output_codec_context->sample_rate;
    /**
     * Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified.
     */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
        qDebug() <<  "Could allocate output frame samples (error '" << error <<  "')\n";
        av_frame_free(frame);
        return error;
    }
    return 0;
}


/**
 * Load one audio frame from the FIFO buffer, encode and write it to the
 * output file.
 */
int MicCapture::p_load_encode_and_write(AVAudioFifo *fifo,
                                 AVFormatContext *output_format_context,
                                 AVCodecContext *output_codec_context)
{
    /** Temporary storage of the output samples of the frame written to the file. */
    AVFrame *output_frame;
    /**
     * Use the maximum number of possible samples per frame.
     * If there is less than the maximum possible frame size in the FIFO
     * buffer use this number. Otherwise, use the maximum possible frame size
     */
    const int frame_size = FFMIN(av_audio_fifo_size(fifo),
                                 output_codec_context->frame_size);
    int data_written;
    /** Initialize temporary storage for one output frame. */
    if (p_init_output_frame(&output_frame, output_codec_context, frame_size))
        return AVERROR_EXIT;
    /**
     * Read as many samples from the FIFO buffer as required to fill the frame.
     * The samples are stored in the frame temporarily.
     */
    m_mute.lock();
    if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size) {
        fprintf(stderr, "Could not read data from FIFO\n");
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    m_mute.unlock();
    /** Encode one frame worth of audio samples. */
    if (p_encode_audio_frame(output_frame, output_format_context,
                           output_codec_context, &data_written)) {
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    av_frame_free(&output_frame);
    return 0;
}

MicCapture::MicCapture(QObject *parent) : QObject(parent)
{

}

bool MicCapture::open(QString formatName, QString inputName, AVFormatContext *outFormatContext){
    m_pts = 0;
    this->m_outFormatContext = outFormatContext;
    AVCodec *input_codec;
    int error;
    AVInputFormat *ifmt = NULL;
    ifmt = av_find_input_format(formatName.toLocal8Bit().data()); //alsa
    if (ifmt == NULL){
        qDebug() << ("av_find_input_format is error!");
        return false;
    }
    m_formatContext = avformat_alloc_context();
    /** Open the input file to read from it. */
    if ((error = avformat_open_input(&m_formatContext, inputName.toLocal8Bit().data(), ifmt, NULL)) < 0) { //pulse
        qDebug() <<  "Could not open input file mic (error '" << error << "')\n";
        m_formatContext = NULL;
        return false;
    }
    /** Get information on the input file (number of streams etc.). */
    if ((error = avformat_find_stream_info(m_formatContext, NULL)) < 0) {
        qDebug() << "Could not open find stream info (error '" << error << "')";
        avformat_close_input(&m_formatContext);
        return false;
    }
    /** Make sure that there is only one stream in the input file. */
    if ((m_formatContext)->nb_streams != 1) {
        qDebug() << "Expected one audio input stream, but found "  << m_formatContext->nb_streams;
        avformat_close_input(&m_formatContext);
        return false;
    }
    m_streamIndex = 0;
    /** Find a decoder for the audio stream. */
    if (!(input_codec = avcodec_find_decoder((m_formatContext)->streams[m_streamIndex]->codec->codec_id))) {
        qDebug() << "Could not find input codec";
        avformat_close_input(&m_formatContext);
        return false;
    }
    /** Open the decoder for the audio stream to use it later. */
    if ((error = avcodec_open2((m_formatContext)->streams[m_streamIndex]->codec, input_codec, NULL)) < 0) {
        qDebug() << "Could not open input codec (error '" << error << "')",
        avformat_close_input(&m_formatContext);
        return false;
    }
    /** Save the decoder context for easier access later. */
    m_codecContext = (m_formatContext)->streams[m_streamIndex]->codec;

    //set out stream
    AVCodec *output_codec;
    AVStream *stream;
    /** Find the encoder to be used by its name. */
    if (!(output_codec = avcodec_find_encoder(AV_CODEC_ID_AAC))) {
        qDebug() << "Could not find an AAC encoder.";
        close();
        return false;
    }
    /** Create a new audio stream in the output file container. */
    if (!(stream = avformat_new_stream(m_outFormatContext, output_codec))) {
        qDebug() << "Could not create new stream.";
        close();
        return false;
    }
    m_outStreamIndex = stream->index;
    /** Save the encoder context for easier access later. */
    m_outCodecContext = stream->codec;
    /**
     * Set the basic encoder parameters.
     * The input file's sample rate is used to avoid a sample rate conversion.
     */
    m_outCodecContext->channels       = m_codecContext->channels;
    m_outCodecContext->channel_layout = av_get_default_channel_layout(m_codecContext->channels);
    m_outCodecContext->sample_rate    = m_codecContext->sample_rate;
    m_outCodecContext->sample_fmt     = output_codec->sample_fmts[0];
    m_outCodecContext->bit_rate       = OUTPUT_BIT_RATE;
    /** Allow the use of the experimental AAC encoder */
    m_outCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    /** Set the sample rate for the container. */
    stream->time_base.den = m_codecContext->sample_rate;
    stream->time_base.num = 1;
    /**
     * Some container formats (like MP4) require global headers to be present
     * Mark the encoder so that it behaves accordingly.
     */
    if (m_outFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
        m_outCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    /** Open the encoder for the audio stream to use it later. */
    if ((error = avcodec_open2(m_outCodecContext, output_codec, NULL)) < 0) {
        qDebug() << "Could not open output codec (error '" << error << "')";
        close();
        return false;
    }

    m_resampleContextl = swr_alloc_set_opts(NULL,
                                          av_get_default_channel_layout(m_outCodecContext->channels),
                                          m_outCodecContext->sample_fmt,
                                          m_outCodecContext->sample_rate,
                                          av_get_default_channel_layout(m_codecContext->channels),
                                          m_codecContext->sample_fmt,
                                          m_codecContext->sample_rate,
                                          0, NULL);
    if (!m_resampleContextl) {
        qDebug() <<  "Could not allocate resample context.";
        return false;
    }
    if (swr_init(m_resampleContextl) < 0) {
        qDebug() << "Could not open resample context.";
        swr_free(&m_resampleContextl);
        return false;
    }
    if (!(m_fifo = av_audio_fifo_alloc(m_outCodecContext->sample_fmt,
                                      m_outCodecContext->channels, 30 * 64))) {
        qDebug() <<  "Could not allocate FIFO.";
        return false;
    }
    return true;
}

bool MicCapture::capture(){
    /** Use the encoder's desired frame size for processing. */

    const int output_frame_size = m_outCodecContext->frame_size;
    int finished                = 0;
    /**
     * Make sure that there is one frame worth of samples in the FIFO
     * buffer so that the encoder can do its work.
     * Since the decoder's and the encoder's frame size may differ, we
     * need to FIFO buffer to store as many frames worth of input samples
     * that they make up at least one frame worth of output samples.
     */
    while (av_audio_fifo_size(m_fifo) < output_frame_size) {
        /**
         * Decode one frame worth of audio samples, convert it to the
         * output sample format and put it into the FIFO buffer.
         */
        if (p_read_decode_convert_and_store(m_fifo, m_formatContext,
                                          m_codecContext,
                                          m_outCodecContext,
                                          m_resampleContextl, &finished))
            close();
        /**
         * If we are at the end of the input file, we continue
         * encoding the remaining audio samples to the output file.
         */
        if (finished)
            break;
    }

    return true;
}

bool MicCapture::encoder(){
    const int output_frame_size = m_outCodecContext->frame_size;
    int finished = 0;
    /**
     * If we have enough samples for the encoder, we encode them.
     * At the end of the file, we pass the remaining samples to
     * the encoder.
     */
    while (av_audio_fifo_size(m_fifo) >= output_frame_size ||
           (finished && av_audio_fifo_size(m_fifo) > 0))
        /**
         * Take one frame worth of audio samples from the FIFO buffer,
         * encode it and write it to the output file.
         */
        if (p_load_encode_and_write(m_fifo, m_outFormatContext,
                                  m_outCodecContext))
            close();
    /**
     * If we are at the end of the input file and have encoded
     * all remaining samples, we can exit this loop and finish.
     */
    if (finished) {
        int data_written;
        /** Flush the encoder as it may have delayed frames. */
        do {
            if (p_encode_audio_frame(NULL, m_outFormatContext,
                                   m_outCodecContext, &data_written))
                close();
        } while (data_written);

    }
    return true;
}

bool MicCapture::flushEncoder(){
   const int output_frame_size = m_outCodecContext->frame_size;
   /**
    * If we have enough samples for the encoder, we encode them.
    * At the end of the file, we pass the remaining samples to
    * the encoder.
    */
   int finished = 1;
   while (av_audio_fifo_size(m_fifo) >= output_frame_size ||
          (finished && av_audio_fifo_size(m_fifo) > 0))
       /**
        * Take one frame worth of audio samples from the FIFO buffer,
        * encode it and write it to the output file.
        */
       if (p_load_encode_and_write(m_fifo, m_outFormatContext,
                                 m_outCodecContext))
           close();
   /**
    * If we are at the end of the input file and have encoded
    * all remaining samples, we can exit this loop and finish.
    */
   if (finished) {
       int data_written;
       /** Flush the encoder as it may have delayed frames. */
       do {
           printf("flush audio \n");
           fflush(stdout);
           if (p_encode_audio_frame(NULL, m_outFormatContext,
                                  m_outCodecContext, &data_written))
               close();
       } while (data_written);

   }
}

bool MicCapture::close(){
    if (m_fifo)
        av_audio_fifo_free(m_fifo);
    swr_free(&m_resampleContextl);
    if (m_outCodecContext)
        avcodec_close(m_outCodecContext);

    if (m_codecContext)
        avcodec_close(m_codecContext);
    if (m_formatContext)
        avformat_close_input(&m_formatContext);
    return true;
}

