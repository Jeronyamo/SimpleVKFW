#include "sound/tracker.h"

#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>


namespace Simple {
    namespace Audio {
        namespace Tracker {
            typedef File::ContentJSON::JSONNode JSONNode;

            static const char* const Note_Names[12] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-" };
            static const char* const Hex_Digits = "0123456789ABCDEF";

            static std::string hex2(uint8_t _value) { return std::string{Hex_Digits[_value >> 4], Hex_Digits[_value & 15u]}; }

            static int32_t hexDigit(char _ch) {
                if (_ch >= '0' && _ch <= '9') return _ch - '0';
                if (_ch >= 'A' && _ch <= 'F') return _ch - 'A' + 10;
                if (_ch >= 'a' && _ch <= 'f') return _ch - 'a' + 10;
                return -1;
            }

            static bool parseHex2(const std::string &_str, uint8_t &_value) {
                if (_str.size() != 2u) return false;
                const int32_t __hi = hexDigit(_str[0]), __lo = hexDigit(_str[1]);
                if (__hi < 0 || __lo < 0) return false;
                _value = uint8_t(__hi * 16 + __lo);
                return true;
            }

            static bool isDots(const std::string &_str) { return !_str.empty() && _str.find_first_not_of('.') == std::string::npos; }


            // Note: definitions for odr-used constants (std::min takes references), required before C++17
            const uint8_t Cell::Note_None, Cell::Note_Off, Cell::Note_Cut, Cell::Note_Max, Cell::Instrument_None, Cell::Volume_None, Cell::Volume_Max;
            const uint32_t Song::Format_Version;

            bool Cell::isEmpty() const { return note == Note_None && instrument == Instrument_None && volume == Volume_None && effect == 0; }

            bool Cell::hasNote() const { return note >= 1u && note <= Note_Max; }

            std::string Cell::noteToString(uint8_t _note) {
                if (_note == Note_Off) return "===";
                if (_note == Note_Cut) return "^^^";
                if (_note < 1u || _note > Note_Max) return "...";
                const uint32_t __id = _note - 1u;
                return std::string(Note_Names[__id % 12u]) + char('0' + __id / 12u);
            }

            bool Cell::noteFromString(const std::string &_str, uint8_t &_note) {
                if (_str == "===") { _note = Note_Off;  return true; }
                if (_str == "^^^") { _note = Note_Cut;  return true; }
                if (isDots(_str))  { _note = Note_None; return true; }
                if (_str.size() != 3u || _str[2] < '0' || _str[2] > '9') return false;
                for (uint32_t i = 0u; i < 12u; ++i)
                    if (std::toupper(_str[0]) == Note_Names[i][0] && _str[1] == Note_Names[i][1]) {
                        _note = uint8_t(1u + i + 12u * uint32_t(_str[2] - '0'));
                        return true;
                    }
                return false;
            }

            std::string Cell::toString() const {
                std::string __res = noteToString(note);
                __res += ' ';
                __res += instrument == Instrument_None ? ".." : hex2(instrument);
                __res += ' ';
                __res += volume == Volume_None ? ".." : hex2(volume);
                __res += ' ';
                __res += effect == 0 ? std::string("...") : effect + hex2(param);
                return __res;
            }

            bool Cell::parse(const std::string &_str, Cell &_cell) {
                std::istringstream __stream(_str);
                std::string __note, __instrument, __volume, __effect;
                __stream >> __note >> __instrument >> __volume >> __effect;
                Cell __res;
                if (!__note.empty() && !noteFromString(__note, __res.note)) return false;
                if (!__instrument.empty() && !isDots(__instrument) && !parseHex2(__instrument, __res.instrument)) return false;
                if (!__volume.empty() && !isDots(__volume)) {
                    if (!parseHex2(__volume, __res.volume)) return false;
                    __res.volume = std::min(__res.volume, Volume_Max);
                }
                if (!__effect.empty() && !isDots(__effect)) {
                    if (__effect.size() != 3u || !std::isalnum((unsigned char)__effect[0]) || !parseHex2(__effect.substr(1u), __res.param)) return false;
                    __res.effect = char(std::toupper(__effect[0]));
                }
                _cell = __res;
                return true;
            }


