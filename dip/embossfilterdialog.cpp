#include "embossfilterdialog.h"
//浮雕滤波  原理与空域滤波类似 只是所用的kernel不同，详见 sdfilterdialog.cpp

//8个核   每个核是一个 3*3 矩阵
float emboss[8][9]=
{
    {
        -1, 0, 0,
         0, 0, 0,
         0, 0, 1
    },
    {
         1, 0, 0,
         0, 0, 0,
         0, 0, -1
    },
    {
         0, 0, -1,
         0, 0, 0,
         1, 0, 0
    },
    {
         0, 0, 1,
         0, 0, 0,
        -1, 0, 0
    },
    {
        -1, 0, -1,
         0, 0,  0,
         1, 0,  1
    },
    {
        -1, 0, 1,
         0, 0, 0,
         1, 0, -1
    },
    {
        1, 0, 1,
        0, 0, 0,
       -1, 0, -1
    },
    {
        1, 0, -1,
        0, 0, 0,
       -1, 0, 1
    }
};


/***********************************************************************************************/

//核心！
//空域滤波函数  为下面的   imageFilter   imageLOGFilter  函数服务
//参数说明：
        //src 原图向量     note:3个连续值为一组rgb值
        //w h  原图宽高

        //kernel 核（模板）向量
        //hkw  hkh  半核宽高

        //dst 目标图向量   note:3个连续值为一组rgb值s
//注意输入图像尺寸 >  输出图像尺寸  因此必要时要进行 padding 操作
static void filterProc(const uchar *src, int w, int h,
            const float *kernel, int hkw, int hkh, uchar *dst)
{
    int i, j, m, n;
    float val_r, val_g, val_b, r, g, b;

    //目标图像宽高
    int nw = w-2*hkw;
    int nh = h-2*hkh;

    //利用半核宽高计算 kernel 的具体 宽高
    int kw = 2*hkw+1;
    //int kh = 2*hkh+1;  //这里没有用到它

    //初始化目标向量为0
    memset(dst, 0, nw*nh*3);

    for (j=hkh; j<h-hkh; j++)  //遍历图片
    {
        for (i=hkw; i<w-hkw; i++)
        {
            val_r = val_g  = val_b = 0;

            //遍历 kernel  做 乘积和
            for (n=-hkh; n<=hkh; n++)
            {
                for (m=-hkw; m<=hkw; m++)
                {

                    r = src[(j-n)*w*3+(i-m)*3       ];
                    g = src[(j-n)*w*3+(i-m)*3  +1];
                    b = src[(j-n)*w*3+(i-m)*3  +2];

                    //三个通道对应的模板值是一样的
                    //注意   n+hkh   m+hkw   是为了完成坐标转换
                    val_r  +=  r    *  kernel[(n+hkh)*kw    +      m+hkw  ];
                    val_g +=  g   *  kernel[(n+hkh)*kw     +     m+hkw  ];
                    val_b +=  b   *  kernel[(n+hkh)*kw     +     m+hkw  ];
                }
            }

            //异值处理
            val_r = val_r>255 ? 255 : val_r;
            val_g = val_g>255 ? 255 : val_g;
            val_b = val_b>255 ? 255 : val_b;

            val_r = val_r<0 ? 0 : val_r;
            val_g = val_g<0 ? 0 : val_g;
            val_b = val_b<0 ? 0 : val_b;

            //将计算结果填入目标向量
            dst[(j-hkh)*nw*3+(i-hkw)*3      ] = (uchar)(val_r);
            dst[(j-hkh)*nw*3+(i-hkw)*3  +1] = (uchar)(val_g);
            dst[(j-hkh)*nw*3+(i-hkw)*3  +2] = (uchar)(val_b);
        }
    }
}


