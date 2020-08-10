#include "sdfilterdialog.h"
//空域滤波  实际上是在做高通滤波 进行边界提取

/***********************************************************************************************/
//各种 kernel
float robertsX[9] = {
     0, 0, 0,
     0,-1, 0,
     0, 0, 1};
float robertsY[9] = {
     0, 0, 0,
     0, 0,-1,
     0, 1, 0};

float sobelX[9] = {
    -1,-2,-1,
     0, 0, 0,
     1, 2, 1};
float sobelY[9] = {
    -1,0,1,
    -2,0,2,
    -1,0,1};

float prewittX[9] = {
    -1,-1,-1,
     0, 0, 0,
     1, 1, 1};
float prewittY[9] = {
    -1,0,1,
    -1,0,1,
    -1,0,1};

//注意下面这两个模板 不分 X Y 方向
float laplacian4[9] = {
    0, 1,0,
    1,-4,1,
    0, 1,0};
float laplacian8[9] = {
    1, 1,1,
    1,-8,1,
    1, 1,1};


//为下面的  generateLOGKernel 函数服务
float logElement(int x, int y, float sigma)
{
    //const float PI = 3.141592653589793f;
    float g = 0;
    for(float ySubPixel = y - 0.5f; ySubPixel <= y + 0.5f; ySubPixel += 0.1f) {
        for(float xSubPixel = x - 0.5f; xSubPixel <= x + 0.5f; xSubPixel += 0.1f) {
            //float s = ((xSubPixel*xSubPixel)+(ySubPixel*ySubPixel)) / (2*sigma*sigma);
            //g += (1/(PI*pow(sigma, 4))) * (s-1) * exp(-s) * 2*PI*sigma*sigma;
            float s = ((xSubPixel*xSubPixel)+(ySubPixel*ySubPixel)) / (2*sigma*sigma);
            g += 2 * (s-1/(sigma*sigma)) * expf(-s);
        }
    }
    g /= 121;

    return g;
}

//生成对数kernel
float *generateLOGKernel(float sigma, int &kernelSize)
{
    kernelSize = (int)(4*sigma+1 + 0.5f) /2 * 2 + 1;
    float *LOGKernel = new float[kernelSize*kernelSize];
    double sum = 0;
    for(int j=0; j<kernelSize; ++j){
        for(int i=0; i<kernelSize; ++i){
            int x = (-kernelSize/2)+i;
            int y = (-kernelSize/2)+j;
            LOGKernel[j*kernelSize+i] = logElement(x, y, sigma);
            sum += LOGKernel[j*kernelSize+i];
        }
    }
    // subtract mean to get zero sum
    double mean = sum / (kernelSize * kernelSize);
    for(int i=0; i<kernelSize*kernelSize; ++i){
        LOGKernel[i] -= mean;
    }

    return LOGKernel;
}

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


