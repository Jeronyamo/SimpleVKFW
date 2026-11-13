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
        //   B. chain: the parent's motion carried over to the child (with a delay), as captured bodies do, and small timing
        //      jitter, so the bones stop moving in sync
        // Note: measured on hand-keyed vs captured dance motions: hand keys put all the acceleration on the key frames (the
        //       velocity changes stepwise there); captured motions have no such breaks. In hand keys 90-99% of the child keys
        //       are on the frames of the parent keys and the angular velocities of parent and child are not coupled (or are
        //       opposed); in captured motions the child repeats a part of the parent's turn, 0-3 frames later
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


            // Stage B profile: what the coupling of parent and child looks like in captured motions (measured once on donor
            // motions, see 'resources/motion/mocap_profile.json'), so the donors are not needed here
            struct ChainPair {
                std::string parent, child;  // names from 'ContentVMD::bone_name_mappings'
                std::vector<float> target;  // coupling (see 'coupling') at the lags 0, 1, ... frames
            };

            struct ChainProfile {
                std::vector<ChainPair> pairs;           // from the root to the leaves
                // Fit grid: the gain of the carried over motion, its delay (frames) and the width of the parent's trend
                // (Gaussian sigma, frames; only the deviation from the trend is carried over, so nothing drifts)
                std::vector<float>    gains { 0.f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };
                std::vector<uint32_t> delays{ 0u, 1u, 2u, 3u };
                std::vector<float>    sigmas{ 10.f, 20.f };
                // Bound of the rotation added to a child (rms, degrees): the coupling of a pair animated against it (e.g.
                // the chest turning back against the waist) is not forced to the target by large turns
                float max_added_deg = 6.f;
                // Timing jitter: each of these bones is played a little ahead or behind (smooth noise, rms in frames)
                std::vector<std::string> jitter_bones;
                float jitter_frames = 0.6f;
                float jitter_rate   = 0.8f;             // noise cells per second
            };

            // Reads '_path' (format: see the file in resources); false and '_error' if it can't
            bool loadChainProfile(const std::string &_path, ChainProfile &_out, std::string &_error);

            struct ChainSettings {
                float    strength = 1.f;                // scales the fitted gains
                float    jitter   = 1.f;                // scales 'ChainProfile::jitter_frames'
                uint32_t seed     = 1u;
            };

            struct ChainPairReport {
                std::string parent, child;
                float    gain = 0.f, sigma = 0.f;
                uint32_t delay = 0u;
                float    error_before = 0.f, error_after = 0.f; // mean square difference from the target coupling
                float    added_deg = 0.f;               // rms of the added rotation, degrees
            };

            struct ChainReport {
                std::vector<ChainPairReport> pairs;     // the pairs found in the motion
                uint32_t bones_jittered = 0u;
                uint32_t bones_written  = 0u;           // bones that got one key per frame
                uint32_t keys_before    = 0u;
                uint32_t keys_after     = 0u;
            };

            // Stage B: for every pair of the profile found in '_vmd' fits the gain, the delay and the trend width so that the
            // coupling of the pair is the closest to the profile's, applies it (child = exp(gain * deviation of the parent,
            // delayed) * child), then warps the time of the jitter bones. The changed bones get one key per frame (linear
            // curves) over the frames of all the bones involved. Works on any motion (keys with curves are baked first), but
            // is meant to run after stage A
            ChainReport chainBones(ContentVMD &_vmd, const ChainProfile &_profile, const ChainSettings &_settings = ChainSettings());

            // Coupling of the angular velocities of parent and child (both in the parent's frame) at the lags 0.._out.size()-1:
            // child later by the lag; 1: the child turns the same way as the parent, 0: unrelated. '_parent' and '_child'
            // are the local rotations, one per frame
            void coupling(const std::vector<Affine::Rotation::Quaternion> &_parent, const std::vector<Affine::Rotation::Quaternion> &_child,
                          std::vector<float> &_out);
        }; // VMDMocap END
    }; // File END
}; // Simple END

#endif