//构造函数
EmbossFilterDialog::EmbossFilterDialog(QImage inputImage)
{
    emboss1Image = inputImage;

    int w = emboss1Image.width();
    int h = emboss1Image.height();
    int pixel_num = w*h;

    rgb = new uchar[3*pixel_num];
    borderType = 0;
    halfKernelSize = 1;
    sigma = 2;
    maxSigma = 7;

    iniUI();  //初始化UI

    // 通道分离
    splitImageChannel(emboss1Image, rgb);

    // padding
    int hkw = 1;
    int hkh = 1;
    int nw = w+hkw*2;
    int nh = h+hkh*2;
    rgbPadded = new uchar[3*nw*nh];
    paddedImage = QImage(nw, nh, QImage::Format_RGB888);
    uchar constBorder[3] = {0};
    copyMakeBorder(rgb, w, h, 3,
                   halfKernelSize, halfKernelSize, halfKernelSize, halfKernelSize, (BorderType)borderType,
                   constBorder, rgbPadded);

    emboss1Image = imageFilter((int)EmbossFilterType::Emboss1);
    emboss2Image = imageFilter((int)EmbossFilterType::Emboss2);
    emboss3Image = imageFilter((int)EmbossFilterType::Emboss3);
    emboss4Image = imageFilter((int)EmbossFilterType::Emboss4);
    emboss5Image = imageFilter((int)EmbossFilterType::Emboss5);
    emboss6Image = imageFilter((int)EmbossFilterType::Emboss6);
    emboss7Image = imageFilter((int)EmbossFilterType::Emboss7);
    emboss8Image = imageFilter((int)EmbossFilterType::Emboss8);

    emboss1ImageLabel->setScaledContents(true);
    emboss1ImageLabel->setPixmap(QPixmap::fromImage(emboss1Image).scaled(emboss1ImageLabel->width(), emboss1ImageLabel->height())); 
    emboss2ImageLabel->setScaledContents(true);
    emboss2ImageLabel->setPixmap(QPixmap::fromImage(emboss2Image).scaled(emboss2ImageLabel->width(), emboss2ImageLabel->height()));
    emboss3ImageLabel->setScaledContents(true);
    emboss3ImageLabel->setPixmap(QPixmap::fromImage(emboss3Image).scaled(emboss3ImageLabel->width(), emboss3ImageLabel->height()));
    emboss4ImageLabel->setScaledContents(true);
    emboss4ImageLabel->setPixmap(QPixmap::fromImage(emboss4Image).scaled(emboss4ImageLabel->width(), emboss4ImageLabel->height()));
    emboss5ImageLabel->setScaledContents(true);
    emboss5ImageLabel->setPixmap(QPixmap::fromImage(emboss5Image).scaled(emboss5ImageLabel->width(), emboss5ImageLabel->height()));
    emboss6ImageLabel->setScaledContents(true);
    emboss6ImageLabel->setPixmap(QPixmap::fromImage(emboss6Image).scaled(emboss6ImageLabel->width(), emboss6ImageLabel->height()));
    emboss7ImageLabel->setScaledContents(true);
    emboss7ImageLabel->setPixmap(QPixmap::fromImage(emboss7Image).scaled(emboss7ImageLabel->width(), emboss7ImageLabel->height()));
    emboss8ImageLabel->setScaledContents(true);
    emboss8ImageLabel->setPixmap(QPixmap::fromImage(emboss8Image).scaled(emboss8ImageLabel->width(), emboss8ImageLabel->height()));

    delete [] rgbPadded;
    rgbPadded = nullptr;
}

//ui设计
void EmbossFilterDialog::iniUI()
{
    //label
    emboss1ImageLabel = new QLabel();
    emboss1ImageLabel->setAlignment(Qt::AlignCenter);
    emboss2ImageLabel = new QLabel();
    emboss2ImageLabel->setAlignment(Qt::AlignCenter);
    emboss3ImageLabel = new QLabel();
    emboss3ImageLabel->setAlignment(Qt::AlignCenter);
    emboss4ImageLabel = new QLabel();
    emboss4ImageLabel->setAlignment(Qt::AlignCenter);
    emboss5ImageLabel = new QLabel();
    emboss5ImageLabel->setAlignment(Qt::AlignCenter);
    emboss6ImageLabel = new QLabel();
    emboss6ImageLabel->setAlignment(Qt::AlignCenter);
    emboss7ImageLabel = new QLabel();
    emboss7ImageLabel->setAlignment(Qt::AlignCenter);
    emboss8ImageLabel = new QLabel();
    emboss8ImageLabel->setAlignment(Qt::AlignCenter);

    // three buttons
    btnOK = new QPushButton(tr("OK"));
    btnCancel = new QPushButton(tr("Cancel"));
    btnClose = new QPushButton(tr("Exit"));

    connect(btnOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(emboss1ImageLabel);
    layout1->addWidget(emboss2ImageLabel);
    layout1->addWidget(emboss3ImageLabel);
    layout1->addWidget(emboss4ImageLabel);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(emboss5ImageLabel);
    layout2->addWidget(emboss6ImageLabel);
    layout2->addWidget(emboss7ImageLabel);
    layout2->addWidget(emboss8ImageLabel);

    QHBoxLayout *layout8 = new QHBoxLayout;
    layout8->addStretch();
    layout8->addWidget(btnOK);
    layout8->addWidget(btnCancel);
    layout8->addStretch();
    layout8->addWidget(btnClose);

    // main layout
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->addLayout(layout1);
    mainlayout->addLayout(layout2);
    mainlayout->addLayout(layout8);
    mainlayout ->setSizeConstraint(QLayout::SetNoConstraint);
    setLayout(mainlayout);
}


//各种浮雕滤波
QImage EmbossFilterDialog::imageFilter(int inputFilterType)
{
    int w = emboss1Image.width();
    int h = emboss1Image.height();
    int hkw = 1;
    int hkh = 1;

    float *filterKernelX;

    filterKernelX = emboss[(int)inputFilterType];

    int nw = w+hkw*2;
    int nh = h+hkh*2;
    rgbFilteredX = new uchar[3*w*h];

    // filtering
    filterProc(rgbPadded, nw, nh, filterKernelX, hkw, hkh, rgbFilteredX);

    QImage dst;
    concatenateImageChannel(rgbFilteredX, w, h, dst);

    delete [] rgbFilteredX;
    rgbFilteredX = nullptr;

    return dst;
}


//将图像贴到标签处
void EmbossFilterDialog::setImage(QImage image, QLabel *label)
{
    QPixmap pix;
    pix.fromImage(image);
    label->setPixmap(pix);
}
