#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <portaudio.h>

#include "erl_nif.h"
#include "portaudio_nif/erl_interop.h"
#include "portaudio_nif/pa_conversions.h"

/**
 * Mark a parameter as unused.
 */
#define UNUSED(x) (void)(x)

static ERL_NIF_TERM portaudio_version_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  UNUSED(argc); UNUSED(argv);

  const char *version_str = Pa_GetVersionText();
  return enif_make_tuple2(env,
                          enif_make_int(env, Pa_GetVersion()),
                          erli_str_to_binary(env, version_str));
}

static ERL_NIF_TERM portaudio_default_host_api_index_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  UNUSED(argc); UNUSED(argv);

  return enif_make_int(env, Pa_GetDefaultHostApi());
}

static ERL_NIF_TERM portaudio_device_index_from_host_api_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  PaHostApiIndex host_api_index;
  PaDeviceIndex host_api_device_index;
  if(argc != 2
     || !enif_get_int(env, argv[0], &host_api_index)
     || !enif_get_int(env, argv[1], &host_api_device_index)) {
    return enif_make_badarg(env);
  }

  PaDeviceIndex device_index = Pa_HostApiDeviceIndexToDeviceIndex(host_api_index,
                                                                  host_api_device_index);
  HANDLE_PA_ERROR(env, device_index);
  return enif_make_int(env, device_index);
}

static ERL_NIF_TERM portaudio_host_api_info_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  PaHostApiIndex index;
  if(argc != 1 || !enif_get_int(env, argv[0], &index)) {
    return enif_make_badarg(env);
  }

  const PaHostApiInfo *info = Pa_GetHostApiInfo(index);
  if(info == NULL) {
    return erli_make_nil(env);
  }

#define N_FIELDS 5
  const ERL_NIF_TERM fields[N_FIELDS] = {
    MAKE_KW_ITEM(env, "type", enif_make_int(env, info->type)),
    MAKE_KW_ITEM(env, "name", erli_str_to_binary(env, info->name)),
    MAKE_KW_ITEM(env, "device_count",
                 enif_make_int(env, info->deviceCount)),
    // FIXME: handle paNoDevice
    MAKE_KW_ITEM(env, "default_input_device",
                 enif_make_int(env, info->defaultInputDevice)),
    MAKE_KW_ITEM(env, "default_output_device",
                 enif_make_int(env, info->defaultOutputDevice))
  };

  return enif_make_list_from_array(env, fields, N_FIELDS);
#undef N_FIELDS
}

static ERL_NIF_TERM portaudio_host_api_index_from_type_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  if(argc != 1) {
    return enif_make_badarg(env);
  }

  PaHostApiTypeId type;

  if(enif_is_atom(env, argv[0])) {
    if(!atom_to_host_api_type_id(env, argv[0], &type)) {
      return erli_make_error_tuple(env, "no_host_api");
    }
  } else if(!enif_get_uint(env, argv[0], &type)) {
    return enif_make_badarg(env);
  }

  PaHostApiIndex host_api_index = Pa_HostApiTypeIdToHostApiIndex(type);
  HANDLE_PA_ERROR(env, host_api_index);
  return enif_make_int(env, host_api_index);
}

static ERL_NIF_TERM portaudio_host_api_count_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  UNUSED(argc); UNUSED(argv);

  return enif_make_int(env, Pa_GetHostApiCount());
}

static ERL_NIF_TERM portaudio_default_input_device_index_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  UNUSED(argc); UNUSED(argv);

  // FIXME: handle paNoDevice
  return enif_make_int(env, Pa_GetDefaultInputDevice());
}

static ERL_NIF_TERM portaudio_default_output_device_index_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  UNUSED(argc); UNUSED(argv);

  return enif_make_int(env, Pa_GetDefaultOutputDevice());
}

static ERL_NIF_TERM portaudio_device_count_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  UNUSED(argc); UNUSED(argv);

  int num_devices = Pa_GetDeviceCount();
  // TODO: consider throwing an exception here
  HANDLE_PA_ERROR(env, num_devices);
  return enif_make_int(env, num_devices);
}

