#include "acedialog.h"
//这是一个相对独立的类，用于图像增强里 adaptive Contrast  Enhancement  的实现

//构造函数   传入的是待处理的原始图片
ACEDialog::ACEDialog(QImage inputImage)
{
    srcImage = inputImage;

    //获取原图片的参数
    int image_width = srcImage.width();
    int image_height = srcImage.height();
    int ori_pixel_num = image_width*image_height;  //原始图片的总像素个数

    //
    int pixel_num = (image_width+2*maxFilterSize)*(image_height+2*maxFilterSize);

    // obtain image channels
    rgb                   =     new float[3*ori_pixel_num];

    rgb_pad            =     new float[3*pixel_num];

    //保存积分图的结果
    rgb_ii                =     new float[3*pixel_num];
    //保存积分图的平方
    rgb_ii_power     =     new float[3*pixel_num];
    //rgb_ii 和  rgb_ii_power 均用于快速算法 详见  imageprocess.cpp

    //将原始图片的三通道值传入 一维数组  rgb 中
    splitImageChannel(srcImage, rgb, rgb+ori_pixel_num, rgb+2*ori_pixel_num);

    //填零  其实现 在 imageprocess.cpp 中
    //传入参数见 imageprocess.cpp
    paddingZeros(rgb, rgb+ori_pixel_num, rgb+2*ori_pixel_num, image_width, image_height,
                 maxFilterSize, maxFilterSize, rgb_pad, rgb_pad+pixel_num, rgb_pad+2*pixel_num);


    for (int i=0; i<3; i++)  //分别对三通道进行滤波操作
    {
        //calculate_integral_image(rgb+i*pixel_num, image_width, image_height, rgb_ii+i*pixel_num);
        //calculate_integral_image_power(rgb+i*pixel_num, image_width, image_height, rgb_ii_power+i*pixel_num);

        calculate_integral_image(rgb_pad+i*pixel_num, image_width+2*maxFilterSize,
                                 image_height+2*maxFilterSize, rgb_ii+i*pixel_num);
        calculate_integral_image_power(rgb_pad+i*pixel_num, image_width+2*maxFilterSize,
                                       image_height+2*maxFilterSize, rgb_ii_power+i*pixel_num);
    }

    adaptiveContrastEnhancement(srcImage, rgb_pad, rgb_ii, rgb_ii_power, maxFilterSize,
                                filterSize,  gainCoef, maxCG, dstImage);

    iniUI();  //初始化ui布局

    //分别将图片贴到标签上

//    srcImageLabel->setPixmap(QPixmap::fromImage(srcImage));
//    dstImageLabel->setPixmap(QPixmap::fromImage(dstImage));

    setImage(srcImage,srcImageLabel);
    setImage(dstImage,dstImageLabel);


}

//把图片放到相应的label上
void ACEDialog::setImage(QImage &image, QLabel *label)
{
    QPixmap pix = QPixmap::fromImage(image);

    //pix.scaled(1000,1000, Qt::KeepAspectRatio, Qt::SmoothTransformation);
   // label->setScaledContents(true);

    label->setPixmap(pix);
    label->show();
}


//根据响应的值更新目标图像  实际上作为三个滑动条的槽函数  当滑动条变动时会触发此函数进行更新
void ACEDialog::updateDstImage(float value)
{
    if (QObject::sender() == filterSizeSlider)
    {
        filterSize = (int)value;
        //更新编辑框的值
        filterSizeEdit->setText(QString("%1").arg((int)value));
    }
    else if (QObject::sender() == gainCoefSlider)
    {
        gainCoef = value;
        gainCoefEdit->setText(QString("%1").arg(value));
    }
    else if (QObject::sender() == maxCGSlider)
    {
        maxCG = value;
        maxCGEdit->setText(QString("%1").arg(value));
    }

    adaptiveContrastEnhancement(srcImage, rgb_pad, rgb_ii, rgb_ii_power, maxFilterSize,
                                filterSize,  gainCoef, maxCG, dstImage);

    //处理完后更新目标图像的label
   // dstImageLabel->setPixmap(QPixmap::fromImage(dstImage));
    setImage(dstImage,dstImageLabel);
}

