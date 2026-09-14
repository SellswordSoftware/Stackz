#include "pdna_audio.h"

#include <math.h>

static const float SEMITONES_PER_OCTAVE = 12.0f;
/* Stackz mix: effects use their authored gain; music sits below them. */
static const float MUSIC_MIX_GAIN = 0.4f;

SoundWaveform pdna_native_waveform(PdnaWaveform waveform)
{
    switch (waveform) {
    case PDNA_WAVEFORM_SQUARE:
        return kWaveformSquare;
    case PDNA_WAVEFORM_TRIANGLE:
        return kWaveformTriangle;
    case PDNA_WAVEFORM_SINE:
        return kWaveformSine;
    case PDNA_WAVEFORM_NOISE:
        return kWaveformNoise;
    case PDNA_WAVEFORM_SAWTOOTH:
        return kWaveformSawtooth;
    }

    return kWaveformSquare;
}

LFOType pdna_native_lfo_shape(PdnaLfoShape shape)
{
    switch (shape) {
    case PDNA_LFO_SQUARE:
        return kLFOTypeSquare;
    case PDNA_LFO_TRIANGLE:
        return kLFOTypeTriangle;
    case PDNA_LFO_SINE:
        return kLFOTypeSine;
    case PDNA_LFO_SAMPLE_AND_HOLD:
        return kLFOTypeSampleAndHold;
    case PDNA_LFO_SAWTOOTH_UP:
        return kLFOTypeSawtoothUp;
    case PDNA_LFO_SAWTOOTH_DOWN:
        return kLFOTypeSawtoothDown;
    }

    return kLFOTypeSine;
}

static void configure_lfo(
    const struct playdate_sound *sound,
    PDSynthLFO *lfo,
    const PdnaLfoPreset *preset,
    float scale
)
{
    sound->lfo->setType(lfo, pdna_native_lfo_shape(preset->shape));
    sound->lfo->setRate(lfo, preset->rate_hz);
    sound->lfo->setCenter(lfo, preset->center * scale);
    sound->lfo->setDepth(lfo, preset->depth * scale);
    sound->lfo->setDelay(lfo, preset->holdoff_s, preset->ramp_s);
    sound->lfo->setStartPhase(lfo, preset->start_phase);
    sound->lfo->setRetrigger(lfo, preset->retrigger);
}

static bool configure_pitch_motion(
    const struct playdate_sound *sound,
    PDSynthLFO *lfo,
    const PdnaPitchMotion *motion
)
{
    if (motion->duration_s <= 0.0f) {
        return false;
    }

    const float start = motion->start_semitones / SEMITONES_PER_OCTAVE;
    const float end = motion->end_semitones / SEMITONES_PER_OCTAVE;
    sound->lfo->setType(
        lfo,
        end >= start ? kLFOTypeSawtoothUp : kLFOTypeSawtoothDown
    );
    sound->lfo->setRate(lfo, 1.0f / motion->duration_s);
    sound->lfo->setCenter(lfo, (start + end) / 2.0f);
    sound->lfo->setDepth(lfo, (end >= start ? end - start : start - end) / 2.0f);
    sound->lfo->setDelay(lfo, 0.0f, 0.0f);
    sound->lfo->setStartPhase(lfo, 0.0f);
    sound->lfo->setRetrigger(lfo, 1);
    return true;
}

