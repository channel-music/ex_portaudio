defmodule PortAudio.Native do
  # TODO: figure out what this actually does?
  @compile {:autoload, false}
  @on_load {:init, 0}

  def init do
    case load_nif() do
      :ok ->
        :ok

      reason ->
        raise "An error occured when loading portaudio: #{inspect(reason)}"
    end
  end

  ############################################################
  # Stubs
  ############################################################
  @type host_api_info :: [
          index: non_neg_integer,
          name: binary,
          type: non_neg_integer,
          device_count: non_neg_integer,
          default_input_device: non_neg_integer | nil,
          default_output_device: non_neg_integer | nil
        ]

  @type device_info :: [
          index: non_neg_integer,
          name: binary,
          host_api: non_neg_integer,
          max_input_channels: integer,
          max_output_channels: integer,
          default_low_input_latency: float,
          default_low_output_latency: float,
          default_high_input_latency: float,
          default_high_output_latency: float,
          default_sample_rate: float
        ]

  @type sample_format :: :float32 | :int32 | :int24 | :int16 | :int8 | :uint8
  @type stream_flag :: :noclip | :nodither | :nodropinput

  @type stream_params ::
          {device :: non_neg_integer, channel_count :: non_neg_integer,
           sample_format :: sample_format, suggested_latency :: float}

  @spec version() :: {integer, binary}

  # TODO: return major, minor and sub
  @doc """
  Returns a tuple containing the version number and version string
  of PortAudio.
  """
  def version, do: nif_error()

  @spec host_api_count() :: non_neg_integer

  @doc """
  Returns the number of host API's available on the system.
  """
  def host_api_count, do: nif_error()

  @spec host_api_info(non_neg_integer) :: {:ok, host_api_info} | {:error, atom}

  @doc """
  Returns the details of the host API at the given index.

  Will return `{:error, :not_found}` if not found.
  """
  def host_api_info(_index), do: nif_error()

  @spec host_api_index_from_type(non_neg_integer | atom) ::
          {:ok, integer} | {:error, :no_host_api}

  @doc """
  Returns a host API index value that has the given type.

  The type can be either a positive integer or an atom. Will return
  the API index on success or an error tuple on failure.
  """
  def host_api_index_from_type(_type), do: nif_error()

  @spec default_host_api_index() :: {:ok, non_neg_integer} | {:error, atom}

  @doc """
  Returns the default host API used by the system.
  """
  def default_host_api_index, do: nif_error()

  @spec device_index_from_host_api(non_neg_integer, non_neg_integer) ::
          {:ok, non_neg_integer} | {:error, atom}

  @doc """
  Returns the standard device index using host API specific device
  index.

  Will return an error tuple if either the host API index or the
  device index doesn't exist.
  """
  def device_index_from_host_api(_host_idx, _dev_idx), do: nif_error()

  @spec default_input_device_index() :: {:ok, non_neg_integer} | {:error, atom}

  @doc """
  Returns the index of the default input device used by the system.
  """
  def default_input_device_index, do: nif_error()

  @spec default_output_device_index() :: {:ok, non_neg_integer} | {:error, atom}

  @doc """
  Returns the index of the default output device used by the system.
  """
  def default_output_device_index, do: nif_error()

  @spec device_count() :: non_neg_integer

  @doc """
  Returns the total number of audio devices available on the system.

  Will return an error tuple if anything goes wrong.
  """
  def device_count, do: nif_error()

  @spec device_info(non_neg_integer) :: {:ok, device_info} | {:error, atom}

  @doc """
  Returns information about a device at the given index or `nil` if
  none exists.
  """
  def device_info(_index), do: nif_error()

  @spec stream_format_supported(
          input :: stream_params,
          output :: stream_params,
          sample_rate :: float
        ) :: boolean

  @doc """
  Returns `true` if the given format is supported or `false` otherwise.
  """
  def stream_format_supported(_input, _output, _sample_format), do: nif_error()

  @spec stream_open_default(
          input_channels :: non_neg_integer,
          output_channels :: non_neg_integer,
          sample_format :: sample_format,
          sample_rate :: float
        ) :: {:ok, reference} | {:error, atom}

  @doc """
  Open a new stream with the default input and output devices.

  If input channels is set to `0` then the device will only use
  output and vice-versa when output channels are set to `0`.
  """
  def stream_open_default(_input_channels, _output_channels, _sample_format, _sample_rate),
    do: nif_error()

  @spec stream_open(
          input_params :: stream_params,
          output_params :: stream_params,
          sample_rate :: float,
          flags :: [stream_flag]
        ) :: {:ok, reference} | {:error, atom}

  @doc """
  Open a new stream with the given input and output parameters.

  If the `input_params` are specified as `nil`, the device will only
  be used for output and vice-versa when `output_params` is `nil`.
  """

  def stream_open(_input_params, _output_params, _sample_rate, _flags), do: nif_error()

  @spec stream_start(reference) :: :ok | {:error, atom}

  @doc """
  Start the given opened stream.

  Will return errors if the stream is already opened or if an exceptional
  circumstance happens internally.
  """

  def stream_start(_stream), do: nif_error()

  @spec stream_stop(reference) :: :ok | {:error, atom}

  @doc """
  Gracefully stop the given stream. May not stop immediately as it waits
  for pending audio buffers to complete.

  Will return errors if the stream is already closed or if an exceptional
  circumtance happens internally.
  """

  def stream_stop(_stream), do: nif_error()

  @spec stream_abort(reference) :: :ok | {:error, atom}

  @doc """
  Terminate the stream immediately without waiting for pending audio
  buffers to complete.

  Will return errors if the stream is already closed or if an exceptional
  circumtance happens internally.
  """
  def stream_abort(_stream), do: nif_error()

  @spec stream_is_active(reference) :: boolean

  @doc """
  Returns `true` if the given stream is active.
  """

  def stream_is_active(_stream), do: nif_error()

  @spec stream_is_stopped(reference) :: boolean

  @doc """
  Returns `true` if the given stream is stopped. This includes streams
  that were aborted.
  """

  def stream_is_stopped(_stream), do: nif_error()

  @spec stream_read(reference) :: {:ok, binary} | {:error, atom}

  @doc """
  Read bytes from the given input stream.

  Will return `{:error, :stream_empty}` if there is no data to be read yet.
  `{:error, :output_only_stream}` will be returned if the device is only
  open for output.

  Other errors may be thrown by PortAudio, but they are considered
  exceptional.
  """

  def stream_read(_stream), do: nif_error()

  @spec stream_write(reference, binary) :: :ok | {:error, atom}

  @doc """
  Write the given data to an output stream. May block if the buffer
  is full.

  Will return `{:error, :input_only_stream}` if the device is only
  opened for input.

  Other errors may be thrown by PortAudio, but they are considered
  exceptional.
  """
  def stream_write(_stream, _data), do: nif_error()

  ############################################################
  # Nif utils
  ############################################################
  defp load_nif do
    path = :filename.join(:code.priv_dir(:portaudio), 'portaudio_nif')
    :erlang.load_nif(path, 0)
  end

  defp nif_error, do: :erlang.nif_error(:nif_not_loaded)
end
