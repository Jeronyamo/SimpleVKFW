#ifndef SOGLFW_IMAGE_H
#define SOGLFW_IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../libs/STBimage/stb_image.h"
#include "../libs/STBimage/stb_image_write.h"

#include "color.h"

#include <stdexcept>


namespace Simple {
    //  ============  Utilities  ============  \\

    struct Image;
    struct Image256;
    template <class C> struct ImageBase;

    template <typename T>
    struct ImBaseType {
        using type = T;
    };
    template <> struct ImBaseType<Image> {
        using type = float;
    };
    template <> struct ImBaseType<Image256> {
        using type = unsigned char;
    };
    template <typename T>
    struct ImBaseType<ImageBase<T>> {
        using type = T;
    };
    template <typename T>
    struct ImBaseType<ColorBase<T>> {
        using type = T;
    };
    template <typename T>
    struct ImBaseType<ColorABase<T>> {
        using type = T;
    };
    template <typename T>
    struct ImBaseType<ImageBase<ColorBase<T>>> {
        using type = T;
    };
    template <typename T>
    struct ImBaseType<ImageBase<ColorABase<T>>> {
        using type = T;
    };

/*
Image with arbitrary number of channels - float and uchar
Image using Color | ColorA - float and uchar
*/


    //  ============  Image classes  ============  \\

    struct Image {
        const char *filename = "";
        int width = 0, height = 0, channels = 0;
        float *image = nullptr;

        Image(int _w, int _h, int _ch = 1) : width(_w), height(_h), channels(_ch), image(new float[_w * _h * _ch]) {}

        Image(const Image &img) :
                width(img.width), height(img.height), channels(img.channels), image(new float[img.width * img.height * img.channels]) {
            std::memcpy(image, img.image, img.width * img.height * img.channels * sizeof(float));
        }

        Image(const Image256 &img);

        template <class T>
        Image(const ImageBase<T> &img) :
                width(img.width), height(img.height), channels(img.channels), image(new float[img.width * img.height * img.channels]) {
            if (!std::is_floating_point<typename ImBaseType<T>::type>::value) {
                unsigned char *pt = (unsigned char*)img.image;
                for (unsigned i = 0; i < img.width * img.height * img.channels; ++i) image[i] = COLORS_UTOF * pt[i];
            }
            else std::memcpy(image, img.image, img.width * img.height * img.channels);
        }

        Image(const char* _fname, bool hdr = false) {
            if (!hdr) {
                unsigned char *_im = stbi_load(_fname, &width, &height, &channels, 0);
                image = new float[width * height * channels];
                for (unsigned i = 0u; i < width * height * channels; ++i)
                    image[i] = COLORS_UTOF * _im[i];
                stbi_image_free(_im);
                filename = _fname;
            }
            else {
                float *_im = stbi_loadf(_fname, &width, &height, &channels, 0);
                image = new float[width * height * channels];
                memcpy(image, _im, width * height * channels * sizeof(float));
                stbi_image_free(_im);
                filename = _fname;
            }
        }

        ~Image() {
            if (image != nullptr)
                delete[] image;
        }


        void saveAsPNG(const char *fname) const;

        Image operator+(const Image &im) {
            UVec3 size{width <= im. width ? width : im. width,
                       height <= im.height ? height : im.height,
                       channels <= im.channels ? channels : im.channels};
            UVec2 strides{width * channels, im.width * im.channels};

            Image res(size.x, size.y, size.z);
            float *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    for (unsigned k = 0u; k < size.z; ++k)
                        *imptr++ = clampCol(image[i * strides.x + j * channels + k] + im.image[i * strides.y + j * im.channels + k]);
            return res;
        }

        Image operator-(const Image &im) {
            UVec3 size{width <= im. width ? width : im. width,
                       height <= im.height ? height : im.height,
                       channels <= im.channels ? channels : im.channels};
            UVec2 strides{width * channels, im.width * im.channels};

            Image res(size.x, size.y, size.z);
            float *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    for (unsigned k = 0u; k < size.z; ++k)
                        *imptr++ = clampCol(image[i * strides.x + j * channels + k] - im.image[i * strides.y + j * im.channels + k]);
            return res;
        }

