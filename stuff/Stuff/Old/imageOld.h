#ifndef SOGLFW_IMAGE_H
#define SOGLFW_IMAGE_H


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../libs/STBimage/stb_image.h"
#include "../libs/STBimage/stb_image_write.h"

//#include "colors.h"

#include <string>


namespace Simple {
    constexpr unsigned IMAGE_UINT_MAX = 255;
    constexpr float IMAGE_FLT_MAX = 1.;
    constexpr unsigned IMAGE_FLT_TO_UINT = IMAGE_UINT_MAX / IMAGE_FLT_MAX;
    constexpr float IMAGE_UINT_TO_FLT = IMAGE_FLT_MAX / IMAGE_UINT_MAX;



    template <int ch> struct ImageBase;
    typedef ImageBase <3> ImageRGB;
    typedef ImageBase <4> ImageRGBA;


    //  ============   blends  ============  \\

    template <class T, class Y> Image ImageADD(const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image ImageSUB(const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image ImageMUL(const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image ImageDIV(const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image ImageMIN(const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image ImageMAX(const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image ImageAVG(const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image ImageDIF(const ImageBase <T> & img1, const ImageBase <Y> & img2);

    template <class T, class Y> void ImageInplaceADD(ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void ImageInplaceSUB(ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void ImageInplaceMUL(ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void ImageInplaceDIV(ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void ImageInplaceMIN(ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void ImageInplaceMAX(ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void ImageInplaceAVG(ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void ImageInplaceDIF(ImageBase <T> &img1, const ImageBase <Y> & img2);


    //  ============  Porter-Duff blend funcs  ============  \\
//MAKE TEMPLATE METHODS, NOT CLASS
    template <class T, class Y> Image PorterDuffBlendOver(const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image PorterDuffBlendIn  (const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image PorterDuffBlendOut (const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image PorterDuffBlendAtop(const ImageBase <T> & img1, const ImageBase <Y> & img2);
    template <class T, class Y> Image PorterDuffBlendXor (const ImageBase <T> & img1, const ImageBase <Y> & img2);

    template <class T, class Y> void PorterDuffBlendInplaceOver(ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void PorterDuffBlendInplaceIn  (ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void PorterDuffBlendInplaceOut (ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void PorterDuffBlendInplaceAtop(ImageBase <T> &img1, const ImageBase <Y> & img2);
    template <class T, class Y> void PorterDuffBlendInplaceXor (ImageBase <T> &img1, const ImageBase <Y> & img2);


    //  ============  Image class  ============  \\

    template <int ch>
    struct ImageBase {
        int width = 0, height = 0, channels = ch;
        std::conditional<ch == 4, RGBA*, RGB*>::type image = nullptr;
        std::string filename = "";

        ImageBase() {}
        ImageBase(const char *filename);
        ImageBase(const ImageBase <ch> &img, bool deepcopy = true);
        template <class Y> ImageBase(const ImageBase <Y> &img);
        template <class Y> ImageBase(int _width, int _height, int _channels, Y init_value = 0)
                                        : width(_width), height(_height), channels(_channels),
                                        image(new T[_width * _height * _channels]) {
            for (unsigned i = 0u; i < _width * _height * _channels; ++i) image[i] = init_value;
        }
        ~ImageBase() { if (image != nullptr) delete[] image; }

        void save();
        void saveAs(const char *filename);

        ImageBase operator +(const ImageBase <Y> &op) { return ImageADD(*this, op); }  //ADD
        ImageBase operator -(const ImageBase <Y> &op) { return ImageSUB(*this, op); }  //SUB
        ImageBase operator *(const ImageBase <Y> &op) { return ImageMUL(*this, op); }  //MUL
        ImageBase operator /(const ImageBase <Y> &op) { return ImageDIV(*this, op); }  //DIV
        ImageBase operator >(const ImageBase <Y> &op) { return PorterDuffBlendOver(*this, op); }  //OVER
        ImageBase operator %(const ImageBase <Y> &op) { return PorterDuffBlendAtop(*this, op); }  //ATOP
        ImageBase operator ^(const ImageBase <Y> &op) { return PorterDuffBlendXor (*this, op); }  //XOR
        ImageBase operator>>(const ImageBase <Y> &op) { return PorterDuffBlendIn  (*this, op); }  //IN
        ImageBase operator<<(const ImageBase <Y> &op) { return PorterDuffBlendOut (*this, op); }  //OUT
    };
}


#endif