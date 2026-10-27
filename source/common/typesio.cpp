#include "common/typesio.h"

namespace Simple {

    TypenameStorage::TypenameStorage() { addCommonNames(); }

    void TypenameStorage::addCommonNames() {
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

    TypenameStorage typename_storage;
}; // Simple END