            void Pattern::resize(uint32_t _rows, uint32_t _old_channels, uint32_t _new_channels) {
                _rows = std::min(std::max(_rows, 1u), SVKFW_TRACKER_MAX_ROWS);
                std::vector<Cell> __cells(size_t(_rows) * _new_channels);
                for (uint32_t r = 0u; r < std::min(rows, _rows); ++r)
                    for (uint32_t ch = 0u; ch < std::min(_old_channels, _new_channels); ++ch)
                        if (size_t(r) * _old_channels + ch < cells.size())
                            __cells[size_t(r) * _new_channels + ch] = cells[size_t(r) * _old_channels + ch];
                rows = _rows;
                cells.swap(__cells);
            }


            Song::Song() {
                setNumChannels(4u);
                addPattern();
                order.push_back(0u);
            }

            uint32_t Song::numChannels() const { return uint32_t(channels.size()); }

            void Song::setNumChannels(uint32_t _num_channels) {
                _num_channels = std::min(std::max(_num_channels, 1u), SVKFW_TRACKER_MAX_CHANNELS);
                const uint32_t __old = numChannels();
                for (size_t i = 0u; i < patterns.size(); ++i)
                    patterns[i].resize(patterns[i].rows, __old, _num_channels);
                channels.resize(_num_channels);
                for (uint32_t i = __old; i < _num_channels; ++i)
                    channels[i].name = "Ch " + std::to_string(i + 1u);
            }

            uint32_t Song::addPattern(uint32_t _rows) {
                patterns.push_back(Pattern{});
                patterns.back().rows = 0u;
                patterns.back().resize(_rows, numChannels(), numChannels());
                return uint32_t(patterns.size()) - 1u;
            }

            void Song::setPatternRows(uint32_t _pattern, uint32_t _rows) {
                if (_pattern < patterns.size())
                    patterns[_pattern].resize(_rows, numChannels(), numChannels());
            }

            Cell& Song::cell(uint32_t _pattern, uint32_t _row, uint32_t _channel) {
                return patterns[_pattern].cells[size_t(_row) * numChannels() + _channel];
            }

            const Cell& Song::cell(uint32_t _pattern, uint32_t _row, uint32_t _channel) const {
                return patterns[_pattern].cells[size_t(_row) * numChannels() + _channel];
            }

            int32_t Song::findInstrument(const std::string &_name) const {
                for (size_t i = 0u; i < instruments.size(); ++i)
                    if (instruments[i].name == _name) return int32_t(i);
                return -1;
            }

            uint8_t Song::addInstrument(const Synth::Instrument &_instrument) {
                if (instruments.size() >= Cell::Instrument_None) return Cell::Instrument_None;
                std::string __name = _instrument.name.empty() ? "instrument" : _instrument.name;
                for (uint32_t i = 2u; findInstrument(__name) >= 0; ++i)
                    __name = _instrument.name + "_" + std::to_string(i);
                instruments.push_back(_instrument);
                instruments.back().name = __name;
                return uint8_t(instruments.size() - 1u);
            }

            uint8_t Song::setInstrument(const Synth::Instrument &_instrument) {
                const int32_t __id = findInstrument(_instrument.name);
                if (__id < 0) return addInstrument(_instrument);
                instruments[__id] = _instrument;
                return uint8_t(__id);
            }

            float Song::rowSeconds() const { return 60.f / (std::max(bpm, 1.f) * float(std::max(rows_per_beat, 1u))); }

            static void setFloat(JSONNode &_node, float _value) {
                char __buf[32]{};
                snprintf(__buf, sizeof(__buf), "%.6g", double(_value));
                _node.setValueNum(std::strtod(__buf, nullptr));
            }

            bool Song::save(const std::string &_fpath) {
                const std::string __fpath = _fpath.empty() ? fpath : _fpath;
                if (__fpath.empty()) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Tracker::Song :: save", "no file path\n"));
                    return false;
                }

