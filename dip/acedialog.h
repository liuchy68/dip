#ifndef ACEDIALOG_H
#define ACEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSlider>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QImage>
#include "imageprocess.h"
#include "floatslider.h"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QSlider;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QBoxLayout;
class QHBoxLayout;
class QVBoxLayout;
QT_END_NAMESPACE



//自定义对话框类 它继承于  QDialog
class ACEDialog : public QDialog
{
    Q_OBJECT
public:
    ACEDialog(QImage inputImage);  //构造函数，其实现见  acedialog.cpp
    ~ACEDialog()  //析构函数，用于释放内存
    {
        if (rgb)
            delete [] rgb;
        if (rgb_pad)
            delete [] rgb_pad;
        if (rgb_ii)
            delete [] rgb_ii;
        if (rgb_ii_power)
            delete [] rgb_ii_power;
    }

    //获取目标图像用于输出
    QImage getImage() {return dstImage;}


private:
    void iniUI();
    QImage srcImage;
    float *rgb = nullptr;
    float *rgb_pad = nullptr;
    float *rgb_ii = nullptr;
    float *rgb_ii_power = nullptr;
    QImage dstImage;
    QLabel *srcImageLabel;
    QLabel *dstImageLabel;

    int maxFilterSize = 100;
    FloatSlider *filterSizeSlider;
    QLineEdit *filterSizeEdit;
    QLabel *filterSizeLabel;

    FloatSlider *maxCGSlider;
    QLineEdit *maxCGEdit;
    QLabel *maxCGLabel;

    FloatSlider *gainCoefSlider;
    QLineEdit *gainCoefEdit;
    QLabel *gainCoefLabel;

    QPushButton     *btnOK;
    QPushButton     *btnCancel;
    QPushButton     *btnClose;

    //初始化信号与槽的链接
    void    iniSignalSlots();

    //预定义参数
    int filterSize = 50;
    float maxCG = 3;
    float gainCoef = 50.0f;

private slots:
    void setImage(QImage &image, QLabel *label);

    void updateDstImage(float value);
};

#endif // ACEDIALOG_H
