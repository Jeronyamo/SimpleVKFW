#ifndef SVKFW_ETCS_TAGS_H
#define SVKFW_ETCS_TAGS_H

#include <cstdint>


namespace Simple {
    // Note: tags are flags without data: an entity has a tag or not. They are stored as bits ('TagMask'), so there can be
    //       at most 64 of them. Data goes into components (any C++ type, see 'ComponentStorage::add' in 'core/etcs_base.h')
    enum struct TagID : uint32_t {
        // Physics ('systems/physics.h'): the motion type of a body (exactly one of them)
        TAG_PHYSICS_STATIC = 0,   // never moves (ground, walls)
        TAG_PHYSICS_KINEMATIC,    // moved by the game (e.g. follows a bone), pushes dynamic bodies, isn't pushed
        TAG_PHYSICS_DYNAMIC,      // simulated
        TAG_PHYSICS_DISABLED,     // skipped by the simulation (body and joint entities)
        TAG_PHYSICS_REPORT_CONTACTS, // the world writes contact events for the body

        // Add new tags here

        TAG_COUNT // must stay <= 64
    }; // TagID END
}; // Simple END

#endif
