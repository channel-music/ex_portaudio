#ifndef _PORTAUDIO_NIF_ERL_INTEROP_
#define _PORTAUDIO_NIF_ERL_INTEROP_

#include <stdbool.h>

#include "erl_nif.h"

/**
 * Uses enif_free, but ensures that the value being freed is `NULL` first.
 */
#define enif_safe_free(x)                       \
        if(x != NULL) { enif_free(x); }

/**
 * Make an erlang keyword list item. Represented as `{KEY, VAL}`.
 */
#define make_kw_item(ENV, KEY, VAL)                                   \
        enif_make_tuple2((ENV), enif_make_atom((ENV), (KEY)), (VAL))

/**
 * Convert a plain C string to an erlang binary.
 */
ERL_NIF_TERM erli_str_to_binary(ErlNifEnv *env, const char *str);

/**
 * Returns `true` if the given erlang term is considered a `nil` value.
 *
 * In this case it is either `undefined`, `null` or `nil`.
 */
bool erli_is_nil(ErlNifEnv *env, ERL_NIF_TERM term);

/**
 * Returns a nil term for use in erlang.
 */
ERL_NIF_TERM erli_make_nil(ErlNifEnv *env);

/**
 * Returns an erlang boolean value using the boolean * value given by `val`.
 */
ERL_NIF_TERM erli_make_bool(ErlNifEnv *env, bool val);

/**
 * Create an error tuple containing `{:error, type}`.
 */
ERL_NIF_TERM erli_make_error_tuple(ErlNifEnv *env, const char *type);

/**
 * Create an OK (success) tuple containing `{:ok, term}`.
 */
ERL_NIF_TERM erli_make_ok_tuple(ErlNifEnv *env, const ERL_NIF_TERM term);

/**
 * Make a map using an array of 2 element tuples.
 */
ERL_NIF_TERM erli_make_map_from_array(ErlNifEnv *env, const ERL_NIF_TERM fields[], const size_t len);

#endif // _PORTAUDIO_NIF_ERL_INTEROP_