        Image operator*(const Image &im) {
            UVec3 size{width <= im. width ? width : im. width,
                       height <= im.height ? height : im.height,
                       channels <= im.channels ? channels : im.channels};
            UVec2 strides{width * channels, im.width * im.channels};

            Image res(size.x, size.y, size.z);
            float *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    for (unsigned k = 0u; k < size.z; ++k)
                        *imptr++ = clampCol(image[i * strides.x + j * channels + k] * im.image[i * strides.y + j * im.channels + k]);
            return res;
        }

        Image operator/(const Image &im) {
            UVec3 size{width <= im. width ? width : im. width,
                       height <= im.height ? height : im.height,
                       channels <= im.channels ? channels : im.channels};
            UVec2 strides{width * channels, im.width * im.channels};

            Image res(size.x, size.y, size.z);
            float *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    for (unsigned k = 0u; k < size.z; ++k)
                        *imptr++ = notCloseToZero(im.image[i * strides.y + j * im.channels + k]) ?
                                            clampCol(   image[i * strides.x + j * channels + k] /
                                                  im.image[i * strides.y + j * im.channels + k]) : 1.f;
            return res;
        }
    };



    struct Image256 {
        const char *filename = "";
        int width = 0, height = 0, channels = 0;
        unsigned char *image = nullptr;

        Image256(int _w, int _h, int _ch = 1) : width(_w), height(_h), channels(_ch), image(new unsigned char[_w * _h * _ch]) {}

        Image256(const Image256 &img) :
                width(img.width), height(img.height), channels(img.channels), image(new unsigned char[img.width * img.height * img.channels]) {
            std::memcpy(image, img.image, img.width * img.height * img.channels);
        }

        Image256(const Image &img) :
                width(img.width), height(img.height), channels(img.channels), image(new unsigned char[img.width * img.height * img.channels]) {
            for (unsigned i = 0u; i < img.width * img.height * img.channels; ++i)
                image[i] = COLORS_FTOU * img.image[i];
        }

        template <class T>
        Image256(const ImageBase<T> &img) :
                width(img.width), height(img.height), channels(img.channels), image(new unsigned char[img.width * img.height * img.channels]) {
            if (std::is_floating_point<typename ImBaseType<T>::type>::value) {
                float *pt = (float*)img.image;
                for (unsigned i = 0; i < img.width * img.height * img.channels; ++i) image[i] = COLORS_FTOU * pt[i];
            }
            else std::memcpy(image, img.image, img.width * img.height * img.channels);
        }

        Image256(const char* _fname) {
            unsigned char *_im = stbi_load(_fname, &width, &height, &channels, 0);
            image = new unsigned char[width * height * channels];
            memcpy(image, _im, width * height * channels);
            stbi_image_free(_im);
            filename = _fname;
        }

        ~Image256() {
            if (image != nullptr)
                delete[] image;
        }


        void saveAsPNG(const char *fname = nullptr) const {
            if (!fname) fname = filename;
            stbi_write_png(fname, width, height, channels, image, width * channels);
        }

        Image256 operator+(const Image256 &im) {
            UVec3 size{width <= im. width ? width : im. width,
                       height <= im.height ? height : im.height,
                       channels <= im.channels ? channels : im.channels};
            UVec2 strides{width * channels, im.width * im.channels};

            Image256 res(size.x, size.y, size.z);
            unsigned char *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    for (unsigned k = 0u; k < size.z; ++k)
                        *imptr++ = clampCol(   image[i * strides.x + j *    channels + k] +
                                         im.image[i * strides.y + j * im.channels + k]);
            return res;
        }