                File::ContentJSON __json;
                JSONNode &__root = __json.root_node;
                __root.addMember("version").setValueNum(Format_Version);
                __root.addMember("name").setValueStr(name);
                {
                    JSONNode &__tuning = __root.addMember("tuning");
                    __tuning.addMember("system").setValueStr(tuning == Music::JUST_INTONATION ? "just" : "12tet");
                    setFloat(__tuning.addMember("ref_freq"), ref_freq);
                }
                setFloat(__root.addMember("bpm"), bpm);
                __root.addMember("rows_per_beat").setValueNum(rows_per_beat);
                __root.addMember("ticks_per_row").setValueNum(ticks_per_row);
                __root.addMember("restart_order").setValueNum(restart_order);
                {
                    JSONNode &__echo = __root.addMember("echo");
                    setFloat(__echo.addMember("delay"),    echo.delay);
                    setFloat(__echo.addMember("feedback"), echo.feedback);
                    setFloat(__echo.addMember("volume"),   echo.volume);
                    setFloat(__echo.addMember("lowpass"),  echo.lowpass);
                }
                {
                    JSONNode &__instruments = __root.addMember("instruments");
                    __instruments.switchValueArr();
                    for (size_t i = 0u; i < instruments.size(); ++i)
                        Synth::InstrumentLibrary::toJSON(instruments[i], __instruments.pushBack());
                }
                {
                    JSONNode &__channels = __root.addMember("channels");
                    __channels.switchValueArr();
                    for (size_t i = 0u; i < channels.size(); ++i) {
                        JSONNode &__channel = __channels.pushBack();
                        __channel.addMember("name").setValueStr(channels[i].name);
                        setFloat(__channel.addMember("volume"), channels[i].volume);
                        setFloat(__channel.addMember("pan"), channels[i].pan);
                        __channel.addMember("muted").setValueBool(channels[i].muted);
                        __channel.addMember("echo").setValueBool(channels[i].echo);
                    }
                }
                {
                    JSONNode &__order = __root.addMember("order");
                    __order.switchValueArr();
                    for (size_t i = 0u; i < order.size(); ++i)
                        __order.pushBack().setValueNum(order[i]);
                }
                {
                    // Note: a row is one string, so patterns stay readable and diffable
                    JSONNode &__patterns = __root.addMember("patterns");
                    __patterns.switchValueArr();
                    for (size_t p = 0u; p < patterns.size(); ++p) {
                        JSONNode &__pattern = __patterns.pushBack();
                        __pattern.addMember("rows").setValueNum(patterns[p].rows);
                        JSONNode &__data = __pattern.addMember("data");
                        __data.switchValueArr();
                        for (uint32_t r = 0u; r < patterns[p].rows; ++r) {
                            std::string __row;
                            for (uint32_t ch = 0u; ch < numChannels(); ++ch)
                                __row += (ch ? " | " : "") + cell(uint32_t(p), r, ch).toString();
                            __data.pushBack().setValueStr(__row);
                        }
                    }
                }

                const std::string __tmp_fpath = __fpath + ".tmp";
                {
                    std::ofstream __file(__tmp_fpath.c_str(), std::ios::binary);
                    __file << __json.writeToString(2u);
                    if (!__file) {
                        fprintf(svkfwwarn, SVKFW_WRAPWARN("Tracker::Song :: save", "could not write '%s'\n"), __tmp_fpath.c_str());
                        return false;
                    }
                }
                // Note: 'rename' doesn't replace existing files on Windows
                std::remove(__fpath.c_str());
                if (std::rename(__tmp_fpath.c_str(), __fpath.c_str()) != 0) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Tracker::Song :: save", "could not rename '%s' to '%s'\n"), __tmp_fpath.c_str(), __fpath.c_str());
                    return false;
                }
                fpath = __fpath;
                return true;
            }

            bool Song::load(const std::string &_fpath) {
                std::ifstream __file(_fpath.c_str(), std::ios::binary);
                if (!__file) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Tracker::Song :: load", "could not open '%s'\n"), _fpath.c_str());
                    return false;
                }
                std::stringstream __text;
                __text << __file.rdbuf();
                const std::string __str = __text.str();