//ui设计
void ACEDialog::iniUI()
{
    // two image labels
    srcImageLabel = new QLabel();
    srcImageLabel->setAlignment(Qt::AlignCenter);
    dstImageLabel = new QLabel();
    dstImageLabel->setAlignment(Qt::AlignCenter);

    // filter size
    filterSizeLabel = new QLabel(tr("Filter Size"));
    filterSizeSlider = new FloatSlider(Qt::Horizontal);
    filterSizeEdit = new QLineEdit("0");

    filterSizeLabel->setMinimumWidth(30);
    filterSizeLabel->setAlignment(Qt::AlignRight);
    filterSizeEdit->setMinimumWidth(16);
    filterSizeSlider->setFloatRange(1, maxFilterSize);
    filterSizeSlider->setFloatStep(2);
    filterSizeSlider->setFloatValue(filterSize);
    filterSizeEdit->setText(QString("%1").arg(filterSize));

    // constrast gain upper limit
    maxCGLabel = new QLabel(tr("MAX CG"), this);
    maxCGLabel->setAlignment(Qt::AlignRight);
    maxCGSlider = new FloatSlider(Qt::Horizontal, this);
    maxCGEdit = new QLineEdit("0", this);
    maxCGLabel->setMinimumWidth(30);
    maxCGEdit->setMinimumWidth(16);
    maxCGSlider->setFloatRange(1, 10);
    maxCGSlider->setFloatStep(0.01f);
    maxCGSlider->setFloatValue(maxCG);
    maxCGEdit->setText(QString::number(maxCG, 'f', 2));

    // gain coef
    gainCoefLabel = new QLabel(tr("Gain Coef"), this);
    gainCoefLabel->setAlignment(Qt::AlignRight);
    gainCoefSlider = new FloatSlider(Qt::Horizontal, this);
    gainCoefEdit = new QLineEdit("0", this);
    gainCoefLabel->setMinimumWidth(30);
    gainCoefEdit->setMinimumWidth(16);
    gainCoefSlider->setFloatRange(1, 500);
    gainCoefSlider->setFloatStep(0.01f);
    gainCoefSlider->setFloatValue(gainCoef);
    gainCoefEdit->setText(QString::number(gainCoef, 'f', 2));

    // signal slot  当滑动条变动时触发相应的操作更新目标图像
    connect(filterSizeSlider, SIGNAL(floatValueChanged(float)), this, SLOT(updateDstImage(float)));
    connect(maxCGSlider, SIGNAL(floatValueChanged(float)), this, SLOT(updateDstImage(float)));
    connect(gainCoefSlider, SIGNAL(floatValueChanged(float)), this, SLOT(updateDstImage(float)));

    filterSizeSlider->setFloatValue(filterSize);
    gainCoefSlider->setFloatValue(gainCoef);
    maxCGSlider->setFloatValue(maxCG);

    // three buttons
    btnOK = new QPushButton(tr("OK"));
    btnCancel = new QPushButton(tr("Cancel"));
    btnClose = new QPushButton(tr("Exit"));

    connect(btnOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));

    //对控件进行布局
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addWidget(srcImageLabel);
    layout1->addWidget(dstImageLabel);


    QHBoxLayout *layout2 = new QHBoxLayout;
    layout2->addWidget(filterSizeLabel, 3);
    layout2->addWidget(filterSizeSlider, 5);
    layout2->addWidget(filterSizeEdit, 2);
    layout2->addStretch();  //添加弹簧

    layout2->addWidget(maxCGLabel, 3);
    layout2->addWidget(maxCGSlider, 5);
    layout2->addWidget(maxCGEdit, 2);
    layout2->addStretch();

    layout2->addWidget(gainCoefLabel,3);
    layout2->addWidget(gainCoefSlider,5);
    layout2->addWidget(gainCoefEdit,2);

    QHBoxLayout *layout3=new QHBoxLayout;
    layout3->addStretch();
    layout3->addWidget(btnOK);
    layout3->addWidget(btnCancel);
    layout3->addStretch();
    layout3->addWidget(btnClose);


    // main layout
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->addLayout(layout1, 8);
    mainlayout->addLayout(layout2, 1);
    mainlayout->addLayout(layout3, 1);

    setLayout(mainlayout);
}

