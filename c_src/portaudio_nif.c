#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
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

// wrap PaStream

static ErlNifResourceType *PORTAUDIO_STREAM_RESOURCE = NULL;

struct erl_stream_resource {
        PaStream *stream;

        short input_sample_size;
        short input_frame_size;

        short output_sample_size;
        short output_frame_size;
};

static struct erl_stream_resource *erl_stream_resource_alloc(void)
{
        struct erl_stream_resource *handle;
        handle = enif_alloc_resource(PORTAUDIO_STREAM_RESOURCE,
                                     sizeof(*handle));
        handle->stream = NULL;
        return handle;
}

static void erl_stream_resource_release(ErlNifEnv *env, void *data)
{
        UNUSED(env);

        struct erl_stream_resource *res = (struct erl_stream_resource *) data;
        assert(res != NULL);

        if(res->stream && Pa_IsStreamActive(res->stream))
                Pa_StopStream(res->stream);
}

static bool erl_stream_resource_register(ErlNifEnv *env)
{
        const ErlNifResourceFlags rt_flags =
                ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER;
        ErlNifResourceType *rt = enif_open_resource_type(env, NULL,
                                                         "PORTAUDIO_STREAM_RESOURCE",
                                                         &erl_stream_resource_release,
                                                         rt_flags, NULL);
        if (rt == NULL)
                return false;
        PORTAUDIO_STREAM_RESOURCE = rt;

        return true;
}

static bool erl_stream_resource_get(ErlNifEnv *env,
                                    ERL_NIF_TERM term,
                                    struct erl_stream_resource **res)
{
        const int ret =
                enif_get_resource(env, term, PORTAUDIO_STREAM_RESOURCE, (void **) res);
        return ret == 1;
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

        const PaError err = Pa_IsFormatSupported(input, output, sample_rate);
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

        const PaError err = Pa_OpenDefaultStream(&res->stream,
                                                 num_input_channels,
                                                 num_output_channels,
                                                 sample_format,
                                                 sample_rate,
                                                 frames_per_buffer,
                                                 NULL, NULL);
        HANDLE_PA_ERROR(env, err);

        // TODO: check num_input_channels
        res->input_sample_size = Pa_GetSampleSize(sample_format);
        res->input_frame_size = res->input_sample_size * num_input_channels;

        res->output_sample_size = Pa_GetSampleSize(sample_format);
        res->output_frame_size = res->output_sample_size * num_output_channels;

        return enif_make_tuple2(env,
                                enif_make_atom(env, "ok"),
                                enif_make_resource(env, res));
}

static ERL_NIF_TERM portaudio_stream_open_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        PaStreamParameters *input_params;
        PaStreamParameters *output_params;
        double sample_rate;
        unsigned long frames_per_buffer;

        if (argc != 4
            || !tuple_to_stream_params(env, argv[0], &input_params)
            || !tuple_to_stream_params(env, argv[1], &output_params)
            || !enif_get_double(env, argv[2], &sample_rate)
            || !enif_get_ulong(env, argv[3], &frames_per_buffer)) {
                return enif_make_badarg(env);
        }

        struct erl_stream_resource *res = erl_stream_resource_alloc();

        const PaError err = Pa_OpenStream(&res->stream,
                                          input_params,
                                          output_params,
                                          sample_rate,
                                          frames_per_buffer,
                                          0, NULL, NULL);
        HANDLE_PA_ERROR(env, err);

        if (input_params != NULL) {
                res->input_sample_size = Pa_GetSampleSize(input_params->sampleFormat);
                res->input_frame_size =
                        res->input_sample_size * input_params->channelCount;
        } else {
                res->input_sample_size = 0;
                res->input_frame_size = 0;
        }

        if (output_params != NULL) {
                res->output_sample_size = Pa_GetSampleSize(output_params->sampleFormat);
                res->output_frame_size =
                        res->output_sample_size * output_params->channelCount;
        } else {
                res->output_sample_size = 0;
                res->input_frame_size = 0;
        }

        return enif_make_tuple2(env,
                                enif_make_atom(env, "ok"),
                                enif_make_resource(env, res));
}

