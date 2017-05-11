#include "outcapture.h"
#include <QDebug>

OutCapture::OutCapture()
{

}

AVFormatContext* OutCapture::open(QString fileName){
    AVIOContext *output_io_context = NULL;
    int error;
    /** Open the output file to write to it. */
    if ((error = avio_open(&output_io_context, fileName.toLocal8Bit().data(), AVIO_FLAG_READ_WRITE)) < 0) {
        qDebug() << "Could not open output file '" << fileName << "' (error '" << error <<  "')\n";
        return NULL;
    }
    /** Create a new format context for the output container format. */
    if (!(m_formatContext = avformat_alloc_context())) {
        qDebug() << "Could not allocate output format context\n";
        return NULL;
    }
    /** Associate the output file (pointer) with the container format context. */
    m_formatContext->pb = output_io_context;
    /** Guess the desired container format based on the file extension. */
    if (!(m_formatContext->oformat = av_guess_format(NULL, fileName.toLocal8Bit().data(), NULL))) {
        qDebug() <<  "Could not find output file format" ;
        close();
    }
    av_strlcpy(m_formatContext->filename, fileName.toLocal8Bit().data(), sizeof(m_formatContext->filename));

    return m_formatContext;
}

bool OutCapture::writeHeader(){
    int error = 0;
    if ((error = avformat_write_header(m_formatContext, NULL)) < 0) {
        qDebug() << "Could not write output file header (error '" << error << "')";
        return false;
    }
    return true;
}

bool OutCapture::wirteTrailer(){
    int error;
    if ((error = av_write_trailer(m_formatContext)) < 0) {
        qDebug() <<  "Could not write output file trailer (error '" << error << "')\n";
        return false;
    }
    return true;
}

void OutCapture::close(){
    avio_closep(&(m_formatContext->pb));
    if (m_formatContext)
        avformat_free_context(m_formatContext);
    m_formatContext = NULL;
}
