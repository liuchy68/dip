#include "imageprocess.h"

//核心部分，许多图像处理函数的都是在这里定义

/************************************************************************************************/
//填零函数

//在acedialog.cpp 中  会被调用
//部分参数说明
//原图三通道
//const float *r, const float *g, const float *b

//单侧填零宽高
//宽  *   高
// const int half_pad_width, const int half_pad_height,


//目标图像三通道
//float *nr, float *ng, float *nb)

void paddingZeros(const float *r, const float *g, const float *b,
                   const int width, const int height,
                   const int half_pad_width, const int half_pad_height,
                   float *nr, float *ng, float *nb)
{

   //目标图像宽高 为原图宽高 + 2倍单侧填零宽度
    int nw = width + 2*half_pad_width;
    int nh = height + 2*half_pad_height;

    //初始化填零
    memset(nr, 0, nh*nw*sizeof(float));
    memset(ng, 0, nh*nw*sizeof(float));
    memset(nb, 0, nh*nw*sizeof(float));

    //用j遍历需要填入原图数据的行
    for (int j=half_pad_height; j<nh-half_pad_height; j++) {

        //内存数据复制
        //memcpy用法：
        //memcpy（目标指针，源指针，要复制的数据长度）

        memcpy(nr+j*nw+half_pad_width,    r+(j-half_pad_height)*width,     width*sizeof(float));
        memcpy(ng+j*nw+half_pad_width,   g+(j-half_pad_height)*width,     width*sizeof(float));
        memcpy(nb+j*nw+half_pad_width,   b+(j-half_pad_height)*width,     width*sizeof(float));
    }
}

/************************************************************************************************/
//分离图像到 相应数组中，注意这里有多个重载版本

//将图像三通道数据分别放入 r g b  三个数组中
void splitImageChannel(QImage &image, float *r, float *g, float *b)
{
    int width = image.width();
    int height = image.height();
    int count = 0;
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {

            //获取当前像素的rgb值
            QRgb pixel = image.pixel(i, j);
             //分别将rgb值写入相应的位置

            r[count] = qRed(pixel);
            g[count] = qGreen(pixel);
            b[count] = qBlue(pixel);

            count++;
        }
    }
}

//将图像三通道数据分别放入数组rgb中         注意是每三个值为一组rgb值
void splitImageChannel(QImage &image, float *rgb)
{
    int width = image.width();
    int height = image.height();
    int count = 0;     //这里没有用到count  可以把它删掉！
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            //获取当前像素的rgb值
            QRgb pixel = image.pixel(i, j);

            //分别将rgb值写入相应的位置    最后形成的数组 每三个值为一组rgb值
            rgb[j*3*width+3*i             ] = (float)qRed(pixel);
            rgb[j*3*width+3*i   +1     ] = (float)qGreen(pixel);
            rgb[j*3*width+3*i   +2     ] = (float)qBlue(pixel);

            count++;
        }
    }
}


//将图像三通道数据分别放入 r g b  三个数组中
void splitImageChannel(QImage &image, uchar *r, uchar *g, uchar *b)
{
    int width = image.width();
    int height = image.height();
    int count = 0;
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            QRgb pixel = image.pixel(i, j);
            r[count] = qRed(pixel);
            g[count] = qGreen(pixel);
            b[count] = qBlue(pixel);
            count++;
        }
    }
}

//将图像三通道数据分别放入数组rgb中
void splitImageChannel(QImage &image, uchar *rgb)
{
    int width = image.width();
    int height = image.height();
    int count = 0;
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            QRgb pixel = image.pixel(i, j);
            rgb[j*3*width+3*i] = (uchar)qRed(pixel);
            rgb[j*3*width+3*i+1] = (uchar)qGreen(pixel);
            rgb[j*3*width+3*i+2] = (uchar)qBlue(pixel);
            count++;
        }
    }
}

/************************************************************************************************/
//由通道值合成rgb图片
void concatenateImageChannel(float *r, float *g, float *b, int w, int h, QImage &image)
{
    image = QImage(w, h, QImage::Format_RGB888);
    for (int j=0; j<h; j++) {
        for (int i=0; i<w; i++) {
            QRgb pixel = qRgb((int)r, (int)g, (int)b);
            image.setPixel(i, j, pixel);
        }
    }
}