        Image256 operator-(const Image256 &im) {
            UVec3 size{width <= im. width ? width : im. width,
                       height <= im.height ? height : im.height,
                       channels <= im.channels ? channels : im.channels};
            UVec2 strides{width * channels, im.width * im.channels};

            Image256 res(size.x, size.y, size.z);
            unsigned char *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    for (unsigned k = 0u; k < size.z; ++k)
                        *imptr++ = clampCol(   image[i * strides.x + j *    channels + k] -
                                         im.image[i * strides.y + j * im.channels + k]);
            return res;
        }

        Image256 operator*(const Image256 &im) {
            UVec3 size{width <= im. width ? width : im. width,
                       height <= im.height ? height : im.height,
                       channels <= im.channels ? channels : im.channels};
            UVec2 strides{width * channels, im.width * im.channels};

            Image256 res(size.x, size.y, size.z);
            unsigned char *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    for (unsigned k = 0u; k < size.z; ++k)
                        *imptr++ = clampCol(   image[i * strides.x + j *    channels + k] *
                                         im.image[i * strides.y + j * im.channels + k]);
            return res;
        }

        Image256 operator/(const Image256 &im) {
            UVec3 size{width <= im. width ? width : im. width,
                       height <= im.height ? height : im.height,
                       channels <= im.channels ? channels : im.channels};
            UVec2 strides{width * channels, im.width * im.channels};

            Image256 res(size.x, size.y, size.z);
            unsigned char *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    for (unsigned k = 0u; k < size.z; ++k)
                        *imptr++ = notCloseToZero(im.image[i * strides.y + j * im.channels + k]) ?
                                            clampCol(   image[i * strides.x + j *    channels + k] /
                                                  im.image[i * strides.y + j * im.channels + k]) : 1.f;
            return res;
        }
    };



    Image::Image(const Image256 &img) :
            width(img.width), height(img.height), channels(img.channels), image(new float[img.width * img.height * img.channels]) {
        for (unsigned i = 0u; i < img.width * img.height * img.channels; ++i)
            image[i] = COLORS_UTOF * img.image[i];
    }

    void Image::saveAsPNG(const char *fname = nullptr) const {
        if (!fname) fname = filename;
        Image256(*this).saveAsPNG(fname);
    }



    template <class T> // T - color type or unsigned char | float (for single channel images)
    struct ImageBase {
        enum InterpolationMode { NEAREST, BILINEAR };
        static InterpolationMode DefaultInterpolationMode;

        const char *filename = "";
        int width = 0, height = 0;
        const int channels = ColChannels<T>::channels;
        T (ImageBase<T>::*interpolation[2])(const Vec2 &) const;
        T *image = nullptr;

        ImageBase(int _w, int _h) : width(_w), height(_h), image(new T[_w * _h]), interpolation{ &ImageBase<T>::nearest, &ImageBase<T>::bilinear } {}

        ImageBase(const ImageBase<T> &img) : width(img.width), height(img.height),
                                             image(new T[img.width * img.height]),
                                             interpolation{ &ImageBase<T>::nearest, &ImageBase<T>::bilinear } {
            std::memcpy(image, img.image, img.width * img.height * sizeof(T));
        }

