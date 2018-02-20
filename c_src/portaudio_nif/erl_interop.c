#include "erl_interop.h"

#include <string.h>
#include <assert.h>

const char *ERL_NIL_TERM = "nil";

ERL_NIF_TERM erli_str_to_binary(ErlNifEnv *env, const char *str)
{
  ERL_NIF_TERM binary;
  char *str_raw = (char *) enif_make_new_binary(env, strlen(str), &binary);
  assert(str_raw!= NULL);
  strcpy(str_raw, str);
  return binary;
}

bool erli_is_nil(ErlNifEnv *env, ERL_NIF_TERM term)
{
  return enif_compare(term, enif_make_atom(env, ERL_NIL_TERM)) == 0;
}

ERL_NIF_TERM erli_make_nil(ErlNifEnv *env)
{
  return enif_make_atom(env, ERL_NIL_TERM);
}

ERL_NIF_TERM erli_make_bool(ErlNifEnv *env, bool val)
{
  return val
    ? enif_make_atom(env, "true")
    : enif_make_atom(env, "false");
}

ERL_NIF_TERM erli_make_error_tuple(ErlNifEnv *env, const char *type)
{
  return enif_make_tuple2(env,
                          enif_make_atom(env, "error"),
                          enif_make_atom(env, type));
}
