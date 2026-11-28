#include "playback/resume_state.h"

#include "core/hash.h"

enum {
    VERSION = 2,
    MAGIC_FIELD = 0,
    VERSION_FIELD = 4,
    SHUFFLE_FIELD = 6,
    REPEAT_FIELD = 7,
    FINGERPRINT_FIELD = 8,
    SEED_FIELD = 12,
    SOURCE_KIND_FIELD = 16,
    SOURCE_GROUP_FIELD = 17,
    SOURCE_SUBJECT_FIELD = 18,
    SOURCE_ARTIST_FIELD = 20,
    ANCHOR_FIELD = 22,
    CURSOR_POSITION_FIELD = 24,
    CURSOR_ALBUM_TRACK_FIELD = 26,
    TRACK_FIELD = 28,
    VOLUME_FIELD = 30,
    POSITION_FIELD = 32,
    FROM_UP_NEXT_FIELD = 36,
    EXPLICIT_COUNT_FIELD = 40,
    EXPLICIT_TRACKS_FIELD = 42,
    CHECKSUM_FIELD = EXPLICIT_TRACKS_FIELD + PLAY_QUEUE_TRACK_LIST_MAX * 2,
    MAGIC_SIZE = 4,
    BITS_PER_BYTE = 8
};

_Static_assert(CHECKSUM_FIELD + 4 == RESUME_STATE_SIZE, "resume state size");

static const uint8_t MAGIC[MAGIC_SIZE] = {'E', 'M', 'R', 'S'};

static void put_u16(uint8_t *bytes, int offset, uint32_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1] = (uint8_t)(value >> BITS_PER_BYTE);
}

static void put_u32(uint8_t *bytes, int offset, uint32_t value)
{
    put_u16(bytes, offset, value & 0xFFFFu);
    put_u16(bytes, offset + 2, value >> 16);
}

static uint16_t get_u16(const uint8_t *bytes, int offset)
{
    return (uint16_t)(bytes[offset] | bytes[offset + 1] << BITS_PER_BYTE);
}

static uint32_t get_u32(const uint8_t *bytes, int offset)
{
    return (uint32_t)get_u16(bytes, offset) | (uint32_t)get_u16(bytes, offset + 2) << 16;
}

static uint32_t checksum_of(const uint8_t *bytes)
{
    return hash_update(HASH_START, bytes, CHECKSUM_FIELD);
}

static void encode_plan(const PlayQueuePlan *plan, uint8_t *bytes)
{
    bytes[SHUFFLE_FIELD] = (uint8_t)plan->shuffle;
    put_u32(bytes, SEED_FIELD, plan->seed);
    bytes[SOURCE_KIND_FIELD] = (uint8_t)plan->source.kind;
    bytes[SOURCE_GROUP_FIELD] = (uint8_t)plan->source.group;
    put_u16(bytes, SOURCE_SUBJECT_FIELD, plan->source.subject);
    put_u16(bytes, SOURCE_ARTIST_FIELD, plan->source.artist);
    put_u16(bytes, ANCHOR_FIELD, plan->anchor_track);
}

void resume_state_encode(const ResumeState *state, uint8_t *bytes)
{
    for (int index = 0; index < RESUME_STATE_SIZE; index++) {
        bytes[index] = index < MAGIC_SIZE ? MAGIC[index] : 0;
    }
    put_u16(bytes, VERSION_FIELD, VERSION);
    bytes[REPEAT_FIELD] = (uint8_t)state->repeat;
    put_u32(bytes, FINGERPRINT_FIELD, state->library_fingerprint);
    encode_plan(&state->plan, bytes);
    put_u16(bytes, CURSOR_POSITION_FIELD, state->cursor.position);
    put_u16(bytes, CURSOR_ALBUM_TRACK_FIELD, state->cursor.album_track);
    put_u16(bytes, TRACK_FIELD, state->track);
    bytes[VOLUME_FIELD] = state->volume;
    put_u32(bytes, POSITION_FIELD, state->position_ms);
    bytes[FROM_UP_NEXT_FIELD] = state->from_up_next ? 1 : 0;
    uint16_t count = state->plan.track_count;
    if (state->plan.tracks == NULL || count > PLAY_QUEUE_TRACK_LIST_MAX) {
        count = 0;
    }
    put_u16(bytes, EXPLICIT_COUNT_FIELD, count);
    for (uint16_t index = 0; index < count; index++) {
        put_u16(bytes, EXPLICIT_TRACKS_FIELD + index * 2, state->plan.tracks[index]);
    }
    put_u32(bytes, CHECKSUM_FIELD, checksum_of(bytes));
}