void pdna_configure_voice(
    const struct playdate_sound *sound,
    PDSynth *synth,
    PDSynthLFO *pitch_lfo,
    PDSynthLFO *amplitude_lfo,
    const PdnaVoicePreset *voice
)
{
    if (sound == NULL || synth == NULL || voice == NULL) {
        return;
    }

    sound->synth->setWaveform(synth, pdna_native_waveform(voice->waveform));
    sound->synth->setAttackTime(synth, voice->envelope.attack_s);
    sound->synth->setDecayTime(synth, voice->envelope.decay_s);
    sound->synth->setSustainLevel(synth, voice->envelope.sustain);
    sound->synth->setReleaseTime(synth, voice->envelope.release_s);
    sound->synth->setVolume(synth, voice->volume, voice->volume);

    PDSynthEnvelope *envelope = sound->synth->getEnvelope(synth);
    if (envelope != NULL) {
        sound->envelope->setCurvature(envelope, voice->envelope.curvature);
        sound->envelope->setVelocitySensitivity(
            envelope,
            voice->envelope.velocity_sensitivity
        );
        sound->envelope->setLegato(envelope, voice->envelope.legato);
        sound->envelope->setRetrigger(envelope, voice->envelope.retrigger);
        if (voice->envelope.rate_scaling != NULL) {
            const PdnaRateScaling *scaling = voice->envelope.rate_scaling;
            sound->envelope->setRateScaling(
                envelope,
                scaling->scaling,
                scaling->start_note,
                scaling->end_note
            );
        } else {
            sound->envelope->setRateScaling(envelope, 1.0f, 0.0f, 127.0f);
        }
    }

    sound->synth->setFrequencyModulator(synth, NULL);
    if (pitch_lfo != NULL) {
        bool configured = false;
        switch (voice->pitch_source.kind) {
        case PDNA_PITCH_SOURCE_NONE:
            break;
        case PDNA_PITCH_SOURCE_VIBRATO:
            configure_lfo(
                sound,
                pitch_lfo,
                &voice->pitch_source.value.vibrato,
                1.0f / SEMITONES_PER_OCTAVE
            );
            configured = true;
            break;
        case PDNA_PITCH_SOURCE_MOTION:
            configured = configure_pitch_motion(
                sound,
                pitch_lfo,
                &voice->pitch_source.value.motion
            );
            break;
        }
        if (configured) {
            sound->synth->setFrequencyModulator(
                synth,
                (PDSynthSignalValue *)pitch_lfo
            );
        }
    }

    sound->synth->setAmplitudeModulator(synth, NULL);
    if (amplitude_lfo != NULL && voice->amplitude_lfo != NULL) {
        configure_lfo(sound, amplitude_lfo, voice->amplitude_lfo, 1.0f);
        sound->synth->setAmplitudeModulator(
            synth,
            (PDSynthSignalValue *)amplitude_lfo
        );
    }
}

PdnaAudioResult pdna_effect_player_init(
    PdnaEffectPlayer *player,
    const struct playdate_sound *sound,
    SoundChannel *channel
)
{
    if (player == NULL || sound == NULL || channel == NULL) {
        return PDNA_AUDIO_ERROR_INVALID_ARGUMENT;
    }

    *player = (PdnaEffectPlayer){
        .sound = sound,
        .channel = channel,
    };

    for (size_t index = 0; index < PDNA_EFFECT_VOICE_COUNT; ++index) {
        PDSynth *synth = sound->synth->newSynth();
        if (synth == NULL) {
            pdna_effect_player_deinit(player);
            return PDNA_AUDIO_ERROR_NEW_EFFECT_SYNTH_FAILED;
        }

        PDSynthLFO *pitch_lfo = sound->lfo->newLFO(kLFOTypeSine);
        if (pitch_lfo == NULL) {
            sound->synth->freeSynth(synth);
            pdna_effect_player_deinit(player);
            return PDNA_AUDIO_ERROR_NEW_EFFECT_LFO_FAILED;
        }

        PDSynthLFO *amplitude_lfo = sound->lfo->newLFO(kLFOTypeSine);
        if (amplitude_lfo == NULL) {
            sound->lfo->freeLFO(pitch_lfo);
            sound->synth->freeSynth(synth);
            pdna_effect_player_deinit(player);
            return PDNA_AUDIO_ERROR_NEW_EFFECT_LFO_FAILED;
        }

        if (!sound->channel->addSource(channel, (SoundSource *)synth)) {
            sound->lfo->freeLFO(amplitude_lfo);
            sound->lfo->freeLFO(pitch_lfo);
            sound->synth->freeSynth(synth);
            pdna_effect_player_deinit(player);
            return PDNA_AUDIO_ERROR_ADD_EFFECT_VOICE_FAILED;
        }

        player->synths[index] = synth;
        player->pitch_lfos[index] = pitch_lfo;
        player->amplitude_lfos[index] = amplitude_lfo;
        player->voice_count += 1;
    }

    return PDNA_AUDIO_OK;
}

