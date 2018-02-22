#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <portaudio.h>

#include "erl_nif.h"
#include "portaudio_nif/erl_interop.h"
#include "portaudio_nif/pa_conversions.h"
#include "portaudio_nif/pa_stream.h"

/**
 * Mark a parameter as unused.
 */
#define UNUSED(x) (void)(x)

static ERL_NIF_TERM portaudio_version_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        UNUSED(argc); UNUSED(argv);

        const char *version_str = Pa_GetVersionText();
        return enif_make_tuple2(env,
                                enif_make_uint(env, Pa_GetVersion()),
                                erli_str_to_binary(env, version_str));
}

////////////////////////////////////////////////////////////
// Native host API's
////////////////////////////////////////////////////////////
static ERL_NIF_TERM portaudio_default_host_api_index_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        UNUSED(argc); UNUSED(argv);

        PaHostApiIndex host_api_index = Pa_GetDefaultHostApi();
        HANDLE_PA_ERROR(env, host_api_index);
        return enif_make_uint(env, host_api_index);
}

static ERL_NIF_TERM portaudio_device_index_from_host_api_nif(ErlNifEnv *env, int argc,
                                                             const ERL_NIF_TERM argv[])
{
        PaHostApiIndex host_api_index;
        PaDeviceIndex host_api_device_index;
        if (argc != 2
            || !enif_get_int(env, argv[0], &host_api_index)
            || !enif_get_int(env, argv[1], &host_api_device_index)) {
                return enif_make_badarg(env);
        }

        PaDeviceIndex device_index =
                Pa_HostApiDeviceIndexToDeviceIndex(host_api_index,
                                                   host_api_device_index);

        HANDLE_PA_ERROR(env, device_index);
        return enif_make_uint(env, device_index);
}

static ERL_NIF_TERM portaudio_host_api_info_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        PaHostApiIndex index;
        // TODO: should be unsigned
        if (argc != 1 || !enif_get_int(env, argv[0], &index)) {
                return enif_make_badarg(env);
        }

        const PaHostApiInfo *info = Pa_GetHostApiInfo(index);
        if (info == NULL) {
                return erli_make_nil(env);
        }

        // Exceptional case
        HANDLE_PA_ERROR(env, info->deviceCount);

#define N_FIELDS 5
        const ERL_NIF_TERM fields[N_FIELDS] = {
                MAKE_KW_ITEM(env, "type", enif_make_int(env, info->type)),
                MAKE_KW_ITEM(env, "name", erli_str_to_binary(env, info->name)),
                MAKE_KW_ITEM(env, "device_count", enif_make_uint(env, info->deviceCount)),
                MAKE_KW_ITEM(env, "default_input_device",
                             pa_device_to_term(env, info->defaultInputDevice)),
                MAKE_KW_ITEM(env, "default_output_device",
                             pa_device_to_term(env, info->defaultOutputDevice))
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

        if (enif_is_atom(env, argv[0])) {
                if (!atom_to_host_api_type_id(env, argv[0], &type))
                        return erli_make_error_tuple(env, "no_host_api");

        } else if (!enif_get_uint(env, argv[0], &type)) {
                return enif_make_badarg(env);
        }

        PaHostApiIndex host_api_index = Pa_HostApiTypeIdToHostApiIndex(type);
        HANDLE_PA_ERROR(env, host_api_index);
        return enif_make_uint(env, host_api_index);
}

static ERL_NIF_TERM portaudio_host_api_count_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        UNUSED(argc); UNUSED(argv);

        PaHostApiIndex host_api_index = Pa_GetHostApiCount();
        HANDLE_PA_ERROR(env, host_api_index);
        return enif_make_uint(env, Pa_GetHostApiCount());
}

