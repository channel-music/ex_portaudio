#ifndef _PORTAUDIO_NIF_PA_CONVERSIONS_
#define _PORTAUDIO_NIF_PA_CONVERSIONS_

#include <stdbool.h>
#include <portaudio.h>

#include "erl_nif.h"

/**
 * Returns `true` if the given status indicates an error.
 */
bool pa_is_error(PaError status);

/**
 * Turn a PortAudio call into an error if there is one.
 */
ERL_NIF_TERM pa_error_to_error_tuple(ErlNifEnv *env, PaError err);

ERL_NIF_TERM pa_error_to_exception(ErlNifEnv *env, PaError err);

/**
 * Returns a PaDeviceIndex encoded as an erlang integer or if the
 * device index doesn't exist, returns an erlang nil instead.
 */
ERL_NIF_TERM pa_device_to_term(ErlNifEnv *env, PaDeviceIndex device);

/**
 * Coverts an erlang tuple in to PortAudio stream parameters.
 *
 * Returns `false` if conversion failed or `true` on success. Note however
 * that `true` may be returned without `stream_params` being initialized if
 * the passed term corresponds to an erlang `nil` value.
 */
bool pa_stream_params_from_tuple(ErlNifEnv *env, ERL_NIF_TERM term, PaStreamParameters **stream_params);

/**
 * Convert an erlang atom to a PortAudio sample format. Returns `true` if the
 * sample format was found or `false` otherwise.
 */
bool pa_sample_format_from_atom(ErlNifEnv *env, ERL_NIF_TERM sample_atom, PaSampleFormat *sample_format);

/**
 * Convert an erlang list to a stream flags for PortAudio. Returns `true`
 * on success, `false` otherwise.
 */
bool pa_stream_flags_from_list(ErlNifEnv *env, ERL_NIF_TERM list, PaStreamFlags *flags);

/**
 * Convert an erlang atom to a PortAudio host API type id. Returns `true` if the
 * type was found or `false` otherwise.
 */
bool pa_host_api_type_id_from_atom(ErlNifEnv *env, ERL_NIF_TERM atom, PaHostApiTypeId *type_id);

// FIXME: bad practice
/**
 * Check a PortAudio return value for errors and return from
 * the calling function with an error tuple if found.
 */
#define handle_pa_error(ENV, STATUS)                                    \
        do {                                                            \
                if (pa_is_error((STATUS))) {                            \
                        return pa_error_to_error_tuple((ENV), (STATUS)); \
                }                                                       \
        } while(0)

// FIXME: bad practice
/**
 * Check a PortAudio device index return value and return from
 * the calling function with an error tuple if found.
 */
#define handle_missing_device(ENV, IDX)                                 \
        do {                                                            \
                if ((IDX) == paNoDevice) {                              \
                        return erli_make_error_tuple((ENV), "no_device");  \
                }                                                       \
        } while(0)

#endif // _PORTAUDIO_NIF_PA_CONVERSIONS_