void pdna_effect_player_play(
    PdnaEffectPlayer *player,
    const PdnaEffectPreset *preset
)
{
    if (player == NULL || preset == NULL ||
        player->voice_count != PDNA_EFFECT_VOICE_COUNT) {
        return;
    }

    const size_t index = player->next_voice;
    player->next_voice = (index + 1) % PDNA_EFFECT_VOICE_COUNT;
    pdna_configure_voice(
        player->sound,
        player->synths[index],
        player->pitch_lfos[index],
        player->amplitude_lfos[index],
        &preset->voice
    );
    player->sound->synth->playMIDINote(
        player->synths[index],
        preset->note,
        preset->velocity,
        preset->duration_s,
        0
    );
}

void pdna_effect_player_deinit(PdnaEffectPlayer *player)
{
    if (player == NULL) {
        return;
    }

    for (size_t index = 0; index < player->voice_count; ++index) {
        player->sound->channel->removeSource(
            player->channel,
            (SoundSource *)player->synths[index]
        );
        player->sound->synth->freeSynth(player->synths[index]);
        player->sound->lfo->freeLFO(player->pitch_lfos[index]);
        player->sound->lfo->freeLFO(player->amplitude_lfos[index]);
    }

    *player = (PdnaEffectPlayer){0};
}

static bool valid_midi_note(MIDINote note)
{
    return isfinite(note) && note >= 0.0f && note <= 127.0f;
}

static bool key_ranges_overlap(PdnaKeyRange left, PdnaKeyRange right)
{
    return left.first <= right.last && right.first <= left.last;
}

static bool track_maps_note(const PdnaTrackPreset *track, MIDINote note)
{
    for (size_t index = 0; index < track->voice_count; ++index) {
        const PdnaKeyRange range = track->voices[index].key_range;
        if (note >= range.first && note <= range.last) {
            return true;
        }
    }

    return false;
}

static bool track_has_pitch_source(const PdnaTrackPreset *track)
{
    for (size_t index = 0; index < track->voice_count; ++index) {
        if (track->voices[index].voice.pitch_source.kind !=
            PDNA_PITCH_SOURCE_NONE) {
            return true;
        }
    }

    return false;
}

