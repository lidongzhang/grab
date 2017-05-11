#include "videowindow.h"
#include <QMouseEvent>

VideoWindow::VideoWindow(QWidget *parent) : QWidget(parent)
{
    initSDL();
}

void VideoWindow::setPostionRect(QRect postionRect){
    this->setGeometry(postionRect);
    this->setFixedSize(postionRect.width(),postionRect.height());
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);

    if ( m_SDL.texture != NULL )
        SDL_DestroyTexture(m_SDL.texture);
    if ( m_SDL.renderer != NULL)
        SDL_DestroyRenderer(m_SDL.renderer);
    if ( m_SDL.window != NULL)
        SDL_DestroyWindow;

    m_SDL.window = SDL_CreateWindowFrom((void*) this->winId());
    m_SDL.renderer = SDL_CreateRenderer(m_SDL.window , -1, SDL_RENDERER_SOFTWARE);
    Uint32 pixformat = SDL_PIXELFORMAT_IYUV;//yuv420p
    m_SDL.texture = SDL_CreateTexture(m_SDL.renderer, pixformat,SDL_TEXTUREACCESS_STREAMING,
                                      postionRect.width(), postionRect.height());
    m_SDL.rect.x = 0;
    m_SDL.rect.y = 0;
    m_SDL.rect.w = postionRect.width();
    m_SDL.rect.h = postionRect.height();
}

void VideoWindow::initSDL(){
    m_SDL.window = NULL;
    m_SDL.texture = NULL;
    m_SDL.renderer = NULL;

    if ( SDL_Init(SDL_INIT_VIDEO))
      {
        qDebug() << "Could not initialize SDL - "  << SDL_GetError();
        return ;
      }
    m_SDL.window = SDL_CreateWindowFrom((void*) this->winId());
    if(!m_SDL.window){
        qDebug() << "SDL: could not create window - exiting:" << SDL_GetError() ;
        return ;
      }
    m_SDL.renderer = SDL_CreateRenderer(m_SDL.window , -1, SDL_RENDERER_SOFTWARE);  //SDL_RENDERER_ACCELERATED don't display
    if (!m_SDL.renderer){
        qDebug() << "SDL: could not create renderer - exiting:" << SDL_GetError() ;
        return ;
    }
}

void VideoWindow::displayCameraVideo(char *buffer){
    SDL_UpdateTexture(m_SDL.texture, NULL, buffer, m_SDL.rect.w);
    SDL_RenderClear(m_SDL.renderer);
    SDL_RenderCopy(m_SDL.renderer, m_SDL.texture, NULL, &(m_SDL.rect));
    SDL_RenderPresent(m_SDL.renderer);
}

//获取鼠标点定位窗体位置
void VideoWindow::mousePressEvent(QMouseEvent *e)
{
    last = e->globalPos();
}
void VideoWindow::mouseMoveEvent(QMouseEvent *e)
{
    int dx = e->globalX() - last.x();
    int dy = e->globalY() - last.y();
    last = e->globalPos();
    move(x()+dx,y()+dy);
}
void VideoWindow::mouseReleaseEvent(QMouseEvent *e)
{
    int dx = e->globalX() - last.x();
    int dy = e->globalY() - last.y();
    move(x()+dx, y()+dy);
}
