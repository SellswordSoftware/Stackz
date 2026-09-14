#ifndef PDNA_AUDIO_H
#define PDNA_AUDIO_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "pd_api.h"

/*
 * PDNA's native Playdate data contract. C modules contain only
 * static instances of these types; this header deliberately owns no SDK
 * objects and performs no runtime parsing.
 */

#define PDNA_TRACK_COUNT 4u
#define PDNA_EFFECT_VOICE_COUNT 2u
#define PDNA_MAX_SONG_VOICES 8u

typedef enum {
    PDNA_WAVEFORM_SQUARE,
    PDNA_WAVEFORM_TRIANGLE,
    PDNA_WAVEFORM_SINE,
    PDNA_WAVEFORM_NOISE,
    PDNA_WAVEFORM_SAWTOOTH,
} PdnaWaveform;

typedef enum {
    PDNA_LFO_SQUARE,
    PDNA_LFO_TRIANGLE,
    PDNA_LFO_SINE,
    PDNA_LFO_SAMPLE_AND_HOLD,
    PDNA_LFO_SAWTOOTH_UP,
    PDNA_LFO_SAWTOOTH_DOWN,
} PdnaLfoShape;

typedef struct {
    float scaling;
    MIDINote start_note;
    MIDINote end_note;
} PdnaRateScaling;

typedef struct {
    float attack_s;
    float decay_s;
    float sustain;
    float release_s;
    float curvature;
    float velocity_sensitivity;
    bool legato;
    bool retrigger;
    const PdnaRateScaling *rate_scaling;
} PdnaEnvelopePreset;

typedef struct {
    PdnaLfoShape shape;
    float rate_hz;
    float center;
    float depth;
    float holdoff_s;
    float ramp_s;
    float start_phase;
    bool retrigger;
} PdnaLfoPreset;

typedef struct {
    float start_semitones;
    float end_semitones;
    float duration_s;
} PdnaPitchMotion;

typedef enum {
    PDNA_PITCH_SOURCE_NONE,
    PDNA_PITCH_SOURCE_VIBRATO,
    PDNA_PITCH_SOURCE_MOTION,
} PdnaPitchSourceKind;

typedef struct {
    PdnaPitchSourceKind kind;
    union {
        PdnaLfoPreset vibrato;
        PdnaPitchMotion motion;
    } value;
} PdnaPitchSource;

typedef struct {
    PdnaWaveform waveform;
    PdnaEnvelopePreset envelope;
    float volume;
    PdnaPitchSource pitch_source;
    const PdnaLfoPreset *amplitude_lfo;
} PdnaVoicePreset;

typedef struct {
    PdnaVoicePreset voice;
    MIDINote note;
    float velocity;
    float duration_s;
} PdnaEffectPreset;

typedef struct {
    uint32_t step;
    uint32_t length;
    MIDINote note;
    float velocity;
} PdnaNoteEvent;

typedef struct {
    uint32_t step;
    float semitones;
    bool interpolate;
} PdnaPitchPoint;

typedef struct {
    MIDINote first;
    MIDINote last;
} PdnaKeyRange;

typedef struct {
    PdnaKeyRange key_range;
    PdnaVoicePreset voice;
} PdnaInstrumentVoicePreset;

typedef struct {
    float volume;
    const PdnaInstrumentVoicePreset *voices;
    size_t voice_count;
    const PdnaNoteEvent *notes;
    size_t note_count;
    const PdnaPitchPoint *pitch_automation;
    size_t pitch_automation_count;
} PdnaTrackPreset;

typedef struct {
    float steps_per_second;
    uint32_t length_steps;
    uint32_t loop_start_step;
    uint32_t loop_end_step_inclusive;
    PdnaTrackPreset tracks[PDNA_TRACK_COUNT];
} PdnaSongPreset;

typedef enum {
    PDNA_AUDIO_OK,
    PDNA_AUDIO_ERROR_INVALID_ARGUMENT,
    PDNA_AUDIO_ERROR_NO_DEFAULT_CHANNEL,
    PDNA_AUDIO_ERROR_NEW_EFFECT_SYNTH_FAILED,
    PDNA_AUDIO_ERROR_NEW_EFFECT_LFO_FAILED,
    PDNA_AUDIO_ERROR_ADD_EFFECT_VOICE_FAILED,
    PDNA_AUDIO_ERROR_INVALID_SONG,
    PDNA_AUDIO_ERROR_NEW_MUSIC_SYNTH_FAILED,
    PDNA_AUDIO_ERROR_NEW_MUSIC_LFO_FAILED,
    PDNA_AUDIO_ERROR_NEW_INSTRUMENT_FAILED,
    PDNA_AUDIO_ERROR_ADD_INSTRUMENT_SOURCE_FAILED,
    PDNA_AUDIO_ERROR_ADD_SONG_VOICE_FAILED,
    PDNA_AUDIO_ERROR_NEW_SEQUENCE_FAILED,
    PDNA_AUDIO_ERROR_NEW_TRACK_FAILED,
    PDNA_AUDIO_ERROR_NEW_PITCH_CONTROL_SIGNAL_FAILED,
    PDNA_AUDIO_ERROR_MULTIPLE_LEGATO_PHRASE_VOICES,
    PDNA_AUDIO_ERROR_LEGATO_VOICE_HAS_PITCH_SOURCE,
} PdnaAudioResult;