//是上面  filterProc  函数 的 高级运用
QImage SDFilterDialog::imageFilter(int inputFilterType)
{
    //因为srcImage 作为 SDFilterDialog 类的成员变量  因此在成员函数中无需转入就可以直接使用

    //获得原图宽高
    int w = srcImage.width();
    int h = srcImage.height();
    int hkw = 1;
    int hkh = 1;

    float *filterKernelX;
    float *filterKernelY;

    switch (inputFilterType) {
    case FilterType::Roberts:
        filterKernelX = robertsX;
        filterKernelY = robertsY;
        break;
    case FilterType::Sobel:
        filterKernelX = sobelX;
        filterKernelY = sobelY;
        break;
    case FilterType::Prewitt:
        filterKernelX = prewittX;
        filterKernelY = prewittY;
        break;

    //注意 Laplacian4   Laplacian8  不分 x y 方向
    //因此没有对 filterKernelY 赋值
    // 如果不懂可以看下面的代码去帮助理解
    case FilterType::Laplacian4:
        filterKernelX = laplacian4;
        break;
    case FilterType::Laplacian8:
        filterKernelX = laplacian8;
        break;
    }


    //计算padding后的图像宽高
    //注意 rgbPadded 为对原图填零后的向量，它也是  SDFilterDialog 类的成员变量 因此在本成员函数中可以直接使用
    int nw = w+hkw*2;
    int nh = h+hkh*2;


    //用于接收滤波后的结果
    rgbFilteredX = new uchar[3*w*h];
    rgbFilteredY = new uchar[3*w*h];

    // 空域滤波  输出的结果会保存在 向量   rgbFilteredX  中
    filterProc(rgbPadded, nw, nh, filterKernelX, hkw, hkh, rgbFilteredX);  //X 方向

    //Laplacian4 Laplacian8 不分 纵横方向 不用做处理 下面的if 语句针对 不是Laplacian4 Laplacian8 的情况
    if ((inputFilterType != (int)FilterType::Laplacian4) && (inputFilterType != (int)FilterType::Laplacian8) ) {

        // 只有是非  Laplacian4 Laplacian8 才执行下面语句

        filterProc(rgbPadded, nw, nh, filterKernelY, hkw, hkh, rgbFilteredY);

        for (int i=0; i<w*h*3; i++) {  //遍历三通道

            //将 X Y 方向数据做融合 并存入到  rgbFilteredX  中
           rgbFilteredX[i] = (rgbFilteredX[i]+rgbFilteredY[i])/2;
        }
    }

    //生成目标图像
    QImage dst;
    concatenateImageChannel(rgbFilteredX, w, h, dst);

     //手动释放内存
    delete [] rgbFilteredX;
    rgbFilteredX = nullptr;
    delete [] rgbFilteredY;
    rgbFilteredY = nullptr;

    return dst;
}

//与上面的 imageFilter 类似  不同点在于kernel 的生成上
QImage SDFilterDialog::imageLOGFilter(float sigma)
{
    int w = srcImage.width();
    int h = srcImage.height();

    float *filterKernel;
    int kernelSize = 0;
    filterKernel = generateLOGKernel(sigma, kernelSize);

    halfKernelSize = kernelSize/2;
    int nw = w+halfKernelSize*2;
    int nh = h+halfKernelSize*2;

    rgbFilteredX = new uchar[3*w*h];  //用于存放空域滤波后的结果

    rgbPadded = new uchar[3*nw*nh];
    paddedImage = QImage(nw, nh, QImage::Format_RGB888);

    // padding
    uchar constBorder[3] = {0};
    copyMakeBorder(rgb, w, h, 3,
                   halfKernelSize, halfKernelSize, halfKernelSize, halfKernelSize, (BorderType)borderType,
                   constBorder, rgbPadded);
    // filtering
    filterProc(rgbPadded, nw, nh, filterKernel,
               halfKernelSize, halfKernelSize, rgbFilteredX);

    QImage dst;
    concatenateImageChannel(rgbFilteredX, w, h, dst);

    delete [] rgbFilteredX;
    rgbFilteredX = nullptr;

    delete [] filterKernel;
    filterKernel = nullptr;

    return dst;
}

