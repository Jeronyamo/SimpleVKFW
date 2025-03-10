#ifndef SOGLFW_IMAGE_H
#define SOGLFW_IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include "color.h"

#include <memory>


namespace Simple {
    //  ============  Image Utilities  ============  \\

    const UnHood::BoolFlag<false> SDR;
    const UnHood::BoolFlag< true> HDR, DCOPY;

    namespace ImFileType {
        enum Type { PNG, JPG, BMP, TGA, HDR };
    };


    // Image base type deduction
    template <class T> struct ImBaseType {
        using type = T;
    };
    template <class T> struct ImBaseType<Colors::ColorBase<T>> {
        using type = T;
    };
    template <class T> struct ImBaseType<Colors::ColorABase<T>> {
        using type = T;
    };

    // Image base type conversion, examples:
    //   uchar, uchar -> uchar
    //   uchar, float -> float
    //   Colors::ColorBase<uchar>, Colors::ColorABase<uchar> -> Colors::ColorABase<uchar>
    //   Colors::ColorBase<float>, Colors::ColorABase<uchar> -> Colors::ColorABase<float>
    template <class T1, class T2, bool fundT1, bool fundT2> struct ImCommonTypeBase {
        using type = T1;
    };
    // true, true
    template <class T1, class T2>
    struct ImCommonTypeBase<T1, T2, true, true> {
        using type = typename std::common_type<T1, T2>::type;
    };
    // false, true
    template <class T1, class T2>
    struct ImCommonTypeBase<Colors::ColorBase<T1>, T2, false, true> {
        using type = Colors::ColorBase<typename std::common_type<T1, T2>::type>;
    };
    template <class T1, class T2>
    struct ImCommonTypeBase<Colors::ColorABase<T1>, T2, false, true> {
        using type = Colors::ColorABase<typename std::common_type<T1, T2>::type>;
    };
    // true, false
    template <class T1, class T2>
    struct ImCommonTypeBase<T1, Colors::ColorBase<T2>, true, false> {
        using type = Colors::ColorBase<typename std::common_type<T1, T2>::type>;
    };
    template <class T1, class T2>
    struct ImCommonTypeBase<T1, Colors::ColorABase<T2>, true, false> {
        using type = Colors::ColorABase<typename std::common_type<T1, T2>::type>;
    };
    // <false, false>
    template <class T1, class T2>
    struct ImCommonTypeBase<Colors::ColorBase<T1>, Colors::ColorBase<T2>, false, false> {
        using type = Colors::ColorBase<typename std::common_type<T1, T2>::type>;
    };
    template <class T1, class T2>
    struct ImCommonTypeBase<Colors::ColorBase<T1>, Colors::ColorABase<T2>, false, false> {
        using type = Colors::ColorABase<typename std::common_type<T1, T2>::type>;
    };
    template <class T1, class T2>
    struct ImCommonTypeBase<Colors::ColorABase<T1>, Colors::ColorBase<T2>, false, false> {
        using type = Colors::ColorABase<typename std::common_type<T1, T2>::type>;
    };
    template <class T1, class T2>
    struct ImCommonTypeBase<Colors::ColorABase<T1>, Colors::ColorABase<T2>, false, false> {
        using type = Colors::ColorABase<typename std::common_type<T1, T2>::type>;
    };
    // Image common type deduction using ImCommonTypeBase (above)
    template <class T1, class T2> struct ImCommonType {
        using type = typename ImCommonTypeBase<T1, T2, std::is_fundamental<T1>::value, std::is_fundamental<T2>::value>::type;
    };


    //  ============  Image base class  ============  \\

    template <class T, int Ch>
    struct ImageBase {
        int width = 0, height = 0, channels = Ch;
        std::shared_ptr<T> image = nullptr;


        ImageBase() {}

        // SDR image loader
        ImageBase(const char* fname) {
            typename ImBaseType<T>::type *_image;
            std::unique_ptr<uchar, FreeDeleter> im{stbi_load(fname, &width, &height, &channels, channels)};
            _image = new typename ImBaseType<T>::type[width * height * channels];
            if (std::is_same<typename ImBaseType<T>::type, uchar>::value) {
                memcpy(_image, im.get(), width * height * channels * sizeof(uchar));
            }
            else {
                for (unsigned i = 0u; i < width * height * channels; ++i)
                    _image[i] = ColTransform<uchar, typename ImBaseType<T>::type>::coef * im.get()[i];
            }
            image.reset(_image);
        }

