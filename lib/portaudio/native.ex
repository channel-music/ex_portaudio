defmodule PortAudio.Native do
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
    name: binary,
    type: non_neg_integer,
    device_count: non_neg_integer,
    default_input_device: non_neg_integer,
    default_output_device: non_neg_integer
  ]

  @type device_info :: [
    name: binary,
    host_api: non_neg_integer,
    max_input_channels: non_neg_integer,
    max_output_channels: non_neg_integer,
    default_low_input_latency: float,
    default_low_output_latency: float,
    default_high_input_latency: float,
    default_high_output_latency: float,
    default_sample_rate: float
  ]


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

  @spec host_api_info(non_neg_integer) :: host_api_info | nil

  @doc """
  Returns the details of the host API at the given index.

  Will return an error if not found.
  """
  def host_api_info(_index), do: nif_error()

  @spec host_api_index_from_type(non_neg_integer | atom)
          :: integer | {:error, :no_host_api}

  @doc """
  Returns a host API index value that has the given type.

  The type can be either a positive integer or an atom. Will return
  the API index on success or an error tuple on failure.
  """
  def host_api_index_from_type(_type), do: nif_error()

  @spec default_host_api_index() :: non_neg_integer

  @doc """
  Returns the default host API used by the system.
  """
  def default_host_api_index, do: nif_error()

  @spec device_index_from_host_api(non_neg_integer, non_neg_integer)
    :: non_neg_integer | {:error, atom}

  @doc """
  Returns the standard device index using host API specific device
  index.

  Will return an error tuple if either the host API index or the
  device index doesn't exist.
  """
  def device_index_from_host_api(_host_idx, _dev_idx),
    do: nif_error()

  @spec default_input_device_index() :: non_neg_integer

  @doc """
  Returns the index of the default input device used by the system.
  """
  def default_input_device_index, do: nif_error()

  @spec default_output_device_index() :: non_neg_integer

  @doc """
  Returns the index of the default output device used by the system.
  """
  def default_output_device_index, do: nif_error()

  @spec device_count() :: non_neg_integer | {:error, atom}

  @doc """
  Returns the total number of audio devices available on the system.

  Will return an error tuple if anything goes wrong.
  """
  def device_count, do: nif_error()

  @spec device_info(non_neg_integer) :: device_info | nil

  @doc """
  Returns information about a device at the given index or `nil` if
  none exists.
  """
  def device_info(_index), do: nif_error()

  @type stream_params :: {device            :: non_neg_integer,
                          channel_count     :: non_neg_integer,
                          # TODO: provide allowed formats
                          sample_format     :: atom,
                          suggested_latency :: float}

  @spec stream_format_supported(
    input       :: stream_params,
    output      :: stream_params,
    sample_rate :: float
  ) :: boolean

  @doc """
  Returns `true` if the given format is supported or `false` otherwise.
  """
  def stream_format_supported(_input, _output, _sample_format),
    do: nif_error()

  ############################################################
  # Nif utils
  ############################################################
  defp load_nif do
    path = :filename.join(:code.priv_dir(:portaudio), 'portaudio_nif')
    :erlang.load_nif(path, 0)
  end

  defp nif_error, do: :erlang.nif_error(:nif_not_loaded)
end