//SDFilterDialog类  构造函数
SDFilterDialog::SDFilterDialog(QImage inputImage)
{
    srcImage = inputImage;

    //获取图像数据
    int w = srcImage.width();
    int h = srcImage.height();
    int pixel_num = w*h;

    //用于保存原图的rgb 三通道值 它是一个一维向量
    rgb = new uchar[3*pixel_num];

    //定义初始padding 的类型
    borderType = BORDER_REPLICATE;

    halfKernelSize = 1;
    sigma = 2;
    maxSigma = 7;

    iniUI();

    // obtain image channels
    splitImageChannel(srcImage, rgb);

    int count = borderTypeComboBox->count();
    borderTypeComboBox->setCurrentIndex(count-1);


    // padding   预处理
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


    //concatenateImageChannel(rgbPadded, nw, nh, paddedImage);

    //paddedImageLabel->setPixmap(QPixmap::fromImage(paddedImage));

    //6种空域模板处理
    robertsImage        = imageFilter((int)FilterType::Roberts);
    sobelImage          = imageFilter((int)FilterType::Sobel);
    prewittImage        = imageFilter((int)FilterType::Prewitt);
    laplacian4Image   = imageFilter((int)FilterType::Laplacian4);
    laplacian8Image   = imageFilter((int)FilterType::Laplacian8);
    LOGImage            = imageLOGFilter(sigma);

    /*
    srcImageLabel->setPixmap(QPixmap::fromImage(srcImage));
    robertsImageLabel->setPixmap(QPixmap::fromImage(robertsImage));
    sobelImageLabel->setPixmap(QPixmap::fromImage(sobelImage));
    prewittImageLabel->setPixmap(QPixmap::fromImage(prewittImage));
    laplacian4ImageLabel->setPixmap(QPixmap::fromImage(laplacian4Image));
    laplacian8ImageLabel->setPixmap(QPixmap::fromImage(laplacian8Image));
    LOGImageLabel->setPixmap(QPixmap::fromImage(LOGImage));
    */

    //将图片放到相应的标签上

    srcImageLabel ->setScaledContents(true);
    srcImageLabel->setPixmap(QPixmap::fromImage(srcImage).scaled(srcImageLabel->width(), srcImageLabel->height(),Qt::KeepAspectRatio));
    robertsImageLabel->setScaledContents(true);
    robertsImageLabel->setPixmap(QPixmap::fromImage(robertsImage).scaled(robertsImageLabel->width(), robertsImageLabel->height()));
    sobelImageLabel->setScaledContents(true);
    sobelImageLabel->setPixmap(QPixmap::fromImage(sobelImage).scaled(sobelImageLabel->width(), sobelImageLabel->height()));
    prewittImageLabel->setScaledContents(true);
    prewittImageLabel->setPixmap(QPixmap::fromImage(prewittImage).scaled(prewittImageLabel->width(), prewittImageLabel->height()));
    laplacian4ImageLabel->setScaledContents(true);
    laplacian4ImageLabel->setPixmap(QPixmap::fromImage(laplacian4Image).scaled(laplacian4ImageLabel->width(), laplacian4ImageLabel->height()));
    laplacian8ImageLabel->setScaledContents(true);
    laplacian8ImageLabel->setPixmap(QPixmap::fromImage(laplacian8Image).scaled(laplacian8ImageLabel->width(), laplacian8ImageLabel->height()));
    LOGImageLabel->setScaledContents(true);
    LOGImageLabel->setPixmap(QPixmap::fromImage(LOGImage).scaled(LOGImageLabel->width(), LOGImageLabel->height()));

    delete [] rgbPadded;
    rgbPadded = nullptr;

}