        // SDR/HDR image loader
        template <bool _flag>
        ImageBase(const char* fname, UnHood::BoolFlag<_flag>) {
            typename ImBaseType<T>::type *_image;
            if (_flag) {
                std::unique_ptr<float, FreeDeleter> im{stbi_loadf(fname, &width, &height, &channels, channels)};
                _image = new typename ImBaseType<T>::type[width * height * channels];
                if (std::is_same<typename ImBaseType<T>::type, float>::value) {
                    memcpy(_image, im.get(), width * height * channels * sizeof(float));
                }
                else {
                    for (unsigned i = 0u; i < width * height * channels; ++i)
                        _image[i] = ColTransform<float, typename ImBaseType<T>::type>::coef * im.get()[i];
                }
                image.reset(_image);
            }
            else {
                std::unique_ptr<uchar, FreeDeleter> im{stbi_load(fname, &width, &height, &channels, channels)};
                _image = new typename ImBaseType<T>::type[width * height * channels];
                if (std::is_same<typename ImBaseType<T>::type, uchar>::value) {
                    memcpy(_image, im.get(), width * height * channels * sizeof(uchar));
                }
                else {
                    for (unsigned i = 0u; i < width * height * channels; ++i)
                        _image[i] = ColTransform<uchar, typename ImBaseType<T>::type>::coef * im.get()[i];
                }
                image.reset(_image);
            }
        }

        // load a specific channel from an SDR image
        ImageBase(const char* fname, int _ch) {
            typename ImBaseType<T>::type *_image;
            std::unique_ptr<uchar, FreeDeleter> im = stbi_load(fname, &width, &height, &channels, 0);
            if (_ch < 0 || _ch >= channels) throw std::invalid_argument("Error: channel argument out of range [0, channels)");
            _image = new typename ImBaseType<T>::type[width * height];
            for (unsigned i = 0u; i < width * height; ++i)
                _image[i] = ColTransform<uchar, typename ImBaseType<T>::type>::coef * im.get()[channels * i + _ch];
            image.reset(_image);
            channels = 1;
        }

        // load a specific channel from an SDR/HDR image
        template <bool _flag>
        ImageBase(const char* fname, int _ch, UnHood::BoolFlag<_flag>) {
            typename ImBaseType<T>::type *_image;
            if (_flag) {
                std::unique_ptr<float, FreeDeleter> im = stbi_loadf(fname, &width, &height, &channels, 0);
                if (_ch < 0 || _ch >= channels) throw std::invalid_argument("Error: channel argument out of range [0, channels)");
                _image = new typename ImBaseType<T>::type[width * height];
                for (unsigned i = 0u; i < width * height; ++i)
                    _image[i] = ColTransform<float, typename ImBaseType<T>::type>::coef * im.get()[channels * i + _ch];
            }
            else {
                std::unique_ptr<uchar, FreeDeleter> im = stbi_load(fname, &width, &height, &channels, 0);
                if (_ch < 0 || _ch >= channels) throw std::invalid_argument("Error: channel argument out of range [0, channels)");
                _image = new typename ImBaseType<T>::type[width * height];
                for (unsigned i = 0u; i < width * height; ++i)
                    _image[i] = ColTransform<uchar, typename ImBaseType<T>::type>::coef * im.get()[channels * i + _ch];
            }
            image.reset(_image);
            channels = 1;
        }

        ImageBase(int _w, int _h, int _ch = Ch, T _val = 0)
                    : width{_w}, height{_h}, channels{Ch ? Ch : _ch}, image{new T[_w * _h * _ch]} {
            if (!channels) throw std::invalid_argument("Error: ImageBase<T, 0> constructor requires non-zero channel argument");
            for (unsigned i = 0u; i < width * height * channels; ++i)
                image.get()[i] = _val;
        }