        ImageBase(const Image &img, int _chan = 0u) : width(img.width), height(img.height),
                                                      image(new T[img.width * img.height]),
                                                      interpolation{ &ImageBase<T>::nearest, &ImageBase<T>::bilinear } {
            if (channels == 1) {
                _chan = _chan >= 0 ? _chan : channels - _chan;
                if (_chan >= img.channels)
                    throw std::out_of_range("Error: image channel index outside of range");
                for (unsigned i = 0u; i < img.width * img.height; ++i)
                    image[i] = img.image[i * img.channels + _chan] * ColTransform<float, T>::coef;
            }
            else if (std::is_same<typename ImBaseType<T>::type, float>::value) {
                if (channels == img.channels)
                    std::memcpy(image, img.image, img.width * img.height * sizeof(T));
                else if (channels < img.channels) {
                    typename ImBaseType<T>::type *ptr = (typename ImBaseType<T>::type*) image;
                    for (unsigned i = 0u; i < height; ++i)
                        for (unsigned j = 0u; j < width; ++j)
                            for (unsigned k = 0u; k < channels; ++k)
                                *ptr++ = img.image[i * width * img.channels + j * img.channels + k];
                }
                else {
                    typename ImBaseType<T>::type *ptr = (typename ImBaseType<T>::type*) image;
                    for (unsigned i = 0u; i < height; ++i)
                        for (unsigned j = 0u; j < width; ++j) {
                            for (unsigned k = 0u; k < img.channels; ++k)
                                ptr[i * width * channels + j * channels + k] = img.image[i * width * img.channels + j * img.channels + k];
                            for (unsigned k = img.channels; k < channels; ++k)
                                ptr[i * width * channels + j * channels + k] = (k == 3);
                        }
                }
            }
            else {
                typename ImBaseType<T>::type *ptr = (typename ImBaseType<T>::type*) image;
                float *imptr = img.image;
                if (channels == img.channels){
                    for (unsigned i = 0u; i < width * height * channels; ++i)
                        *ptr++ = ColTransform<float, typename ImBaseType<T>::type>::coef * *imptr++;
                }
                else if (channels < img.channels) {
                    for (unsigned i = 0u; i < height; ++i)
                        for (unsigned j = 0u; j < width; ++j)
                            for (unsigned k = 0u; k < channels; ++k)
                                *ptr++ = ColTransform<float, typename ImBaseType<T>::type>::coef * img.image[i * width * img.channels + j * img.channels + k];
                }
                else {
                    for (unsigned i = 0u; i < height; ++i)
                        for (unsigned j = 0u; j < width; ++j) {
                            for (unsigned k = 0u; k < img.channels; ++k)
                                ptr[i * width * channels + j * channels + k] = ColTransform<float, typename ImBaseType<T>::type>::coef * *imptr++;
                            for (unsigned k = img.channels; k < channels; ++k)
                                ptr[i * width * channels + j * channels + k] = 255 * (k == 3);
                        }
                }
            }
        }

        ImageBase(const Image256 &img, int _chan = 0u) : width(img.width), height(img.height),
                                                         image(new T[img.width * img.height]),
                                                         interpolation{ &ImageBase<T>::nearest, &ImageBase<T>::bilinear } {
            if (channels == 1) {
                _chan = _chan >= 0 ? _chan : channels - _chan;
                if (_chan >= img.channels)
                    throw std::out_of_range("Error: image channel index outside of range");
                for (unsigned i = 0u; i < img.width * img.height; ++i)
                    image[i] = img.image[i * img.channels + _chan] * ColTransform<unsigned char, T>::coef;
            }
            else if (std::is_same<typename ImBaseType<T>::type, unsigned char>::value) {
                if (channels == img.channels)
                    std::memcpy(image, img.image, img.width * img.height * sizeof(T));
                else if (channels < img.channels) {
                    typename ImBaseType<T>::type *ptr = (typename ImBaseType<T>::type*) image;
                    for (unsigned i = 0u; i < height; ++i)
                        for (unsigned j = 0u; j < width; ++j)
                            for (unsigned k = 0u; k < channels; ++k)
                                *ptr++ = img.image[i * width * img.channels + j * img.channels + k];
                }
                else {
                    typename ImBaseType<T>::type *ptr = (typename ImBaseType<T>::type*) image;
                    unsigned char *imptr = img.image;
                    for (unsigned i = 0u; i < height; ++i)
                        for (unsigned j = 0u; j < width; ++j) {
                            for (unsigned k = 0u; k < img.channels; ++k)
                                ptr[i * width * channels + j * channels + k] = *imptr++;
                            for (unsigned k = img.channels; k < channels; ++k)
                                ptr[i * width * channels + j * channels + k] = 255 * (k == 3);
                        }
                }
            }
            else {
                typename ImBaseType<T>::type *ptr = (typename ImBaseType<T>::type*) image;
                unsigned char *imptr = img.image;

                if (channels == img.channels){
                    for (unsigned i = 0u; i < width * height * channels; ++i)
                        *ptr++ = ColTransform<unsigned char, typename ImBaseType<T>::type>::coef * *imptr++;
                }
                else if (channels < img.channels) {
                    for (unsigned i = 0u; i < height; ++i)
                        for (unsigned j = 0u; j < width; ++j)
                            for (unsigned k = 0u; k < channels; ++k)
                                *ptr++ = ColTransform<unsigned char, typename ImBaseType<T>::type>::coef * img.image[i * width * img.channels + j * img.channels + k];
                }
                else {
                    typename ImBaseType<T>::type *ptr = (typename ImBaseType<T>::type*) image;
                    for (unsigned i = 0u; i < height; ++i)
                        for (unsigned j = 0u; j < width; ++j) {
                            for (unsigned k = 0u; k < img.channels; ++k)
                                ptr[i * width * channels + j * channels + k] = ColTransform<unsigned char, typename ImBaseType<T>::type>::coef * *imptr++;
                            for (unsigned k = img.channels; k < channels; ++k)
                                ptr[i * width * channels + j * channels + k] = (k == 3);
                        }
                }
            }
        }

