#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "playback/play_queue.h"

enum {
    RESUME_STATE_SIZE = 558
};

typedef struct ResumeState {
    uint32_t library_fingerprint;
    PlayQueuePlan plan;
    RepeatMode repeat;
    uint8_t volume;
    PlayQueueCursor cursor;
    uint16_t track;
    uint32_t position_ms;
    bool from_up_next;
    uint16_t explicit_count;
    uint16_t explicit_tracks[PLAY_QUEUE_TRACK_LIST_MAX];
} ResumeState;

void resume_state_encode(const ResumeState *state, uint8_t *bytes);
bool resume_state_decode(const uint8_t *bytes, uint32_t length, ResumeState *state);
