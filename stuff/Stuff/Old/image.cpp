#include "image.h"

#include <type_traits>


template <class C> inline bool notCloseToZero(C  _val) { return _val <= -SOGLFW_EPS_FLT || _val >= SOGLFW_EPS_FLT; }

template <typename T>
Simple::ImageBase<T>::ImageBase(const char *_filename) {
    static_assert(std::is_floating_point<T>::value, "");
    unsigned char *_image = stbi_load(_filename, &width, &height, &channels, 0);
    if (!_image) throw "Error: could not load image";

    for (unsigned i = 0u; i < width * height * channels; ++i)
        image[i] = T(_image[i]) * IMAGE_UINT_TO_FLT;

    stbi_image_free(_image);
    filename = _filename;
}

template <typename T>
Simple::ImageBase<T>::ImageBase(const char *_filename) {
    static_assert(std::is_integral<T>::value, "");
    unsigned char *_image = stbi_load(_filename, &width, &height, &channels, 0);
    if (!_image) throw "Error: could not load image";

    for (unsigned i = 0u; i < width * height * channels; ++i)
        image[i] = _image[i];

    stbi_image_free(_image);
    filename = _filename;
}

template <class T>
Simple::ImageBase<T>::ImageBase(const ImageBase<T> &img, bool deepcopy)
                   : width(img.width), height(img.height), channels(img.channels) {
    if (image != nullptr) delete[] image;
    image = img.image;

    if (deepcopy) {
        image = new T[width * height * channels];
        memcpy(image, img.image, width * height * channels * sizeof(T));
        //for (unsigned i = 0u; i < width * height * channels; ++i)
        //    image[i] = img.image[i];
    }
}

template <class T>
template <class Y>
Simple::ImageBase<T>::ImageBase(const ImageBase<Y> &img)
                    : width(img.width), height(img.height), channels(img.channels) {
    if (image != nullptr) delete[] image;
    image = img.image;

    image = new T[width * height * channels];
    for (unsigned i = 0u; i < width * height * channels; ++i)
        image[i] = img.image[i];
}


void Simple::Image256::save() {
    stbi_write_png(filename.c_str(), width, height, channels, image, 0);
}

void Simple::Image256::saveAs(const char *_filename) {
    stbi_write_png(_filename, width, height, channels, image, 0);
}


//  ============   blends  ============  //

template <class T, class Y> Image ImageADD(const ImageBase <T> & img1, const ImageBase <Y> & img2) {
    Image img(img1, true);

    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                int res = int(img1.image[i * row1 + j * col1 + k]) + int(img2.image[i * row2 + j * col2 + k]);
                img.image[i * row1 + j * col1 + k] = res < 256 ? res : 255;
            }

    return img;
}

Image256 ImageSUB(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                int res = int(img1.image[i * row1 + j * col1 + k]) - int(img2.image[i * row2 + j * col2 + k]);
                img.image[i * row1 + j * col1 + k] = res < 0 ? 0 : res;
            }

    return img;
}

Image256 ImageMUL(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                int res = int(img1.image[i * row1 + j * col1 + k]) * int(img2.image[i * row2 + j * col2 + k]);
                img.image[i * row1 + j * col1 + k] = res / 255;
            }

    return img;
}

Image256 ImageDIV(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                float res = img2.image[i * row2 + j * col2 + k] != 0 ? img2.image[i * row2 + j * col2 + k] : 65025;
                img.image[i * row1 + j * col1 + k] = float(img1.image[i * row1 + j * col1 + k]) / res * 255;
            }

    return img;
}

Image256 ImageMIN(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                unsigned char val1 = img1.image[i * row1 + j * col1 + k], val2 = img2.image[i * row2 + j * col2 + k];
                img.image[i * row1 + j * col1 + k] = val1 < val2 ? val1 : val2;
            }

    return img;
}

Image256 ImageMAX(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                unsigned char val1 = img1.image[i * row1 + j * col1 + k], val2 = img2.image[i * row2 + j * col2 + k];
                img.image[i * row1 + j * col1 + k] = val1 > val2 ? val1 : val2;
            }

    return img;
}