        template <class Y = T>
        ImageBase(const char* _fname) : interpolation{ &ImageBase<T>::nearest, &ImageBase<T>::bilinear } {
            int ch = 0;
            void *_im = nullptr;
            if (std::is_floating_point<typename ImBaseType<T>::type>::value)
                 _im = stbi_loadf(_fname, &width, &height, &ch, channels);
            else _im = stbi_load (_fname, &width, &height, &ch, channels);
            image = new T[width * height];
            memcpy(image, _im, width * height * sizeof(T));
            stbi_image_free(_im);
        }

        ~ImageBase() {
            if (image != nullptr) delete[] image;
        }


        template <typename Y>
        void setChannel(int _chan, const ImageBase<Y> &img, int _fromCh = 0) {
            if (width != img.width || height != img.height)
                throw std::invalid_argument("Error: shapes of the images don't match");

            _chan = _chan >= 0 ? _chan : channels - _chan;
            _fromCh = _fromCh >= 0 ? _fromCh : img.channels - _fromCh;
            if (_chan >= channels || _fromCh >= img.channels)
                throw std::out_of_range("Error: image channel index outside of range");

            for (unsigned i = 0u; i < width * height; ++i)
                image[i * channels + _chan] = img.image[i * img.channels + _fromCh] * ColTransform<Y, T>::coef;
        }

        ImageBase<typename ImBaseType<T>::type> getChannel(int _chan) {
            _chan = _chan >= 0 ? _chan : channels - _chan;
            if (_chan >= channels)
                throw std::out_of_range("Error: image channel index outside of range");

            ImageBase<typename ImBaseType<T>::type> _channel{width, height};
            auto *_ptr = _channel.image;

            for (unsigned i = _chan; i < width * height * channels; i += channels)
                *_ptr++ = image[i];
            
            return _channel;
        }

        void saveAsPNG(const char *fname = nullptr) const {
            if (!fname) fname = filename;
            Image256(*this).saveAsPNG(fname);
        }

        T nearest(const Vec2 &texcoords) const {
            UVec2 coord{round(texcoords.x * width), round(texcoords.y * height)};
            return image[coord.y * width + coord.x];
        }

        T bilinear(const Vec2 &texcoords) const {
            Vec2 uv = texcoords * Vec2(width, height);
            UVec2 ind = uv;
            uv -= ind;

            T pix[4]{ ind.x      < width &&  ind.y      < height ? image[ ind.y      * width + ind.x    ] : T(0),
                     (ind.x + 1) < width &&  ind.y      < height ? image[ ind.y      * width + ind.x + 1] : T(0), 
                      ind.x      < width && (ind.y + 1) < height ? image[(ind.y + 1) * width + ind.x    ] : T(0), 
                     (ind.x + 1) < width && (ind.y + 1) < height ? image[(ind.y + 1) * width + ind.x + 1] : T(0) };
            return (1 - uv.y) * ((1 - uv.x) * pix[0] + uv.x * pix[1]) +
                        uv.y  * ((1 - uv.x) * pix[2] + uv.x * pix[3]);
        }

