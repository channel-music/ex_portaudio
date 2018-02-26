#include "pa_conversions.h"

#include <assert.h>
#include <portaudio.h>

#include "erl_interop.h"
#include "util.h"

/**
 * Structure for defining conversions between PortAudio errors
 * and strings, to eventually be converted in to erlang atoms.
 */
struct err_to_str {
        PaError err;
        const char *str;
};

/**
 * Conversions from PortAudio errors to strings that can be easily converted
 * to erlang atoms.
 */
static struct err_to_str pa_errors[] = {
        { paNotInitialized,                        "not_initialized" },
        { paUnanticipatedHostError,                "unanticipated_host_error" },
        { paInvalidChannelCount,                   "invalid_channel_count" },
        { paInvalidSampleRate,                     "invalid_sample_rate" },
        { paInvalidDevice,                         "invalid_device" },
        { paInvalidFlag,                           "invalid_flag" },
        { paSampleFormatNotSupported,              "sample_format_unsupported" },
        { paBadIODeviceCombination,                "bad_device_combo" },
        { paInsufficientMemory,                    "insufficient_memory" },
        { paBufferTooBig,                          "buffer_too_big" },
        { paBufferTooSmall,                        "buffer_too_small" },
        { paNullCallback,                          "no_callback" },
        { paBadStreamPtr,                          "bad_callback" },
        { paTimedOut,                              "timeout" },
        { paInternalError,                         "internal_error" },
        { paDeviceUnavailable,                     "device_unavailable" },
        { paIncompatibleHostApiSpecificStreamInfo, "incompatible_host_stream_info" },
        { paStreamIsStopped,                       "stream_stopped" },
        { paStreamIsNotStopped,                    "stream_not_stopped" },
        { paInputOverflowed,                       "input_overflowed" },
        { paOutputUnderflowed,                     "output_underflowed" },
        { paHostApiNotFound,                       "no_host_api" },
        { paInvalidHostApi,                        "invalid_host_api" },
        { paCanNotReadFromACallbackStream,         "no_read_callback" },
        { paCanNotWriteToACallbackStream,          "no_write_callback" },
        { paCanNotReadFromAnOutputOnlyStream,      "output_only_stream" },
        { paCanNotWriteToAnInputOnlyStream,        "input_only_stream" },
        { paIncompatibleStreamHostApi,             "incompatible_host_api" },
        { paBadBufferPtr,                          "bad_buffer" },
        { 0, NULL } // SENTINEL
};

/**
 * Structure for defining conversions between PortAudio host API
 * type ID's to strings to eventually be converted in to erlang atoms.
 */
struct api_type_to_str {
        PaHostApiTypeId type_id;
        const char *str;
};

static struct api_type_to_str pa_drivers[] = {
        { paInDevelopment,   "in_development" },
        { paDirectSound,     "direct_sound" },
        { paMME,             "mme" },
        { paASIO,            "asio" },
        { paSoundManager,    "sound_manager" },
        { paCoreAudio,       "core_audio" },
        { paOSS,             "oss" },
        { paALSA,            "alsa" },
        { paAL,              "al" },
        { paBeOS,            "beos" },
        { paWDMKS,           "wdmks" },
        { paJACK,            "jack" },
        { paWASAPI,          "wasapi" },
        { paAudioScienceHPI, "audio_science_hpi" },
        { 0, NULL } // SENTINEL
};

/**
 * Structure for defining conversions between PortAudio sample formats
 * and erlang atoms.
 */
struct sample_format_to_str {
        PaSampleFormat fmt;
        const char *str;
};

static struct sample_format_to_str pa_sample_formats[] = {
        { paFloat32, "float32" },
        { paInt32,   "int32" },
        { paInt24,   "int24" },
        { paInt16,   "int16" },
        { paInt8,    "int8" },
        { paUInt8,   "uint8" },
        { 0, NULL } // SENTINEL
};

/**
 * Structure for defining conversions between PortAudio strema flags
 * and erlang atoms.
 */
struct stream_flags_to_str {
  PaStreamFlags flags;
  const char *str;
};

