#ifndef SVKFW_EVENT_TYPES_H
#define SVKFW_EVENT_TYPES_H

#include <cstdint>


namespace Simple {
    namespace Event {
//  == === ==== ================================== ==== === ==  \\
                    Event types
//  == === ==== ================================== ==== === ==  \\

        // Note: one range per category, so the producer of an event is found by its number:
        //         1-99     engine   ('core/engine.h')
        //         100-199  physics  ('systems/physics.h')
        //         200-299  scene    (scene editors and the systems that follow the scene, e.g. 'systems/physics_scene.h')
        //         300-399  input
        //         400-499  audio
        //         1024+    game code ('EV_TYPE_USER + n')
        //       Payloads are trivially copyable structs of at most 'Event::PAYLOAD_SIZE' bytes ('core/event_handler.h')
        enum EventType : uint32_t {
            EV_TYPE_NONE = 0,

        // Engine
            // Starts the editor / the game ('Engine::registerApp'); ignored while an app is running
            EV_TYPE_RUN_EDITOR = 1,
            EV_TYPE_RUN_GAME   = 2,
            // Stops the running app and the engine loop
            EV_TYPE_QUIT       = 3,

        // Physics: two bodies started / stopped touching; payload: 'Physics::ContactEvent'.
        //          Written for bodies with TAG_PHYSICS_REPORT_CONTACTS
            EV_TYPE_PHYSICS_CONTACT_BEGIN = 100,
            EV_TYPE_PHYSICS_CONTACT_END   = 101,

        // Scene: dispatched immediately (the scene is in the described state during the call)
            // A model was added under 'SceneModelEvent::root' (its transforms are up to date); payload: 'SceneModelEvent'
            EV_TYPE_SCENE_MODEL_ADDED = 200,
            // All models are about to be removed (the scene is rebuilt; MODEL_ADDED follows for the models that stay)
            EV_TYPE_SCENE_CLEARED     = 201,
            // A jump in time (seek, stop, edits, moved models): simulations go back to the animated pose
            EV_TYPE_SCENE_RESET       = 202,
            // Queued: the scene got new meshes, materials or skins (the renderer must upload it again)
            EV_TYPE_SCENE_CHANGED     = 203,

        // Input
            // An action of an input layer; payload: 'InputActionEvent' (optional: layers call their handlers directly)
            EV_TYPE_INPUT_ACTION = 300,

            EV_TYPE_USER = 1024
        }; // EventType END


    // Payloads

        // EV_TYPE_SCENE_MODEL_ADDED (indices of 'SceneGraph::Scene', UINT32_MAX: none)
        struct SceneModelEvent {
            uint32_t root = UINT32_MAX;   // the node the model was appended under
            uint32_t skin = UINT32_MAX;   // the skin of the model (none for static models)
        }; // SceneModelEvent END

        // EV_TYPE_INPUT_ACTION
        struct InputActionEvent {
            uint32_t layer  = 0u;  // the producer's id (e.g. its index in the layer stack)
            uint32_t action = 0u;
            uint32_t pressed = 0u; // 1: pressed / triggered, 0: released
        }; // InputActionEvent END
    }; // Event END
}; // Simple END

#endif // SVKFW_EVENT_TYPES_H