PdnaSongValidationError pdna_validate_song(const PdnaSongPreset *song)
{
    if (song == NULL) {
        return PDNA_SONG_ERROR_INVALID_ARGUMENT;
    }
    if (!isfinite(song->steps_per_second) || song->steps_per_second <= 0.0f) {
        return PDNA_SONG_ERROR_INVALID_TEMPO;
    }
    if (song->length_steps == 0) {
        return PDNA_SONG_ERROR_INVALID_LENGTH;
    }
    if (song->loop_start_step > song->loop_end_step_inclusive ||
        song->loop_end_step_inclusive >= song->length_steps) {
        return PDNA_SONG_ERROR_INVALID_LOOP;
    }

    size_t total_voice_count = 0;
    for (size_t track_index = 0; track_index < PDNA_TRACK_COUNT; ++track_index) {
        const PdnaTrackPreset *track = &song->tracks[track_index];
        if (track->voice_count == 0 || track->voices == NULL) {
            return PDNA_SONG_ERROR_EMPTY_TRACK_VOICES;
        }
        if (track->note_count != 0 && track->notes == NULL) {
            return PDNA_SONG_ERROR_INVALID_ARGUMENT;
        }
        if (track->pitch_automation_count != 0 &&
            track->pitch_automation == NULL) {
            return PDNA_SONG_ERROR_INVALID_ARGUMENT;
        }
        if (track->voice_count > PDNA_MAX_SONG_VOICES ||
            total_voice_count > PDNA_MAX_SONG_VOICES - track->voice_count) {
            return PDNA_SONG_ERROR_TOO_MANY_VOICES;
        }
        total_voice_count += track->voice_count;

        for (size_t voice_index = 0; voice_index < track->voice_count;
             ++voice_index) {
            const PdnaInstrumentVoicePreset *voice =
                &track->voices[voice_index];
            if (!valid_midi_note(voice->key_range.first) ||
                !valid_midi_note(voice->key_range.last) ||
                voice->key_range.first > voice->key_range.last) {
                return PDNA_SONG_ERROR_INVALID_KEY_RANGE;
            }
            if (voice->voice.pitch_source.kind < PDNA_PITCH_SOURCE_NONE ||
                voice->voice.pitch_source.kind > PDNA_PITCH_SOURCE_MOTION) {
                return PDNA_SONG_ERROR_INVALID_ARGUMENT;
            }
            for (size_t prior_index = 0; prior_index < voice_index;
                 ++prior_index) {
                if (key_ranges_overlap(
                        voice->key_range,
                        track->voices[prior_index].key_range)) {
                    return PDNA_SONG_ERROR_OVERLAPPING_KEY_RANGE;
                }
            }
        }

        for (size_t note_index = 0; note_index < track->note_count;
             ++note_index) {
            const PdnaNoteEvent *note = &track->notes[note_index];
            if (note->length == 0) {
                return PDNA_SONG_ERROR_INVALID_NOTE_LENGTH;
            }
            if (!valid_midi_note(note->note) || !isfinite(note->velocity)) {
                return PDNA_SONG_ERROR_INVALID_ARGUMENT;
            }
            if (note->step >= song->length_steps ||
                note->length > song->length_steps - note->step) {
                return PDNA_SONG_ERROR_NOTE_OUT_OF_BOUNDS;
            }
            if (note_index != 0 &&
                note->step < track->notes[note_index - 1].step) {
                return PDNA_SONG_ERROR_NOTE_EVENTS_OUT_OF_ORDER;
            }
            if (!track_maps_note(track, note->note)) {
                return PDNA_SONG_ERROR_UNMAPPED_NOTE;
            }
        }

        if (track->pitch_automation_count != 0 && track_has_pitch_source(track)) {
            return PDNA_SONG_ERROR_CONFLICTING_PITCH_MODULATION;
        }
        for (size_t point_index = 0;
             point_index < track->pitch_automation_count; ++point_index) {
            const PdnaPitchPoint *point = &track->pitch_automation[point_index];
            if (!isfinite(point->semitones) || point->step >= song->length_steps) {
                return PDNA_SONG_ERROR_INVALID_PITCH_AUTOMATION;
            }
            if (point_index != 0 &&
                point->step < track->pitch_automation[point_index - 1].step) {
                return PDNA_SONG_ERROR_PITCH_AUTOMATION_OUT_OF_ORDER;
            }
        }
    }

    return PDNA_SONG_VALID;
}

static size_t voice_index_for_note(const PdnaTrackPreset *track, MIDINote note)
{
    for (size_t index = 0; index < track->voice_count; ++index) {
        const PdnaKeyRange range = track->voices[index].key_range;
        if (note >= range.first && note <= range.last) {
            return index;
        }
    }

    return SIZE_MAX;
}

static uint32_t note_end(const PdnaNoteEvent *note)
{
    return note->step + note->length;
}

static size_t phrase_start_index(
    const PdnaTrackPreset *track,
    size_t note_index,
    size_t voice_index
)
{
    size_t start = note_index;
    while (true) {
        size_t prior_index = SIZE_MAX;
        for (size_t index = 0; index < start; ++index) {
            const PdnaNoteEvent *prior = &track->notes[index];
            if (voice_index_for_note(track, prior->note) != voice_index) {
                continue;
            }
            if (note_end(prior) > track->notes[start].step) {
                prior_index = index;
            }
        }
        if (prior_index == SIZE_MAX) {
            return start;
        }
        start = prior_index;
    }
}