        T &operator[](const UVec2 &ind) { return image[ind.y * width + ind.x]; }
        const T &operator[](const UVec2 &ind) const { return image[ind.y * width + ind.x]; }
        T operator[](const Vec2 &texcoords) const { return (this->*(interpolation[DefaultInterpolationMode]))(texcoords); }


        void operator=(const ImageBase<T> &img) {
            width = img.width;
            height = img.height;
            if (image != nullptr) delete[] image;
            image = new T[width * height];
            std::memcpy(image, img.image, img.width * img.height * sizeof(T));
        }

        ImageBase<T> operator+(const ImageBase<T> &im) {
            UVec2 size{width <= im.width ? width : im.width,
                      height <= im.height ? height : im.height};

            ImageBase<T> res(size.x, size.y);
            T *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    *imptr++ = image[i * width + j] + im.image[i * im.width + j];
            return res;
        }

        ImageBase<T> operator-(const ImageBase<T> &im) {
            UVec2 size{width <= im.width ? width : im.width,
                      height <= im.height ? height : im.height};

            ImageBase<T> res(size.x, size.y);
            T *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    *imptr++ = image[i * width + j] - im.image[i * im.width + j];
            return res;
        }

        ImageBase<T> operator*(const ImageBase<T> &im) {
            UVec2 size{width <= im.width ? width : im.width,
                      height <= im.height ? height : im.height};

            ImageBase<T> res(size.x, size.y);
            T *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    *imptr++ = image[i * width + j] * im.image[i * im.width + j];
            return res;
        }

        ImageBase<T> operator/(const ImageBase<T> &im) {
            UVec2 size{width <= im.width ? width : im.width,
                      height <= im.height ? height : im.height};

            ImageBase<T> res(size.x, size.y);
            T *imptr = res.image;

            for (unsigned i = 0u; i < size.y; ++i)
                for (unsigned j = 0u; j < size.x; ++j)
                    *imptr++ = image[i * width + j] / im.image[i * im.width + j];
            return res;
        }
    };

    template <class T>
    typename std::enable_if<!std::is_fundamental<T>::value, ImageBase<T>>::type
    PorterDuffImageBlendOver(const ImageBase<T> &i1, const ImageBase<T> &i2) {
        UVec2 size{i1.width <= i2. width ? i1.width : i2. width,
                    i1.height <= i2.height ? i1.height : i2.height};

        ImageBase<T> res(size.x, size.y);
        T *imptr = res.image;

        for (unsigned i = 0u; i < size.y; ++i)
            for (unsigned j = 0u; j < size.x; ++j)
                *imptr++ = PorterDuffBlendOver(i1.image[i * i1.width + j], i2.image[i * i2.width + j]);
        return res;
    }

    template <class T>
    typename std::enable_if<!std::is_fundamental<T>::value, ImageBase<T>>::type
    PorterDuffImageBlendIn  (const ImageBase<T> &i1, const ImageBase<T> &i2) {
        UVec2 size{i1.width <= i2. width ? i1.width : i2. width,
                    i1.height <= i2.height ? i1.height : i2.height};

        ImageBase<T> res(size.x, size.y);
        T *imptr = res.image;

        for (unsigned i = 0u; i < size.y; ++i)
            for (unsigned j = 0u; j < size.x; ++j)
                *imptr++ = PorterDuffBlendIn(i1.image[i * i1.width + j], i2.image[i * i2.width + j]);
        return res;
    }