void concatenateImageChannel(float *rgb, int w, int h, QImage &image)
{
    image = QImage(w, h, QImage::Format_RGB888);
    for (int j=0; j<h; j++) {
        for (int i=0; i<w; i++) {
            int r = (int)rgb[j*3*w+3*i];
            int g = (int)rgb[j*3*w+3*i+1];
            int b = (int)rgb[j*3*w+3*i+2];
            QRgb pixel = qRgb(r, g, b);
            image.setPixel(i, j, pixel);
        }
    }
}

void concatenateImageChannel(uchar *r, uchar *g, uchar *b, int w, int h, QImage &image)
{
    image = QImage(w, h, QImage::Format_RGB888);
    for (int j=0; j<h; j++) {
        for (int i=0; i<w; i++) {
            QRgb pixel = qRgb((int)r, (int)g, (int)b);
            image.setPixel(i, j, pixel);
        }
    }
}

void concatenateImageChannel(uchar *rgb, int w, int h, QImage &image)
{
    image = QImage(w, h, QImage::Format_RGB888);
    for (int j=0; j<h; j++) {
        for (int i=0; i<w; i++) {
            int r = (int)rgb[j*3*w+3*i];
            int g = (int)rgb[j*3*w+3*i+1];
            int b = (int)rgb[j*3*w+3*i+2];
            QRgb pixel = qRgb(r, g, b);
            image.setPixel(i, j, pixel);
        }
    }
}

/************************************************************************************************/
//各种格式转换

void rgb2ycrcb(uchar *r, uchar *g, uchar *b, int size, float *y, float *cr, float *cb)
{
    for (int i=0; i<size; i++)
    {
        y[i] = 0.256789 * r[i] + 0.504129 * g[i] + 0.097906 * b[i] + 16;
        cb[i] = -0.148223 * r[i] - 0.290992 * g[i] + 0.439215 * b[i] + 128;
        cr[i] = 0.439215 * r[i] - 0.367789 * g[i] - 0.071426 * b[i] + 128;
    }
}

void ycrcb2rgb(float *y, float *cr, float *cb, int size, uchar *r, uchar *g, uchar *b)
{
    for (int i=0; i<size; i++)
    {
        r[i] = 1.164383 * (y[i]-16) + 1.596027 * (cr[i]-128);
        g[i] = 1.164383 * (y[i]-16) - 0.391762 * (cb[i]-128)- 0.812969 * (cr[i]-128);
        b[i] = 1.164383 * (y[i]-16) + 2.017230 * (cb[i]-128);
    }
}

void qimage2ycrcb(QImage image, float *y, float *cr, float *cb)
{
    int pixel_num = image.width()*image.height();
    // obtain image channels
    uchar *channels = new uchar[pixel_num*3];
    uchar *r = channels;
    uchar *g = channels+pixel_num;
    uchar *b = channels+2*pixel_num;
    splitImageChannel(image, r, g, b);

    // rgb to ycrcb
    rgb2ycrcb(r, g, b, pixel_num, y, cr, cb);

    delete [] channels;
}

void ycrcb2qimage(float *y, float *cr, float *cb, int width, int height, QImage &image)
{
    int pixel_num = width*height;

    uchar *channels = new uchar[pixel_num*3];
    uchar *r = channels;
    uchar *g = channels+pixel_num;
    uchar *b = channels+2*pixel_num;

    // ycrcb to rgb
    ycrcb2rgb(y, cr, cb, pixel_num, r, g, b);

    // update image
    int count = 0;
    image = QImage(width, height, QImage::Format_RGB888);
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            int nr = r[count];
            int ng = g[count];
            int nb = b[count];
            count++;
            image.setPixel(i, j, qRgb(nr,ng,nb));
        }
    }
    delete [] channels;
}