static uint32_t phrase_end(
    const PdnaTrackPreset *track,
    size_t phrase_start,
    size_t voice_index
)
{
    uint32_t end = note_end(&track->notes[phrase_start]);
    for (size_t index = phrase_start + 1; index < track->note_count; ++index) {
        const PdnaNoteEvent *note = &track->notes[index];
        if (note->step >= end) {
            break;
        }
        if (voice_index_for_note(track, note->note) == voice_index &&
            note_end(note) > end) {
            end = note_end(note);
        }
    }

    return end;
}

static PdnaAudioResult legato_phrase_voice_index(
    const PdnaTrackPreset *track,
    size_t *out_voice_index,
    bool *out_has_legato_phrase
)
{
    *out_has_legato_phrase = false;
    *out_voice_index = SIZE_MAX;
    for (size_t note_index = 0; note_index < track->note_count; ++note_index) {
        const PdnaNoteEvent *note = &track->notes[note_index];
        const size_t voice_index = voice_index_for_note(track, note->note);
        const PdnaVoicePreset *voice = &track->voices[voice_index].voice;
        if (!voice->envelope.legato ||
            phrase_start_index(track, note_index, voice_index) == note_index) {
            continue;
        }
        if (*out_has_legato_phrase && *out_voice_index != voice_index) {
            return PDNA_AUDIO_ERROR_MULTIPLE_LEGATO_PHRASE_VOICES;
        }
        *out_has_legato_phrase = true;
        *out_voice_index = voice_index;
    }

    if (*out_has_legato_phrase &&
        track->voices[*out_voice_index].voice.pitch_source.kind !=
            PDNA_PITCH_SOURCE_NONE) {
        return PDNA_AUDIO_ERROR_LEGATO_VOICE_HAS_PITCH_SOURCE;
    }

    return PDNA_AUDIO_OK;
}

static PdnaAudioResult create_song_voice(
    PdnaSongPlayer *player,
    PDSynthInstrument *instrument,
    const PdnaInstrumentVoicePreset *preset,
    size_t destination_index
)
{
    PDSynth *synth = player->sound->synth->newSynth();
    if (synth == NULL) {
        return PDNA_AUDIO_ERROR_NEW_MUSIC_SYNTH_FAILED;
    }

    PDSynthLFO *pitch_lfo = NULL;
    if (preset->voice.pitch_source.kind != PDNA_PITCH_SOURCE_NONE) {
        pitch_lfo = player->sound->lfo->newLFO(kLFOTypeSine);
        if (pitch_lfo == NULL) {
            player->sound->synth->freeSynth(synth);
            return PDNA_AUDIO_ERROR_NEW_MUSIC_LFO_FAILED;
        }
    }

    PDSynthLFO *amplitude_lfo = NULL;
    if (preset->voice.amplitude_lfo != NULL) {
        amplitude_lfo = player->sound->lfo->newLFO(kLFOTypeSine);
        if (amplitude_lfo == NULL) {
            if (pitch_lfo != NULL) {
                player->sound->lfo->freeLFO(pitch_lfo);
            }
            player->sound->synth->freeSynth(synth);
            return PDNA_AUDIO_ERROR_NEW_MUSIC_LFO_FAILED;
        }
    }

    pdna_configure_voice(
        player->sound,
        synth,
        pitch_lfo,
        amplitude_lfo,
        &preset->voice
    );
    if (!player->sound->instrument->addVoice(
            instrument,
            synth,
            preset->key_range.first,
            preset->key_range.last,
            0.0f)) {
        if (amplitude_lfo != NULL) {
            player->sound->lfo->freeLFO(amplitude_lfo);
        }
        if (pitch_lfo != NULL) {
            player->sound->lfo->freeLFO(pitch_lfo);
        }
        player->sound->synth->freeSynth(synth);
        return PDNA_AUDIO_ERROR_ADD_SONG_VOICE_FAILED;
    }

    player->synths[destination_index] = synth;
    player->pitch_lfos[destination_index] = pitch_lfo;
    player->amplitude_lfos[destination_index] = amplitude_lfo;
    return PDNA_AUDIO_OK;
}

