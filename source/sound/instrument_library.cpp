#include "sound/instrument_library.h"

#include <cstdio>
#include <fstream>
#include <sstream>


namespace Simple {
    namespace Audio {
        namespace Synth {
            typedef File::ContentJSON::JSONNode JSONNode;

            // Note: floats are written with 6 significant digits, else 0.15f turns into 0.150000005960464
            static void setFloat(JSONNode &_node, float _value) {
                char __buf[32]{};
                snprintf(__buf, sizeof(__buf), "%.6g", double(_value));
                _node.setValueNum(std::strtod(__buf, nullptr));
            }

            static float getFloat(const JSONNode &_node, const std::string &_key, float _default) { return float(_node.getNum(_key, _default)); }

            static void writeOscillator(const Oscillator &_osc, JSONNode &_node) {
                _node.addMember("wave").setValueStr(waveformName(_osc.wave));
                setFloat(_node.addMember("volume"), _osc.volume);
                setFloat(_node.addMember("detune"), _osc.detune);
                if (_osc.phase != 0.f)
                    setFloat(_node.addMember("phase"), _osc.phase);
                if (_osc.bits > 0u)
                    _node.addMember("bits").setValueNum(_osc.bits);

                if (_osc.wave == WAVE_PULSE) {
                    setFloat(_node.addMember("duty"), _osc.duty);
                    if (_osc.pwm_depth != 0.f) {
                        setFloat(_node.addMember("pwm_depth"), _osc.pwm_depth);
                        setFloat(_node.addMember("pwm_rate"),  _osc.pwm_rate);
                    }
                }
                if (_osc.wave == WAVE_PULSE || _osc.wave == WAVE_SAW)
                    _node.addMember("antialias").setValueBool(_osc.antialias);
                if (_osc.wave == WAVE_TABLE) {
                    // Note: written as one string of integers, a JSON array would take a line per value
                    std::string __table;
                    const std::vector<uint32_t> __values = _osc.getTable();
                    for (size_t i = 0u; i < __values.size(); ++i)
                        __table += (i ? " " : "") + std::to_string(__values[i]);
                    _node.addMember("table_bits").setValueNum(_osc.table_bits);
                    _node.addMember("table").setValueStr(__table);
                }
            }

            static void readOscillator(const JSONNode &_node, Oscillator &_osc, const std::string &_where) {
                const std::string __wave = _node.getStr("wave", "pulse");
                if (!waveformFromName(__wave, _osc.wave))
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: load", "%s: unknown wave '%s', using pulse\n"), _where.c_str(), __wave.c_str());
                _osc.volume    = getFloat(_node, "volume", 1.f);
                _osc.detune    = getFloat(_node, "detune", 0.f);
                _osc.phase     = getFloat(_node, "phase", 0.f);
                _osc.bits      = uint8_t(std::min(std::max(_node.getNum("bits", 0.), 0.), 16.));
                _osc.duty      = getFloat(_node, "duty", 0.5f);
                _osc.pwm_depth = getFloat(_node, "pwm_depth", 0.f);
                _osc.pwm_rate  = getFloat(_node, "pwm_rate", 0.f);
                _osc.antialias = _node.getBool("antialias", true);

                std::vector<uint32_t> __values;
                std::istringstream __table(_node.getStr("table", ""));
                uint32_t __val;
                while (__table >> __val) __values.push_back(__val);
                if (__values.size() > SVKFW_SYNTH_MAX_TABLE)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: load", "%s: wavetable has %u values, only %u are used\n"),
                            _where.c_str(), unsigned(__values.size()), SVKFW_SYNTH_MAX_TABLE);
                _osc.setTable(__values, uint8_t(std::min(std::max(_node.getNum("table_bits", 4.), 1.), 8.)));
            }


            const uint32_t InstrumentLibrary::Format_Version;

            uint32_t InstrumentLibrary::size() const { return uint32_t(instruments.size()); }

            int32_t InstrumentLibrary::find(const std::string &_name) const {
                for (size_t i = 0u; i < instruments.size(); ++i)
                    if (instruments[i].name == _name) return int32_t(i);
                return -1;
            }

            std::string InstrumentLibrary::uniqueName(const std::string &_base) const {
                const std::string __base = _base.empty() ? "instrument" : _base;
                if (find(__base) < 0) return __base;
                for (uint32_t i = 2u; ; ++i) {
                    const std::string __name = __base + "_" + std::to_string(i);
                    if (find(__name) < 0) return __name;
                }
            }