/************************************************************************************************/
//计算及绘制直方图
QImage calculateHistogram(QImage &image, ImageChannel channel)
{
    // obtain gray image
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);
    //获取宽高数据
    int width = grayImage.width();
    int height = grayImage.height();

    //创建三个一维数组接收图像的三通道值
    uchar *r = new uchar[width*height];
    uchar *g = new uchar[width*height];
    uchar *b = new uchar[width*height];

    // 分离图像三通道值到向量 r g  b  中
    splitImageChannel(image, r, g, b);

    QRgb hist_ior;  //用于指定待显示的直方图的颜色

    //bits 指针经过下面的switch后会指向待处理的那个通道的图像数据
    uchar *bits = nullptr;


    switch (channel) {
        case ImageChannel::Y:
              // R  红  G 绿  B 蓝  A 透明度（数值越大越不透明）
            hist_ior = qRgba(128, 128, 128, 255);     //灰色
            bits = grayImage.bits();
            break;
        case ImageChannel::R:
            hist_ior = qRgba(255, 0, 0, 255);   //红色
            bits = r;
            break;
        case ImageChannel::G:
            hist_ior = qRgba(0, 255, 0, 255);  //绿色
            bits = g;
            break;
        case ImageChannel::B:
            hist_ior = qRgba(0, 0, 255, 255);  //蓝色
            bits = b;
            break;
    }

    const int gray_level = 256;   //8bit

    //初始化灰度值统计数组为0用于计数
    int hist[gray_level] = {0};

    // 遍历所有像素点并做计数
    for (int i=0; i<width*height; i++)
    {
       int val = bits[i];  //获取当前点的灰度值
       hist[val]++;         //对应灰度等级的灰度自加一
    }


    // 遍历 hist  数组  寻找最大值，用于作直方图归一化
    int max_hist_val = hist[0];  //先假设一个最大值
    for (int i=1; i<gray_level; i++)
    {
        max_hist_val =                hist[i] > max_hist_val   ?       hist[i] : max_hist_val;    //相当于if else做判断处理
    }


    int s_w = 2;  //控制条形图的粗细
    float s_h = 0.8;//max_hist_val 占 直方图图片高度h   的比例


    //定义整个直方图 图片宽高
    int w = s_w*gray_level;
    int h = w;


    for (int i=0; i<gray_level; i++)
    {
        int v = hist[i];
        float fre = v*1.0/max_hist_val;  //归一化后的频率值

        hist[i] = int( h *    fre   * s_h);  //直方图归一化
    }

    //绘制直方图
    QImage hist_image(w, h, QImage::Format_RGBA8888);
    QRgb value;

    for (int j=0; j<h; j++)   // j 纵向
    {
        for (int i=0; i<w; i++)   //i横向
        {
            if ( (hist[i/s_w] > 0) && (j >= h-hist[i/s_w]) )
                value = hist_ior;
            else
                value = qRgba(255, 255, 255, 255);  //背景颜色


            hist_image.setPixel(i, j, value);
        }
    }


    //手动释放内存
    delete [] r;
    delete [] g;
    delete [] b;

    return hist_image;
}

/************************************************************************************************/
//负片
QImage calculateNegative(QImage &image, ImageChannel channel)
{
    // obtain gray image
    int width = image.width();
    int height = image.height();

    QImage newImage = image;  //目标图片


    //遍历所有点进行处理
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {

           // 获取rgb像素点
            QRgb pixel = image.pixel(i, j);

            //将当前位置的rgb三通道值分别存入整型变量   r  g   b   中
             int r, g, b;
            r = qRed(pixel);
            g = qGreen(pixel);
            b = qBlue(pixel);

            QRgb newPixel;

            switch (channel) {
                case ImageChannel::Y:   //所有通道一起取负片
                    newPixel = qRgb(255-r, 255-g, 255-b);
                    break;
                case ImageChannel::R:   //红 通道取负片，其他通道不变
                    newPixel = qRgb(255-r, g, b);
                    break;
                case ImageChannel::G:   //绿 通道取负片，其他通道不变
                    newPixel = qRgb(r, 255-g, b);
                    break;
                case ImageChannel::B:   //蓝 通道取负片，其他通道不变
                    newPixel = qRgb(r, g, 255-b);
                    break;
            }
            newImage.setPixel(i, j, newPixel);
          }
    }

    return newImage;
}