static void add_track_notes(
    PdnaSongPlayer *player,
    SequenceTrack *sequence_track,
    const PdnaTrackPreset *track,
    size_t legato_voice_index,
    ControlSignal *signal
)
{
    for (size_t note_index = 0; note_index < track->note_count; ++note_index) {
        const PdnaNoteEvent *note = &track->notes[note_index];
        if (voice_index_for_note(track, note->note) != legato_voice_index) {
            player->sound->track->addNoteEvent(
                sequence_track,
                note->step,
                note->length,
                note->note,
                note->velocity
            );
            continue;
        }

        const size_t phrase_start = phrase_start_index(
            track,
            note_index,
            legato_voice_index
        );
        if (phrase_start == note_index) {
            player->sound->track->addNoteEvent(
                sequence_track,
                note->step,
                phrase_end(track, note_index, legato_voice_index) - note->step,
                note->note,
                note->velocity
            );
            player->sound->controlsignal->addEvent(signal, note->step, 0.0f, 0);
        } else {
            const MIDINote base_note = track->notes[phrase_start].note;
            player->sound->controlsignal->addEvent(
                signal,
                note->step,
                (note->note - base_note) / SEMITONES_PER_OCTAVE,
                1
            );
        }
    }
}

static PdnaAudioResult pdna_song_player_init_with_looping(
    PdnaSongPlayer *player,
    const struct playdate_sound *sound,
    SoundChannel *channel,
    const PdnaSongPreset *song,
    bool loop_forever
)
{
    if (player == NULL || sound == NULL || channel == NULL || song == NULL) {
        return PDNA_AUDIO_ERROR_INVALID_ARGUMENT;
    }
    *player = (PdnaSongPlayer){0};
    if (pdna_validate_song(song) != PDNA_SONG_VALID) {
        return PDNA_AUDIO_ERROR_INVALID_SONG;
    }

    *player = (PdnaSongPlayer){ .sound = sound, .channel = channel };
    for (size_t track_index = 0; track_index < PDNA_TRACK_COUNT; ++track_index) {
        PDSynthInstrument *instrument = sound->instrument->newInstrument();
        if (instrument == NULL) {
            pdna_song_player_deinit(player);
            return PDNA_AUDIO_ERROR_NEW_INSTRUMENT_FAILED;
        }
        const float track_volume = song->tracks[track_index].volume *
            MUSIC_MIX_GAIN;
        sound->instrument->setVolume(instrument, track_volume, track_volume);
        if (!sound->channel->addSource(channel, (SoundSource *)instrument)) {
            sound->instrument->freeInstrument(instrument);
            pdna_song_player_deinit(player);
            return PDNA_AUDIO_ERROR_ADD_INSTRUMENT_SOURCE_FAILED;
        }
        player->instruments[track_index] = instrument;
        player->instrument_count += 1;
    }

    for (size_t track_index = 0; track_index < PDNA_TRACK_COUNT; ++track_index) {
        const PdnaTrackPreset *track = &song->tracks[track_index];
        player->track_voice_starts[track_index] = player->voice_count;
        for (size_t voice_index = 0; voice_index < track->voice_count;
             ++voice_index) {
            const PdnaAudioResult result = create_song_voice(
                player,
                player->instruments[track_index],
                &track->voices[voice_index],
                player->voice_count
            );
            if (result != PDNA_AUDIO_OK) {
                pdna_song_player_deinit(player);
                return result;
            }
            player->voice_count += 1;
        }
        player->track_voice_counts[track_index] =
            player->voice_count - player->track_voice_starts[track_index];
    }

    player->sequence = sound->sequence->newSequence();
    if (player->sequence == NULL) {
        pdna_song_player_deinit(player);
        return PDNA_AUDIO_ERROR_NEW_SEQUENCE_FAILED;
    }
    for (size_t track_index = 0; track_index < PDNA_TRACK_COUNT; ++track_index) {
        const PdnaTrackPreset *track = &song->tracks[track_index];
        SequenceTrack *sequence_track = sound->sequence->addTrack(player->sequence);
        if (sequence_track == NULL) {
            pdna_song_player_deinit(player);
            return PDNA_AUDIO_ERROR_NEW_TRACK_FAILED;
        }
        sound->track->setInstrument(
            sequence_track,
            player->instruments[track_index]
        );

        size_t legato_voice_index;
        bool has_legato_phrase;
        const PdnaAudioResult legato_result = legato_phrase_voice_index(
            track,
            &legato_voice_index,
            &has_legato_phrase
        );
        if (legato_result != PDNA_AUDIO_OK) {
            pdna_song_player_deinit(player);
            return legato_result;
        }
        if (has_legato_phrase) {
            ControlSignal *signal = sound->track->getSignalForController(
                sequence_track,
                1,
                1
            );
            if (signal == NULL) {
                pdna_song_player_deinit(player);
                return PDNA_AUDIO_ERROR_NEW_PITCH_CONTROL_SIGNAL_FAILED;
            }
            const size_t synth_index =
                player->track_voice_starts[track_index] + legato_voice_index;
            sound->synth->setFrequencyModulator(
                player->synths[synth_index],
                (PDSynthSignalValue *)signal
            );
            add_track_notes(
                player,
                sequence_track,
                track,
                legato_voice_index,
                signal
            );
        } else {
            for (size_t note_index = 0; note_index < track->note_count;
                 ++note_index) {
                const PdnaNoteEvent *note = &track->notes[note_index];
                sound->track->addNoteEvent(
                    sequence_track,
                    note->step,
                    note->length,
                    note->note,
                    note->velocity
                );
            }
        }

        if (track->pitch_automation_count != 0) {
            ControlSignal *signal = sound->track->getSignalForController(
                sequence_track,
                1,
                1
            );
            if (signal == NULL) {
                pdna_song_player_deinit(player);
                return PDNA_AUDIO_ERROR_NEW_PITCH_CONTROL_SIGNAL_FAILED;
            }
            for (size_t point_index = 0;
                 point_index < track->pitch_automation_count; ++point_index) {
                const PdnaPitchPoint *point =
                    &track->pitch_automation[point_index];
                sound->controlsignal->addEvent(
                    signal,
                    point->step,
                    point->semitones / SEMITONES_PER_OCTAVE,
                    point->interpolate
                );
            }
            const size_t voice_start = player->track_voice_starts[track_index];
            const size_t voice_end = voice_start +
                player->track_voice_counts[track_index];
            for (size_t voice_index = voice_start; voice_index < voice_end;
                 ++voice_index) {
                sound->synth->setFrequencyModulator(
                    player->synths[voice_index],
                    (PDSynthSignalValue *)signal
                );
            }
        }
    }

    sound->sequence->setTempo(player->sequence, song->steps_per_second);
    if (loop_forever) {
        sound->sequence->setLoops(
            player->sequence,
            (int)song->loop_start_step,
            (int)song->loop_end_step_inclusive,
            0
        );
    }
    return PDNA_AUDIO_OK;
}

