#include "song1.h"

static const PdnaNoteEvent stackzgameover_lead_notes[] = {
    { 0, 1, 67.0f, 0.7f },
    { 2, 1, 64.0f, 0.7f },
    { 4, 1, 60.0f, 0.7f },
    { 16, 1, 65.0f, 0.7f },
    { 20, 1, 65.0f, 0.7f },
    { 22, 1, 64.0f, 0.7f },
    { 23, 1, 67.0f, 0.7f },
    { 28, 1, 60.0f, 0.7f },
    { 30, 1, 62.0f, 0.7f },
    { 32, 1, 67.0f, 0.7f },
    { 34, 1, 64.0f, 0.7f },
    { 36, 1, 60.0f, 0.7f },
    { 48, 1, 65.0f, 0.7f },
    { 52, 1, 65.0f, 0.7f },
    { 54, 1, 64.0f, 0.7f },
    { 55, 1, 67.0f, 0.7f },
    { 60, 1, 60.0f, 0.7f },
    { 62, 1, 62.0f, 0.7f },
};

static const PdnaInstrumentVoicePreset stackzgameover_lead_voices[] = {
    {
        .key_range = { 0.0f, 127.0f },
        .voice = {
            .waveform = PDNA_WAVEFORM_SQUARE,
            .envelope = { 0.01f, 0.15f, 0.55f, 0.31f, 0.0f, 1.0f, false, true, NULL },
            .volume = 0.3f,
            .pitch_source = { .kind = PDNA_PITCH_SOURCE_VIBRATO, .value.vibrato = { PDNA_LFO_SINE, 5.0f, 0.0f, 0.11f, 0.0f, 0.08f, 0.0f, true } },
        },
    },
};

static const PdnaNoteEvent stackzgameover_harmony_notes[] = {
    { 0, 1, 60.0f, 0.7f },
    { 4, 1, 67.0f, 0.7f },
    { 8, 1, 77.0f, 0.7f },
    { 10, 1, 79.0f, 0.7f },
    { 11, 1, 76.0f, 0.7f },
    { 13, 1, 74.0f, 0.7f },
    { 16, 3, 74.0f, 0.7f },
    { 20, 2, 74.0f, 0.7f },
    { 22, 1, 72.0f, 0.7f },
    { 23, 1, 76.0f, 0.7f },
    { 28, 1, 74.0f, 0.7f },
    { 30, 1, 72.0f, 0.7f },
    { 32, 1, 60.0f, 0.7f },
    { 36, 1, 67.0f, 0.7f },
    { 40, 1, 77.0f, 0.7f },
    { 42, 1, 79.0f, 0.7f },
    { 43, 1, 76.0f, 0.7f },
    { 45, 1, 74.0f, 0.7f },
    { 48, 3, 74.0f, 0.7f },
    { 52, 2, 74.0f, 0.7f },
    { 54, 1, 76.0f, 0.7f },
    { 55, 1, 72.0f, 0.7f },
    { 60, 1, 67.0f, 0.7f },
    { 62, 1, 69.0f, 0.7f },
};

static const PdnaInstrumentVoicePreset stackzgameover_harmony_voices[] = {
    {
        .key_range = { 0.0f, 127.0f },
        .voice = {
            .waveform = PDNA_WAVEFORM_TRIANGLE,
            .envelope = { 0.005f, 0.04f, 0.55f, 0.03f, 0.0f, 1.0f, false, true, NULL },
            .volume = 0.7f,
            .pitch_source = { .kind = PDNA_PITCH_SOURCE_VIBRATO, .value.vibrato = { PDNA_LFO_SINE, 5.0f, 0.0f, 0.2f, 0.0f, 0.08f, 0.0f, true } },
        },
    },
};

static const PdnaNoteEvent stackzgameover_bass_notes[] = {
    { 0, 1, 36.0f, 0.7f },
    { 3, 1, 36.0f, 0.7f },
    { 4, 1, 48.0f, 0.7f },
    { 6, 1, 36.0f, 0.7f },
    { 7, 1, 47.0f, 0.7f },
    { 16, 1, 40.0f, 0.7f },
    { 20, 1, 40.0f, 0.7f },
    { 23, 1, 41.0f, 0.7f },
    { 27, 1, 40.0f, 0.7f },
    { 28, 1, 41.0f, 0.7f },
    { 29, 1, 43.0f, 0.7f },
    { 30, 1, 41.0f, 0.7f },
    { 31, 1, 40.0f, 0.7f },
    { 32, 1, 36.0f, 0.7f },
    { 35, 1, 36.0f, 0.7f },
    { 36, 1, 48.0f, 0.7f },
    { 38, 1, 36.0f, 0.7f },
    { 39, 1, 47.0f, 0.7f },
    { 48, 1, 40.0f, 0.7f },
    { 52, 1, 40.0f, 0.7f },
    { 55, 1, 41.0f, 0.7f },
    { 59, 1, 40.0f, 0.7f },
    { 60, 1, 41.0f, 0.7f },
    { 61, 1, 43.0f, 0.7f },
    { 62, 1, 41.0f, 0.7f },
    { 63, 1, 40.0f, 0.7f },
};

