#ifndef SVKFW_VMD_MOCAP_H
#define SVKFW_VMD_MOCAP_H

#include <string>
#include <vector>

#include "data/filetypes.h"


namespace Simple {
    namespace File {
//  ============  Mocap-like processing of hand-keyed VMD motions  ============  \\

        // Turns a hand-keyed model motion (poses every few frames, linear curves) into a full-key 30 FPS motion that is closer
        // to motion capture. Stages (each can be applied on its own):
        //   A. smooth: C2 (velocity and acceleration continuous) curves through the key poses instead of linear segments
        // Note: measured on hand-keyed vs captured dance motions: hand keys put all the acceleration on the key frames (the
        //       velocity changes stepwise there); captured motions have no such breaks
        namespace VMDMocap {
            struct SmoothSettings {
                // Segments with non-linear Bezier curves keep the animator's curve (the joints become C0 there)
                bool keep_curves = true;
                // Per channel monotone limit of the key velocities (no overshoot past the neighbor keys)
                bool limit_overshoot = true;
                // Bound of the acceleration term of a segment: |a * H^2| <= acc_bound * |change over the segment|; keeps long
                // segments (holds of the neighbors, slow moves) from being bent by the acceleration of a short neighbor
                float acc_bound = 8.f;
                // Keys at most this many frames apart are a cut (a snap): no smoothing across it
                uint32_t cut_gap = 1u;
                // Bones left as they are (names from 'ContentVMD::bone_name_mappings'; unmapped bones are processed)
                std::vector<std::string> skip_bones{ "eyes", "eye_r", "eye_l", "mouth_base", "eyebrow_base" };
            };

            struct Report {
                uint32_t bones_processed = 0u;
                uint32_t bones_skipped   = 0u; // in 'skip_bones', a single key or all keys equal
                uint32_t keys_before     = 0u;
                uint32_t keys_after      = 0u;
            };

            // Stage A: replaces the keys of every processed bone with one key per frame (linear curves) between its first and
            // last key; the key poses are kept exactly
            Report smoothBones(ContentVMD &_vmd, const SmoothSettings &_settings = SmoothSettings());

            // One channel of stage A: samples the C2 curve through the keys '_values' at frames '_frames' (strictly
            // increasing) into '_out' (one value per frame from '_frames.front()' to '_frames.back()'); '_hold[i]' marks the
            // segment (i, i+1) as a hold, '_cut[i]' as a cut. Used by 'smoothBones' for every position/rotation component
            void smoothChannel(const std::vector<uint32_t> &_frames, const std::vector<float> &_values,
                               const std::vector<uint8_t> &_hold, const std::vector<uint8_t> &_cut,
                               const SmoothSettings &_settings, std::vector<float> &_out);
        }; // VMDMocap END
    }; // File END
}; // Simple END

#endif
