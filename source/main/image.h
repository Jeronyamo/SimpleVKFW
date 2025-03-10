#ifndef SVKFW_IMAGE_H
#define SVKFW_IMAGE_H

// Always use this header when need STBI

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "STBI/stb_image.h"
#include "STBI/stb_image_write.h"

#include "main/color.h"

#include <memory>


namespace Simple {
//  ========================================  \\
                Image base class
//  ========================================  \\

    template <typename T, int Ch>
    struct ImageBase {
        using ColorType = UtilCol::BaseChColorT<T, Ch>;
        std::shared_ptr<ColorType> img;
        int width = 0, height = 0;
        const int channels = Ch;

        ImageBase() {}
        ImageBase(const ImageBase<T, Ch> &_image) : img{_image.img}, width{_image.width}, height{_image.height} {}
        ImageBase(ColorType *_imptr, int _w, int _h, int _c = 0) : img{_imptr}, width{_w}, height{_h} {}
        ImageBase(int _w, int _h) : img{new ColorType[_w * _h]}, width{_w}, height{_h} {}
        ImageBase(int _w, int _h, ColorType _col) : width{_w}, height{_h} {
            img = new ColorType[_w * _h];
            for (unsigned i = 0u; i < _w * _h; ++i)
                img.get()[i] = _col;
        }

        ImageBase<UtilCol::BaseColorType<UtilCol::OtherBType<ColorType>>, Ch> convert() const {
            ImageBase<UtilCol::BaseColorType<UtilCol::OtherBType<ColorType>>, Ch> __img{width, height};

            for (unsigned i = 0u; i < width * height; ++i)
                __img.img.get()[i] = Color::convert(img.get()[i]);

            return __img;
        }
    };


// Image class with variable number of channels
    template <typename T>
    struct ImageBase<T, 0> {
        using ColorType = UtilCol::BaseChColorT<T, 0>;
        std::shared_ptr<ColorType> img;
        int width = 0, height = 0, channels = 0;

        ImageBase() {}
        ImageBase(const ImageBase<T, 0> &_image) :
                img{_image.img}, width{_image.width}, height{_image.height}, channels{_image.channels} {}
        ImageBase(ColorType *_imptr, int _w, int _h, int _c) :
                img{_imptr}, width{_w}, height{_h}, channels{_c} {}
        ImageBase(int _w, int _h, int _c) :
                img{new ColorType[_w * _h * _c]}, width{_w}, height{_h}, channels{_c} {}


        ImageBase<UtilCol::BaseColorType<UtilCol::OtherBType<ColorType>>, 0> convert() const {
            ImageBase<UtilCol::BaseColorType<UtilCol::OtherBType<ColorType>>, 0> __img{width, height, channels};

            for (unsigned i = 0u; i < width * height * channels; ++i)
                __img.img.get()[i] = Color::convert(img.get()[i]);

            return __img;
        }
    };


//  ============  Image classes declaration  ============  \\

    typedef ImageBase<Color1f, 0> Image;
    typedef ImageBase<Color1u, 0> Image256;

    typedef ImageBase<Color1f, 1> Image1f;
    typedef ImageBase<Color1u, 1> Image1u;
    typedef ImageBase<Color1f, 2> Image2f;
    typedef ImageBase<Color1u, 2> Image2u;
    typedef ImageBase<Color1f, 3> Image3f;
    typedef ImageBase<Color1u, 3> Image3u;
    typedef ImageBase<Color1f, 4> Image4f;
    typedef ImageBase<Color1u, 4> Image4u;

    // for .exr, TODO: test, type conversions might not work in some places
    typedef ImageBase<float, 0> ImageHDR;
    typedef ImageBase<float, 1> ImageHDR1;
    typedef ImageBase<float, 2> ImageHDR2;
    typedef ImageBase<float, 3> ImageHDR3;
    typedef ImageBase<float, 4> ImageHDR4;



//  ============  Image utilities  ============  \\

    namespace UtilCol {
        template <typename T, int Ch>
        struct Channels<ImageBase<T, Ch>> { static const int num = Ch; };
    };


//  ============  Image-related functions  ============  \\

    namespace Img {
        enum FileType { PNG, JPG, BMP, TGA, HDR }; // file extensions

//  Image Save/Load
        template <FileType E, typename T, int Ch>
        void save(const char *_fname, const ImageBase<T, Ch> &_img) {
            if (E == HDR) {
                if (std::is_floating_point<UtilCol::BaseType<T>>::value) {
                    stbi_write_hdr(_fname, _img.width, _img.height, Ch ? Ch : _img.channels, (float*)  _img.img.get());
                }
                else {
                    auto __img = _img.convert();
                    stbi_write_hdr(_fname, _img.width, _img.height, Ch ? Ch : _img.channels, (float*) __img.img.get());
                }
            }
            else {
                if (std::is_floating_point<UtilCol::BaseType<T>>::value) {
                    auto __img = _img.convert();
                    if (E == PNG) stbi_write_png(_fname, _img.width, _img.height, Ch ? Ch : _img.channels,
                                                 (uchar*) __img.img.get(), 0);
                    if (E == JPG) stbi_write_jpg(_fname, _img.width, _img.height, Ch ? Ch : _img.channels,
                                                 (uchar*) __img.img.get(), 0);
                    if (E == PNG) stbi_write_bmp(_fname, _img.width, _img.height, Ch ? Ch : _img.channels,
                                                 (uchar*) __img.img.get());
                    if (E == PNG) stbi_write_tga(_fname, _img.width, _img.height, Ch ? Ch : _img.channels,
                                                 (uchar*) __img.img.get());
                }
                else {
                    if (E == PNG) stbi_write_png(_fname, _img.width, _img.height, Ch ? Ch : _img.channels,
                                                 (uchar*)  _img.img.get(), 0);
                    if (E == JPG) stbi_write_jpg(_fname, _img.width, _img.height, Ch ? Ch : _img.channels,
                                                 (uchar*)  _img.img.get(), 0);
                    if (E == PNG) stbi_write_bmp(_fname, _img.width, _img.height, Ch ? Ch : _img.channels,
                                                 (uchar*)  _img.img.get());
                    if (E == PNG) stbi_write_tga(_fname, _img.width, _img.height, Ch ? Ch : _img.channels,
                                                 (uchar*)  _img.img.get());
                }
            }
        }

        template <typename T>
        T load(const char *_fname) {
            void *__imgptr = nullptr;
            int __w = 0, __h = 0, __ch = 0;

            if (std::is_floating_point<UtilCol::BaseType<typename T::ColorType>>::value)
                __imgptr = stbi_loadf(_fname, &__w, &__h, &__ch, UtilCol::Channels<T>::num);
            else
                __imgptr = stbi_load (_fname, &__w, &__h, &__ch, UtilCol::Channels<T>::num);

            return { (typename T::ColorType *) __imgptr, __w, __h, __ch };
        }

        const auto &loadf  = load<Image>;
        const auto &loadu  = load<Image256>;
        const auto &load1f = load<Image1f>;
        const auto &load1u = load<Image1u>;
        const auto &load2f = load<Image2f>;
        const auto &load2u = load<Image2u>;
        const auto &load3f = load<Image3f>;
        const auto &load3u = load<Image3u>;
        const auto &load4f = load<Image4f>;
        const auto &load4u = load<Image4u>;
    };
};
#endif