        // (Shallow) Copy constructor
        ImageBase(const ImageBase<T, Ch> &_im)
                    : width{_im.width}, height{_im.height}, channels{_im.channels}, image{_im.image} {}
        // Deepcopy constructor
        ImageBase(const ImageBase<T, Ch> &_im, UnHood::BoolFlag<true>)
                    : width{_im.width}, height{_im.height}, channels{_im.channels},
                      image{new T[_im.width * _im.height * (Ch ? 1 : _im.channels)]} {
            memcpy(image.get(), _im.image.get(), width * height * (Ch ? 1 : _im.channels) * sizeof(T));
        }
        // transforms (like Colors::ColorBase<float> -> Colors::ColorABase<uchar>)
        template <typename T1, int Ch1>
        ImageBase(const ImageBase<T1, Ch1> &_im)
                    : width{_im.width}, height{_im.height}, channels{Ch ? Ch : _im.channels},
                      image{new T[_im.width * _im.height * (Ch ? 1 : _im.channels)]} {
            if (Ch == 1 && (Ch1 > 1 || Ch1 == 0 && _im.channels != 1)) throw std::invalid_argument("Error: select a channel to copy");
            T *ImPtr = image.get();
            T1 *_Ptr = _im.image.get();
            for (unsigned i = 0u; i < width * height; ++i)
                ImPtr[i] = typename ImBaseType<T>::type(((Ch == 1 || Ch == 0) && (Ch1 == 1 || Ch1 == 0) ?
                            ColTransform<typename ImBaseType<T1>::type, typename ImBaseType<T>::type>::coef : 1) * _Ptr[i]);
        }
        // channel copy
        template <typename T1, int Ch1>
        ImageBase(const ImageBase<T1, Ch1> &_im, int _ch)
                    : width{_im.width}, height{_im.height}, channels{Ch ? Ch : 1},
                      image{new T[_im.width * _im.height]} {
            if (_ch < 0 || _ch >= _im.channels) throw std::invalid_argument("Error: channel argument out of range [0, channels)");
            T *ImPtr = image.get();
            typename ImBaseType<T1>::type *_Ptr = _im.image.get();
            for (unsigned i = 0u; i < width * height; ++i)
                ImPtr[i] = typename ImBaseType<T>::type(ColTransform<typename ImBaseType<T1>::type, typename ImBaseType<T>::type>::coef *
                            _Ptr[i * (Ch1 ? Ch1 : _im.channels) + _ch]);
        }


        //  ============  Save image  ============  \\

        template <ImFileType::Type _t>
        void saveAs(const char *fname) {
            if (_t == ImFileType::HDR) {
                if (std::is_floating_point<typename ImBaseType<T>::type>::value) {
                    stbi_write_hdr(fname, width, height, channels, image.get());
                }
                else {
                    ImageBase<float, 0> tmp{*this}; // maybe create an array and transform image right here
                    stbi_write_hdr(fname, width, height, channels, tmp.image.get());
                }
            }
            else {
                if (std::is_floating_point<typename ImBaseType<T>::type>::value) {
                    ImageBase<uchar, 0> tmp{*this}; // maybe create an array and transform image right here
                    if (_t == ImFileType::PNG) stbi_write_png(fname, width, height, channels, tmp.image.get());
                    if (_t == ImFileType::JPG) stbi_write_jpg(fname, width, height, channels, tmp.image.get());
                    if (_t == ImFileType::BMP) stbi_write_bmp(fname, width, height, channels, tmp.image.get());
                    if (_t == ImFileType::TGA) stbi_write_tga(fname, width, height, channels, tmp.image.get());
                }
                else {
                    if (_t == ImFileType::PNG) stbi_write_png(fname, width, height, channels, image.get());
                    if (_t == ImFileType::JPG) stbi_write_jpg(fname, width, height, channels, image.get());
                    if (_t == ImFileType::BMP) stbi_write_bmp(fname, width, height, channels, image.get());
                    if (_t == ImFileType::TGA) stbi_write_tga(fname, width, height, channels, image.get());
                }
            }
        }
        // additional parameter for png|jpg images can be provided
        template <ImFileType::Type _t>
        void saveAs(const char *fname, typename std::enable_if<_t == ImFileType::PNG || _t == ImFileType::JPG, int>::type param) {
            //if (_t == ImFileType::PNG || _t == ImFileType::JPG) {
            if (std::is_floating_point<typename ImBaseType<T>::type>::value) {
                ImageBase<uchar, 0> tmp{*this}; // maybe create an array and transform image right here
                if (_t == ImFileType::PNG) stbi_write_png(fname, width, height, channels, tmp.image.get(), param);
                if (_t == ImFileType::JPG) stbi_write_jpg(fname, width, height, channels, tmp.image.get(), param);
            }
            else {
                if (_t == ImFileType::PNG) stbi_write_png(fname, width, height, channels, image.get(), param);
                if (_t == ImFileType::JPG) stbi_write_jpg(fname, width, height, channels, image.get(), param);
            }
            //}
        }