            uint32_t InstrumentLibrary::add(const Instrument &_instrument) {
                // Note: the name is chosen before the push, else the new instrument would collide with itself
                const std::string __name = uniqueName(_instrument.name);
                instruments.push_back(_instrument);
                instruments.back().name = __name;
                is_modified = true;
                return size() - 1u;
            }

            uint32_t InstrumentLibrary::addDefault(const std::string &_name) {
                Instrument __instrument;
                __instrument.name = _name;
                return add(__instrument);
            }

            uint32_t InstrumentLibrary::duplicate(uint32_t _id) {
                SVKFW_ASSERT(_id < size(), std::out_of_range, "InstrumentLibrary :: duplicate", "no instrument " + std::to_string(_id));
                const Instrument __copy = instruments[_id]; // Note: copied, 'add' may reallocate the vector
                return add(__copy);
            }

            bool InstrumentLibrary::rename(uint32_t _id, const std::string &_name) {
                if (_id >= size() || _name.empty()) return false;
                const int32_t __other = find(_name);
                if (__other >= 0) return __other == int32_t(_id);
                instruments[_id].name = _name;
                is_modified = true;
                return true;
            }

            void InstrumentLibrary::remove(uint32_t _id) {
                if (_id >= size()) return;
                instruments.erase(instruments.begin() + _id);
                is_modified = true;
            }

            void InstrumentLibrary::clear() {
                instruments.clear();
                is_modified = true;
            }


            void InstrumentLibrary::toJSON(const Instrument &_instrument, JSONNode &_node) {
                const InstrumentParams &__params = _instrument.params;
                _node.switchValueObj();
                _node.addMember("name").setValueStr(_instrument.name);
                setFloat(_node.addMember("volume"), __params.volume);
                setFloat(_node.addMember("pan"), __params.pan);
                _node.addMember("octave").setValueNum(__params.octave);

                JSONNode &__oscillators = _node.addMember("oscillators");
                __oscillators.switchValueArr();
                for (uint32_t i = 0u; i < std::min(__params.num_oscillators, SVKFW_SYNTH_MAX_OSCILLATORS); ++i)
                    writeOscillator(__params.oscillators[i], __oscillators.pushBack());

                JSONNode &__amp_env = _node.addMember("amp_env");
                setFloat(__amp_env.addMember("attack"),  __params.amp_env.attack);
                setFloat(__amp_env.addMember("decay"),   __params.amp_env.decay);
                setFloat(__amp_env.addMember("sustain"), __params.amp_env.sustain);
                setFloat(__amp_env.addMember("release"), __params.amp_env.release);
                __amp_env.addMember("curve").setValueStr(curveName(__params.amp_env.curve));

                if (__params.vibrato.depth != 0.f) {
                    JSONNode &__vibrato = _node.addMember("vibrato");
                    setFloat(__vibrato.addMember("depth"), __params.vibrato.depth);
                    setFloat(__vibrato.addMember("rate"),  __params.vibrato.rate);
                    setFloat(__vibrato.addMember("delay"), __params.vibrato.delay);
                }
                if (__params.pitch_env.semitones != 0.f && __params.pitch_env.time > 0.f) {
                    JSONNode &__pitch_env = _node.addMember("pitch_env");
                    setFloat(__pitch_env.addMember("semitones"), __params.pitch_env.semitones);
                    setFloat(__pitch_env.addMember("time"),      __params.pitch_env.time);
                }
            }

            bool InstrumentLibrary::fromJSON(const JSONNode &_node, Instrument &_instrument) {
                if (_node.node_type != File::JSON_TYPE_OBJ) return false;
                _instrument = Instrument{};
                InstrumentParams &__params = _instrument.params;
                _instrument.name = _node.getStr("name", "instrument");
                __params.volume  = getFloat(_node, "volume", 0.8f);
                __params.pan     = getFloat(_node, "pan", 0.f);
                __params.octave  = int32_t(_node.getNum("octave", 0.));

                const JSONNode *__oscillators = _node.find("oscillators");
                __params.num_oscillators = 0u;
                if (__oscillators != nullptr && __oscillators->node_type == File::JSON_TYPE_ARR) {
                    if (__oscillators->value_arr_obj.size() > SVKFW_SYNTH_MAX_OSCILLATORS)
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: load", "'%s' has %u oscillators, only %u are used\n"),
                                _instrument.name.c_str(), unsigned(__oscillators->value_arr_obj.size()), SVKFW_SYNTH_MAX_OSCILLATORS);
                    for (size_t i = 0u; i < __oscillators->value_arr_obj.size() && __params.num_oscillators < SVKFW_SYNTH_MAX_OSCILLATORS; ++i)
                        if (__oscillators->value_arr_obj[i].node_type == File::JSON_TYPE_OBJ)
                            readOscillator(__oscillators->value_arr_obj[i], __params.oscillators[__params.num_oscillators++], "'" + _instrument.name + "'");
                }