QImage intensityControl(QImage &image, int delta)
{
    // obtain gray image
    int width = image.width();
    int height = image.height();

    QImage newImage = image;  //目标图片


    //遍历所有点进行处理
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {

           // 获取rgb像素点
            QRgb pixel = image.pixel(i, j);

            //将当前位置的rgb三通道值分别存入整型变量   r  g   b   中
             int r, g, b;
            r = qRed(pixel);
            g = qGreen(pixel);
            b = qBlue(pixel);

            //边界判断及处理
            if( 0<=r+delta  &&  r+delta<=255   &&
                 0<=g+delta  &&  g+delta<=255  &&
                 0<=b+delta  &&  b+delta<=255          ){

                 r  +=delta;
                 g  +=delta;
                 b  +=delta;

            }




            QRgb newPixel = qRgb(r,g,b);
            newImage.setPixel(i, j, newPixel);
          }
    }

    return newImage;
}


QImage hueControl(QImage &image, int delta,colorToneType toneType )
{
    // obtain gray image
    int width = image.width();
    int height = image.height();

    QImage newImage = image;  //目标图片


    //遍历所有点进行处理
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {

           // 获取rgb像素点
            QRgb pixel = image.pixel(i, j);

            //将当前位置的rgb三通道值分别存入整型变量   r  g   b   中
             int r, g, b;
            r = qRed(pixel);
            g = qGreen(pixel);
            b = qBlue(pixel);



            switch (toneType) {
                case colorToneType::warm:   //暖色处理
                    r  += delta;
                    g += delta;
                    break;
                case colorToneType::cold:   //冷色处理
                    b +=delta;
                    break;

            }

            //异值处理
            r  = qBound(0,r,255);
            g = qBound(0,g,255);
            b = qBound(0,b,255);


            QRgb newPixel = qRgb(r,g,b);
            newImage.setPixel(i, j, newPixel);
          }
    }

    return newImage;
}





/************************************************************************************************/
//单通道
QImage calculateSingleChannel(QImage &image, ImageChannel channel)
{
    // obtain gray image
    int width = image.width();
    int height = image.height();

    QImage newImage = image;  //目标图片


    //遍历所有点进行处理
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {

           // 获取rgb像素点
            QRgb pixel = image.pixel(i, j);

            //将当前位置的rgb三通道值分别存入整型变量   r  g   b   中
             int r, g, b;
            r = qRed(pixel);
            g = qGreen(pixel);
            b = qBlue(pixel);

            QRgb newPixel;

            switch (channel) {
                case ImageChannel::R:   // R 保留 其他置零
                    newPixel = qRgb(r, 0, 0);
                    break;
                case ImageChannel::G:   // G 保留 其他置零
                    newPixel = qRgb(0, g, 0);
                    break;
                case ImageChannel::B:   // G 保留 其他置零
                   newPixel = qRgb(0, 0, b);
                    break;
            }
            newImage.setPixel(i, j, newPixel);
          }
    }

    return newImage;
}




/************************************************************************************************/
//伪彩色
QImage convertToPseudoColor(QImage &image, ColorMap map)
{
    //获取宽高数据
    int width = image.width();
    int height = image.height();

    // 先将原图转化为灰度图片，用于伪彩色映射
    //注意这个灰度图像依然存在三通道，在下面的处理中只需要提取任意一个通道的值即可
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);

    // 目标伪彩色图片
    QImage newImage(width, height, QImage::Format_RGB888);

    //遍历所有像素点
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {

            int index, r, g, b;

            //间接读取灰度数据到 index 中
            QRgb pixel = grayImage.pixel(i, j);
            index = qRed(pixel);

            //建立灰度到彩色的颜色映射表  映射表详见  imageprocess.h  要注意的地方是映射表一一维向量的方式保存，每隔三个为一组
            switch (map) {
                case ColorMap::Jet:
                    r = jet_table[index*3];
                    g = jet_table[index*3+1];               
                    b = jet_table[index*3+2];
                    break;
                case ColorMap::Parula:
                    r = parula_table[index*3];
                    g = parula_table[index*3+1];
                    b = parula_table[index*3+2];
                    break;
                case ColorMap::Hot:
                    r = hot_table[index*3];
                    g = hot_table[index*3+1];
                    b = hot_table[index*3+2];
                    break;
            }

            QRgb  newPixel = qRgb(r, g, b);  //合成彩色像素点

            newImage.setPixel(i, j, newPixel);
          }
    }


    return newImage;
}