    template <class T>
    typename std::enable_if<!std::is_fundamental<T>::value, ImageBase<T>>::type
    PorterDuffImageBlendOut (const ImageBase<T> &i1, const ImageBase<T> &i2) {
        UVec2 size{i1.width <= i2. width ? i1.width : i2. width,
                    i1.height <= i2.height ? i1.height : i2.height};

        ImageBase<T> res(size.x, size.y);
        T *imptr = res.image;

        for (unsigned i = 0u; i < size.y; ++i)
            for (unsigned j = 0u; j < size.x; ++j)
                *imptr++ = PorterDuffBlendOut(i1.image[i * i1.width + j], i2.image[i * i2.width + j]);
        return res;
    }

    template <class T>
    typename std::enable_if<!std::is_fundamental<T>::value, ImageBase<T>>::type
    PorterDuffImageBlendAtop(const ImageBase<T> &i1, const ImageBase<T> &i2) {
        UVec2 size{i1.width <= i2. width ? i1.width : i2. width,
                    i1.height <= i2.height ? i1.height : i2.height};

        ImageBase<T> res(size.x, size.y);
        T *imptr = res.image;

        for (unsigned i = 0u; i < size.y; ++i)
            for (unsigned j = 0u; j < size.x; ++j)
                *imptr++ = PorterDuffBlendAtop(i1.image[i * i1.width + j], i2.image[i * i2.width + j]);
        return res;
    }

    template <class T>
    typename std::enable_if<!std::is_fundamental<T>::value, ImageBase<T>>::type
    PorterDuffImageBlendXor (const ImageBase<T> &i1, const ImageBase<T> &i2) {
        UVec2 size{i1.width <= i2. width ? i1.width : i2. width,
                    i1.height <= i2.height ? i1.height : i2.height};

        ImageBase<T> res(size.x, size.y);
        T *imptr = res.image;

        for (unsigned i = 0u; i < size.y; ++i)
            for (unsigned j = 0u; j < size.x; ++j)
                *imptr++ = PorterDuffBlendXor(i1.image[i * i1.width + j], i2.image[i * i2.width + j]);
        return res;
    }

    template <class T>
    typename std::enable_if<!std::is_fundamental<T>::value, ImageBase<T>>::type
    operator<<(const ImageBase<T> &i2, const ImageBase<T> &i1) {
        UVec2 size{i1.width  <= i2.width  ? i1.width  : i2. width,
                   i1.height <= i2.height ? i1.height : i2.height};

        ImageBase<T> res(size.x, size.y);
        T *imptr = res.image;

        for (unsigned i = 0u; i < size.y; ++i)
            for (unsigned j = 0u; j < size.x; ++j)
                *imptr++ = PorterDuffBlendOver(i1.image[i * i1.width + j], i2.image[i * i2.width + j]);
        return res;
    }

    template <class T>
    typename std::enable_if<!std::is_fundamental<T>::value, ImageBase<T>>::type
    operator>>(const ImageBase<T> &i1, const ImageBase<T> &i2) {
        UVec2 size{i1.width  <= i2.width  ? i1.width  : i2. width,
                   i1.height <= i2.height ? i1.height : i2.height};

        ImageBase<T> res(size.x, size.y);
        T *imptr = res.image;

        for (unsigned i = 0u; i < size.y; ++i)
            for (unsigned j = 0u; j < size.x; ++j)
                *imptr++ = PorterDuffBlendOver(i1.image[i * i1.width + j], i2.image[i * i2.width + j]);
        return res;
    }

    template <class T> typename ImageBase<T>::InterpolationMode ImageBase<T>::DefaultInterpolationMode = BILINEAR;


    //  ============  Image types declaration  ============  \\

    typedef ImageBase<float> ImageCh;
    typedef ImageBase<unsigned char> ImageCh256;
    typedef ImageBase<Color > Image3;
    typedef ImageBase<ColorA> Image4;
    typedef ImageBase<Color256 > Image3_256;
    typedef ImageBase<ColorA256> Image4_256;
};


#endif