static ERL_NIF_TERM portaudio_device_info_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  int i;
  if(argc != 1 || !enif_get_int(env, argv[0], &i)) {
    return enif_make_badarg(env);
  }

  const PaDeviceInfo *device_info = Pa_GetDeviceInfo(i);
  if(device_info == NULL) {
    return erli_make_nil(env);
  }

#define N_FIELDS 9
  // TODO: use a record or something
  const ERL_NIF_TERM fields[N_FIELDS] = {
    MAKE_KW_ITEM(env, "name",
                 erli_str_to_binary(env, device_info->name)),
    MAKE_KW_ITEM(env, "host_api",
                 enif_make_int(env, device_info->hostApi)),
    MAKE_KW_ITEM(env, "max_input_channels",
                 enif_make_int(env, device_info->maxInputChannels)),
    MAKE_KW_ITEM(env, "max_output_channels",
                 enif_make_int(env, device_info->maxOutputChannels)),
    MAKE_KW_ITEM(env,
                 "default_low_input_latency",
                 enif_make_double(env, device_info->defaultLowInputLatency)),
    MAKE_KW_ITEM(env,
                 "default_low_output_latency",
                 enif_make_double(env, device_info->defaultLowOutputLatency)),
    MAKE_KW_ITEM(env,
                 "default_high_input_latency",
                 enif_make_double(env, device_info->defaultHighInputLatency)),
    MAKE_KW_ITEM(env,
                 "default_high_output_latency",
                 enif_make_double(env, device_info->defaultHighOutputLatency)),
    MAKE_KW_ITEM(env, "default_sample_rate",
                 enif_make_double(env, device_info->defaultSampleRate))
  };

  return enif_make_list_from_array(env, fields, N_FIELDS);
#undef N_FIELDS
}

static ERL_NIF_TERM portaudio_stream_format_supported_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  PaStreamParameters *input = NULL;
  PaStreamParameters *output = NULL;
  double sample_rate;

  if(argc != 3
     || !tuple_to_stream_params(env, argv[0], &input)
     || !tuple_to_stream_params(env, argv[1], &output)
     || !enif_get_double(env, argv[2], &sample_rate)) {
    ENIF_SAFE_FREE(input);
    ENIF_SAFE_FREE(output);
    return enif_make_badarg(env);
  }

  ENIF_SAFE_FREE(input);
  ENIF_SAFE_FREE(output);

  PaError err = Pa_IsFormatSupported(input, output, sample_rate);
  return erli_make_bool(env, err == paFormatIsSupported);
}

static ErlNifFunc portaudio_nif_funcs[] =
  {
    {"version", 0, portaudio_version_nif, 0},
    // Native Host API
    {"host_api_count",             0, portaudio_host_api_count_nif,             0},
    {"host_api_info",              1, portaudio_host_api_info_nif,              0},
    {"host_api_index_from_type",   1, portaudio_host_api_index_from_type_nif,   0},
    {"default_host_api_index",     0, portaudio_default_host_api_index_nif,     0},
    {"device_index_from_host_api", 2, portaudio_device_index_from_host_api_nif, 0},
    // TODO: add support for checking types
    // Devices
    {"device_count",                0, portaudio_device_count_nif,                0},
    {"device_info",                 1, portaudio_device_info_nif,                 0},
    {"default_input_device_index",  0, portaudio_default_input_device_index_nif,  0},
    {"default_output_device_index", 0, portaudio_default_output_device_index_nif, 0},
    // Streams
    {"stream_format_supported", 3, portaudio_stream_format_supported_nif, 0}
  };


static int on_load(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info)
{
  UNUSED(env); UNUSED(priv_data); UNUSED(load_info);

  const PaError err = Pa_Initialize();
  if (err != paNoError) {
    return err;
  }

  return 0;
}

static void on_unload(ErlNifEnv *env, void *priv_data)
{
  UNUSED(env); UNUSED(priv_data);

  Pa_Terminate();
}

ERL_NIF_INIT(Elixir.PortAudio.Native,
             portaudio_nif_funcs,
             &on_load, NULL, NULL, &on_unload);
