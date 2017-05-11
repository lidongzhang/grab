#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QWidget>
#include <SDL2/SDL.h>
#undef main
#include <QDebug>

struct SDLStruct{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture* texture;
    SDL_Rect  rect;
};

class VideoWindow : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWindow(QWidget *parent = 0);
    void setPostionRect(QRect postionRect);
    void displayCameraVideo(char *buffer);
private:
    SDLStruct m_SDL;
    void initSDL();
    QPoint last;
signals:

public slots:


protected:
    //鼠标按下
    void mousePressEvent(QMouseEvent *e);
    //鼠标移动
    void mouseMoveEvent(QMouseEvent *e);
    //鼠标释放
    void mouseReleaseEvent(QMouseEvent *e);
};

#endif // VIDEOWINDOW_H
