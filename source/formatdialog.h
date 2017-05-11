#ifndef FORMATDIALOG_H
#define FORMATDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QDebug>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
extern "C" {
#include <type.h>
}
#include "utils.h"
#include "cameracaptrue.h"


class FormatDialog : public QDialog
{
     Q_OBJECT

public:
    FormatDialog(struct global_param_struct *gloabl_param, CameraCaptrue *cameraCaptrue);

private:
    CameraCaptrue *m_cameraCaptrue;
    Utils *m_utils;
    struct global_param_struct * m_global_param;

    QComboBox *cmbVideoSourceDesktopFormat;
    QComboBox *cmbVideoSourceDesktopInput;
    QComboBox *cmbVideoSourceCameraFormat;
    QComboBox *cmbVideoSourceCameraInput;
    QComboBox *cmbAudioSourceMicFormat;
    QComboBox *cmbAudioSourceMicInput;
    QComboBox *cmbAudioSourcePcFormat;
    QComboBox *cmbAudioSourcePcInput;

    QLabel *labelVideoSourceDesktopTest;
    QLabel *labelVideoSourceCameraTest;
    QLabel *labelAudioSourceMicTest;
    QLabel *labelAudioSourcePcTest;

    QRadioButton *rdbFullScreen, *rdbMouseScreen, *rdbCustomRect;
    QLineEdit *edtVideoRectLeft, *edtVideoRectTop, *edtVideoRectWidth, *edtVideoRectHeight;
    QRadioButton *rdbMic, *rdbPc, *rdbMicAndPc;

    QCheckBox *chkCameraIsShowVideo;
    QLineEdit *edtCameraShowVideoRectLeft, *edtCameraShowVideoRectTop, *edtCameraShowVideoRectWidth, *edtCameraShowVideoRectHeight;

private:
    QTabWidget* CreateTab();
    void initWin();
    void initData();
private slots:
    void okSlot();
    void cancelSlot();
protected:
    void showEvent(QShowEvent *event);

};

#endif // FORMATDIALOG_H
