#ifndef SVKFW_IMAGE_H
#define SVKFW_IMAGE_H

// Always use this header when need STBI

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "STBI/stb_image.h"
#include "STBI/stb_image_write.h"

#include "main/color.h"

#include <vector>
#include <memory>


namespace Simple {
//  ========================================  \\
                Image base class
//  ========================================  \\

    // Declaration & alias

    namespace Img {
        template <typename ChanT, int Ch>
        struct ImageBase;

        template <class ColorT>
        using ImgBase = ImageBase<Color::Util::ChannelType<ColorT>, Color::Util::Channels<ColorT>::value>;


        // Definition

        template <typename T, int Ch>
        struct ImageBase {
            static_assert(Ch >= 0 && Ch < 5);
            using ColorType = Color::Util::ColorType<T, Ch>;
            using ChannelType = T;
            using ChannelOpsType = Color::Util::ChannelOpsType<T>;

            std::vector<ColorType> img;
            int width = 0, height = 0;
            int channels = Color::Util::Channels<ColorType>::value;

            ImageBase() {}
            ImageBase(const ImageBase<T, Ch> &_image) : img{_image.img}, width{_image.width}, height{_image.height}, channels{_image.channels} {}
            ImageBase(ImageBase<T, Ch> &&_image) : img{std::move(_image.img)}, width{_image.width}, height{_image.height}, channels{_image.channels} {}
            ImageBase(int _w, int _h, const std::vector<ColorType> &_imptr) : img{_imptr}, width{_w}, height{_h} {
                uint32_t __size_check = _w * _h;
                if (Ch < 2) {
                    channels = _imptr.size() / (_w * _h);
                    __size_check *= channels;
                }
                if (_imptr.size() != __size_check)
                    throw std::invalid_argument(SVKFW_WRAPERR("ImageBase Constructor", "color array size != width * height"));
            }
            ImageBase(int _w, int _h, ColorType _col = 0)
                            : img((size_t)_w * _h, _col), width{_w}, height{_h} {}
            template <int ChNum = Ch>
            ImageBase(int _w, int _h, int _c, typename std::enable_if<ChNum < 2, ChannelType>::type _col = 0)
                            : img((size_t)_w * _h * _c, _col), width{_w}, height{_h}, channels{_c} {}


            // Default number of channels for a type
            static int defChannels() { return Ch; }
            int  getChannels() const { return channels; }
            void setChannels(int _channels) {}

            template <int ChNum = Ch, class ColT = Color::Util::OtherChanType<T>>
            typename std::enable_if<ChNum != 0, ImageBase<ColT, ChNum>>::type convert() const {
                ImageBase<ColT, ChNum> __img{width, height};

                for (unsigned i = 0u; i < width * height; ++i)
                    __img.img[i] = Color::Func::convert<ColorType, Color::Util::ColorType<ColT, ChNum>>(img[i]);

                return __img;
            }
            template <int ChNum = Ch, class ColT = Color::Util::OtherChanType<T>>
            typename std::enable_if<ChNum == 0, ImageBase<ColT, ChNum>>::type convert() const {
                ImageBase<ColT, 0> __img{width, height, channels};

                for (unsigned i = 0u; i < width * height * channels; ++i)
                    __img.img[i] = Color::Func::convert<ColorType, Color::Util::ColorType<ColT, ChNum>>(img[i]);

                return __img;
            }
            template <class ChanT = ChannelType>
            ImageBase<ChanT, 1> convert(int _ch1) const {
                ImageBase<ChanT, 1> __img{width, height};

                const ChannelType *__imgdata = img.data();
                for (unsigned i = 0u; i < width * height; i += 1)
                    __img.img[i] = Color::Func::convert<ChannelType, ChanT>(__imgdata[i*std::max(channels, 1) + _ch1]);

                return __img;
            }
            template <class ChanT = ChannelType>
            ImageBase<ChanT, 2> convert(int _ch1, int _ch2) const {
                ImageBase<ChanT, 2> __img{width, height};

                const ChannelType *__imgdata = img.data();
                for (unsigned i = 0u; i < width * height; i += 1)
                    __img.img[i] = { Color::Func::convert<ChannelType, ChanT>(__imgdata[i*std::max(channels, 1) + _ch1]),
                                     Color::Func::convert<ChannelType, ChanT>(__imgdata[i*std::max(channels, 1) + _ch2]) };

                return __img;
            }
            template <class ChanT = ChannelType>
            ImageBase<ChanT, 3> convert(int _ch1, int _ch2, int _ch3) const {
                ImageBase<ChanT, 3> __img{width, height};

                const ChannelType *__imgdata = img.data();
                for (unsigned i = 0u; i < width * height; i += 1)
                    __img.img[i] = { Color::Func::convert<ChannelType, ChanT>(__imgdata[i*std::max(channels, 1) + _ch1]),
                                     Color::Func::convert<ChannelType, ChanT>(__imgdata[i*std::max(channels, 1) + _ch2]),
                                     Color::Func::convert<ChannelType, ChanT>(__imgdata[i*std::max(channels, 1) + _ch3]) };

                return __img;
            }
            template <class ChanT = ChannelType>
            ImageBase<ChanT, 4> convert(int _ch1, int _ch2, int _ch3, int _ch4) const {
                ImageBase<ChanT, 4> __img{width, height};

                const ChannelType *__imgdata = img.data();
                for (unsigned i = 0u; i < width * height; i += 1)
                    __img.img[i] = { Color::Func::convert<ChannelType, ChanT>(__imgdata[i*std::max(channels, 1) + _ch1]),
                                     Color::Func::convert<ChannelType, ChanT>(__imgdata[i*std::max(channels, 1) + _ch2]),
                                     Color::Func::convert<ChannelType, ChanT>(__imgdata[i*std::max(channels, 1) + _ch3]),
                                     Color::Func::convert<ChannelType, ChanT>(__imgdata[i*std::max(channels, 1) + _ch4]) };

                return __img;
            }
        }; // ImageBase END
    }; // Img END


//  ============  Image classes declaration  ============  \\