static const PdnaInstrumentVoicePreset stackzgameover_bass_voices[] = {
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

static const PdnaNoteEvent stackzgameover_percussion_notes[] = {
    { 0, 1, 36.0f, 0.7f },
    { 4, 1, 36.0f, 0.7f },
    { 8, 1, 36.0f, 0.7f },
    { 10, 1, 36.0f, 0.7f },
    { 14, 1, 36.0f, 0.7f },
    { 16, 1, 36.0f, 0.7f },
    { 20, 1, 36.0f, 0.7f },
    { 24, 1, 36.0f, 0.7f },
    { 26, 1, 36.0f, 0.7f },
    { 30, 1, 36.0f, 0.7f },
    { 32, 1, 36.0f, 0.7f },
    { 36, 1, 36.0f, 0.7f },
    { 40, 1, 36.0f, 0.7f },
    { 42, 1, 36.0f, 0.7f },
    { 46, 1, 36.0f, 0.7f },
    { 48, 1, 36.0f, 0.7f },
    { 52, 1, 36.0f, 0.7f },
    { 56, 1, 36.0f, 0.7f },
    { 58, 1, 36.0f, 0.7f },
    { 62, 1, 36.0f, 0.7f },
};

static const PdnaInstrumentVoicePreset stackzgameover_percussion_voices[] = {
    {
        .key_range = { 0.0f, 34.0f },
        .voice = {
            .waveform = PDNA_WAVEFORM_SQUARE,
            .envelope = { 0.001f, 0.08f, 0.0f, 0.4f, 0.13f, 1.0f, false, true, NULL },
            .volume = 0.5f,
            .pitch_source = { .kind = PDNA_PITCH_SOURCE_NONE },
        },
    },
    {
        .key_range = { 35.0f, 127.0f },
        .voice = {
            .waveform = PDNA_WAVEFORM_NOISE,
            .envelope = { 0.001f, 0.08f, 0.0f, 0.4f, 0.0f, 1.0f, false, true, NULL },
            .volume = 0.7f,
            .pitch_source = { .kind = PDNA_PITCH_SOURCE_NONE },
        },
    },
};

const PdnaSongPreset song1 = {
    .steps_per_second = 6.0f,
    .length_steps = 64,
    .loop_start_step = 0,
    .loop_end_step_inclusive = 63,
    .tracks = {
        {
            .volume = 1.0f,
            .voices = stackzgameover_lead_voices,
            .voice_count = sizeof(stackzgameover_lead_voices) / sizeof(stackzgameover_lead_voices[0]),
            .notes = stackzgameover_lead_notes,
            .note_count = sizeof(stackzgameover_lead_notes) / sizeof(stackzgameover_lead_notes[0]),
        },
        {
            .volume = 1.0f,
            .voices = stackzgameover_harmony_voices,
            .voice_count = sizeof(stackzgameover_harmony_voices) / sizeof(stackzgameover_harmony_voices[0]),
            .notes = stackzgameover_harmony_notes,
            .note_count = sizeof(stackzgameover_harmony_notes) / sizeof(stackzgameover_harmony_notes[0]),
        },
        {
            .volume = 1.0f,
            .voices = stackzgameover_bass_voices,
            .voice_count = sizeof(stackzgameover_bass_voices) / sizeof(stackzgameover_bass_voices[0]),
            .notes = stackzgameover_bass_notes,
            .note_count = sizeof(stackzgameover_bass_notes) / sizeof(stackzgameover_bass_notes[0]),
        },
        {
            .volume = 1.0f,
            .voices = stackzgameover_percussion_voices,
            .voice_count = sizeof(stackzgameover_percussion_voices) / sizeof(stackzgameover_percussion_voices[0]),
            .notes = stackzgameover_percussion_notes,
            .note_count = sizeof(stackzgameover_percussion_notes) / sizeof(stackzgameover_percussion_notes[0]),
        },
    },
};
