#ifndef SVKFW_SCENE_ANIMATION_H
#define SVKFW_SCENE_ANIMATION_H

#include <cstdint>
#include <string>
#include <vector>

#include "scene/scene_graph.h"


namespace Simple {
    namespace File { struct ContentVMD; };

    namespace SceneGraph {

//  == === ==== ================================== ==== === ==  \\
                    Skeletal animation
//  == === ==== ================================== ==== === ==  \\

        // Note: a clip is a set of tracks found by name (bones, IK chains), so one clip plays on any skeleton that has those
        //       names; 'bindAnimation' resolves them once per skeleton. Time is in frames ('frames_per_second' gives seconds).
        //       Keys are relative to the rest pose, like in MMD: the bone gets 'rest.translation + key.translation' and
        //       'rest.rotation * key.rotation' (the key rotation is applied first, in the bone's rest space).
        //
        //       AnimationClip    dance   = loadVMD("resources/assets/anim/dance.vmd");
        //       AnimationBinding binding = bindAnimation(dance, scene.skeletons[skel]); // warns about the missing bones
        //       ...
        //       applyAnimation(dance, binding, scene.skeletons[skel], time * dance.frames_per_second, scene.pose(skin));
        //       scene.updateSkins();

        // MMD interpolation: a cubic Bezier curve from (0,0) to (1,1) with control points (x1,y1), (x2,y2) in [0, 127].
        // The default one is linear
        struct AnimationCurve {
            uint8_t x1 = 20u, y1 = 20u, x2 = 107u, y2 = 107u;

            bool  isLinear() const;
            // The eased fraction for the time fraction '_t' in [0, 1]
            float evaluate(float _t) const;
        }; // AnimationCurve END

        enum AnimationChannel { ANIM_TRANSLATION_X = 0, ANIM_TRANSLATION_Y, ANIM_TRANSLATION_Z, ANIM_ROTATION, ANIM_CHANNEL_COUNT };

        // Note: the curves shape the way from the previous key to this one (MMD)
        struct BoneKey {
            float          frame = 0.f;
            vec3           translation = {0.f, 0.f, 0.f};
            Quat           rotation;
            AnimationCurve curves[ANIM_CHANNEL_COUNT];
        }; // BoneKey END

        struct BoneTrack {
            std::string          bone;
            bool                 name_cut = false; // the file cut the name (VMD keeps 15 bytes): matched by prefix too
            std::vector<BoneKey> keys;             // sorted by frame
        }; // BoneTrack END

        // Note: the IK chain keeps the state of the last key at or before the frame (a step, no blending)
        struct IKKey {
            float frame   = 0.f;
            bool  enabled = true;
        }; // IKKey END

        struct IKTrack {
            std::string        ik;    // the name of the IK chain (its goal bone in PMX)
            std::vector<IKKey> keys;  // sorted by frame
        }; // IKTrack END

        // Note: morph weights are interpolated linearly (MMD)
        struct MorphKey {
            float frame  = 0.f;
            float weight = 0.f;
        }; // MorphKey END

        struct MorphTrack {
            std::string           morph;
            std::vector<MorphKey> keys;  // sorted by frame
        }; // MorphTrack END

        struct AnimationClip {
            std::string            name;
            float                  frames_per_second = 30.f; // MMD
            float                  length = 0.f;             // the last key frame
            std::vector<BoneTrack> bones;
            std::vector<IKTrack>   iks;
            std::vector<MorphTrack> morphs;

            // Sorts the keys of all tracks and updates 'length' (after filling the tracks by hand)
            void finalize();
        }; // AnimationClip END

        // Note: track -> bone / IK chain of one skeleton ('SCENE_NONE': the skeleton has no such one, the track is skipped)
        struct AnimationBinding {
            std::vector<uint32_t> bones;
            std::vector<uint32_t> iks;
            std::vector<uint32_t> morphs;
            uint32_t              missing_bones = 0u, missing_iks = 0u, missing_morphs = 0u;
        }; // AnimationBinding END

        // Resolves the tracks by name; with '_warn' prints the missing bones, IK chains and morphs (one warning each)
        AnimationBinding bindAnimation(const AnimationClip &_clip, const Skeleton &_skeleton, bool _warn = true);

        // Sets the local transforms of the bound bones, the IK switches and the morph weights at '_frame' (clamped to
        // [0, length]); the other bones and morphs keep their pose. Marks the pose dirty
        void applyAnimation(const AnimationClip &_clip, const AnimationBinding &_binding, const Skeleton &_skeleton,
                            float _frame, Pose &_pose);

        // Bone, morph and IK tracks of an MMD motion (VMD); names are converted from Shift-JIS to UTF-8.
        // Camera keys: see 'convertVMDCamera'; light keys are not converted yet. Asserts (std::runtime_error) if the file can't be read
        AnimationClip loadVMD(const std::string &_path);
        // The same for read VMD data (e.g. edited keyframes); '_name' becomes the clip name
        AnimationClip convertVMD(const File::ContentVMD &_vmd, const std::string &_name = std::string());


//  == === ==== ================================== ==== === ==  \\
                    MMD camera
//  == === ==== ================================== ==== === ==  \\

        // Note: an MMD camera looks at 'center' from 'distance' (negative: in front of the models, which face -Z) along +Z
        //       rotated by 'rotation' (radians: x - pitch, y - yaw, z - roll), in the MMD space (the engine's one).
        //       'fov' is the vertical field of view in degrees
        struct CameraPose {
            vec3  center   = {0.f, 10.f, 0.f};
            vec3  rotation = {0.f, 0.f, 0.f};
            float distance = 45.f;     // as shown in MMD: positive is farther (the VMD stores it negated)
            float fov      = 30.f;
            bool  perspective = true;

            // The eye position and the up vector (for 'View::ViewMat')
            void lookAt(vec3 &_eye, vec3 &_up) const;
        }; // CameraPose END

        enum CameraChannel { CAMERA_X = 0, CAMERA_Y, CAMERA_Z, CAMERA_ROTATION, CAMERA_DISTANCE, CAMERA_FOV, CAMERA_CHANNEL_COUNT };

        // Note: like bone keys, the curves shape the way from the previous key to this one. Keys one frame apart are a cut
        struct CameraKey {
            float          frame = 0.f;
            CameraPose     pose;
            AnimationCurve curves[CAMERA_CHANNEL_COUNT];
        }; // CameraKey END

        struct CameraClip {
            std::string            name;
            float                  frames_per_second = 30.f;
            float                  length = 0.f;
            std::vector<CameraKey> keys; // sorted by frame

            bool empty() const;
            void finalize();
            // The camera at '_frame' (clamped to [0, length]); the default pose without keys
            CameraPose evaluate(float _frame) const;
        }; // CameraClip END

        CameraClip convertVMDCamera(const File::ContentVMD &_vmd, const std::string &_name = std::string());
    }; // SceneGraph END
}; // Simple END

#endif
