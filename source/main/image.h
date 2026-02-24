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

    namespace Img {
        enum FileType { PNG, JPG, BMP, TGA, HDR };

        enum ImgSampleType {
            SAMPLE_TYPE_BILINEAR,
        };

        enum ImgExtensionType {
            EXT_TYPE_REPEAT,
            EXT_TYPE_REPEAT_MIRRORED,
            EXT_TYPE_CLAMP,
        };

        struct Sampler {
            ImgSampleType sample_type = SAMPLE_TYPE_BILINEAR;
            ImgExtensionType ext_type = EXT_TYPE_REPEAT;

            vec4f validateUV(vec2f _uv) const {
                vec2f __offset_sign{1.f};
                int rep_x, rep_y;
                if ((_uv >= 0.f).all() && (_uv < 1.f).all()) return { _uv, __offset_sign };

                switch (ext_type) {
                    case EXT_TYPE_REPEAT:
                    case EXT_TYPE_REPEAT_MIRRORED:
                        rep_x = std::modf(_uv.x, &_uv.x);
                        rep_y = std::modf(_uv.y, &_uv.y);
                        _uv.x = (rep_x & ext_type) ? __offset_sign.x=-1.f, 1.f - _uv.x : _uv.x;
                        _uv.y = (rep_y & ext_type) ? __offset_sign.y=-1.f, 1.f - _uv.y : _uv.y;
                        break;
                    case EXT_TYPE_CLAMP:
                        _uv = Math::clampCL(_uv, {0.f}, {1.f});
                        break;
                }
                return { _uv, __offset_sign };
            }
        } def_sampler; // Sampler END


        template <typename T, int Ch>
        struct Image {
            static_assert(Ch > 0 && Ch < 5);
            using ColorType = Color::Util::ColorConstructor<T, Ch>;
            using ChannelType = Color::Util::ColorChanType<T>;
            using ChannelOpsType = Color::Util::ColorChanOpsType<T>;

            std::vector<ColorType> img;
            int width = 0, height = 0;
            static constexpr int channels = Ch;

            Image() {}
            Image(const Image<T, Ch>  &_image) : img{_image.img},            width{_image.width}, height{_image.height} {}
            Image(      Image<T, Ch> &&_image) : img{std::move(_image.img)}, width{_image.width}, height{_image.height} { _image.width = 0; _image.height = 0; }
            Image(const std::string   &_fname) { imgLoad(_fname); }
            Image(int _w, int _h, ColorType _col = 0)     : img((size_t)_w * _h, _col), width{_w}, height{_h} {}
            Image(int _w, int _h, const std::vector<ColorType> &_imptr)  : img{_imptr}, width{_w}, height{_h} {
                SVKFW_ASSERT(_imptr.size() == _w * _h, std::invalid_argument,
                                "Img :: Image Constructor", "Color array size != width * height: " + std::to_string(_imptr.size()) + "!=" + std::to_string(_w * _h));
            }
            ~Image() {
                img.clear();
                width  = 0;
                height = 0;
            }

            Image<T, Ch>& operator=(const Image<T, Ch> &_img) {
                img = _img.img;
                width  = _img.width;
                height = _img.height;
                return *this;
            }

            Image<T, Ch>& operator=(Image<T, Ch> &&_img) {
                img = std::move(_img.img);
                width  = _img.width;
                height = _img.height;
                _img.width  = 0;
                _img.height = 0;
                return *this;
            }


        // Color-related methods

            ColorType& operator[](const vec2u &_ind2d) {
                return img[_ind2d.y * width + _ind2d.x];
            }

            ColorType  operator[](const vec2u &_ind2d) const {
                return img[_ind2d.y * width + _ind2d.x];
            }


            void colSet(const ColorType &_col) {
                std::fill(img.begin(), img.end(), _col);
            }


        //  Color compression

            void colCompressClamp() {
                for (auto& pixel : img)
                    pixel = Math::clampCL(pixel, {0}, {Color::Util::ChanMax<ChannelType>::val});
            }


        // Image transforms

            void imgMirror(bool _mirror_x, bool _mirror_y) {
                if (_mirror_x) {
                    #pragma omp parallel for
                    for (int j = 0u; j < height; ++j)
                        for (int i = 0u; i < (width>>1); ++i)
                            std::swap(img[j*width+i], img[j*width+( width-1-i)]);
                }
                if (_mirror_y) {
                    #pragma omp parallel for
                    for (int j = 0u; j < (height>>1); ++j)
                        for (int i = 0u; i < width; ++i)
                            std::swap(img[j*width+i], img[(height-1-j)*width+i]);
                }
            }

            void imgResize(int _w, int _h) {
                width  = _w;
                height = _h;
                img.resize(width*height);
            }

            Image<T, Ch> imgResample(int _new_w, int _new_h, const Sampler &_sampler = def_sampler) {
                Image<T, Ch> __res{_new_w, _new_h};

                #pragma omp parallel for collapse(2)
                for (int i = 0; i < _new_w; ++i)
                    for (int j = 0; j < _new_h; ++j)
                        __res.img[j*_new_w+i] = sampleUV({float(i)/_new_w, float(j)/_new_h}, _sampler);
                return __res;
            }

            // row-first order  <->  column-first order
            void imgTranspose() {
                std::vector<ColorType> __res((size_t)width*height);

                for (int i = 0u; i < width; ++i)
                    for (int j = 0u; j < height; ++j)
                        __res[i*height+j] = img[j*width+i];

                img = std::move(__res);
                std::swap(width, height);
            }


        // Image conversions

            Image<Color::Util::OtherChanType<ChannelType>, Ch> imgSwitchType() const {
                Image<Color::Util::OtherChanType<ChannelType>, Ch> __res{width, height};
                for (uint32_t pixel_id = 0u; pixel_id < img.size(); ++pixel_id)
                    __res.img[pixel_id] = Color::Func::convert<Color::Util::OtherChanType<ChannelType>>(img[pixel_id]);
                return __res;
            }

            Image<ChannelType, 1> imgGetChannels(int _ch1) const {
                Image<ChannelType, 1> __res{width, height};
                for (uint32_t pixel_id = 0u; pixel_id < img.size(); ++pixel_id)
                    __res.img[pixel_id] = Color::Func::getChannel(img[pixel_id], _ch1);
                return __res;
            }

            Image<ChannelType, 2> imgGetChannels(int _ch1, int _ch2) const {
                Image<ChannelType, 2> __res{width, height};
                for (uint32_t pixel_id = 0u; pixel_id < img.size(); ++pixel_id) {
                    __res.img[pixel_id].x = Color::Func::getChannel(img[pixel_id], _ch1);
                    __res.img[pixel_id].y = Color::Func::getChannel(img[pixel_id], _ch2);
                }
                return __res;
            }

            Image<ChannelType, 3> imgGetChannels(int _ch1, int _ch2, int _ch3) const {
                Image<ChannelType, 3> __res{width, height};
                for (uint32_t pixel_id = 0u; pixel_id < img.size(); ++pixel_id) {
                    __res.img[pixel_id].x = Color::Func::getChannel(img[pixel_id], _ch1);
                    __res.img[pixel_id].y = Color::Func::getChannel(img[pixel_id], _ch2);
                    __res.img[pixel_id].z = Color::Func::getChannel(img[pixel_id], _ch3);
                }
                return __res;
            }

            Image<ChannelType, 4> imgGetChannels(int _ch1, int _ch2, int _ch3, int _ch4) const {
                Image<ChannelType, 4> __res{width, height};
                for (uint32_t pixel_id = 0u; pixel_id < img.size(); ++pixel_id) {
                    __res.img[pixel_id].x = Color::Func::getChannel(img[pixel_id], _ch1);
                    __res.img[pixel_id].y = Color::Func::getChannel(img[pixel_id], _ch2);
                    __res.img[pixel_id].z = Color::Func::getChannel(img[pixel_id], _ch3);
                    __res.img[pixel_id].w = Color::Func::getChannel(img[pixel_id], _ch4);
                }
                return __res;
            }

            template <typename ResChanT>
            Image<Color::Util::ColorChanType<ResChanT>, 1> imgCastChannels(int _ch1) const {
                Image<ChannelType, 1> __res{width, height};
                for (uint32_t pixel_id = 0u; pixel_id < img.size(); ++pixel_id)
                    __res.img[pixel_id]   = Color::Func::convert<Color::Util::ColorChanType<ResChanT>>(Color::Func::getChannel(img[pixel_id], _ch1));
                return __res;
            }

            template <typename ResChanT>
            Image<Color::Util::ColorChanType<ResChanT>, 2> imgCastChannels(int _ch1, int _ch2) const {
                Image<Color::Util::ColorChanType<ResChanT>, 2> __res{width, height};
                for (uint32_t pixel_id = 0u; pixel_id < img.size(); ++pixel_id) {
                    __res.img[pixel_id].x = Color::Func::convert<Color::Util::ColorChanType<ResChanT>>(Color::Func::getChannel(img[pixel_id], _ch1));
                    __res.img[pixel_id].y = Color::Func::convert<Color::Util::ColorChanType<ResChanT>>(Color::Func::getChannel(img[pixel_id], _ch2));
                }
                return __res;
            }

            template <typename ResChanT>
            Image<Color::Util::ColorChanType<ResChanT>, 3> imgCastChannels(int _ch1, int _ch2, int _ch3) const {
                Image<Color::Util::ColorChanType<ResChanT>, 3> __res{width, height};
                for (uint32_t pixel_id = 0u; pixel_id < img.size(); ++pixel_id) {
                    __res.img[pixel_id].x = Color::Func::convert<Color::Util::ColorChanType<ResChanT>>(Color::Func::getChannel(img[pixel_id], _ch1));
                    __res.img[pixel_id].y = Color::Func::convert<Color::Util::ColorChanType<ResChanT>>(Color::Func::getChannel(img[pixel_id], _ch2));
                    __res.img[pixel_id].z = Color::Func::convert<Color::Util::ColorChanType<ResChanT>>(Color::Func::getChannel(img[pixel_id], _ch3));
                }
                return __res;
            }

            template <typename ResChanT>
            Image<Color::Util::ColorChanType<ResChanT>, 4> imgCastChannels(int _ch1, int _ch2, int _ch3, int _ch4) const {
                Image<Color::Util::ColorChanType<ResChanT>, 4> __res{width, height};
                for (uint32_t pixel_id = 0u; pixel_id < img.size(); ++pixel_id) {
                    __res.img[pixel_id].x = Color::Func::convert<Color::Util::ColorChanType<ResChanT>>(Color::Func::getChannel(img[pixel_id], _ch1));
                    __res.img[pixel_id].y = Color::Func::convert<Color::Util::ColorChanType<ResChanT>>(Color::Func::getChannel(img[pixel_id], _ch2));
                    __res.img[pixel_id].z = Color::Func::convert<Color::Util::ColorChanType<ResChanT>>(Color::Func::getChannel(img[pixel_id], _ch3));
                    __res.img[pixel_id].w = Color::Func::convert<Color::Util::ColorChanType<ResChanT>>(Color::Func::getChannel(img[pixel_id], _ch4));
                }
                return __res;
            }


        // Image sampling

            ColorType sampleBilinear(vec2f _coords, const Sampler &_sampler = def_sampler) const {
                SVKFW_ASSERT(width > 0 && height > 0 && !img.empty(), std::runtime_error,
                                "Img :: Image :: sampleBilinear", "Bad image size: [" + std::to_string(width) + "x" + std::to_string(height) + "], or array is empty: " + std::to_string(img.size()));
                // pixel offs: x0, y0, x1, y1 -> 0,0; 0,1; 1,0; 1,1
                const vec2f __wh{float(width), float(height)};
                vec4f __pixel_offs = _sampler.validateUV( _coords                     / __wh);
                __pixel_offs["zw"] = _sampler.validateUV((_coords+__pixel_offs["zw"]) / __wh)["xy"];
                __pixel_offs["xy"] *= __wh;
                __pixel_offs["zw"] *= __wh;

                vec4u __pixel_ids = __pixel_offs.cast<uint32_t>();
                __pixel_offs["xy"] -= __pixel_ids["xy"];

                return (ColorType) img[__pixel_ids.y * width + __pixel_ids.x] * (1.f - __pixel_offs.y) * (1.f - __pixel_offs.x) +
                                   img[__pixel_ids.y * width + __pixel_ids.z] * (1.f - __pixel_offs.y) * (      __pixel_offs.x) +
                                   img[__pixel_ids.w * width + __pixel_ids.x] * (      __pixel_offs.y) * (1.f - __pixel_offs.x) +
                                   img[__pixel_ids.w * width + __pixel_ids.z] * (      __pixel_offs.y) * (      __pixel_offs.x);
            }

            ColorType sample(vec2f _coords, const Sampler &_sampler) const {
                switch (_sampler.sample_type) {
                    case SAMPLE_TYPE_BILINEAR: return sampleBilinear(_coords, _sampler);
                }
                return { 0 };
            }

            ColorType sampleUV(vec2f _uv, const Sampler &_sampler) const {
                return sample(_uv * vec2f{float(width), float(height)}, _sampler);
            }


        //  Image save/load

            template <FileType E>
            void imgSave(const std::string &_fname) const {
                if (E == HDR) {
                    if (std::is_floating_point<Color::Util::ColorChanType<T>>::value) {
                        stbi_write_hdr(_fname.c_str(), width, height, channels, (float*)  img.data());
                    }
                    else {
                        auto __img{imgSwitchType()};
                        stbi_write_hdr(_fname.c_str(), width, height, channels, (float*) __img.img.data());
                    }
                }
                else {
                    if (std::is_floating_point<Color::Util::ColorChanType<T>>::value) {
                        auto __img{imgSwitchType()};
                        if (E == PNG) stbi_write_png(_fname.c_str(), width, height, channels, (Color::Color1u*) __img.img.data(), 0);
                        if (E == JPG) stbi_write_jpg(_fname.c_str(), width, height, channels, (Color::Color1u*) __img.img.data(), 0);
                        if (E == BMP) stbi_write_bmp(_fname.c_str(), width, height, channels, (Color::Color1u*) __img.img.data());
                        if (E == TGA) stbi_write_tga(_fname.c_str(), width, height, channels, (Color::Color1u*) __img.img.data());
                    }
                    else {
                        if (E == PNG) stbi_write_png(_fname.c_str(), width, height, channels, (Color::Color1u*)  img.data(), 0);
                        if (E == JPG) stbi_write_jpg(_fname.c_str(), width, height, channels, (Color::Color1u*)  img.data(), 0);
                        if (E == BMP) stbi_write_bmp(_fname.c_str(), width, height, channels, (Color::Color1u*)  img.data());
                        if (E == TGA) stbi_write_tga(_fname.c_str(), width, height, channels, (Color::Color1u*)  img.data());
                    }
                }
            }

            void imgLoad(const std::string &_fname) {
                void *__imgptr = nullptr;
                int __channels = channels;

                if (std::is_floating_point<ChannelType>::value)
                    __imgptr = stbi_loadf(_fname.c_str(), &width, &height, &__channels, channels);
                else
                    __imgptr = stbi_load (_fname.c_str(), &width, &height, &__channels, channels);

                SVKFW_ASSERT(__imgptr != nullptr, std::invalid_argument, "Img :: Image :: load", "File '" + _fname + "' doesn't exist");

                SVKFW_WASSERT(__channels == channels,
                                "Img :: Image :: load", "Channel number mismatch: expected " + std::to_string(channels) + ", got " + std::to_string(__channels) + ".\n>> File: '" + _fname + "'\n");

                img.resize(width * height);
                for (uint32_t i = 0u; i < width * height; ++i)
                    img[i] = ((ColorType *)__imgptr)[i];
            }
        }; // Image END


//  ============  Image classes declaration  ============  \\

        typedef Img::Image<Color::Color1f, 1> Image1f;
        typedef Img::Image<Color::Color1u, 1> Image1u;
        typedef Img::Image<Color::Color1f, 2> Image2f;
        typedef Img::Image<Color::Color1u, 2> Image2u;
        typedef Img::Image<Color::Color1f, 3> Image3f;
        typedef Img::Image<Color::Color1u, 3> Image3u;
        typedef Img::Image<Color::Color1f, 4> Image4f;
        typedef Img::Image<Color::Color1u, 4> Image4u;


//  ============  Image-related functions  ============  \\

// Combining channels

    // Combine: 2

        template<typename T, int Ch1, int Ch2>
        static typename std::enable_if< Ch1+Ch2 <= 4, Image<T, Ch1+Ch2> >::type
                CombineChannels(const Image<T, Ch1> &_img1, const Image<T, Ch2> &_img2) {
            Image<T, Ch1+Ch2> __res;
            SVKFW_ASSERT(_img1.width == _img2.width && _img1.height == _img2.height, std::invalid_argument,
                            "Img :: Image :: CombineChannels (1)", "Input images size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img2.width) + 'x' + std::to_string(_img2.height) + "]\n");
            __res.imgResize(_img1.width, _img1.height);

            #pragma omp parallel for collapse(2)
            for (int j = 0; j < _img1.height; ++j)
                for (int i = 0; i < _img1.width; ++i)
                    __res.img[j*_img1.width + i] = { _img1.img[j*_img1.width + i], _img2.img[j*_img1.width + i] };
            return __res;
        }

    // Combine: 3

        template<typename T, int Ch1, int Ch2, int Ch3>
        static typename std::enable_if< Ch1+Ch2+Ch3 <= 4, Image<T, Ch1+Ch2+Ch3> >::type
                CombineChannels(const Image<T, Ch1> &_img1, const Image<T, Ch2> &_img2, const Image<T, Ch3> &_img3) {
            Image<T, Ch1+Ch2+Ch3> __res;
            SVKFW_ASSERT(_img1.width == _img2.width && _img1.height == _img2.height, std::invalid_argument,
                            "Img :: Image :: CombineChannels (2)", "Input images 0 and 1 size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img2.width) + 'x' + std::to_string(_img2.height) + "]\n");
            SVKFW_ASSERT(_img1.width == _img3.width && _img1.height == _img3.height, std::invalid_argument,
                            "Img :: Image :: CombineChannels (2)", "Input images 0 and 2 size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img3.width) + 'x' + std::to_string(_img3.height) + "]\n");
            __res.imgResize(_img1.width, _img1.height);

            #pragma omp parallel for collapse(2)
            for (int j = 0; j < _img1.height; ++j)
                for (int i = 0; i < _img1.width; ++i)
                    __res.img[j*_img1.width + i] = { _img1.img[j*_img1.width + i], _img2.img[j*_img1.width + i], _img3.img[j*_img1.width + i] };
            return __res;
        }

    // Combine: 4

        template<typename T>
        static Image<T, 4>
                CombineChannels(const Image<T, 1> &_img1, const Image<T, 1> &_img2, const Image<T, 1> &_img3, const Image<T, 1> &_img4) {
            Image<T, 4> __res;
            SVKFW_ASSERT(_img1.width == _img2.width && _img1.height == _img2.height, std::invalid_argument,
                            "Img :: Image :: CombineChannels (3)", "Input images 0 and 1 size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img2.width) + 'x' + std::to_string(_img2.height) + "]\n");
            SVKFW_ASSERT(_img1.width == _img3.width && _img1.height == _img3.height, std::invalid_argument,
                            "Img :: Image :: CombineChannels (3)", "Input images 0 and 2 size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img3.width) + 'x' + std::to_string(_img3.height) + "]\n");
            SVKFW_ASSERT(_img1.width == _img4.width && _img1.height == _img4.height, std::invalid_argument,
                            "Img :: Image :: CombineChannels (3)", "Input images 0 and 2 size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img4.width) + 'x' + std::to_string(_img4.height) + "]\n");
            __res.imgResize(_img1.width, _img1.height);

            #pragma omp parallel for collapse(2)
            for (int j = 0; j < _img1.height; ++j)
                for (int i = 0; i < _img1.width; ++i)
                    __res.img[j*_img1.width + i] = { _img1.img[j*_img1.width + i], _img2.img[j*_img1.width + i], _img3.img[j*_img1.width + i] };
            return __res;
        }

//  Image blending

        template <typename T, int Ch1, int Ch2>
        Image<T, 4> blendPDOver(const Image<T, Ch1> &_img1, const Image<T, Ch2> &_img2) {
            Image<T, 4> __res;
            SVKFW_ASSERT(_img1.width == _img2.width && _img1.height == _img2.height, std::invalid_argument,
                            "Img :: blendPDOver", "Input images size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img2.width) + 'x' + std::to_string(_img2.height) + "]\n");

            __res.resize(_img1.width, _img1.height);
            for (int j = 0; j < _img1.height; ++j)
                for (int i = 0; i < _img1.width; ++i)
                    __res.img[j*_img1.width + i] = Color::PorterDuffBlendOver(_img1.img[j*_img1.width + i], _img2.img[j*_img1.width + i]);
            return __res;
        }

        template <typename T, int Ch1, int Ch2>
        Image<T, 4> blendPDIn(const Image<T, Ch1> &_img1, const Image<T, Ch2> &_img2) {
            Image<T, 4> __res;
            SVKFW_ASSERT(_img1.width == _img2.width && _img1.height == _img2.height, std::invalid_argument,
                            "Img :: blendPDIn", "Input images size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img2.width) + 'x' + std::to_string(_img2.height) + "]\n");

            __res.resize(_img1.width, _img1.height);
            for (int j = 0; j < _img1.height; ++j)
                for (int i = 0; i < _img1.width; ++i)
                    __res.img[j*_img1.width + i] = Color::PorterDuffBlendIn(_img1.img[j*_img1.width + i], _img2.img[j*_img1.width + i]);
            return __res;
        }

        template <typename T, int Ch1, int Ch2>
        Image<T, 4> blendPDOut(const Image<T, Ch1> &_img1, const Image<T, Ch2> &_img2) {
            Image<T, 4> __res;
            SVKFW_ASSERT(_img1.width == _img2.width && _img1.height == _img2.height, std::invalid_argument,
                            "Img :: blendPDOut", "Input images size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img2.width) + 'x' + std::to_string(_img2.height) + "]\n");

            __res.resize(_img1.width, _img1.height);
            for (int j = 0; j < _img1.height; ++j)
                for (int i = 0; i < _img1.width; ++i)
                    __res.img[j*_img1.width + i] = Color::PorterDuffBlendOut(_img1.img[j*_img1.width + i], _img2.img[j*_img1.width + i]);
            return __res;
        }

        template <typename T, int Ch1, int Ch2>
        Image<T, 4> blendPDAtop(const Image<T, Ch1> &_img1, const Image<T, Ch2> &_img2) {
            Image<T, 4> __res;
            SVKFW_ASSERT(_img1.width == _img2.width && _img1.height == _img2.height, std::invalid_argument,
                            "Img :: blendPDAtop", "Input images size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img2.width) + 'x' + std::to_string(_img2.height) + "]\n");

            __res.resize(_img1.width, _img1.height);
            for (int j = 0; j < _img1.height; ++j)
                for (int i = 0; i < _img1.width; ++i)
                    __res.img[j*_img1.width + i] = Color::PorterDuffBlendAtop(_img1.img[j*_img1.width + i], _img2.img[j*_img1.width + i]);
            return __res;
        }

        template <typename T, int Ch1, int Ch2>
        Image<T, 4> blendPDXor(const Image<T, Ch1> &_img1, const Image<T, Ch2> &_img2) {
            Image<T, 4> __res;
            SVKFW_ASSERT(_img1.width == _img2.width && _img1.height == _img2.height, std::invalid_argument,
                            "Img :: blendPDXor", "Input images size mismatch: [" + std::to_string(_img1.width) + 'x' + std::to_string(_img1.height) + "] and [" + std::to_string(_img2.width) + 'x' + std::to_string(_img2.height) + "]\n");

            __res.resize(_img1.width, _img1.height);
            for (int j = 0; j < _img1.height; ++j)
                for (int i = 0; i < _img1.width; ++i)
                    __res.img[j*_img1.width + i] = Color::PorterDuffBlendXor(_img1.img[j*_img1.width + i], _img2.img[j*_img1.width + i]);
            return __res;
        }
    }; // Img END
}; // Simple END

#endif