                const JSONNode *__amp_env = _node.find("amp_env");
                if (__amp_env != nullptr) {
                    __params.amp_env.attack  = getFloat(*__amp_env, "attack",  __params.amp_env.attack);
                    __params.amp_env.decay   = getFloat(*__amp_env, "decay",   __params.amp_env.decay);
                    __params.amp_env.sustain = getFloat(*__amp_env, "sustain", __params.amp_env.sustain);
                    __params.amp_env.release = getFloat(*__amp_env, "release", __params.amp_env.release);
                    const std::string __curve = __amp_env->getStr("curve", "exp");
                    if (!curveFromName(__curve, __params.amp_env.curve))
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: load", "'%s': unknown curve '%s'\n"), _instrument.name.c_str(), __curve.c_str());
                }
                const JSONNode *__vibrato = _node.find("vibrato");
                if (__vibrato != nullptr) {
                    __params.vibrato.depth = getFloat(*__vibrato, "depth", 0.f);
                    __params.vibrato.rate  = getFloat(*__vibrato, "rate",  __params.vibrato.rate);
                    __params.vibrato.delay = getFloat(*__vibrato, "delay", 0.f);
                }
                const JSONNode *__pitch_env = _node.find("pitch_env");
                if (__pitch_env != nullptr) {
                    __params.pitch_env.semitones = getFloat(*__pitch_env, "semitones", 0.f);
                    __params.pitch_env.time      = getFloat(*__pitch_env, "time", 0.f);
                }
                return true;
            }


            bool InstrumentLibrary::load(const std::string &_fpath) {
                std::ifstream __file(_fpath.c_str(), std::ios::binary);
                if (!__file) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: load", "could not open '%s'\n"), _fpath.c_str());
                    return false;
                }
                std::stringstream __text;
                __text << __file.rdbuf();
                const std::string __str = __text.str();

                File::ContentJSON __json;
                __json.readFromString(__str.c_str());
                const JSONNode *__instruments = __json.root_node.find("instruments");
                if (__instruments == nullptr || __instruments->node_type != File::JSON_TYPE_ARR) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: load", "'%s' has no \"instruments\" array\n"), _fpath.c_str());
                    return false;
                }
                const uint32_t __version = uint32_t(__json.root_node.getNum("version", Format_Version));
                if (__version > Format_Version)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: load", "'%s' has version %u, newer than %u\n"), _fpath.c_str(), __version, Format_Version);

                instruments.clear();
                for (size_t i = 0u; i < __instruments->value_arr_obj.size(); ++i) {
                    Instrument __instrument;
                    if (!fromJSON(__instruments->value_arr_obj[i], __instrument)) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: load", "'%s': instrument %u is not an object\n"), _fpath.c_str(), unsigned(i));
                        continue;
                    }
                    if (find(__instrument.name) >= 0)
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: load", "'%s': duplicate name '%s' renamed\n"), _fpath.c_str(), __instrument.name.c_str());
                    add(__instrument);
                }
                fpath = _fpath;
                is_modified = false;
                return true;
            }

            bool InstrumentLibrary::save(const std::string &_fpath) {
                const std::string __fpath = _fpath.empty() ? fpath : _fpath;
                if (__fpath.empty()) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: save", "no file path\n"));
                    return false;
                }

                File::ContentJSON __json;
                __json.root_node.addMember("version").setValueNum(Format_Version);
                JSONNode &__instruments = __json.root_node.addMember("instruments");
                __instruments.switchValueArr();
                for (size_t i = 0u; i < instruments.size(); ++i)
                    toJSON(instruments[i], __instruments.pushBack());

                const std::string __tmp_fpath = __fpath + ".tmp";
                {
                    std::ofstream __file(__tmp_fpath.c_str(), std::ios::binary);
                    __file << __json.writeToString(2u);
                    if (!__file) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: save", "could not write '%s'\n"), __tmp_fpath.c_str());
                        return false;
                    }
                }
                // Note: 'rename' doesn't replace existing files on Windows
                std::remove(__fpath.c_str());
                if (std::rename(__tmp_fpath.c_str(), __fpath.c_str()) != 0) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("InstrumentLibrary :: save", "could not rename '%s' to '%s'\n"), __tmp_fpath.c_str(), __fpath.c_str());
                    return false;
                }
                fpath = __fpath;
                is_modified = false;
                return true;
            }
        }; // Synth END
    }; // Audio END
}; // Simple END