static ERL_NIF_TERM portaudio_stream_start_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        struct erl_stream_resource *res;

        if (argc != 1 || !erl_stream_resource_get(env, argv[0], &res))
                return enif_make_badarg(env);

        HANDLE_PA_ERROR(env, Pa_StartStream(res->stream));
        return enif_make_atom(env, "ok");
}

// TODO: rename and move
static ERL_NIF_TERM portaudio_stream_stop_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        struct erl_stream_resource *res;

        if (argc != 1 || !erl_stream_resource_get(env, argv[0], &res))
                return enif_make_badarg(env);


        HANDLE_PA_ERROR(env, Pa_StopStream(res->stream));

        return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM portaudio_stream_abort_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        struct erl_stream_resource *res;

        if (argc != 1 || !erl_stream_resource_get(env, argv[0], &res))
                return enif_make_badarg(env);


        HANDLE_PA_ERROR(env, Pa_AbortStream(res->stream));
        return enif_make_atom(env, "ok");
}

static ERL_NIF_TERM portaudio_stream_is_active_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        struct erl_stream_resource *res;

        if(argc != 1 || !erl_stream_resource_get(env, argv[0], &res))
                return enif_make_badarg(env);

        const PaError status = Pa_IsStreamActive(res->stream);
        HANDLE_PA_ERROR(env, status);
        return erli_make_bool(env, status == 1);
}

static ERL_NIF_TERM portaudio_stream_is_stopped_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        struct erl_stream_resource *res;

        if(argc != 1 || !erl_stream_resource_get(env, argv[0], &res))
                return enif_make_badarg(env);

        const PaError status = Pa_IsStreamStopped(res->stream);
        HANDLE_PA_ERROR(env, status);
        return erli_make_bool(env, status == 1);
}

static ERL_NIF_TERM portaudio_stream_read_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        struct erl_stream_resource *res;

        if (argc != 1 || !erl_stream_resource_get(env, argv[0], &res))
                return enif_make_badarg(env);

        const long frames_available = Pa_GetStreamReadAvailable(res->stream);

        // TODO: handle error
        assert(frames_available >= 0);
        if (frames_available == 0) {
                return erli_make_error_tuple(env, "stream_empty");
        }

        const long bytes_available = frames_available * res->input_frame_size;
        assert(bytes_available >= 0);

        ErlNifBinary input_bin;
        // FIXME: system failure
        assert(enif_alloc_binary(bytes_available, &input_bin));

        const PaError err = Pa_ReadStream(res->stream, input_bin.data, frames_available);
        HANDLE_PA_ERROR(env, err);
        return enif_make_tuple2(env,
                                enif_make_atom(env, "ok"),
                                enif_make_binary(env, &input_bin));
}

#define log(fmt, ...) \
        do {                                                  \
                fprintf(stderr, fmt, ##__VA_ARGS__);          \
                fflush(stderr);                               \
        } while(0)

static ERL_NIF_TERM portaudio_stream_write_nif(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
        struct erl_stream_resource *res;
        ErlNifBinary input_bin;
        if (argc != 2
            || !erl_stream_resource_get(env, argv[0], &res)
            || !enif_inspect_iolist_as_binary(env, argv[1], &input_bin)) {
                return enif_make_badarg(env);
        }

        const long frames_to_write = input_bin.size / res->output_frame_size;
        const PaError err = Pa_WriteStream(res->stream, input_bin.data, frames_to_write);
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
        // TODO: support flags
        {"stream_open",             4, portaudio_stream_open_nif,             0},
        {"stream_start",            1, portaudio_stream_start_nif,            0},
        {"stream_stop",             1, portaudio_stream_stop_nif,             0},
        {"stream_abort",            1, portaudio_stream_abort_nif,            0},
        {"stream_is_active",        1, portaudio_stream_is_active_nif,        0},
        {"stream_is_stopped",       1, portaudio_stream_is_stopped_nif,       0},
        {"stream_read",             1, portaudio_stream_read_nif, ERL_NIF_DIRTY_JOB_IO_BOUND},
        {"stream_write",            2, portaudio_stream_write_nif, ERL_NIF_DIRTY_JOB_IO_BOUND}
};

static int on_load(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info)
{
        UNUSED(priv_data); UNUSED(load_info);

        if(!erl_stream_resource_register(env))
                return -1;

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