PdnaAudioResult pdna_song_player_init(
    PdnaSongPlayer *player,
    const struct playdate_sound *sound,
    SoundChannel *channel,
    const PdnaSongPreset *song
)
{
    return pdna_song_player_init_with_looping(player, sound, channel, song, true);
}

void pdna_song_player_start(PdnaSongPlayer *player)
{
    if (player != NULL && player->sequence != NULL) {
        player->sound->sequence->play(player->sequence, NULL, NULL);
    }
}

void pdna_song_player_stop(PdnaSongPlayer *player)
{
    if (player != NULL && player->sequence != NULL) {
        player->sound->sequence->stop(player->sequence);
    }
}

void pdna_song_player_deinit(PdnaSongPlayer *player)
{
    if (player == NULL) {
        return;
    }

    pdna_song_player_stop(player);
    if (player->sequence != NULL) {
        player->sound->sequence->freeSequence(player->sequence);
    }
    for (size_t index = 0; index < player->instrument_count; ++index) {
        player->sound->channel->removeSource(
            player->channel,
            (SoundSource *)player->instruments[index]
        );
        player->sound->instrument->freeInstrument(player->instruments[index]);
    }
    for (size_t index = 0; index < player->voice_count; ++index) {
        player->sound->synth->freeSynth(player->synths[index]);
        if (player->pitch_lfos[index] != NULL) {
            player->sound->lfo->freeLFO(player->pitch_lfos[index]);
        }
        if (player->amplitude_lfos[index] != NULL) {
            player->sound->lfo->freeLFO(player->amplitude_lfos[index]);
        }
    }

    *player = (PdnaSongPlayer){0};
}