/************************************************************************************************/
//直方图均衡
//核心 利用cdf做均衡
QImage equalizeHistogramProc1(QImage &image)
{
    //获取图像参数
    int width = image.width();
    int height = image.height();
    int pixel_num = width*height;

    // 将图片数据拆分到 r g b 三数组中
    uchar *channels = new uchar[pixel_num*3];
    uchar *r = channels;
    uchar *g = channels+pixel_num;
    uchar *b = channels+2*pixel_num;
    splitImageChannel(image, r, g, b);

    // rgb to ycrcb
    float *ycrcb = new float[pixel_num*3];
    float *y = ycrcb;
    float *cr = ycrcb+pixel_num;
    float *cb = ycrcb+2*pixel_num;
    rgb2ycrcb(r, g, b, pixel_num, y, cr, cb);

    // calculate hist/pdf
    int *hist = new int[pixel_num]; // hist/pdf
    const int gray_level = 256;
    float *gray_distribution = new float[gray_level];// cdf

    uchar *gray_equal = new uchar[gray_level]; // equalized gray

        // calculate pdf
        memset(hist, 0, pixel_num*sizeof(int));
        for (int i=0; i<pixel_num; i++)
        {
            int index = (int)y[i]; // gray scale
            hist[index]++;
        }

        // calculate cdf
        memset(gray_distribution, 0, gray_level*sizeof(float));
        gray_distribution[0] = hist[0]*1.0f/pixel_num;
        for (int i = 1; i < gray_level; i++)
        {
            gray_distribution[i] = gray_distribution[i-1] + hist[i]*1.0f/pixel_num;
        }

        // recalculate equalized gray
        memset(gray_equal, 0, gray_level*sizeof(uchar));
        for (int i = 0; i < gray_level; i++)
        {
            gray_equal[i] = (uchar)(255 * gray_distribution[i] + 0.5);
        }

        // new gray channel
        for (int i=0; i<pixel_num; i++)
        {
            int index = (int)y[i]; // gray scale
            y[i] = gray_equal[index];
        }

        // ycrcb to rgb
    ycrcb2rgb(y, cr, cb, pixel_num, r, g, b);

    // update image
    int count = 0;
    QImage newImage = image;//grayImage;
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            int nr = r[count];
            int ng = g[count];
            int nb = b[count];
            count++;
            newImage.setPixel(i, j, qRgb(nr,ng,nb));
        }
    }
    delete [] gray_equal;
    delete [] gray_distribution;
    delete [] channels;
    delete [] ycrcb;

    return newImage;
}

QImage equalizeHistogramProc(QImage &image)
{
    int width = image.width();
    int height = image.height();
    int pixel_num = width*height;

    // obtain gray image
    uchar *channels = new uchar[width*height*3];
    uchar *r = channels;
    uchar *g = channels+width*height;
    uchar *b = channels+2*width*height;
    splitImageChannel(image, r, g, b);

    uchar *c[4] = {r, g, b, 0};

    // calculate hist/pdf
    int *hist = new int[pixel_num]; // hist/pdf
    const int gray_level = 256;
    float *gray_distribution = new float[gray_level];// cdf

    uchar *gray_equal = new uchar[gray_level]; // equalized gray
    for (uchar **p=c; (*p) != 0; p++)
    {
        // calculate pdf
        memset(hist, 0, pixel_num*sizeof(int));
        for (int i=0; i<pixel_num; i++)
        {
            int index = (*p)[i]; // gray scale
            hist[index]++;
        }

        // calculate cdf
        memset(gray_distribution, 0, gray_level*sizeof(float));
        gray_distribution[0] = hist[0]*1.0f/pixel_num;
        for (int i = 1; i < gray_level; i++)
        {
            gray_distribution[i] = gray_distribution[i-1] + hist[i]*1.0f/pixel_num;
        }

        // recalculate equalized gray
        memset(gray_equal, 0, gray_level*sizeof(uchar));
        for (int i = 0; i < gray_level; i++)
        {
            gray_equal[i] = (uchar)(255 * gray_distribution[i] + 0.5);
        }

        // new gray channel
        for (int i=0; i<pixel_num; i++)
        {
            int index = (*p)[i]; // gray scale
            (*p)[i] = gray_equal[index];
        }
    }

    // update image
    int count = 0;
    QImage newImage = image;//grayImage;
    for (int j=0; j<height; j++) {
        for (int i=0; i<width; i++) {
            int nr = r[count];
            int ng = g[count];
            int nb = b[count];
            count++;
            newImage.setPixel(i, j, qRgb(nr,ng,nb));
        }
    }
    delete [] gray_equal;
    delete [] gray_distribution;
    delete [] channels;

    return newImage;
}