Image256 ImageAVG(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                int res = int(img1.image[i * row1 + j * col1 + k]) + int(img2.image[i * row2 + j * col2 + k]);
                img.image[i * row1 + j * col1 + k] = (res >> 1);
            }

    return img;
}

Image256 ImageDIF(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                int res = int(img1.image[i * row1 + j * col1 + k]) - int(img2.image[i * row2 + j * col2 + k]);
                img.image[i * row1 + j * col1 + k] = res >= 0 ? res : -res;
            }

    return img;
}



template <class T, class Y>
void ImageInplaceADD(ImageBase <T> &img1, const ImageBase <Y> & img2) {
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    const float im_max = std::is_integral<T>::value ? IMAGE_UINT_MAX : IMAGE_FLT_MAX;
    float im2_coef = std::is_integral<T>::value && std::is_floating_point<Y>::value ? IMAGE_FLT_TO_UINT : 1.;
    im2_coef = std::is_floating_point<T>::value && std::is_integral<Y>::value ? IMAGE_UINT_TO_FLT : 1.;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                float res = float(img1.image[i * row1 + j * col1 + k]) +
                                         float(img2.image[i * row2 + j * col2 + k]) * im2_coef;
                img1.image[i * row1 + j * col1 + k] = res <= im_max ? res : im_max;
            }
}

template <class T, class Y>
void ImageInplaceSUB(ImageBase <T> &img1, const ImageBase <Y> & img2) {
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    float im2_coef = std::is_integral<T>::value && std::is_floating_point<Y>::value ? IMAGE_FLT_TO_UINT : 1.;
    im2_coef = std::is_floating_point<T>::value && std::is_integral<Y>::value ? IMAGE_UINT_TO_FLT : 1.;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                float res = float(img1.image[i * row1 + j * col1 + k]) -
                                         float(img2.image[i * row2 + j * col2 + k]) * im2_coef;
                img1.image[i * row1 + j * col1 + k] = res < 0 ? 0 : res;
            }
}

template <class T, class Y>
void ImageInplaceMUL(ImageBase <T> &img1, const ImageBase <Y> & img2) {
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    const float im_max_del = std::is_integral<T>::value ? IMAGE_UINT_TO_FLT : 1. / IMAGE_FLT_MAX;
    float im2_coef = std::is_integral<T>::value && std::is_floating_point<Y>::value ? IMAGE_FLT_TO_UINT : 1.;
    im2_coef = std::is_floating_point<T>::value && std::is_integral<Y>::value ? IMAGE_UINT_TO_FLT : 1.;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                float res = float(img1.image[i * row1 + j * col1 + k]) *
                                         float(img2.image[i * row2 + j * col2 + k]) * im2_coef;
                img1.image[i * row1 + j * col1 + k] = res * im_max_del;
            }
}

template <class T, class Y>
void ImageInplaceDIV(ImageBase <T> &img1, const ImageBase <Y> & img2) {
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    const float im2_max = std::is_integral<T>::value ? IMAGE_UINT_MAX : IMAGE_FLT_MAX;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                T &ind1 = img1.image[i * row1 + j * col1 + k];
                Y &ind2 = img2.image[i * row2 + j * col2 + k];
                ind1 = notCloseToZero(ind2) ? float(ind1) * im2_max / ind2 : 0;
            }
}

template <class T, class Y>
void ImageInplaceMIN(ImageBase <T> &img1, const ImageBase <Y> & img2) {
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    float im2_coef = std::is_integral<T>::value && std::is_floating_point<Y>::value ? IMAGE_FLT_TO_UINT : 1.;
    im2_coef = std::is_floating_point<T>::value && std::is_integral<Y>::value ? IMAGE_UINT_TO_FLT : 1.;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                T &val1 = img1.image[i * row1 + j * col1 + k];
                float val2 = im2_coef * img2.image[i * row2 + j * col2 + k];
                val1 = val1 < val2 ? val1 : val2;
            }
}

template <class T, class Y>
void ImageInplaceMAX(ImageBase <T> &img1, const ImageBase <Y> & img2) {
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    float im2_coef = std::is_integral<T>::value && std::is_floating_point<Y>::value ? IMAGE_FLT_TO_UINT : 1.;
    im2_coef = std::is_floating_point<T>::value && std::is_integral<Y>::value ? IMAGE_UINT_TO_FLT : 1.;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                T &val1 = img1.image[i * row1 + j * col1 + k];
                float val2 = im2_coef * img2.image[i * row2 + j * col2 + k];
                val1 = val1 > val2 ? val1 : val2;
            }
}