PdnaAudioResult pdna_audio_init(
    PdnaAudio *audio,
    const struct playdate_sound *sound,
    const PdnaSongPreset *song
)
{
    if (audio == NULL || sound == NULL || song == NULL) {
        return PDNA_AUDIO_ERROR_INVALID_ARGUMENT;
    }

    *audio = (PdnaAudio){0};
    SoundChannel *channel = sound->getDefaultChannel();
    if (channel == NULL) {
        return PDNA_AUDIO_ERROR_NO_DEFAULT_CHANNEL;
    }

    PdnaAudioResult result = pdna_effect_player_init(
        &audio->effect_player,
        sound,
        channel
    );
    if (result != PDNA_AUDIO_OK) {
        return result;
    }
    result = pdna_song_player_init(&audio->song_player, sound, channel, song);
    if (result != PDNA_AUDIO_OK) {
        pdna_effect_player_deinit(&audio->effect_player);
        return result;
    }

    return PDNA_AUDIO_OK;
}

void pdna_audio_play_effect(PdnaAudio *audio, const PdnaEffectPreset *preset)
{
    if (audio != NULL) {
        pdna_effect_player_play(&audio->effect_player, preset);
    }
}

void pdna_audio_start_music(PdnaAudio *audio)
{
    if (audio != NULL) {
        pdna_song_player_start(&audio->song_player);
    }
}

void pdna_audio_stop_music(PdnaAudio *audio)
{
    if (audio != NULL) {
        pdna_song_player_stop(&audio->song_player);
    }
}

static PdnaAudioResult pdna_audio_replace_song(
    PdnaAudio *audio,
    const PdnaSongPreset *song,
    bool loop_forever
)
{
    if (audio == NULL || song == NULL || audio->song_player.sound == NULL ||
        audio->song_player.channel == NULL) {
        return PDNA_AUDIO_ERROR_INVALID_ARGUMENT;
    }

    const struct playdate_sound *sound = audio->song_player.sound;
    SoundChannel *channel = audio->song_player.channel;
    pdna_song_player_deinit(&audio->song_player);
    const PdnaAudioResult result = pdna_song_player_init_with_looping(
        &audio->song_player,
        sound,
        channel,
        song,
        loop_forever
    );
    if (result == PDNA_AUDIO_OK) {
        pdna_song_player_start(&audio->song_player);
    }
    return result;
}

PdnaAudioResult pdna_audio_play_song_loop(
    PdnaAudio *audio,
    const PdnaSongPreset *song
)
{
    return pdna_audio_replace_song(audio, song, true);
}

PdnaAudioResult pdna_audio_play_song_once(
    PdnaAudio *audio,
    const PdnaSongPreset *song
)
{
    return pdna_audio_replace_song(audio, song, false);
}

void pdna_audio_deinit(PdnaAudio *audio)
{
    if (audio == NULL) {
        return;
    }

    pdna_song_player_deinit(&audio->song_player);
    pdna_effect_player_deinit(&audio->effect_player);
    *audio = (PdnaAudio){0};
}