                File::ContentJSON __json;
                __json.readFromString(__str.c_str());
                const JSONNode &__root = __json.root_node;
                const JSONNode *__patterns = __root.find("patterns");
                if (__root.node_type != File::JSON_TYPE_OBJ || __patterns == nullptr || __patterns->node_type != File::JSON_TYPE_ARR) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("Tracker::Song :: load", "'%s' has no \"patterns\" array\n"), _fpath.c_str());
                    return false;
                }

                Song __song;
                __song.patterns.clear();
                __song.order.clear();
                __song.name         = __root.getStr("name", "Untitled");
                if (const JSONNode *__tuning = __root.find("tuning")) {
                    __song.tuning   = __tuning->getStr("system", "12tet") == "just" ? Music::JUST_INTONATION : Music::EQUAL_TEMPERAMENT_12T;
                    __song.ref_freq = float(__tuning->getNum("ref_freq", 440.));
                }
                __song.bpm           = float(__root.getNum("bpm", 125.));
                __song.rows_per_beat = uint32_t(std::max(__root.getNum("rows_per_beat", 4.), 1.));
                __song.ticks_per_row = uint32_t(std::max(__root.getNum("ticks_per_row", 6.), 1.));
                __song.restart_order = uint32_t(std::max(__root.getNum("restart_order", 0.), 0.));
                if (const JSONNode *__echo = __root.find("echo")) {
                    __song.echo.delay    = std::min(float(__echo->getNum("delay", __song.echo.delay)), SVKFW_TRACKER_MAX_ECHO);
                    __song.echo.feedback = float(__echo->getNum("feedback", __song.echo.feedback));
                    __song.echo.volume   = float(__echo->getNum("volume",   __song.echo.volume));
                    __song.echo.lowpass  = float(__echo->getNum("lowpass",  __song.echo.lowpass));
                }
                if (const JSONNode *__instruments = __root.find("instruments"))
                    if (__instruments->node_type == File::JSON_TYPE_ARR)
                        for (size_t i = 0u; i < __instruments->value_arr_obj.size(); ++i) {
                            // Note: invalid entries stay as default instruments, so the cell indices keep pointing at the right ones
                            Synth::Instrument __instrument;
                            if (!Synth::InstrumentLibrary::fromJSON(__instruments->value_arr_obj[i], __instrument))
                                fprintf(svkfwwarn, SVKFW_WRAPWARN("Tracker::Song :: load", "'%s': instrument %u is not an object\n"), _fpath.c_str(), unsigned(i));
                            __song.instruments.push_back(__instrument);
                        }

                const JSONNode *__channels = __root.find("channels");
                const uint32_t __num_channels = (__channels != nullptr && __channels->node_type == File::JSON_TYPE_ARR) ? uint32_t(__channels->value_arr_obj.size()) : 4u;
                __song.channels.clear();
                __song.setNumChannels(__num_channels);
                for (uint32_t i = 0u; __channels != nullptr && i < __song.numChannels() && i < __channels->value_arr_obj.size(); ++i) {
                    const JSONNode &__channel = __channels->value_arr_obj[i];
                    ChannelSettings &__settings = __song.channels[i];
                    __settings.name   = __channel.getStr("name", __settings.name);
                    __settings.volume = float(__channel.getNum("volume", 1.));
                    __settings.pan    = float(__channel.getNum("pan", 0.));
                    __settings.muted  = __channel.getBool("muted", false);
                    __settings.echo   = __channel.getBool("echo", false);
                }

                for (size_t p = 0u; p < __patterns->value_arr_obj.size(); ++p) {
                    const JSONNode &__pattern = __patterns->value_arr_obj[p];
                    const uint32_t __id = __song.addPattern(uint32_t(std::max(__pattern.getNum("rows", 64.), 1.)));
                    const JSONNode *__data = __pattern.find("data");
                    if (__data == nullptr || __data->node_type != File::JSON_TYPE_ARR) continue;
                    for (uint32_t r = 0u; r < __song.patterns[__id].rows && r < __data->value_arr_obj.size(); ++r) {
                        if (__data->value_arr_obj[r].node_type != File::JSON_TYPE_STR) continue;
                        const std::string &__row = __data->value_arr_obj[r].value_str;
                        size_t __begin = 0u;
                        for (uint32_t ch = 0u; ch < __song.numChannels() && __begin <= __row.size(); ++ch) {
                            size_t __end = __row.find('|', __begin);
                            if (__end == std::string::npos) __end = __row.size();
                            if (!Cell::parse(__row.substr(__begin, __end - __begin), __song.cell(__id, r, ch)))
                                fprintf(svkfwwarn, SVKFW_WRAPWARN("Tracker::Song :: load", "'%s': bad cell in pattern %u, row %u, channel %u\n"),
                                        _fpath.c_str(), __id, r, ch);
                            __begin = __end + 1u;
                        }
                    }
                }
                if (__song.patterns.empty()) __song.addPattern();

                if (const JSONNode *__order = __root.find("order"))
                    if (__order->node_type == File::JSON_TYPE_ARR)
                        for (size_t i = 0u; i < __order->value_arr_obj.size(); ++i)
                            if (__order->value_arr_obj[i].node_type == File::JSON_TYPE_NUM)
                                __song.order.push_back(std::min(uint32_t(std::max(__order->value_arr_obj[i].value_num, 0.)), uint32_t(__song.patterns.size() - 1u)));
                if (__song.order.empty()) __song.order.push_back(0u);

                __song.fpath = _fpath;
                *this = __song;
                return true;
            }


    //  == === ====     SongPlayer     ==== === ==  \\

            SongPlayer::SongPlayer() : playing{false}, play_order{0u}, play_row{0u} { setSampleRate(sample_rate); }

            SongPlayer::~SongPlayer() {}

            void SongPlayer::setSong(Song *_song) {
                std::lock_guard<std::mutex> __lock(mutex);
                song = _song;
                playing = false;
                resetChannels_();
                refreshTuning_();
            }

            void SongPlayer::refreshTuning_() {
                if (song != nullptr)
                    note_handler = Music::NoteHandler(song->tuning, Music::NoteReference(Music::Note::A, 4, song->ref_freq));
            }

            void SongPlayer::setSampleRate(uint32_t _sample_rate) {
                sample_rate = std::max(_sample_rate, 1u);
                for (uint32_t i = 0u; i < SVKFW_TRACKER_MAX_CHANNELS; ++i) {
                    channels[i].voice.setSampleRate(sample_rate);
                    channels[i].tail.setSampleRate(sample_rate);
                }
                echo_buffer.assign(size_t(SVKFW_TRACKER_MAX_ECHO * sample_rate) + 1u, vec2f{});
                echo_pos = 0u;
            }

            void SongPlayer::resetChannels_() {
                for (uint32_t i = 0u; i < SVKFW_TRACKER_MAX_CHANNELS; ++i) {
                    ChannelState &__state = channels[i];
                    __state.voice.cut();
                    __state.tail.cut();
                    __state.instrument = -1;
                    __state.volume = 1.f;
                    __state.pan = __state.slide = __state.porta_target = __state.vibrato_pos = 0.f;
                    __state.cell = __state.delayed = Cell{};
                    __state.delay_tick = 0u;
                    __state.mem_slide_up = __state.mem_slide_down = __state.mem_porta = __state.mem_vibrato = __state.mem_volslide = 0u;
                }
                std::fill(echo_buffer.begin(), echo_buffer.end(), vec2f{});
                echo_lp = vec2f{};
            }

            void SongPlayer::play(uint32_t _order, uint32_t _row) {
                std::lock_guard<std::mutex> __lock(mutex);
                if (song == nullptr) return;
                resetChannels_();
                pattern_mode = false;
                play_order = std::min(_order, uint32_t(song->order.size() - 1u));
                play_row = std::min(_row, song->patterns[currentPattern_()].rows - 1u);
                tick = 0u;
                bpm = song->bpm;
                ticks_per_row = song->ticks_per_row;
                samples_to_tick = 0.;
                jump_order = jump_row = queued_order = -1;
                playing = true;
            }

            void SongPlayer::playPattern(uint32_t _pattern, uint32_t _row) {
                std::lock_guard<std::mutex> __lock(mutex);
                if (song == nullptr) return;
                resetChannels_();
                pattern_mode = true;
                pattern_mode_id = std::min(_pattern, uint32_t(song->patterns.size() - 1u));
                play_row = std::min(_row, song->patterns[pattern_mode_id].rows - 1u);
                tick = 0u;
                bpm = song->bpm;
                ticks_per_row = song->ticks_per_row;
                samples_to_tick = 0.;
                jump_order = jump_row = queued_order = -1;
                playing = true;
            }

            void SongPlayer::stop() {
                std::lock_guard<std::mutex> __lock(mutex);
                playing = false;
                for (uint32_t i = 0u; i < SVKFW_TRACKER_MAX_CHANNELS; ++i)
                    channels[i].voice.noteOff();
            }

            void SongPlayer::stopNow() {
                std::lock_guard<std::mutex> __lock(mutex);
                playing = false;
                resetChannels_();
            }

            void SongPlayer::jumpToOrder(uint32_t _order, bool _at_pattern_end) {
                std::lock_guard<std::mutex> __lock(mutex);
                if (song == nullptr || _order >= song->order.size()) return;
                if (_at_pattern_end) {
                    queued_order = int32_t(_order);
                    return;
                }
                pattern_mode = false;
                play_order = _order;
                play_row = 0u;
                tick = 0u;
                samples_to_tick = 0.;
            }

            bool SongPlayer::isSilent() {
                std::lock_guard<std::mutex> __lock(mutex);
                if (playing) return false;
                for (uint32_t i = 0u; i < SVKFW_TRACKER_MAX_CHANNELS; ++i)
                    if (channels[i].voice.isActive() || channels[i].tail.isActive()) return false;
                return true;
            }

            void SongPlayer::previewCell(uint32_t _channel, const Cell &_cell) {
                std::lock_guard<std::mutex> __lock(mutex);
                if (song == nullptr || _channel >= song->numChannels()) return;
                channels[_channel].cell = Cell{};
                triggerCell_(_channel, _cell);
                channels[_channel].voice.velocity = channels[_channel].volume;
            }

            uint32_t SongPlayer::currentPattern_() const {
                const uint32_t __pattern = pattern_mode ? pattern_mode_id : (play_order < song->order.size() ? song->order[play_order] : 0u);
                return std::min(__pattern, uint32_t(song->patterns.size() - 1u));
            }

            const Synth::InstrumentParams* SongPlayer::instrumentParams_(int32_t _song_instrument) const {
                if (song == nullptr || _song_instrument < 0 || size_t(_song_instrument) >= song->instruments.size()) return nullptr;
                return &song->instruments[_song_instrument].params;
            }

            void SongPlayer::triggerCell_(uint32_t _ch, const Cell &_cell) {
                ChannelState &__state = channels[_ch];
                // Note: as in XM, an instrument without a volume resets the volume
                if (_cell.instrument != Cell::Instrument_None) {
                    __state.instrument = _cell.instrument;
                    __state.volume = 1.f;
                }
                if (_cell.volume != Cell::Volume_None)
                    __state.volume = float(std::min(_cell.volume, Cell::Volume_Max)) / Cell::Volume_Max;

                if (_cell.note == Cell::Note_Off)
                    __state.voice.noteOff();
                else if (_cell.note == Cell::Note_Cut)
                    __state.voice.cut();
                else if (_cell.hasNote()) {
                    const float __freq = note_handler.getNoteFreq(Music::NoteFullId(_cell.note - 1u));
                    if (_cell.effect == '3' && __state.voice.isActive() && __state.base_freq > 0.f) {
                        // Portamento slides to the note instead of playing it
                        __state.porta_target = 12.f * std::log2(__freq / __state.base_freq);
                        return;
                    }
                    const Synth::InstrumentParams *__params = instrumentParams_(__state.instrument);
                    if (__params == nullptr) return;
                    if (__state.voice.isActive()) {
                        __state.tail = __state.voice;
                        __state.tail.noteOff();
                    }
                    __state.voice.noteOn(*__params, __freq, __state.volume);
                    __state.base_freq = __freq;
                    __state.slide = __state.porta_target = __state.vibrato_pos = 0.f;
                }
            }

            void SongPlayer::applyEffectTick_(uint32_t _ch) {
                ChannelState &__state = channels[_ch];
                const uint8_t __param = __state.cell.param;
                float __arpeggio = 0.f, __vibrato = 0.f;

                switch (__state.cell.effect) {
                    case '0': // Arpeggio: note, +x, +y semitones in turn
                        if (__param != 0u) {
                            const uint32_t __step = tick % 3u;
                            __arpeggio = __step == 1u ? float(__param >> 4) : (__step == 2u ? float(__param & 15u) : 0.f);
                        }
                        break;
                    case '1': // Slide up, xx/16 semitones per tick
                        if (__param != 0u) __state.mem_slide_up = __param;
                        if (tick > 0u) __state.slide += __state.mem_slide_up / 16.f;
                        break;
                    case '2': // Slide down
                        if (__param != 0u) __state.mem_slide_down = __param;
                        if (tick > 0u) __state.slide -= __state.mem_slide_down / 16.f;
                        break;
                    case '3': // Portamento to the note, xx/16 semitones per tick
                        if (__param != 0u) __state.mem_porta = __param;
                        if (tick > 0u) {
                            const float __step = __state.mem_porta / 16.f;
                            __state.slide = __state.slide < __state.porta_target ? std::min(__state.slide + __step, __state.porta_target)
                                                                                 : std::max(__state.slide - __step, __state.porta_target);
                        }
                        break;
                    case '4': // Vibrato: x - speed (x/64 cycles per tick), y - depth (y/8 semitones)
                        if (__param != 0u) __state.mem_vibrato = __param;
                        if (tick > 0u) __state.vibrato_pos += (__state.mem_vibrato >> 4) / 64.f;
                        __vibrato = std::sin(6.2831853f * __state.vibrato_pos) * (__state.mem_vibrato & 15u) / 8.f;
                        break;
                    case '8': // Pan, 80 - center
                        __state.pan = (float(__param) - 128.f) / 128.f;
                        break;
                    case 'A': // Volume slide: x up, y down, 1/64 per tick
                        if (__param != 0u) __state.mem_volslide = __param;
                        if (tick > 0u)
                            __state.volume = std::min(std::max(__state.volume + (float(__state.mem_volslide >> 4) - float(__state.mem_volslide & 15u)) / 64.f, 0.f), 1.f);
                        break;
                    case 'E': // ECx - note cut at tick x
                        if ((__param >> 4) == 0xCu && tick == (__param & 15u))
                            __state.voice.cut();
                        break;
                    default: break;
                }
                __state.voice.pitch_offset = __state.slide + __arpeggio + __vibrato;
                __state.voice.velocity = __state.volume;
            }

            void SongPlayer::processTick_() {
                const uint32_t __pattern = currentPattern_();
                const uint32_t __num_channels = song->numChannels();

                if (tick == 0u) {
                    const uint32_t __row = std::min(uint32_t(play_row), song->patterns[__pattern].rows - 1u);
                    for (uint32_t ch = 0u; ch < __num_channels; ++ch) {
                        const Cell &__cell = song->cell(__pattern, __row, ch);
                        ChannelState &__state = channels[ch];
                        __state.cell = __cell;
                        __state.delay_tick = 0u;
                        if (__cell.effect == 'E' && (__cell.param >> 4) == 0xDu && (__cell.param & 15u) > 0u) {
                            __state.delayed = __cell;
                            __state.delay_tick = __cell.param & 15u;
                        }
                        else triggerCell_(ch, __cell);

                        switch (__cell.effect) {
                            case 'B': jump_order = __cell.param; break;
                            case 'D': jump_row   = __cell.param; break;
                            case 'F':
                                if (__cell.param > 0u && __cell.param < 0x20u) ticks_per_row = __cell.param;
                                else if (__cell.param >= 0x20u) bpm = float(__cell.param);
                                break;
                            default: break;
                        }
                    }
                }

                for (uint32_t ch = 0u; ch < __num_channels; ++ch) {
                    ChannelState &__state = channels[ch];
                    if (__state.delay_tick > 0u && tick == __state.delay_tick) {
                        triggerCell_(ch, __state.delayed);
                        __state.delay_tick = 0u;
                    }
                    applyEffectTick_(ch);
                }

                if (++tick >= ticks_per_row) {
                    tick = 0u;
                    nextRow_();
                }
            }

            void SongPlayer::nextRow_() {
                uint32_t __order = play_order, __row = play_row;
                if (pattern_mode) {
                    __row = jump_row >= 0 ? uint32_t(jump_row) : __row + 1u;
                    if (__row >= song->patterns[pattern_mode_id].rows) {
                        __row = 0u;
                        if (!loop) playing = false;
                    }
                }
                else {
                    if (jump_order >= 0 || jump_row >= 0) {
                        __order = jump_order >= 0 ? uint32_t(jump_order) : __order + 1u;
                        __row   = jump_row   >= 0 ? uint32_t(jump_row)   : 0u;
                    }
                    else if (++__row >= song->patterns[currentPattern_()].rows) {
                        __row = 0u;
                        __order = queued_order >= 0 ? uint32_t(queued_order) : __order + 1u;
                        queued_order = -1;
                    }
                    if (__order >= song->order.size()) {
                        __order = std::min(song->restart_order, uint32_t(song->order.size() - 1u));
                        if (!loop) playing = false;
                    }
                }
                jump_order = jump_row = -1;
                play_order = __order;
                play_row = std::min(__row, song->patterns[currentPattern_()].rows - 1u);

                if (!playing)
                    for (uint32_t i = 0u; i < SVKFW_TRACKER_MAX_CHANNELS; ++i)
                        channels[i].voice.noteOff();
            }

            vec2f SongPlayer::renderFrame() {
                std::lock_guard<std::mutex> __lock(mutex);
                if (song == nullptr) return vec2f{};

                if (playing) {
                    if (samples_to_tick <= 0.) {
                        processTick_();
                        samples_to_tick += double(sample_rate) * 60. / (double(std::max(bpm, 1.f)) * std::max(song->rows_per_beat, 1u) * std::max(ticks_per_row, 1u));
                    }
                    samples_to_tick -= 1.;
                }

                bool __any_solo = false;
                const uint32_t __num_channels = song->numChannels();
                for (uint32_t ch = 0u; ch < __num_channels; ++ch)
                    __any_solo = __any_solo || song->channels[ch].solo;

                const float __peak_decay = 1.f - 8.f / sample_rate; // About 1/8 s to fall
                vec2f __mix{}, __echo_in{};
                for (uint32_t ch = 0u; ch < __num_channels; ++ch) {
                    ChannelState &__state = channels[ch];
                    const ChannelSettings &__settings = song->channels[ch];
                    const float __val = __state.voice.render() + __state.tail.render();
                    const vec2f __out = (__val * __settings.volume) * Synth::panGains(__settings.pan + __state.pan + __state.voice.params.pan);
                    __state.peak = vec2f{ std::max(__state.peak.x * __peak_decay, std::fabs(__out.x)), std::max(__state.peak.y * __peak_decay, std::fabs(__out.y)) };
                    if (__settings.muted || (__any_solo && !__settings.solo)) continue;
                    __mix += __out;
                    if (__settings.echo) __echo_in += __out;
                }

                if (echo_buffer.size() > 1u) {
                    const EchoSettings &__echo = song->echo;
                    const uint32_t __size  = uint32_t(echo_buffer.size());
                    const uint32_t __delay = std::min(std::max(uint32_t(__echo.delay * sample_rate), 1u), __size - 1u);
                    const float __lowpass = std::min(std::max(__echo.lowpass, 0.f), 0.99f);
                    echo_lp = echo_lp * __lowpass + echo_buffer[(echo_pos + __size - __delay) % __size] * (1.f - __lowpass);
                    echo_buffer[echo_pos] = __echo_in + echo_lp * std::min(std::max(__echo.feedback, 0.f), 0.95f);
                    echo_pos = (echo_pos + 1u) % __size;
                    __mix += echo_lp * __echo.volume;
                }

                // Note: soft clipping, almost linear below 0.3
                __mix *= master_volume;
                return vec2f{ std::tanh(__mix.x), std::tanh(__mix.y) };
            }

            void SongPlayer::render(std::vector<vec2f> &_frames, uint32_t _num_frames) {
                _frames.resize(_num_frames);
                for (uint32_t i = 0u; i < _num_frames; ++i)
                    _frames[i] = renderFrame();
            }

            std::vector<vec2f> SongPlayer::renderSong(Song &_song, uint32_t _sample_rate, float _max_seconds) {
                SongPlayer __player;
                __player.setSampleRate(_sample_rate);
                __player.setSong(&_song);
                __player.loop = false;
                __player.play();

                // Note: renders until the notes and the echo fade out
                const size_t __max_frames = size_t(std::max(_max_seconds, 0.f) * _sample_rate);
                uint32_t __quiet_frames = 0u;
                std::vector<vec2f> __res;
                while (__res.size() < __max_frames) {
                    const vec2f __frame = __player.renderFrame();
                    __res.push_back(__frame);
                    const bool __quiet = std::fabs(__frame.x) < 1e-4f && std::fabs(__frame.y) < 1e-4f;
                    __quiet_frames = __quiet ? __quiet_frames + 1u : 0u;
                    if (__quiet_frames > _sample_rate / 10u && __player.isSilent()) break;
                }
                return __res;
            }

            float SongPlayer::sample(float _t) {
                const vec2f __frame = renderFrame();
                return 0.5f * (__frame.x + __frame.y);
            }

            vec2i16 SongPlayer::sample2(double _t) {
                const vec2f __frame = Math::clampCL(renderFrame(), vec2f{-1.f}, vec2f{1.f}) * float(INT16_MAX);
                return __frame.cast<int16_t>();
            }
        }; // Tracker END
    }; // Audio END
}; // Simple END