////////////////////////////////////////////////////////////
// Devices
////////////////////////////////////////////////////////////
static ERL_NIF_TERM portaudio_default_input_device_index_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        UNUSED(argc); UNUSED(argv);

        PaDeviceIndex device_index = Pa_GetDefaultInputDevice();
        HANDLE_MISSING_DEVICE(env, device_index);
        return enif_make_uint(env, device_index);
}

static ERL_NIF_TERM portaudio_default_output_device_index_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        UNUSED(argc); UNUSED(argv);

        PaDeviceIndex device_index = Pa_GetDefaultOutputDevice();
        HANDLE_MISSING_DEVICE(env, device_index);
        return enif_make_uint(env, device_index);
}

static ERL_NIF_TERM portaudio_device_count_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        UNUSED(argc); UNUSED(argv);

        int num_devices = Pa_GetDeviceCount();
        HANDLE_PA_ERROR(env, num_devices);
        return enif_make_uint(env, num_devices);
}

static ERL_NIF_TERM portaudio_device_info_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        int i;
        if (argc != 1 || !enif_get_int(env, argv[0], &i)) {
                return enif_make_badarg(env);
        }

        const PaDeviceInfo *device_info = Pa_GetDeviceInfo(i);
        if (device_info == NULL) {
                return erli_make_nil(env);
        }

#define N_FIELDS 9
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

////////////////////////////////////////////////////////////
// Streams
////////////////////////////////////////////////////////////

static ErlNifResourceType *ERL_STREAM_RESOURCE = NULL;

struct erl_stream_resource {
        struct pa_stream_handle *stream;

        // The default environment to use
        ErlNifEnv *env;

        // Communication PID's
        ErlNifPid *owner_pid;
        ErlNifPid *reader_pid;
};

static struct erl_stream_resource *erl_stream_resource_alloc(void)
{
        struct erl_stream_resource *res;
        res = enif_alloc_resource(ERL_STREAM_RESOURCE, sizeof(*res));

        res->stream = pa_stream_alloc();
        res->env = enif_alloc_env();
        // TODO: handle null
        res->owner_pid = (ErlNifPid *) enif_alloc(sizeof(ErlNifPid));
        res->reader_pid = (ErlNifPid *) enif_alloc(sizeof(ErlNifPid));

        return res;
}

static void erl_stream_resource_release(ErlNifEnv *env, void *data)
{
        UNUSED(env);

        struct erl_stream_resource *res = (struct erl_stream_resource *) data;
        assert(res != NULL);

        if (res->stream) {
                // Ensure the stream is properly aborted and wait
                // for it to finish before continuing.
                pa_stream_abort(res->stream);
                pa_stream_wait_for_termination(res->stream);
                pa_stream_dealloc(res->stream);
        }

        if (res->env)
                enif_free_env(res->env);

        if (res->owner_pid)
                enif_free(res->owner_pid);

        if (res->reader_pid)
                enif_free(res->reader_pid);
}

static bool erl_stream_resource_get(ErlNifEnv *env,
                                    ERL_NIF_TERM term,
                                    struct erl_stream_resource **res)
{
        int status = enif_get_resource(env, term, ERL_STREAM_RESOURCE, (void **) res);

        return status == 1;
}

static ERL_NIF_TERM portaudio_stream_format_supported_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        PaStreamParameters *input = NULL;
        PaStreamParameters *output = NULL;
        double sample_rate;

        if (argc != 3
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

static ERL_NIF_TERM portaudio_stream_open_default_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        int num_input_channels;
        int num_output_channels;
        PaSampleFormat sample_format;
        double sample_rate;
        unsigned long frames_per_buffer;
        if (argc != 5
            || !enif_get_int(env, argv[0], &num_input_channels)
            || !enif_get_int(env, argv[1], &num_output_channels)
            || !atom_to_sample_format(env, argv[2], &sample_format)
            || !enif_get_double(env, argv[3], &sample_rate)
            || !enif_get_ulong(env, argv[4], &frames_per_buffer)) {
                return enif_make_badarg(env);
        }

        struct erl_stream_resource *res = erl_stream_resource_alloc();

        // REVIEW: Should the owner be the one that opens or starts a stream?
        /* enif_self(env, handle->owner_pid); */
        /* enif_self(env, handle->reader_pid); */

        PaError err = pa_stream_open_default(res->stream, num_input_channels,
                                             num_output_channels, sample_format,
                                             sample_rate, frames_per_buffer);
        HANDLE_PA_ERROR(env, err);

        // Give ownership of handle to erlang
        /* enif_release_resource(handle); */
        return enif_make_tuple2(env,
                                enif_make_atom(env, "ok"),
                                enif_make_resource(env, res));
}

