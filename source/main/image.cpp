#include "main/image.h"

// Note: STBI implementation is compiled once, here (the headers are already included by image.h, so only the implementation is added)
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "STBI/stb_image.h"
#include "STBI/stb_image_write.h"

namespace Simple {
    namespace Img {

        vec4f Sampler::validateUV(vec2f _uv) const {
            const float __max_uv = 0.99999994f; // largest float < 1, so that 'uv * size' is a valid pixel index
            vec2f __offset_sign{1.f};
            if ((_uv >= 0.f).all() && (_uv < 1.f).all()) return { _uv, __offset_sign };

            switch (ext_type) {
                case EXT_TYPE_REPEAT:
                case EXT_TYPE_REPEAT_MIRRORED: {
                    // Note: floor (not modf) keeps the fractional part positive for negative uv
                    vec2f __rep{std::floor(_uv.x), std::floor(_uv.y)};
                    _uv -= __rep;
                    if (ext_type == EXT_TYPE_REPEAT_MIRRORED) {
                        if (int(__rep.x) & 1) { __offset_sign.x = -1.f; _uv.x = 1.f - _uv.x; }
                        if (int(__rep.y) & 1) { __offset_sign.y = -1.f; _uv.y = 1.f - _uv.y; }
                    }
                    _uv = Math::clampCL(_uv, vec2f{0.f}, vec2f{__max_uv});
                    break;
                }
                case EXT_TYPE_CLAMP:
                    _uv = Math::clampCL(_uv, vec2f{0.f}, vec2f{__max_uv});
                    break;
            }
            return { _uv, __offset_sign };
        }

        Sampler def_sampler;
    }; // Img END
}; // Simple END