static struct stream_flags_to_str pa_stream_flags[] = {
  { paClipOff,        "noclip" },
  { paDitherOff,      "nodither" },
  { paNeverDropInput, "nodropinput" },
  { paNoFlag, NULL }
};

/**
 * Return an error message for the given error code.
 */
const char *_pa_error_to_char(PaError err)
{
        struct err_to_str *cur = &pa_errors[0];
        ensure(cur != NULL);

        while (cur->str != NULL) {
                if (cur->err == err)
                        return cur->str;
                cur++;
        }

        return "unknown_error";
}

ERL_NIF_TERM pa_error_to_error_tuple(ErlNifEnv *env, PaError err)
{
        return erli_make_error_tuple(env, _pa_error_to_char(err));
}

bool pa_is_error(PaError status)
{
        return status < 0;
}

ERL_NIF_TERM pa_device_to_term(ErlNifEnv *env, PaDeviceIndex device)
{
        return device == paNoDevice
                ? erli_make_nil(env)
                : enif_make_uint(env, device);
}

bool pa_stream_params_from_tuple(ErlNifEnv *env, ERL_NIF_TERM term, PaStreamParameters **stream_params)
{
        *stream_params = NULL;

        if (!enif_is_tuple(env, term))
                // Should still continue if term is nil
                return erli_is_nil(env, term);

        int arity;
        unsigned int device, channel_count;
        double suggested_latency;
        const ERL_NIF_TERM *tuple;
        if (!enif_get_tuple(env, term, &arity, &tuple)
            || arity != 4
            || !enif_get_uint(env, tuple[0], &device)
            || !enif_get_uint(env, tuple[1], &channel_count)
            || !enif_is_atom(env, tuple[2])
            || !enif_get_double(env, tuple[3], &suggested_latency)) {
                return false;
        }

        PaStreamParameters *params = enif_alloc(sizeof(PaStreamParameters));
        params->device = device;
        params->channelCount = channel_count;

        if (!pa_sample_format_from_atom(env, tuple[2], &params->sampleFormat))
                return false;

        params->suggestedLatency = suggested_latency;
        params->hostApiSpecificStreamInfo = NULL;
        *stream_params = params;

        return true;
}

bool pa_sample_format_from_atom(ErlNifEnv *env, ERL_NIF_TERM sample_atom, PaSampleFormat *sample_format)
{
        struct sample_format_to_str *cur = &pa_sample_formats[0];
        ensure(cur != NULL);

        while (cur->str != NULL) {
                if (enif_compare(sample_atom, enif_make_atom(env, cur->str)) == 0) {
                        *sample_format = cur->fmt;
                        return true;
                }
                cur++;
        }

        return false;
}

static PaStreamFlags _atom_to_stream_flags(ErlNifEnv *env, ERL_NIF_TERM term)
{
        struct stream_flags_to_str *cur = &pa_stream_flags[0];
        ensure(cur != NULL);

        while (cur->str != NULL) {
                if (enif_compare(term, enif_make_atom(env, cur->str)) == 0)
                        return cur->flags;
        }

        return paNoFlag;
}

bool pa_stream_flags_from_list(ErlNifEnv *env, ERL_NIF_TERM list, PaStreamFlags *flags)
{
        unsigned int length = 0;
        if (!enif_get_list_length(env, list, &length))
                return false;

        *flags = paNoFlag;

        if(length == 0)
                return true; // allow empty lists

        ERL_NIF_TERM cell;
        while (enif_get_list_cell(env, list, &cell, &list)) {
                const int flag = _atom_to_stream_flags(env, cell);
                if (flag != paNoFlag) {
                        *flags |= flag;
                }
        }

        return true;
}

bool pa_host_api_type_id_from_atom(ErlNifEnv *env, ERL_NIF_TERM atom, PaHostApiTypeId *type_id)
{
        struct api_type_to_str *cur = &pa_drivers[0];
        ensure(cur != NULL);

        while (cur->str != NULL) {
                if (enif_compare(enif_make_atom(env, cur->str), atom) == 0) {
                        *type_id = cur->type_id;
                        return true;
                }
                cur++;
        }

        return false;
}