        //  ============  Color model conversion  ============  \\

        template <class T, int Ch>
        void convertColor(typename std::enable_if<Ch == 3 || Ch == 4, ImageBase<T, Ch>>::type &img, ColModel::Type colTypeFrom, ColModel::Type colTypeTo) {
            throw not_implemented_error("Error: not implemented!");
        }


        //  ============  Image channel manipulation  ============  \\

        template <typename T1 = typename ImBaseType<T>::type>
        ImageBase<T1, 1> getChannel(unsigned _chan) {
            if (_chan >= channels) throw std::invalid_argument("Error: channel argument outside of range");

            ImageBase<T1, 1> _tmp{width, height};
            typename ImBaseType<T>::type *ptrIm = image.get();
            T1 *ptrCh = _tmp.image.get();
            for (unsigned i = 0; i < width * height; ++i)
                ptrCh[i] = ColTransform<typename ImBaseType<T>::type, T1>::coef * ptrIm[i * channels + _chan];

            return _tmp;
        }

        template <typename T1, int Ch1>
        void setChannel(const ImageBase<T1, Ch1> &_im, unsigned chanTo, unsigned chanFrom = 0u) {
            if (chanTo > (Ch ? Ch : channels))
                throw std::invalid_argument("Error: argument 'chanTo' is bigger than the number of destination image channels");
            if (chanFrom > (Ch1 ? Ch1 : _im.channels))
                throw std::invalid_argument("Error: argument 'chanFrom' is bigger than the number of '_im' image channels");
            if (width != _im.width || height != _im.height) throw std::invalid_argument("Error: image sizes mismatch");

            typename ImBaseType<T >::type *destImgPtr = image.get();
            typename ImBaseType<T1>::type *_imPtr = _im.image.get();
            for (unsigned i = 0u; i < width * height; ++i)
                destImgPtr[i * (Ch ? Ch : channels) + chanTo] = ColTransform<typename ImBaseType<T>::type, typename ImBaseType<T1>::type>::coef *
                                                                _imPtr[i * (Ch1 ? Ch1 : _im.channels) + chanFrom];
        }


        //  ============  Indexing  ============  \\

              T& operator[](unsigned _ind)       { return image.get()[_ind]; }
        const T& operator[](unsigned _ind) const { return image.get()[_ind]; }

              T& operator[](UVec2 _ind)       { return image.get()[_ind.x * width + _ind.y]; }
        const T& operator[](UVec2 _ind) const { return image.get()[_ind.x * width + _ind.y]; }


        //  ============  Arithmetic operations  ============  \\

        template <class T1, int Ch1>
        typename std::enable_if<Ch && Ch1, ImageBase<typename ImCommonType<T, T1>::type, UnHood::MaxConstexpr<Ch, Ch1>::maxv>>::type
        operator+(const ImageBase<T1, Ch1> &_im) {
            int _w = min(width, _im.width), _h = min(height, _im.height);
            ImageBase<typename ImCommonType<T, T1>::type, UnHood::MaxConstexpr<Ch, Ch1>::maxv> tmp_img{_w, _h};

            for (unsigned i = 0u; i < _h; ++i)
                for (unsigned j = 0u; j < _w; ++j)
                    tmp_img[i * _w + j] = typename ImCommonType<T, T1>::type((*this)[i * width + j]) +
                                          typename ImCommonType<T, T1>::type(_im[i * _im.width + j]);
            return tmp_img;
        }

        template <class T1, int Ch1>
        typename std::enable_if<Ch && Ch1, ImageBase<typename ImCommonType<T, T1>::type, UnHood::MaxConstexpr<Ch, Ch1>::maxv>>::type
        operator-(const ImageBase<T1, Ch1> &_im) {
            int _w = min(width, _im.width), _h = min(height, _im.height);
            ImageBase<typename ImCommonType<T, T1>::type, UnHood::MaxConstexpr<Ch, Ch1>::maxv> tmp_img{_w, _h};

            for (unsigned i = 0u; i < _h; ++i)
                for (unsigned j = 0u; j < _w; ++j)
                    tmp_img[i * _w + j] = typename ImCommonType<T, T1>::type((*this)[i * width + j]) -
                                          typename ImCommonType<T, T1>::type(_im[i * _im.width + j]);
            return tmp_img;
        }

