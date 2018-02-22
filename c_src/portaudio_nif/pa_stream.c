#include "pa_stream.h"
#include "erl_nif.h"

#include <assert.h>
#include <stdbool.h>
#include <portaudio.h>

// TODO: rename
/**
 * Represents a handle to a callback, containing the
 * actual callback function and the user data to be attached
 * to it.
 */
struct pa_stream_callback_handle {
        /**
         * The data to be passed to the callback.
         */
        void *user_data;
        /**
         * The callback to be called wehn a portaudio stream event
         * happens.
         */
        pa_stream_callback callback;
};

struct pa_stream_handle {
        // Threading
        // REVIEW: normal lock or rwlock
        //
        // I think an rwlock makes more sense as callbacks can call read-only
        // functions without having a deadlock.
        /**
         * A lock to manage synchronization between the main erlang NIF
         * thread and the thread opened when handling streams.
         */
        ErlNifRWLock *lock;
        /**
         * Value that represents if the stream is currently running.
         */
        bool         running;
        /**
         * Value that indicates if the stream thread should be terminated.
         */
        bool         terminate;
        /**
         * A handle to the running erlang thread.
         */
        ErlNifTid    thread;

        // PortAudio
        /**
         * The plain PortAudio stream.
         */
        PaStream *stream;

        // Buffers and constants
        short input_sample_size;
        short input_frame_size;

        short output_sample_size;
        short output_frame_size;

        // Callbacks
        struct pa_stream_callback_handle *input_callback;
};

struct pa_stream_handle *pa_stream_alloc(void)
{
        struct pa_stream_handle *handle =
                malloc(sizeof(struct pa_stream_handle));
        assert(handle != NULL);
        // Initialize
        handle->lock = enif_rwlock_create("PORTAUDIO_STREAM_MUTEX");
        handle->running = false;
        handle->terminate = false;
        handle->stream = NULL;
        handle->input_callback = NULL;
        // Check struct values
        assert(handle->lock != NULL);
        return handle;
}

void pa_stream_dealloc(struct pa_stream_handle *handle)
{
        assert(handle != NULL);

        if(handle->lock)
                // FIXME: may be locked
                enif_rwlock_destroy(handle->lock);

        if(handle->stream) // TODO: handle return
                Pa_AbortStream(handle->stream);

        if(handle->input_callback)
                free(handle->input_callback);

        free(handle);
}

PaError pa_stream_open_default(struct pa_stream_handle *handle,
                                      int num_input_channels,
                                      int num_output_channels,
                                      PaSampleFormat sample_format,
                                      double sample_rate,
                                      unsigned long frames_per_buffer)
{
        PaError stream_err = Pa_OpenDefaultStream(&handle->stream,
                                                  num_input_channels,
                                                  num_output_channels,
                                                  sample_format,
                                                  sample_rate,
                                                  frames_per_buffer,
                                                  NULL,
                                                  NULL);
        if (stream_err != paNoError) {
                return stream_err;
        }

        handle->input_sample_size = Pa_GetSampleSize(sample_format);
        handle->input_frame_size =
                handle->input_sample_size * num_input_channels;
        assert(handle->input_frame_size != 0);

        return paNoError;
}

static bool pa_stream_thread_input(struct pa_stream_handle *handle)
{
        enif_rwlock_rlock(handle->lock);
        const long frames_available = Pa_GetStreamReadAvailable(handle->stream);
        enif_rwlock_runlock(handle->lock);

        if (frames_available > 0) {
                enif_rwlock_rlock(handle->lock);
                const long bytes_available = frames_available * handle->input_frame_size;
                enif_rwlock_runlock(handle->lock);

                void *input_buf = malloc(bytes_available);
                // TODO: handle properly, not with assertions
                assert(input_buf != NULL);

                // FIXME: read or write?
                enif_rwlock_rlock(handle->lock);
                const PaError err = Pa_ReadStream(handle->stream, input_buf, frames_available);

                struct pa_stream_callback_handle *cb_handle =
                        handle->input_callback;
                // FIXME: is casting safe?
                cb_handle->callback(err, input_buf, (size_t) bytes_available,
                                    cb_handle->user_data);
                enif_rwlock_runlock(handle->lock);

                free(input_buf);

                // Kill if we have an error with reading
                return err == paNoError;
        } else if (frames_available < 0) {
                // Notify owner
                return false;
        }

        return true;
}

static void *pa_stream_thread_handler(void *data)
{
        struct pa_stream_handle *handle =
                (struct pa_stream_handle *) data;
        assert(handle != NULL);
        assert(handle->input_callback != NULL);

        enif_rwlock_rlock(handle->lock);
        assert(Pa_IsStreamActive(handle->stream) == true);
        enif_rwlock_runlock(handle->lock);

        enif_rwlock_rwlock(handle->lock);
        // Our stream handler starts from here on
        handle->running = true;
        enif_rwlock_rwunlock(handle->lock);

        while (true) {
                enif_rwlock_rlock(handle->lock);
                if(!Pa_IsStreamActive(handle->stream) || handle->terminate)
                        break;
                enif_rwlock_runlock(handle->lock);

                if(!pa_stream_thread_input(handle))
                        break;

        }

        enif_rwlock_rwlock(handle->lock);
        handle->running = false;
        enif_rwlock_rwunlock(handle->lock);

        return NULL;
}

PaError pa_stream_start(struct pa_stream_handle *handle,
                            pa_stream_callback input_callback,
                            void *user_data)
{
        // Just make sure this assertion area is critical so
        // as to avoid issues when incorrectly calling this function.
        enif_rwlock_rlock(handle->lock);
        assert(handle->running == false);
        enif_rwlock_runlock(handle->lock);

        enif_rwlock_rwlock(handle->lock);
        PaError stream_err = Pa_StartStream(handle->stream);
        enif_rwlock_rwunlock(handle->lock);

        // This area is no longer critical
        if(stream_err != paNoError) {
                return stream_err;
        }

        struct pa_stream_callback_handle *cb_handle =
                malloc(sizeof(struct pa_stream_callback_handle));
        cb_handle->callback = input_callback;
        cb_handle->user_data = user_data;
        handle->input_callback = cb_handle;

        int erl_err = enif_thread_create("PORTAUDIO_STREAM_THREAD",
                                         &handle->thread,
                                         pa_stream_thread_handler,
                                         handle, NULL);
        // Should not just be done in development
        assert(erl_err == 0);

        return paNoError;
}

static PaError pa_stream_exec_stop(struct pa_stream_handle *handle,
                                   PaError (*pa_func)(PaStream*))
{
        enif_rwlock_rwlock(handle->lock);

        assert(Pa_IsStreamActive(handle->stream) == true);
        PaError stream_err = pa_func(handle->stream);
        handle->terminate = true;

        enif_rwlock_rwunlock(handle->lock);

        return stream_err;
}

PaError pa_stream_stop(struct pa_stream_handle *handle)
{
        return pa_stream_exec_stop(handle, Pa_StopStream);
}

PaError pa_stream_abort(struct pa_stream_handle *handle)
{
        return pa_stream_exec_stop(handle, Pa_AbortStream);
}

void pa_stream_wait_for_termination(struct pa_stream_handle *handle)
{
        enif_thread_join(handle->thread, NULL);
}