typedef struct {
    const struct playdate_sound *sound;
    SoundChannel *channel;
    PDSynth *synths[PDNA_EFFECT_VOICE_COUNT];
    PDSynthLFO *pitch_lfos[PDNA_EFFECT_VOICE_COUNT];
    PDSynthLFO *amplitude_lfos[PDNA_EFFECT_VOICE_COUNT];
    size_t voice_count;
    size_t next_voice;
} PdnaEffectPlayer;

typedef struct {
    const struct playdate_sound *sound;
    SoundChannel *channel;
    SoundSequence *sequence;
    PDSynth *synths[PDNA_MAX_SONG_VOICES];
    PDSynthLFO *pitch_lfos[PDNA_MAX_SONG_VOICES];
    PDSynthLFO *amplitude_lfos[PDNA_MAX_SONG_VOICES];
    PDSynthInstrument *instruments[PDNA_TRACK_COUNT];
    size_t track_voice_starts[PDNA_TRACK_COUNT];
    size_t track_voice_counts[PDNA_TRACK_COUNT];
    size_t voice_count;
    size_t instrument_count;
} PdnaSongPlayer;

typedef struct {
    PdnaEffectPlayer effect_player;
    PdnaSongPlayer song_player;
} PdnaAudio;

typedef enum {
    PDNA_SONG_VALID,
    PDNA_SONG_ERROR_INVALID_ARGUMENT,
    PDNA_SONG_ERROR_INVALID_TEMPO,
    PDNA_SONG_ERROR_INVALID_LENGTH,
    PDNA_SONG_ERROR_INVALID_LOOP,
    PDNA_SONG_ERROR_EMPTY_TRACK_VOICES,
    PDNA_SONG_ERROR_INVALID_KEY_RANGE,
    PDNA_SONG_ERROR_OVERLAPPING_KEY_RANGE,
    PDNA_SONG_ERROR_TOO_MANY_VOICES,
    PDNA_SONG_ERROR_UNMAPPED_NOTE,
    PDNA_SONG_ERROR_NOTE_EVENTS_OUT_OF_ORDER,
    PDNA_SONG_ERROR_INVALID_NOTE_LENGTH,
    PDNA_SONG_ERROR_NOTE_OUT_OF_BOUNDS,
    PDNA_SONG_ERROR_INVALID_PITCH_AUTOMATION,
    PDNA_SONG_ERROR_PITCH_AUTOMATION_OUT_OF_ORDER,
    PDNA_SONG_ERROR_CONFLICTING_PITCH_MODULATION,
} PdnaSongValidationError;

SoundWaveform pdna_native_waveform(PdnaWaveform waveform);
LFOType pdna_native_lfo_shape(PdnaLfoShape shape);
void pdna_configure_voice(
    const struct playdate_sound *sound,
    PDSynth *synth,
    PDSynthLFO *pitch_lfo,
    PDSynthLFO *amplitude_lfo,
    const PdnaVoicePreset *voice
);
PdnaAudioResult pdna_effect_player_init(
    PdnaEffectPlayer *player,
    const struct playdate_sound *sound,
    SoundChannel *channel
);
void pdna_effect_player_play(
    PdnaEffectPlayer *player,
    const PdnaEffectPreset *preset
);
void pdna_effect_player_deinit(PdnaEffectPlayer *player);
PdnaSongValidationError pdna_validate_song(const PdnaSongPreset *song);
PdnaAudioResult pdna_song_player_init(
    PdnaSongPlayer *player,
    const struct playdate_sound *sound,
    SoundChannel *channel,
    const PdnaSongPreset *song
);
void pdna_song_player_start(PdnaSongPlayer *player);
void pdna_song_player_stop(PdnaSongPlayer *player);
void pdna_song_player_deinit(PdnaSongPlayer *player);
PdnaAudioResult pdna_audio_init(
    PdnaAudio *audio,
    const struct playdate_sound *sound,
    const PdnaSongPreset *song
);
void pdna_audio_play_effect(PdnaAudio *audio, const PdnaEffectPreset *preset);
void pdna_audio_start_music(PdnaAudio *audio);
void pdna_audio_stop_music(PdnaAudio *audio);
PdnaAudioResult pdna_audio_play_song_loop(
    PdnaAudio *audio,
    const PdnaSongPreset *song
);
PdnaAudioResult pdna_audio_play_song_once(
    PdnaAudio *audio,
    const PdnaSongPreset *song
);
void pdna_audio_deinit(PdnaAudio *audio);

#endif