    // Note: ImageBase<ChannelType, ChannelNumber>
    //        OR ImgBase<ColorType> - for fixed-channel images only

    typedef Img::ImageBase<Color::Color1f,0> Image;
    typedef Img::ImageBase<Color::Color1u,0> Image8;
    typedef Img::ImgBase<Color::Color1f> Image1f;
    typedef Img::ImgBase<Color::Color1u> Image1u;
    typedef Img::ImgBase<Color::Color2f> Image2f;
    typedef Img::ImgBase<Color::Color2u> Image2u;
    typedef Img::ImgBase<Color::Color3f> Image3f;
    typedef Img::ImgBase<Color::Color3u> Image3u;
    typedef Img::ImgBase<Color::Color4f> Image4f;
    typedef Img::ImgBase<Color::Color4u> Image4u;


//  ============  Image-related functions  ============  \\

    namespace Img {
        enum FileType { PNG, JPG, BMP, TGA, HDR }; // file extensions

//  Image Save/Load
        template <FileType E, typename T, int Ch>
        void save(const char *_fname, const ImageBase<T, Ch> &_img) {
            const uint32_t w = _img.width, h = _img.height, c = _img.channels;

            if (E == HDR) {
                if (std::is_floating_point<Color::Util::ChannelType<T>>::value) {
                    stbi_write_hdr(_fname, w, h, c, (float*)  _img.img.data());
                }
                else {
                    auto __img{_img.convert()};
                    stbi_write_hdr(_fname, w, h, c, (float*) __img.img.data());
                }
            }
            else {
                if (std::is_floating_point<Color::Util::ChannelType<T>>::value) {
                    auto __img{_img.convert()};
                    if (E == PNG) stbi_write_png(_fname, w, h, Ch ? Ch : c, (Color::Color1u*) __img.img.data(), 0);
                    if (E == JPG) stbi_write_jpg(_fname, w, h, Ch ? Ch : c, (Color::Color1u*) __img.img.data(), 0);
                    if (E == BMP) stbi_write_bmp(_fname, w, h, Ch ? Ch : c, (Color::Color1u*) __img.img.data());
                    if (E == TGA) stbi_write_tga(_fname, w, h, Ch ? Ch : c, (Color::Color1u*) __img.img.data());
                }
                else {
                    if (E == PNG) stbi_write_png(_fname, w, h, Ch ? Ch : c, (Color::Color1u*)  _img.img.data(), 0);
                    if (E == JPG) stbi_write_jpg(_fname, w, h, Ch ? Ch : c, (Color::Color1u*)  _img.img.data(), 0);
                    if (E == BMP) stbi_write_bmp(_fname, w, h, Ch ? Ch : c, (Color::Color1u*)  _img.img.data());
                    if (E == TGA) stbi_write_tga(_fname, w, h, Ch ? Ch : c, (Color::Color1u*)  _img.img.data());
                }
            }
        }

        template <typename T>
        T load(const char *_fname) {
            void *__imgptr = nullptr;
            T __res_img;

            if (std::is_floating_point<typename T::ChannelType>::value)
                __imgptr = stbi_loadf(_fname, &__res_img.width, &__res_img.height, &__res_img.channels, T::defChannels());
            else
                __imgptr = stbi_load (_fname, &__res_img.width, &__res_img.height, &__res_img.channels, T::defChannels());

            uint32_t __elem_size = __res_img.width * __res_img.height;
            if (T::defChannels() == 0)
                __elem_size *= __res_img.channels;
            else if (__res_img.channels != T::defChannels()) {
                std::string __msg = SVKFW_WRAPWARN("Img :: load", "channel number mismatch: expected " + std::to_string(T::defChannels()) + ", got " + std::to_string(__res_img.channels));
                fprintf(svkfwwarn, "%s\n", __msg.c_str());
                __res_img.channels = T::defChannels();
            }

            __res_img.img.resize(__elem_size);
            for (uint32_t i = 0u; i < __elem_size; ++i)
                __res_img.img[i] = ((typename T::ColorType *)__imgptr)[i];

            return __res_img;
        }

        const auto &loadf  = load<Image>;
        const auto &loadu  = load<Image8>;
        const auto &load1f = load<Image1f>;
        const auto &load1u = load<Image1u>;
        const auto &load2f = load<Image2f>;
        const auto &load2u = load<Image2u>;
        const auto &load3f = load<Image3f>;
        const auto &load3u = load<Image3u>;
        const auto &load4f = load<Image4f>;
        const auto &load4u = load<Image4u>;
    }; // Img END
}; // Simple END

#endif