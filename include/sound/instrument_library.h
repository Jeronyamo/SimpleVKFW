#ifndef SVKFW_INSTRUMENT_LIBRARY_H
#define SVKFW_INSTRUMENT_LIBRARY_H

#include "data/filetypes.h"
#include "sound/synth.h"


namespace Simple {
    namespace Audio {
        namespace Synth {
            // All instruments of a project, stored in one JSON file. Instruments are identified by their unique names.
            struct InstrumentLibrary {
                static const uint32_t Format_Version = 1u;

                std::vector<Instrument> instruments;
                std::string fpath;        // Set by 'load'/'save'
                bool is_modified = false; // Set by the editing methods, the UI sets it when it edits parameters directly

                uint32_t size() const;
                int32_t find(const std::string &_name) const; // -1 if there is no such instrument

                // '_base' if it's free, else '_base_2', '_base_3', ...
                std::string uniqueName(const std::string &_base) const;

                // Add methods make the name unique and return the new index
                uint32_t add(const Instrument &_instrument);
                uint32_t addDefault(const std::string &_name = "instrument");
                uint32_t duplicate(uint32_t _id);
                bool rename(uint32_t _id, const std::string &_name); // False if the name is empty or taken
                void remove(uint32_t _id);
                void clear();

                bool load(const std::string &_fpath);
                // Writes a temporary file first, so a failed save keeps the previous library. Empty path - last used one.
                bool save(const std::string &_fpath = "");

                static void toJSON(const Instrument &_instrument, File::ContentJSON::JSONNode &_node);
                static bool fromJSON(const File::ContentJSON::JSONNode &_node, Instrument &_instrument);
            }; // InstrumentLibrary END
        }; // Synth END
    }; // Audio END
}; // Simple END

#endif
