#include "common/basic_math.h"

namespace Simple {
    namespace Math {

        bool notZero (float  _val) { return _val <= -SVKFW_AEPS_FLT || _val >= SVKFW_AEPS_FLT; }

        bool notZero (double _val) { return _val <= -SVKFW_AEPS_DBL || _val >= SVKFW_AEPS_DBL; }

        float  toRad(float  _angle_deg) { return _angle_deg * SVKFW_DEG_TO_RAD; }

        double toRad(double _angle_deg) { return _angle_deg * SVKFW_DEG_TO_RAD_D; }

        float  toDeg(float  _angle_rad) { return _angle_rad * SVKFW_RAD_TO_DEG; }

        double toDeg(double _angle_rad) { return _angle_rad * SVKFW_RAD_TO_DEG_D; }
    }; // Math END
}; // Simple END