template <class T, class Y>
void ImageInplaceAVG(ImageBase <T> &img1, const ImageBase <Y> & img2) {
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    float im2_coef = std::is_integral<T>::value && std::is_floating_point<Y>::value ? IMAGE_FLT_TO_UINT : 1.;
    im2_coef = std::is_floating_point<T>::value && std::is_integral<Y>::value ? IMAGE_UINT_TO_FLT : 1.;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                img1.image[i * row1 + j * col1 + k] = (float(img1.image[i * row1 + j * col1 + k]) +
                                                       float(img2.image[i * row2 + j * col2 + k]) * im2_coef) * 0.5;
            }
}

template <class T, class Y>
void ImageInplaceDIF(ImageBase <T> &img1, const ImageBase <Y> & img2) {
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img1.channels < img2.channels ? img1.channels : img2.channels;

    float im2_coef = std::is_integral<T>::value && std::is_floating_point<Y>::value ? IMAGE_FLT_TO_UINT : 1.;
    im2_coef = std::is_floating_point<T>::value && std::is_integral<Y>::value ? IMAGE_UINT_TO_FLT : 1.;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j)
            for (int k = 0; k < resChannels; ++k) {
                float res = float(img1.image[i * row1 + j * col1 + k]) - float(img2.image[i * row2 + j * col2 + k]) * im2_coef;
                img1.image[i * row1 + j * col1 + k] = res >= 0 ? res : -res;
            }
}



//  ============  Porter-Duff blend funcs  ============  //

Image256 PorterDuffBlendOver(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    if (img1.channels < 4)
        return img;

    const bool hasAlpha2 = img2.channels == 4;
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img2.channels < 3 ? img2.channels : 3;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j) {
            float a2 = 255 - img1.image[i * row1 + j * col1 + 3];
            for (int k = 0; k < resChannels; ++k) {
                int res = int(img1.image[i * row1 + j * col1 + k]) + a2 * int(img2.image[i * row2 + j * col2 + k]) / 255;
                img.image[i * row1 + j * col1 + k] = res < 256 ? res : 255;
            }
            if (hasAlpha2) {
                int res = int(img1.image[i * row1 + j * col1 + 3]) + a2 * int(img2.image[i * row2 + j * col2 + 3]) / 255;
                img.image[i * row1 + j * col1 + 3] = res < 256 ? res : 255;
            }
        }

    return img;
}

Image256 PorterDuffBlendIn(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    if (img2.channels < 4)
        return img;

    const bool hasAlpha1 = img1.channels == 4;
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img2.channels < 3 ? img2.channels : 3;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j) {
            int a2 = img2.image[i * row1 + j * col1 + 3];
            for (int k = 0; k < resChannels; ++k) {
                img.image[i * row1 + j * col1 + k] = a2 * int(img1.image[i * row1 + j * col1 + k]) / 255;
            }
            if (hasAlpha1) {
                img.image[i * row1 + j * col1 + 3] = a2 * int(img1.image[i * row1 + j * col1 + 3]) / 255;
            }
        }

    return img;
}

Image256 PorterDuffBlendOut(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    if (img2.channels < 4)
        return img;

    const bool hasAlpha1 = img1.channels == 4;
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img2.channels < 3 ? img2.channels : 3;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j) {
            int a2 = 255 - img2.image[i * row1 + j * col1 + 3];
            for (int k = 0; k < resChannels; ++k) {
                img.image[i * row1 + j * col1 + k] = a2 * int(img1.image[i * row1 + j * col1 + k]) / 255;
            }
            if (hasAlpha1) {
                img.image[i * row1 + j * col1 + 3] = a2 * int(img1.image[i * row1 + j * col1 + 3]) / 255;
            }
        }

    return img;
}

