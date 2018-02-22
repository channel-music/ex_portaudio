#ifndef _PORTAUDIO_NIF_PA_STREAM_
#define _PORTAUDIO_NIF_PA_STREAM_

#include <portaudio.h>
#include <stdlib.h>

/**
 * Opaque struct representing a handle to a PortAudio stream.
 */
struct pa_stream_handle;

// TODO: use a more specific type for input_buf
/**
 * A callback function for PortAudio stream events.
 */
typedef void (*pa_stream_callback)(PaError status,
                                   void *input_buf,
                                   size_t in_len,
                                   void *user_data);

/**
 * Allocates a new stream handle, retuning NULL on failure.
 */
struct pa_stream_handle *pa_stream_alloc(void);

/**
 * Deallocate all data related to the stream handle. This will not
 * ensure the proper termination of threads, `pa_stream_abort` or
 * `pa_stream_stop` must be called first.
 */
void pa_stream_dealloc(struct pa_stream_handle *handle);

/**
 * Open a new stream using the default input and output devices.
 */
PaError pa_stream_open_default(struct pa_stream_handle *handle,
                                      int num_input_channels,
                                      int num_output_channels,
                                      PaSampleFormat sample_format,
                                      double sample_rate,
                                      unsigned long frames_per_buffer);

/**
 * Start a PortAudio stream (assumes that it is already initialized
 * with `pa_stream_open`).
 *
 * Accepts an `input_callback` that is called when data from an
 * input stream is received. This callback is thread safe. Avoid running
 * blocking functions in this callback as it will slow down the stream
 * handling process.
 *
 * Also accepts `user_data`, a pointer to some data that will be passed
 * to the callback.
 */
PaError pa_stream_start(struct pa_stream_handle *handle,
                        pa_stream_callback input_callback,
                        void *user_data);

/**
 * Stop a PortAudio stream. This may not take action imedietaly before
 * everything is properly cleaned up.
 */
PaError pa_stream_stop(struct pa_stream_handle *handle);

/**
 * Abort the running stream. This will forcefully kill both the
 * background thread and the audio stream itself.
 */
PaError pa_stream_abort(struct pa_stream_handle *handle);

/**
 * Wait for the termination of any background threads.
 */
void pa_stream_wait_for_termination(struct pa_stream_handle *handle);

#endif // _PORTAUDIO_NIF_PA_STREAM_