        template <class T1, int Ch1>
        typename std::enable_if<Ch && Ch1, ImageBase<typename ImCommonType<T, T1>::type, UnHood::MaxConstexpr<Ch, Ch1>::maxv>>::type
        operator*(const ImageBase<T1, Ch1> &_im) {
            int _w = min(width, _im.width), _h = min(height, _im.height);
            ImageBase<typename ImCommonType<T, T1>::type, UnHood::MaxConstexpr<Ch, Ch1>::maxv> tmp_img{_w, _h};

            for (unsigned i = 0u; i < _h; ++i)
                for (unsigned j = 0u; j < _w; ++j)
                    tmp_img[i * _w + j] = typename ImCommonType<T, T1>::type((*this)[i * width + j]) *
                                          typename ImCommonType<T, T1>::type(_im[i * _im.width + j]);
            return tmp_img;
        }

        template <class T1, int Ch1>
        typename std::enable_if<Ch && Ch1, ImageBase<typename ImCommonType<T, T1>::type, UnHood::MaxConstexpr<Ch, Ch1>::maxv>>::type
        operator/(const ImageBase<T1, Ch1> &_im) {
            int _w = min(width, _im.width), _h = min(height, _im.height);
            ImageBase<typename ImCommonType<T, T1>::type, UnHood::MaxConstexpr<Ch, Ch1>::maxv> tmp_img{_w, _h};

            for (unsigned i = 0u; i < _h; ++i)
                for (unsigned j = 0u; j < _w; ++j)
                    tmp_img[i * _w + j] = typename ImCommonType<T, T1>::type((*this)[i * width + j]) /
                                          typename ImCommonType<T, T1>::type(_im[i * _im.width + j]);
            return tmp_img;
        }
    };


    //  ============  Porter-Duff blends for images  ============  \\