// TODO: rename and move
static ERL_NIF_TERM portaudio_stream_stop_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        struct erl_stream_resource *res;

        if (argc != 1 || !erl_stream_resource_get(env, argv[0], &res)) {
                return enif_make_badarg(env);
        }

        HANDLE_PA_ERROR(env, pa_stream_stop(res->stream));
        return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM portaudio_stream_abort_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        struct erl_stream_resource *res;

        if (argc != 1 || !erl_stream_resource_get(env, argv[0], &res)) {
                return enif_make_badarg(env);
        }

        HANDLE_PA_ERROR(env, pa_stream_stop(res->stream));
        return enif_make_atom(env, "ok");
}

static void portaudio_stream_input_callback(PaError status,
                                            void *in_buf, size_t in_len,
                                            void *user_data)
{
        UNUSED(status); UNUSED(in_buf); UNUSED(user_data);

        struct erl_stream_resource *res = (struct erl_stream_resource *) user_data;
        assert(res != NULL);

        if (status == paNoError) {
                ErlNifBinary input_bin;
                enif_alloc_binary(in_len, &input_bin);
                memcpy(input_bin.data, in_buf, in_len);

                // TODO: cleanup style
                const ERL_NIF_TERM msg =
                        enif_make_tuple3(res->env,
                                         enif_make_atom(res->env, "portaudio"),
                                         enif_make_resource(res->env, res),
                                         enif_make_tuple2(res->env,
                                                          enif_make_atom(res->env, "pcmdata"),
                                                          enif_make_binary(res->env, &input_bin)));

                // TODO: handle return
                enif_send(NULL, res->reader_pid, res->env, msg);

                enif_release_binary(&input_bin);
        }
}

static ERL_NIF_TERM portaudio_stream_start_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        struct erl_stream_resource *res;

        if (argc != 1 || !erl_stream_resource_get(env, argv[0], &res)) {
                return enif_make_badarg(env);
        }

        PaError err = pa_stream_start(res->stream, portaudio_stream_input_callback,
                                      (void *) res);
        HANDLE_PA_ERROR(env, err);

        return enif_make_atom(env, "ok");
}

static ErlNifFunc portaudio_nif_funcs[] = {
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
        {"stream_format_supported", 3, portaudio_stream_format_supported_nif, 0},
        {"stream_open_default",     5, portaudio_stream_open_default_nif,     0},
        {"stream_stop",             1, portaudio_stream_stop_nif,             0},
        {"stream_abort",            1, portaudio_stream_abort_nif,            0},
        {"stream_start",            1, portaudio_stream_start_nif,            0}
};

static int on_load(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info)
{
        UNUSED(priv_data); UNUSED(load_info);

        // TODO: move me
        const ErlNifResourceFlags flags = ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER;
        ErlNifResourceType *rt = enif_open_resource_type(env, NULL,
                                                         "PORTAUDIO_STREAM_RESOURCE",
                                                         &erl_stream_resource_release,
                                                         flags, NULL);
        if (rt == NULL)
                return -1;
        ERL_STREAM_RESOURCE = rt;

        // Initialize portaudio
        const PaError err = Pa_Initialize();
        if (err != paNoError)
                return err;


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