Image256 PorterDuffBlendAtop(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1, true);

    if (img1.channels < 4 && img2.channels < 4)
        return img;
    if (img1.channels == 4 && img2.channels < 4)
        return PorterDuffBlendOver(img1, img2);
    if (img1.channels < 4 && img2.channels == 4)
        return PorterDuffBlendIn(img1, img2);

    const unsigned row1 = img1.width * 4, col1 = 4;
    const unsigned row2 = img2.width * 4, col2 = 4;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j) {
            int a1 = img2.image[i * row2 + j * col2 + 3], a2 = 255 - img1.image[i * row1 + j * col1 + 3];
            for (int k = 0; k < 3; ++k) {
                int res = (a1 * int(img1.image[i * row1 + j * col1 + k]) + a2 * int(img2.image[i * row2 + j * col2 + k])) / 255;
                img.image[i * row1 + j * col1 + k] = res < 256 ? res : 255;
            }
            img.image[i * row1 + j * col1 + 3] = a1;
        }

    return img;
}

Image256 PorterDuffBlendXor(const Image256 &img1, const Image256 &img2) {
    Image256 img(img1.width, img1.height, img1.channels);

    if (img1.channels < 4 && img2.channels < 4)
        return img;
    if (img1.channels == 4 && img2.channels < 4)
        return PorterDuffBlendOut(img1, img2);
    if (img1.channels < 4 && img2.channels == 4)
        return PorterDuffBlendOut(img2, img1);

    const unsigned row1 = img1.width * 4, col1 = 4;
    const unsigned row2 = img2.width * 4, col2 = 4;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j) {
            int a1 = 255 - img2.image[i * row2 + j * col2 + 3], a2 = 255 - img1.image[i * row1 + j * col1 + 3];
            for (int k = 0; k < 4; ++k) {
                int res = (a1 * int(img1.image[i * row1 + j * col1 + k]) + a2 * int(img2.image[i * row2 + j * col2 + k])) / 255;
                img.image[i * row1 + j * col1 + k] = res < 256 ? res : 255;
            }
        }

    return img;
}



template <class T, class Y>
void PorterDuffBlendInplaceOver(ImageBase <T> &img1, const ImageBase <Y> & img2) {
    if (img1.channels < 4)
        return;

    const bool hasAlpha2 = img2.channels == 4;
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img2.channels < 3 ? img2.channels : 3;

    const float im_max = std::is_integral<T>::value ? IMAGE_UINT_MAX : IMAGE_FLT_MAX;
    float im2_coef = std::is_integral<T>::value && std::is_floating_point<Y>::value ? IMAGE_FLT_TO_UINT : 1.;
    im2_coef = std::is_floating_point<T>::value && std::is_integral<Y>::value ? IMAGE_UINT_TO_FLT : 1.;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j) {
            int a2 = 255 - img1.image[i * row1 + j * col1 + 3];
            for (int k = 0; k < resChannels; ++k) {
                int res = float(img1.image[i * row1 + j * col1 + k]) +
                          a2 * float(img2.image[i * row2 + j * col2 + k]) * im2_coef;
                img1.image[i * row1 + j * col1 + k] = res <= im_max ? res : im_max;
            }
            if (hasAlpha2) {
                int res = float(img1.image[i * row1 + j * col1 + 3]) +
                          a2 * float(img2.image[i * row2 + j * col2 + 3]) * im2_coef;
                img1.image[i * row1 + j * col1 + 3] = res <= im_max ? res : im_max;
            }
        }
}

template <class T, class Y>
void PorterDuffBlendInplaceIn(ImageBase <T> &img1, const ImageBase <Y> & img2) {
    if (img2.channels < 4)
        return;

    const bool hasAlpha1 = img1.channels == 4;
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img2.channels < 3 ? img2.channels : 3;

    float im2_coef = std::is_integral<Y>::value ? IMAGE_UINT_TO_FLT : 1. / IMAGE_FLT_MAX;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j) {
            float a2 = img2.image[i * row1 + j * col1 + 3] * im2_coef;
            for (int k = 0; k < resChannels; ++k) {
                img1.image[i * row1 + j * col1 + k] = a2 * float(img1.image[i * row1 + j * col1 + k]);
            }
            if (hasAlpha1) {
                img1.image[i * row1 + j * col1 + 3] = a2 * float(img1.image[i * row1 + j * col1 + 3]);
            }
        }
}