/************************************************************************************************/
// src 卷积  kernel 输出  dst
void filter(uchar *src, int image_width, int image_height,
            float *kernel, int kernel_width, int kernel_height, uchar *dst)
{
    int i, j, m, n;
    float val;

    //初始化目标图片为0
    memset(dst, 0, image_width*image_height);

    //注意卷积后的图片实际数据要比原图小
    for (j=kernel_height/2; j<image_height  - kernel_height/2; j++)
    {
        for (i = kernel_width/2; i<image_width -  kernel_width/2; i++)
        {

            //计算当前位置的卷积结果
            val = 0;
            //遍历 kernel
            for (n=-kernel_height/2; n<=kernel_height/2; n++)
            {
                for (m=-kernel_width/2; m<=kernel_width/2; m++)
                {
                    val += src[(j-n)*image_width  +   (i-m)]    *
                               kernel[(n+  kernel_height/2)*kernel_width +   m+  kernel_width/2];
                }
            }
            val =   val>255 ? 255 : val;  //异常值处理
            dst[j*image_width+i] = (uchar)(val<0 ? 0 : val);
        }
    }
}

/************************************************************************************************/
//自适应对比度增强
//基本原理见  链接：   https://www.cnblogs.com/Imageshop/p/3324282.html
//核心在于CG的大小会跟随局部数据自适应地变化   这也是 ACE 这个名字的由来

void adaptiveContrastEnhancement(QImage &src_image, float *rgb, float *rgb_ii, float *rgb_ii_power, int max_window_size,
                                 int half_window_size, float alpha, float max_cg, QImage &dst_image)
{
    dst_image = src_image;

    int image_width = src_image.width();
    int image_height = src_image.height();
    int pixel_num = image_width*image_height;

    int max_image_width = src_image.width() + 2*max_window_size;
    int max_image_height = src_image.height() + 2*max_window_size;
    int max_pixel_num = max_image_width*max_image_height;

    int max_kernel_height = 2*max_window_size+1;
    int max_kernel_width = 2*max_window_size+1;


    int kernel_height = 2*half_window_size+1;
    int kernel_width = 2*half_window_size+1;
    int kernel_size = kernel_height*kernel_width;

    float image_mean=0, image_std=0;   // 全域 均值和方差
    //下面将会利用 利用box_integral 加快  mean  和 std   的求解

    for (int c=0; c<3; c++)  //遍历三通道   注意下面的造作都是建立在单通道的基础上的
    {
        // image mean
        //box_integral 的参数含义详见下面的 box_integral 的实现
        image_mean = box_integral(rgb_ii+c*max_pixel_num, max_image_width, max_image_height,
                               max_window_size, max_window_size + image_width-1,
                               max_window_size, max_window_size + image_height-1);
        image_mean /= pixel_num;

        // image std
        image_std = box_integral(rgb_ii_power+c*max_pixel_num, max_image_width, max_image_height,
                               max_window_size, max_window_size + image_width-1,
                               max_window_size, max_window_size + image_height-1);

        image_std /= pixel_num;
        image_std -= image_mean*image_mean;

        image_std = sqrtf(image_std);

        // local area mean and std          利用局部均值方差计算cg值
        for (int j=max_kernel_height/2; j<max_image_height-max_kernel_height/2; j++)
        {
            for (int i=max_kernel_width/2; i<max_image_width-max_kernel_width/2; i++)
            {
                // mean
                float mean = box_integral(rgb_ii+c*max_pixel_num, max_image_width, max_image_height,
                                       i-kernel_width/2, i+kernel_width/2,
                                       j-kernel_height/2, j+kernel_height/2);
                mean /= kernel_size;

                // std
                float std= box_integral(rgb_ii_power+c*max_pixel_num, max_image_width, max_image_height,
                                       i-kernel_width/2, i+kernel_width/2,
                                       j-kernel_height/2, j+kernel_height/2);
                std = std/kernel_size - mean*mean;
                std = sqrtf(std);

                // constrast gain
                float cg = alpha*image_std/std;
                if (cg>max_cg) cg = max_cg;

                //核心公式 右半部分表示要增强的细节
                float dst_val = mean + cg * (rgb[c*max_pixel_num + j*max_image_width+i] - mean);

                //异常值处理
                if (dst_val > 255) dst_val = 255;
                if (dst_val < 0) dst_val = 0;
                QRgb temp = dst_image.pixel(i-max_kernel_width/2, j-max_kernel_height/2);

                int temp_r = qRed(temp);
                int temp_g = qGreen(temp);
                int temp_b = qBlue(temp);

                if (c==0) temp_r = dst_val;
                if (c==1) temp_g = dst_val;
                if (c==2) temp_b = dst_val;
                dst_image.setPixel(i-max_kernel_width/2, j-max_kernel_height/2, qRgb(temp_r, temp_g, temp_b));
            }
        }
    }
}