    template <class T1, int Ch1, class T2, int Ch2>
    typename std::enable_if<(Ch1 == 3 || Ch1 == 4) && (Ch2 == 3 || Ch2 == 4) && (Ch1 == 4 || Ch2 == 4),
             ImageBase<typename ImCommonType<T1, T2>::type, UnHood::MaxConstexpr<Ch1, Ch2>::maxv>>::type
    ImPorterDuffBlendOver(const ImageBase<T1, Ch1> &_im1, const ImageBase<T2, Ch2> &_im2) {
        int _w = min(_im1.width, _im2.width), _h = min(_im1.height, _im2.height);
        ImageBase<typename ImCommonType<T1, T2>::type, UnHood::MaxConstexpr<Ch1, Ch2>::maxv> tmp_img{_w, _h};

        for (unsigned i = 0u; i < _h; ++i)
            for (unsigned j = 0u; j < _w; ++j)
                    tmp_img[i * _w + j] = PorterDuffBlendOver(
                                            typename ImCommonType<T1, T2>::type(_im1[i * _im1.width + j]),
                                            typename ImCommonType<T1, T2>::type(_im2[i * _im2.width + j]));
        return tmp_img;
    }
    template <class T1, int Ch1, class T2, int Ch2>
    typename std::enable_if<(Ch1 == 3 || Ch1 == 4) && (Ch2 == 3 || Ch2 == 4) && (Ch1 == 4 || Ch2 == 4),
             ImageBase<typename ImCommonType<T1, T2>::type, UnHood::MaxConstexpr<Ch1, Ch2>::maxv>>::type
    ImPorterDuffBlendIn  (const ImageBase<T1, Ch1> &_im1, const ImageBase<T2, Ch2> &_im2) {
        int _w = min(_im1.width, _im2.width), _h = min(_im1.height, _im2.height);
        ImageBase<typename ImCommonType<T1, T2>::type, UnHood::MaxConstexpr<Ch1, Ch2>::maxv> tmp_img{_w, _h};

        for (unsigned i = 0u; i < _h; ++i)
            for (unsigned j = 0u; j < _w; ++j)
                    tmp_img[i * _w + j] = PorterDuffBlendIn(
                                            typename ImCommonType<T1, T2>::type(_im1[i * _im1.width + j]),
                                            typename ImCommonType<T1, T2>::type(_im2[i * _im2.width + j]));
        return tmp_img;
    }
    template <class T1, int Ch1, class T2, int Ch2>
    typename std::enable_if<(Ch1 == 3 || Ch1 == 4) && (Ch2 == 3 || Ch2 == 4) && (Ch1 == 4 || Ch2 == 4),
             ImageBase<typename ImCommonType<T1, T2>::type, UnHood::MaxConstexpr<Ch1, Ch2>::maxv>>::type
    ImPorterDuffBlendOut (const ImageBase<T1, Ch1> &_im1, const ImageBase<T2, Ch2> &_im2) {
        int _w = min(_im1.width, _im2.width), _h = min(_im1.height, _im2.height);
        ImageBase<typename ImCommonType<T1, T2>::type, UnHood::MaxConstexpr<Ch1, Ch2>::maxv> tmp_img{_w, _h};

        for (unsigned i = 0u; i < _h; ++i)
            for (unsigned j = 0u; j < _w; ++j)
                    tmp_img[i * _w + j] = PorterDuffBlendOut(
                                            typename ImCommonType<T1, T2>::type(_im1[i * _im1.width + j]),
                                            typename ImCommonType<T1, T2>::type(_im2[i * _im2.width + j]));
        return tmp_img;
    }
    template <class T1, int Ch1, class T2, int Ch2>
    typename std::enable_if<(Ch1 == 3 || Ch1 == 4) && (Ch2 == 3 || Ch2 == 4) && (Ch1 == 4 || Ch2 == 4),
             ImageBase<typename ImCommonType<T1, T2>::type, UnHood::MaxConstexpr<Ch1, Ch2>::maxv>>::type
    ImPorterDuffBlendAtop(const ImageBase<T1, Ch1> &_im1, const ImageBase<T2, Ch2> &_im2) {
        int _w = min(_im1.width, _im2.width), _h = min(_im1.height, _im2.height);
        ImageBase<typename ImCommonType<T1, T2>::type, UnHood::MaxConstexpr<Ch1, Ch2>::maxv> tmp_img{_w, _h};

        for (unsigned i = 0u; i < _h; ++i)
            for (unsigned j = 0u; j < _w; ++j)
                    tmp_img[i * _w + j] = PorterDuffBlendAtop(
                                            typename ImCommonType<T1, T2>::type(_im1[i * _im1.width + j]),
                                            typename ImCommonType<T1, T2>::type(_im2[i * _im2.width + j]));
        return tmp_img;
    }
    template <class T1, int Ch1, class T2, int Ch2>
    typename std::enable_if<(Ch1 == 3 || Ch1 == 4) && (Ch2 == 3 || Ch2 == 4) && (Ch1 == 4 || Ch2 == 4),
             ImageBase<typename ImCommonType<T1, T2>::type, UnHood::MaxConstexpr<Ch1, Ch2>::maxv>>::type
    ImPorterDuffBlendXor (const ImageBase<T1, Ch1> &_im1, const ImageBase<T2, Ch2> &_im2) {
        int _w = min(_im1.width, _im2.width), _h = min(_im1.height, _im2.height);
        ImageBase<typename ImCommonType<T1, T2>::type, UnHood::MaxConstexpr<Ch1, Ch2>::maxv> tmp_img{_w, _h};

        for (unsigned i = 0u; i < _h; ++i)
            for (unsigned j = 0u; j < _w; ++j)
                    tmp_img[i * _w + j] = PorterDuffBlendXor(
                                            typename ImCommonType<T1, T2>::type(_im1[i * _im1.width + j]),
                                            typename ImCommonType<T1, T2>::type(_im2[i * _im2.width + j]));
        return tmp_img;
    }


    //  ============  Image types declaration  ============  \\

    typedef ImageBase<float, 0> Image;
    typedef ImageBase<uchar, 0> ImageUC;
    typedef ImageBase<float, 1> Image1;
    typedef ImageBase<uchar, 1> ImageUC1;
    typedef ImageBase<Colors::ColorBase <float>, 3> Image3;
    typedef ImageBase<Colors::ColorBase <uchar>, 3> ImageUC3;
    typedef ImageBase<Colors::ColorABase<float>, 4> Image4;
    typedef ImageBase<Colors::ColorABase<uchar>, 4> ImageUC4;
};

#endif