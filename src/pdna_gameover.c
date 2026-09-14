#include "pdna_gameover.h"

static const PdnaNoteEvent over_lead_notes[] = {
    { 0, 1, 55.0f, 0.7f },
    { 4, 1, 54.0f, 0.7f },
    { 8, 1, 53.0f, 0.7f },
    { 12, 1, 52.0f, 0.7f },
};

static const PdnaInstrumentVoicePreset over_lead_voices[] = {
    {
        .key_range = { 0.0f, 127.0f },
        .voice = {
            .waveform = PDNA_WAVEFORM_SQUARE,
            .envelope = { 0.005f, 0.04f, 0.55f, 0.03f, 0.0f, 1.0f, false, true, NULL },
            .volume = 0.4f,
            .pitch_source = { .kind = PDNA_PITCH_SOURCE_MOTION, .value.motion = { 4.0f, 0.0f, 0.12f } },
        },
    },
};

static const PdnaNoteEvent over_harmony_notes[] = {
    { 0, 3, 67.0f, 0.7f },
    { 4, 3, 66.0f, 0.7f },
    { 8, 3, 65.0f, 0.7f },
    { 12, 4, 64.0f, 0.7f },
};

static const PdnaInstrumentVoicePreset over_harmony_voices[] = {
    {
        .key_range = { 0.0f, 127.0f },
        .voice = {
            .waveform = PDNA_WAVEFORM_TRIANGLE,
            .envelope = { 0.005f, 0.04f, 0.55f, 0.03f, 0.0f, 1.0f, false, true, NULL },
            .volume = 1.0f,
            .pitch_source = { .kind = PDNA_PITCH_SOURCE_VIBRATO, .value.vibrato = { PDNA_LFO_SINE, 5.0f, 0.0f, 0.2f, 0.0f, 0.08f, 0.0f, true } },
        },
    },
};

static const PdnaNoteEvent over_bass_notes[] = {
    { 0, 1, 47.0f, 0.7f },
    { 1, 1, 43.0f, 0.7f },
    { 2, 1, 46.0f, 0.7f },
    { 4, 1, 45.0f, 0.7f },
    { 5, 1, 41.0f, 0.7f },
    { 6, 1, 44.0f, 0.7f },
    { 8, 1, 43.0f, 0.7f },
    { 9, 1, 42.0f, 0.7f },
    { 10, 1, 43.0f, 0.7f },
    { 11, 1, 42.0f, 0.7f },
    { 12, 1, 43.0f, 0.7f },
};

static const PdnaInstrumentVoicePreset over_bass_voices[] = {
    {
        .key_range = { 0.0f, 127.0f },
        .voice = {
            .waveform = PDNA_WAVEFORM_SAWTOOTH,
            .envelope = { 0.005f, 0.04f, 0.55f, 0.03f, 0.0f, 1.0f, false, true, NULL },
            .volume = 1.0f,
            .pitch_source = { .kind = PDNA_PITCH_SOURCE_NONE },
        },
    },
};

static const PdnaInstrumentVoicePreset over_percussion_voices[] = {
    {
        .key_range = { 0.0f, 127.0f },
        .voice = {
            .waveform = PDNA_WAVEFORM_NOISE,
            .envelope = { 0.005f, 0.04f, 0.55f, 0.03f, 0.0f, 1.0f, false, true, NULL },
            .volume = 0.16f,
            .pitch_source = { .kind = PDNA_PITCH_SOURCE_NONE },
        },
    },
};

const PdnaSongPreset pdna_over = {
    .steps_per_second = 8.0f,
    .length_steps = 20,
    .loop_start_step = 0,
    .loop_end_step_inclusive = 19,
    .tracks = {
        {
            .volume = 1.0f,
            .voices = over_lead_voices,
            .voice_count = sizeof(over_lead_voices) / sizeof(over_lead_voices[0]),
            .notes = over_lead_notes,
            .note_count = sizeof(over_lead_notes) / sizeof(over_lead_notes[0]),
        },
        {
            .volume = 1.0f,
            .voices = over_harmony_voices,
            .voice_count = sizeof(over_harmony_voices) / sizeof(over_harmony_voices[0]),
            .notes = over_harmony_notes,
            .note_count = sizeof(over_harmony_notes) / sizeof(over_harmony_notes[0]),
        },
        {
            .volume = 1.0f,
            .voices = over_bass_voices,
            .voice_count = sizeof(over_bass_voices) / sizeof(over_bass_voices[0]),
            .notes = over_bass_notes,
            .note_count = sizeof(over_bass_notes) / sizeof(over_bass_notes[0]),
        },
        {
            .volume = 1.0f,
            .voices = over_percussion_voices,
            .voice_count = sizeof(over_percussion_voices) / sizeof(over_percussion_voices[0]),
            .notes = NULL,
            .note_count = 0,
        },
    },
};