//下面这些函数都是为实现ACE服务的

//计算积分图
//核心： 通过数据共享节省计算新数据的时间
void calculate_integral_image(float *image, int width, int height, float *integral_image)
{
    int i, j;

    // first row only  遍历第一行，计算累积和
    float rs = 0;  //rs ： row sum
    for(j=0; j<width; j++)
    {
        rs += image[j];
        integral_image[j] = rs;
    }

    //纵向移动计算其余行的 累积和
    for(i=1; i<height; ++i)
    {
        rs = 0;   //rs ： row sum

        for(j=0; j<width; ++j)  //遍历 行
        {
            rs += image[i*width+j];

            //利用已获得的积分图结果计算当前点的积分图
            integral_image[i*width + j] = rs + integral_image[(i-1)*width +  j];
        }
    }
}

//计算积分图的平方
void calculate_integral_image_power(float *image, int width, int height, float *integral_image)
{
    int i, j;

    // first row only
    float rs = 0;
    for(j=0; j<width; j++)
    {
        rs += image[j]*image[j];
        integral_image[j] = rs;
    }
    for(i=1; i<height; ++i)
    {
        rs = 0;
        for(j=0; j<width; ++j)
        {
            rs += image[i*width+j]*image[i*width+j];
            integral_image[i*width+j] = rs + integral_image[(i-1)*width+j];
        }
    }
}


//inline是C++关键字，在函数声明或定义中，函数返回类型前加上关键字inline，即可以把函数指定为内联函数。
//这样可以解决一些频繁调用的函数大量消耗栈空间（栈内存）的问题

//利用积分图计算 以 (r1,c1)  和 (r2,c2)  为顶点的 box  的和值
__inline float box_integral(float *integral_image, int width, int height, int c1, int c2, int r1, int r2)
{
   float a, b, c, d;

   //边界处理
   a =  (c1-1<0 || r1-1<0)	    ?   0 : integral_image[(r1-1) * width + (c1-1)];
   b =  r1-1<0				        ?   0 : integral_image[(r1-1) * width + c2];
   c =  c1-1<0				        ?   0 : integral_image[r2 * width + (c1-1)];
   d =  integral_image[r2 * width + c2];

   return a - b - c + d;
}

//关于快速算法，还有一个更加优化的版本，优势在于不用计算积分图
//其名字为 box filter  基本思想与积分图类似，都是通过数据共享节省计算新数据的时间
//box filter  链接： https://blog.csdn.net/lxy201700/article/details/25104887

