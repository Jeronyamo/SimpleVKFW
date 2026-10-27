// #ifndef SVKFW_TYPESIO_H
// #define SVKFW_TYPESIO_H

#include <cstdint>
#include <typeinfo>
#include <typeindex>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>


namespace Simple {

#ifndef SVKFW_TYPESIO_OUT
#define SVKFW_TYPESIO_OUT
    struct TypenameStorage {
        std::unordered_map<std::type_index, std::string> typeindex_to_str;

        TypenameStorage() { addCommonNames(); }

        void addCommonNames() {
            typeindex_to_str[std::type_index(typeid(float   ))] = "float"   ;
            typeindex_to_str[std::type_index(typeid(double  ))] = "double"  ;
            typeindex_to_str[std::type_index(typeid(int8_t  ))] = "int8_t"  ;
            typeindex_to_str[std::type_index(typeid(int16_t ))] = "int16_t" ;
            typeindex_to_str[std::type_index(typeid(int32_t ))] = "int32_t" ;
            typeindex_to_str[std::type_index(typeid(int64_t ))] = "int64_t" ;
            typeindex_to_str[std::type_index(typeid(uint8_t ))] = "uint8_t" ;
            typeindex_to_str[std::type_index(typeid(uint16_t))] = "uint16_t";
            typeindex_to_str[std::type_index(typeid(uint32_t))] = "uint32_t";
            typeindex_to_str[std::type_index(typeid(uint64_t))] = "uint64_t";
            typeindex_to_str[std::type_index(typeid(std::string))] = "string";
        }

        template <class T>
        bool has()         const { return typeindex_to_str.count(std::type_index(typeid(T))) != 0u; }
        template <class T>
        bool has(const T&) const { return has<T>(); }

        template <class T>
        void setNew(const std::string &_t_name) {
            if (!has<T>())  typeindex_to_str[std::type_index(typeid(T))] = _t_name;
        }
        template <class T>
        void setNew(const std::string &_t_name, const T&) {
            setNew<T>(_t_name);
        }

        // Name of a registered (non-container) type
        template <class T>
        std::string getRegistered() const {
            return has<T>() ? typeindex_to_str.at(std::type_index(typeid(T))) : std::string("UnknownType(") + std::type_index(typeid(T)).name() + " | " + std::to_string(sizeof(T)) + " B)";
        }

        // Note: function templates can't be partially specialized, so containers are handled by 'TypesioInner::NameOf' specializations
        template <class T>
        std::string get()         const;
        template <class T>
        std::string get(const T&) const { return get<T>(); }
    } typename_storage; // TypenameStorage END

    namespace TypesioInner {
        template <class T>
        struct NameOf { static std::string get(const TypenameStorage &_s) { return _s.getRegistered<T>(); } };

        template <class T>
        struct NameOf<std::vector<T>> { static std::string get(const TypenameStorage &_s) { return "vector<" + _s.get<T>() + ">"; } };
        template <class T>
        struct NameOf<std::set<T>> { static std::string get(const TypenameStorage &_s) { return "set<" + _s.get<T>() + ">"; } };
        template <class T>
        struct NameOf<std::unordered_set<T>> { static std::string get(const TypenameStorage &_s) { return "unordered_set<" + _s.get<T>() + ">"; } };
        template <class T1, class T2>
        struct NameOf<std::pair<T1, T2>> { static std::string get(const TypenameStorage &_s) { return "pair<" + _s.get<T1>() + ", " + _s.get<T2>() + ">"; } };
        template <class T1, class T2>
        struct NameOf<std::map<T1, T2>> { static std::string get(const TypenameStorage &_s) { return "map<" + _s.get<T1>() + ", " + _s.get<T2>() + ">"; } };
        template <class T1, class T2>
        struct NameOf<std::unordered_map<T1, T2>> { static std::string get(const TypenameStorage &_s) { return "unordered_map<" + _s.get<T1>() + ", " + _s.get<T2>() + ">"; } };
    }; // TypesioInner END

    template <class T>
    std::string TypenameStorage::get() const { return TypesioInner::NameOf<T>::get(*this); }
#endif


#ifdef SVKFW_VECTORS_H
#ifndef SVKFW_VECTORS_OUT
#define SVKFW_VECTORS_OUT
    template <typename T>
    std::string toStr(const Vec2Base<T> &_obj) {
        return '[' + std::to_string(_obj.x) + ", " + std::to_string(_obj.y) + ']';
    }
    template <typename T>
    std::string toStr(const Vec3Base<T> &_obj) {
        return '[' + std::to_string(_obj.x) + ", " + std::to_string(_obj.y) + ", " + std::to_string(_obj.z) + ']';
    }
    template <typename T>
    std::string toStr(const Vec4Base<T> &_obj) {
        return '[' + std::to_string(_obj.x) + ", " + std::to_string(_obj.y) + ", " + std::to_string(_obj.z) + ", " + std::to_string(_obj.w) + ']';
    }


    template <typename T>
    std::string toStr(const Mat2Base<T> &_obj) {
        return "\n[ " + std::to_string(_obj.M[0][0]) + ", " + std::to_string(_obj.M[0][1]) +
               "\n  " + std::to_string(_obj.M[1][0]) + ", " + std::to_string(_obj.M[1][1]) + " ]\n";
    }
    template <typename T>
    std::string toStr(const Mat3Base<T> &_obj) {
        return "\n[ " + std::to_string(_obj.M[0][0]) + ", " + std::to_string(_obj.M[0][1]) + ", " + std::to_string(_obj.M[0][2]) +
               "\n  " + std::to_string(_obj.M[1][0]) + ", " + std::to_string(_obj.M[1][1]) + ", " + std::to_string(_obj.M[1][2]) +
               "\n  " + std::to_string(_obj.M[2][0]) + ", " + std::to_string(_obj.M[2][1]) + ", " + std::to_string(_obj.M[2][2]) + " ]\n";
    }
    template <typename T>
    std::string toStr(const Mat4Base<T> &_obj) {
        return "\n[ " + std::to_string(_obj.M[0][0]) + ", " + std::to_string(_obj.M[0][1]) + ", " + std::to_string(_obj.M[0][2]) + ", " + std::to_string(_obj.M[0][3]) +
               "\n  " + std::to_string(_obj.M[1][0]) + ", " + std::to_string(_obj.M[1][1]) + ", " + std::to_string(_obj.M[1][2]) + ", " + std::to_string(_obj.M[1][3]) +
               "\n  " + std::to_string(_obj.M[2][0]) + ", " + std::to_string(_obj.M[2][1]) + ", " + std::to_string(_obj.M[2][2]) + ", " + std::to_string(_obj.M[2][3]) +
               "\n  " + std::to_string(_obj.M[3][0]) + ", " + std::to_string(_obj.M[3][1]) + ", " + std::to_string(_obj.M[3][2]) + ", " + std::to_string(_obj.M[3][3]) + " ]\n";
    }
#endif
#endif


#ifdef SVKFW_IMAGE_H
#ifndef SVKFW_IMAGE_OUT
#define SVKFW_IMAGE_OUT
    template <typename T, int Ch>
    std::string toStr(const Img::Image<T, Ch> &_obj) {
        return "[ Image: " + std::to_string(_obj.width) + 'x' + std::to_string(_obj.height) + 'x' + std::to_string(Ch) + ", " + typename_storage.get<typename Img::Image<T, Ch>::ChannelType>() + " ]";
    }
#endif
#endif
};

//#endif