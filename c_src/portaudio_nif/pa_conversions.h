#ifndef _PORTAUDIO_NIF_PA_CONVERSIONS_
#define _PORTAUDIO_NIF_PA_CONVERSIONS_

#include <stdbool.h>
#include <portaudio.h>

#include "erl_nif.h"

/**
 * Turn a PortAudio call into an error if there is one.
 */
ERL_NIF_TERM pa_error_to_error_tuple(ErlNifEnv *env, PaError err);

/**
 * Check a PortAudio return value for errors and return from
 * the current function with the related erlang term if found.
 */
#define HANDLE_PA_ERROR(ENV, ERR_OR_IDX)                    \
  {                                                         \
    if((ERR_OR_IDX) < 0) {                                  \
      return pa_error_to_error_tuple((ENV), (ERR_OR_IDX));  \
    }                                                       \
  }


/**
 * Coverts an erlang tuple in to PortAudio stream parameters.
 *
 * Returns `false` if conversion failed or `true` on success. Note however
 * that `true` may be returned without `stream_params` being initialized if
 * the passed term corresponds to an erlang `nil` value.
 */
bool tuple_to_stream_params(ErlNifEnv *env, ERL_NIF_TERM term, PaStreamParameters **stream_params);

/**
 * Convert an erlang atom to a PortAudio sample format. Returns `true` if the
 * sample format was found or `false` otherwise.
 */
bool atom_to_sample_format(ErlNifEnv *env, ERL_NIF_TERM sample_atom, PaSampleFormat *sample_format);

/**
 * Convert an erlang atom to a PortAudio host API type id. Returns `true` if the
 * type was found or `false` otherwise.
 */
bool atom_to_host_api_type_id(ErlNifEnv *env, ERL_NIF_TERM atom, PaHostApiTypeId *type_id);

#endif // _PORTAUDIO_NIF_PA_CONVERSIONS_