//设计空域滤波的ui
void SDFilterDialog::iniUI()
{
    // four image labels
    srcImageLabel = new QLabel();
    srcImageLabel->setAlignment(Qt::AlignCenter);

    robertsImageLabel = new QLabel();
    robertsImageLabel->setAlignment(Qt::AlignCenter);

    sobelImageLabel = new QLabel();
    sobelImageLabel->setAlignment(Qt::AlignCenter);

    prewittImageLabel = new QLabel();
    prewittImageLabel->setAlignment(Qt::AlignCenter);

    laplacian4ImageLabel = new QLabel();
    laplacian4ImageLabel->setAlignment(Qt::AlignCenter);

    laplacian8ImageLabel = new QLabel();
    laplacian8ImageLabel->setAlignment(Qt::AlignCenter);

    LOGImageLabel = new QLabel();
    LOGImageLabel->setAlignment(Qt::AlignCenter);
    LOGImageLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    srcLabel = new QLabel(tr("Original"));
    srcLabel->setAlignment(Qt::AlignCenter);

    robertsLabel = new QLabel(tr("Roberts"));
    robertsLabel->setAlignment(Qt::AlignCenter);

    sobelLabel = new QLabel(tr("Sobel"));
    sobelLabel->setAlignment(Qt::AlignCenter);

    prewittLabel = new QLabel(tr("Prewitt"));
    prewittLabel->setAlignment(Qt::AlignCenter);

    laplacian4Label = new QLabel(tr("Laplacian4"));
    laplacian4Label->setAlignment(Qt::AlignCenter);

    laplacian8Label = new QLabel(tr("Laplacian8"));
    laplacian8Label->setAlignment(Qt::AlignCenter);

    LOGLabel = new QLabel(tr("LOG"));
    LOGLabel->setAlignment(Qt::AlignCenter);

    // border type
    borderTypeLabel = new QLabel(tr("Border"));
    borderTypeLabel->setAlignment(Qt::AlignRight);
    borderTypeComboBox = new QComboBox;
    borderTypeComboBox->addItem(tr("replicate"));
    borderTypeComboBox->addItem(tr("reflect"));
    borderTypeComboBox->addItem(tr("reflect101"));
    borderTypeComboBox->addItem(tr("wrap"));
    borderTypeComboBox->addItem(tr("zero padding"));

    // border size
    sigmaLabel = new QLabel(tr("Size"));
    sigmaSlider = new FloatSlider(Qt::Horizontal);
    sigmaEdit = new QLineEdit();

    //sigmaLabel->setMinimumWidth(30);
    sigmaLabel->setAlignment(Qt::AlignRight);
    //sigmaEdit->setMinimumWidth(16);
    sigmaSlider->setFloatRange(2, maxSigma);
    sigmaSlider->setFloatValue(sigma);
    sigmaSlider->setFloatStep(0.2f);
    sigmaEdit->setText(QString("%1").arg(sigma));

    // signal slot   当滑动 slider 时 激活 updateLOGImage  函数 进行更新
    connect(sigmaSlider, SIGNAL(floatValueChanged(float)), this, SLOT(updateLOGImage(float)));

    // three buttons
    btnOK = new QPushButton(tr("OK"));
    btnCancel = new QPushButton(tr("Cancel"));
    btnClose = new QPushButton(tr("Exit"));

    connect(btnOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(srcImageLabel);
    layout1->addWidget(robertsImageLabel);
    layout1->addWidget(sobelImageLabel);
    layout1->setSizeConstraint(QLayout::SetDefaultConstraint);

    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(srcLabel);
    layout2->addWidget(robertsLabel);
    layout2->addWidget(sobelLabel);

    QHBoxLayout *layout3 = new QHBoxLayout;
    layout3->addWidget(prewittImageLabel);
    layout3->addWidget(laplacian4ImageLabel);
    layout3->addWidget(laplacian8ImageLabel);

    QHBoxLayout *layout4 = new QHBoxLayout;
    layout4->addWidget(prewittLabel);
    layout4->addWidget(laplacian4Label);
    layout4->addWidget(laplacian8Label);

    QHBoxLayout *layout5 = new QHBoxLayout;
    layout5->addWidget(LOGImageLabel);

    QHBoxLayout *layout6 = new QHBoxLayout;
    layout6->addWidget(LOGLabel);

    QHBoxLayout *layout7 = new QHBoxLayout;

    layout7->addWidget(sigmaLabel, 2);
    layout7->addWidget(sigmaSlider, 6);
    layout7->addWidget(sigmaEdit, 2);

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
    mainlayout->addLayout(layout3);
    mainlayout->addLayout(layout4);
    mainlayout->addLayout(layout5);
    mainlayout->addLayout(layout6);
    mainlayout->addLayout(layout7);
    mainlayout->addLayout(layout8);   
   mainlayout ->setSizeConstraint(QLayout::SetNoConstraint);
   setLayout(mainlayout);
}


//将图像贴到响应的标签上
void SDFilterDialog::setImage(QImage image, QLabel *label)
{
    QPixmap pix;
    pix.fromImage(image);
    label->setPixmap(pix);
}


//更新log 模板 的 处理结果
void SDFilterDialog::updateLOGImage(float value)
{
    if (QObject::sender() == sigmaSlider)
    {
        sigma = value;
        sigmaEdit->setText(QString("%1").arg(value));
        LOGImage = imageLOGFilter(sigma);
        LOGImageLabel->setPixmap(QPixmap::fromImage(LOGImage).scaled(LOGImageLabel->width(), LOGImageLabel->height()));
    }
}
