// #ifndef SVKFW_TYPESIO_H
// #define SVKFW_TYPESIO_H

#include <iostream>


namespace Simple {
#ifdef SVKFW_VECTORS_H
#ifndef SVKFW_VECTORS_OUT
#define SVKFW_VECTORS_OUT
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Vec2Base<T>& _vec) {
        os << _vec.x << ", " << _vec.y;
        return os;
    }
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Vec3Base<T>& _vec) {
        os << _vec.x << ", " << _vec.y << ", " << _vec.z;
        return os;
    }
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Vec4Base<T>& _vec) {
        os << _vec.x << ", " << _vec.y << ", " << _vec.z << ", " << _vec.w;
        return os;
    }


    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Mat2Base<T>& _m) {
        os << _m.M[0][0] << ", " << _m.M[0][1] << "\n";
        os << _m.M[1][0] << ", " << _m.M[1][1];
        return os;
    }
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Mat3Base<T>&  _m) {
        os << _m.M[0][0] << ", " << _m.M[0][1] << ", " << _m.M[0][2] << "\n";
        os << _m.M[1][0] << ", " << _m.M[1][1] << ", " << _m.M[1][2] << "\n";
        os << _m.M[2][0] << ", " << _m.M[2][1] << ", " << _m.M[2][2];
        return os;
    }
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Mat4Base<T>&  _m) {
        os << _m.M[0][0] << ", " << _m.M[0][1] << ", " << _m.M[0][2] << ", " << _m.M[0][3] << "\n";
        os << _m.M[1][0] << ", " << _m.M[1][1] << ", " << _m.M[1][2] << ", " << _m.M[1][3] << "\n";
        os << _m.M[2][0] << ", " << _m.M[2][1] << ", " << _m.M[2][2] << ", " << _m.M[2][3] << "\n";
        os << _m.M[3][0] << ", " << _m.M[3][1] << ", " << _m.M[3][2] << ", " << _m.M[3][3];
        return os;
    }
#endif
#endif


#ifdef SVKFW_COLOR_H
#ifndef SVKFW_COLOR_OUT
#define SVKFW_COLOR_OUT
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const ColorBase<T>&  _vec) {
        os << _vec.x << ", " << _vec.y << ", " << _vec.z;
        return os;
    }
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const ColorABase<T>&  _vec) {
        os << _vec.x << ", " << _vec.y << ", " << _vec.z << ", " << _vec.a;
        return os;
    }
#endif
#endif


#ifdef SVKFW_IMAGE_H
#ifndef SVKFW_IMAGE_OUT
#define SVKFW_IMAGE_OUT
    std::ostream& operator<<(std::ostream& os, const Image& img) {
        os << "Image    : " << img.width << "x" << img.height << ", " << img.channels << " channels, base type: float";
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const ImageUC& img) {
        os << "ImageUC  : " << img.width << "x" << img.height << ", " << img.channels << " channels, base type: uchar";
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const Image1& img) {
        os << "Image1   : " << img.width << "x" << img.height << ", " << img.channels << " channels, base type: float";
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const ImageUC1& img) {
        os << "ImageUC1 : " << img.width << "x" << img.height << ", " << img.channels << " channels, base type: uchar";
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const Image3& img) {
        os << "Image3   : " << img.width << "x" << img.height << ", " << img.channels << " channels, base type: Color (3 channels, float)";
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const ImageUC3& img) {
        os << "ImageUC3 : " << img.width << "x" << img.height << ", " << img.channels << " channels, base type: Color256 (3 channels, uchar)";
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const Image4& img) {
        os << "Image4   : " << img.width << "x" << img.height << ", " << img.channels << " channels, base type: ColorA (4 channels, float)";
        return os;
    }
    std::ostream& operator<<(std::ostream& os, const ImageUC4& img) {
        os << "ImageUC4 : " << img.width << "x" << img.height << ", " << img.channels << " channels, base type: ColorA256 (4 channels, uchar)";
        return os;
    }
    template <class T, int Ch>
    std::ostream& operator<<(std::ostream& os, const ImageBase<T, Ch>& img) {
        os << "ImageCustom (" << Ch << " channels): " << img.width << "x" << img.height << ", " << img.channels << " channels, base typeid name: " << typeid(T).name();
        return os;
    }
#endif
#endif
};

//#endif