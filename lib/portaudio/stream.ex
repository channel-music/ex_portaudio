defmodule PortAudio.StreamError do
  defexception [:reason, :message]
end

defmodule PortAudio.Stream do
  defstruct [:resource]

  @type t :: %PortAudio.Stream{}

  @type stream_params :: %{
          device: PortAudio.Device.t(),
          channel_count: non_neg_integer,
          sample_format: PortAudio.Native.sample_format(),
          suggested_latency: float
        }

  @spec new(
          input_params :: stream_params | nil,
          output_params :: stream_params | nil,
          sample_rate :: float
        ) :: {:ok, t} | {:error, atom}

  # TODO: accept flags
  @doc """
  Open a stream with the given input and output parameters.

  If the input parameters are nil then only then output device will be used
  and vice-versa for output parameters.
  """
  def new(input_params, output_params, sample_rate) do
    input_params =
      if input_params do
        param_map_to_native(input_params)
      end

    output_params =
      if output_params do
        param_map_to_native(input_params)
      end

    with {:ok, s} <- PortAudio.Native.stream_open(input_params, output_params, sample_rate, []) do
      {:ok, %PortAudio.Stream{resource: s}}
    end
  end

  @spec new!(
          input_params :: PortAudio.Native.stream_params() | nil,
          output_params :: PortAudio.Native.stream_params() | nil,
          sample_rate :: float
        ) :: t | no_return

  @doc """
  Same as `new`, but will throw a `PortAudio.StreamError` on failure instead
  of returning `{:error, reason}`.
  """
  def new!(input_params, output_params, sample_rate) do
    case new(input_params, output_params, sample_rate) do
      {:ok, s} ->
        s

      {:error, reason} ->
        raise PortAudio.StreamError, reason: reason
    end
  end

  defp param_map_to_native(map) do
    {
      map.device.index,
      map.channel_count,
      map.sample_format,
      map.suggested_latency
    }
  end

  @spec start(t) :: {:ok, t} | {:error, atom}

  @doc """
  Start the stream, returning `{:ok, stream}` on success or `{:error, reason}`
  otherwise.
  """
  def start(%PortAudio.Stream{resource: s} = stream) do
    with :ok <- PortAudio.Native.stream_start(s) do
      {:ok, stream}
    end
  end

  @spec stop(t) :: {:ok, t} | {:error, atom}

  @doc """
  Stop the stream, returning `{:ok, stream}` on success or `{:error, reason}`
  otherwise.
  """
  def stop(%PortAudio.Stream{resource: s} = stream) do
    with :ok <- PortAudio.Native.stream_stop(s) do
      {:ok, stream}
    end
  end

  @spec abort(t) :: {:ok, t} | {:error, atom}

  @doc """
  Abort the stream, immediately killing any processes. Returns `{:ok, stream}`
  on success or `{:error, reason}` on failure.
  """
  def abort(%PortAudio.Stream{resource: s} = stream) do
    with :ok <- PortAudio.Native.stream_abort(s) do
      {:ok, stream}
    end
  end

  @spec active?(t) :: boolean

  @doc """
  Returns `true` if the stream is active.
  """
  def active?(%PortAudio.Stream{resource: s}) do
    PortAudio.Native.stream_is_active(s)
  end

  @spec stopped?(t) :: boolean

  @doc """
  Returns `true` if the stream has been stopped either through calling `stop`
  or `abort`.
  """
  def stopped?(%PortAudio.Stream{resource: s}) do
    PortAudio.Native.stream_is_stopped(s)
  end

  @spec read(t) :: {:ok, binary} | {:error, atom}

  @doc """
  Read a binary from the audio stream. Will return `{:ok, binary}` on success
  or `{:error, reason}` on failure.
  """
  def read(%PortAudio.Stream{resource: s}) do
    PortAudio.Native.stream_read(s)
  end

  @spec read!(t) :: binary | no_return

  @doc """
  Same as `read`, but throws a `PortAudio.StreamError` instead of returning
  `{:error, reason}` on failure.
  """
  def read!(%PortAudio.Stream{} = stream) do
    case read(stream) do
      {:ok, data} ->
        data

      {:error, reason} ->
        raise PortAudio.StreamError, reason: reason
    end
  end

  @spec write(t, binary) :: :ok | {:error, atom}

  @doc """
  Attempt to write binary data to the stream, returning `:ok` on success or
  `{:error, reason}` on failure.
  """
  def write(%PortAudio.Stream{resource: s}, data) do
    PortAudio.Native.stream_write(s, data)
  end

  @spec write!(t, binary) :: :ok | no_return

  @doc """
  Same as `write`, but throws a `PortAudio.StreamError` instead of returning
  `{:error, reason}` on failure.
  """
  def write!(%PortAudio.Stream{} = stream, data) do
    case write(stream, data) do
      :ok ->
        :ok

      {:error, reason} ->
        raise PortAudio.StreamError, reason: reason
    end
  end

  defimpl Inspect do
    def inspect(_stream, _opts) do
      "#PortAudio.PortAudio.Stream<>"
    end
  end
end