static bool header_valid(const uint8_t *bytes, uint32_t length)
{
    for (int index = 0; index < MAGIC_SIZE; index++) {
        if (bytes[MAGIC_FIELD + index] != MAGIC[index]) {
            return false;
        }
    }
    uint32_t checksum_field = length - 4;
    return get_u16(bytes, VERSION_FIELD) == VERSION &&
           get_u32(bytes, (int)checksum_field) == hash_update(HASH_START, bytes, checksum_field);
}

static bool group_valid(uint8_t group)
{
    return (group >= LIBRARY_ARTISTS && group <= LIBRARY_GENRES) || group == LIBRARY_PLAYLISTS;
}

static bool modes_valid(const uint8_t *bytes)
{
    return bytes[SHUFFLE_FIELD] < SHUFFLE_MODE_COUNT && bytes[REPEAT_FIELD] < REPEAT_MODE_COUNT &&
           bytes[SOURCE_KIND_FIELD] <= BROWSE_FOLDER_TRACKS &&
           group_valid(bytes[SOURCE_GROUP_FIELD]);
}

static PlayQueuePlan decode_plan(const uint8_t *bytes)
{
    return (PlayQueuePlan){
        .source =
            {
                .kind = (BrowseKind)bytes[SOURCE_KIND_FIELD],
                .group = (LibrarySection)bytes[SOURCE_GROUP_FIELD],
                .subject = get_u16(bytes, SOURCE_SUBJECT_FIELD),
                .artist = get_u16(bytes, SOURCE_ARTIST_FIELD),
            },
        .anchor_track = get_u16(bytes, ANCHOR_FIELD),
        .shuffle = (ShuffleMode)bytes[SHUFFLE_FIELD],
        .seed = get_u32(bytes, SEED_FIELD),
    };
}

bool resume_state_decode(const uint8_t *bytes, uint32_t length, ResumeState *state)
{
    if ((length != RESUME_STATE_SIZE && length != RESUME_STATE_MIN_SIZE) ||
        !header_valid(bytes, length) || !modes_valid(bytes)) {
        return false;
    }
    uint16_t count = get_u16(bytes, EXPLICIT_COUNT_FIELD);
    uint32_t track_bytes = length - EXPLICIT_TRACKS_FIELD - 4;
    if (count > PLAY_QUEUE_TRACK_LIST_MAX || count > track_bytes / 2) {
        return false;
    }
    *state = (ResumeState){
        .library_fingerprint = get_u32(bytes, FINGERPRINT_FIELD),
        .plan = decode_plan(bytes),
        .repeat = (RepeatMode)bytes[REPEAT_FIELD],
        .volume = bytes[VOLUME_FIELD],
        .cursor = {get_u16(bytes, CURSOR_POSITION_FIELD), get_u16(bytes, CURSOR_ALBUM_TRACK_FIELD)},
        .track = get_u16(bytes, TRACK_FIELD),
        .position_ms = get_u32(bytes, POSITION_FIELD),
        .from_up_next = bytes[FROM_UP_NEXT_FIELD] != 0,
        .explicit_count = count,
    };
    for (uint16_t index = 0; index < count; index++) {
        state->explicit_tracks[index] =
            get_u16(bytes, EXPLICIT_TRACKS_FIELD + index * 2);
    }
    state->plan.track_count = count;
    state->plan.tracks = count > 0 ? state->explicit_tracks : NULL;
    return true;
}