template <class T, class Y>
void PorterDuffBlendInplaceOut(ImageBase <T> &img1, const ImageBase <Y> &img2) {
    if (img2.channels < 4)
        return;

    const bool hasAlpha1 = img1.channels == 4;
    const unsigned row1 = img1.width * img1.channels, col1 = img1.channels;
    const unsigned row2 = img2.width * img2.channels, col2 = img2.channels;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;
    const int resChannels = img2.channels < 3 ? img2.channels : 3;

    const float im2_max = std::is_integral<Y>::value ? IMAGE_UINT_MAX : IMAGE_FLT_MAX;
    const float im2_coef = 1. / im2_max;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j) {
            float a2 = (im2_max - img2.image[i * row1 + j * col1 + 3]) * im2_coef;
            for (int k = 0; k < resChannels; ++k) {
                img1.image[i * row1 + j * col1 + k] = a2 * float(img1.image[i * row1 + j * col1 + k]);
            }
            if (hasAlpha1) {
                img1.image[i * row1 + j * col1 + 3] = a2 * float(img1.image[i * row1 + j * col1 + 3]);
            }
        }
}

template <class T, class Y>
void PorterDuffBlendInplaceAtop(ImageBase <T> &img1, const ImageBase <Y> &img2) {
    if (img1.channels < 4 && img2.channels < 4)
        return;
    if (img1.channels == 4 && img2.channels < 4) {
        PorterDuffBlendInplaceOver(img1, img2);
        return;
    }
    if (img1.channels < 4 && img2.channels == 4) {
        PorterDuffBlendInplaceIn(img1, img2);
        return;
    }

    const unsigned row1 = img1.width * 4, col1 = 4;
    const unsigned row2 = img2.width * 4, col2 = 4;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;

    const float im_max = std::is_integral<T>::value ? IMAGE_UINT_MAX : IMAGE_FLT_MAX;
    const float im_coef = 1. / im_max;
    const float im2_coef = 1. / im_max;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j) {
            int a1 = img2.image[i * row2 + j * col2 + 3], a2 = im_max - img1.image[i * row1 + j * col1 + 3];
            for (int k = 0; k < 3; ++k) {
                int res = (a1 * int(img1.image[i * row1 + j * col1 + k]) + a2 * int(img2.image[i * row2 + j * col2 + k])) / 255;
                img1.image[i * row1 + j * col1 + k] = res < 256 ? res : 255;
            }
            img1.image[i * row1 + j * col1 + 3] = a1;
        }
}

void PorterDuffBlendInplaceXor(Image256 &img1, const Image256 &img2) {
    if (img1.channels < 4 && img2.channels < 4) {
        img1 = Image256(img1.width, img1.height, img1.channels);
        return;
    }
    if (img1.channels == 4 && img2.channels < 4) {
        PorterDuffBlendInplaceOut(img1, img2);
        return;
    }
    if (img1.channels < 4 && img2.channels == 4) {
        img1 = PorterDuffBlendOut(img2, img1);
        return;
    }

    const unsigned row1 = img1.width * 4, col1 = 4;
    const unsigned row2 = img2.width * 4, col2 = 4;
    const int resWidth = img1.width < img2.width ? img1.width : img2.width;
    const int resHeight = img1.height < img2.height ? img1.height : img2.height;

    for (int i = 0; i < resHeight; ++i)
        for (int j = 0; j < resWidth; ++j) {
            int a1 = 255 - img2.image[i * row2 + j * col2 + 3], a2 = 255 - img1.image[i * row1 + j * col1 + 3];
            for (int k = 0; k < 4; ++k) {
                int res = (a1 * int(img1.image[i * row1 + j * col1 + k]) + a2 * int(img2.image[i * row2 + j * col2 + k])) / 255;
                img1.image[i * row1 + j * col1 + k] = res < 256 ? res : 255;
            }
        }
}

/*
int main() {
    Image hor("./img/hor.png");
    Image horA("./img/horA.png");
    Image horQ("./img/horQ.png");
    Image horAQ("./img/horAQ.png");
    Image vert("./img/vert.png");
    Image vertA("./img/vertA.png");
    Image vertQ("./img/vertQ.png");
    Image vertAQ("./img/vertAQ.png");

    ImageInplaceMAX(horQ, horA);
    horQ.saveAs("res.png");
    return 0;
}*/