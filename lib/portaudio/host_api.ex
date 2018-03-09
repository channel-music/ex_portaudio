defmodule PortAudio.HostAPI do
  alias PortAudio.HostAPI

  defstruct [
    :index,
    :name,
    # TODO: tranform to atom
    :type,
    :device_count,
    # TODO: suffix with _index
    :default_input_device,
    :default_output_device
  ]

  @type t :: %HostAPI{}

  @spec new(non_neg_integer) :: {:ok, t} | {:error, atom}

  @doc """
  Fetch a host API with the given device index.

  Will return `{:ok, %HostAPI{}}` if found or `{:error, msg}` otherwise.
  """
  def new(index) do
    with {:ok, host} <- PortAudio.Native.host_api_info(index) do
      {:ok, from_native(host)}
    end
  end

  @spec new(non_neg_integer) :: t | no_return

  @doc """
  Same as `new`, but will throw an exception if an error occurs.
  """
  def new!(index) do
    case new(index) do
      {:ok, host} ->
        host

      {:error, reason} ->
        raise reason
    end
  end

  @spec from_type(atom) :: {:ok, t} | {:error, atom}

  @doc """
  Fetch a device with the given `type` as an atom.

  ## Example

      iex> PortAudio.HostAPI.from_type(:alsa)
      {:ok, %PortAudio.HostAPI{}}
      iex> PortAudio.HostAPI.from_type(:blabla)
      {:error, :no_host_api}
  """
  def from_type(type) do
    with {:ok, index} <- PortAudio.Native.host_api_index_from_type(type) do
      new(index)
    end
  end

  defp from_native(host_api) do
    struct(HostAPI, host_api)
  end

  @spec default_input_device(t) :: {:ok, PortAudio.Device.t()} | {:error, atom}

  @doc """
  Return the default input device for the given host API, if any.
  """
  def default_input_device(%HostAPI{} = host_api) do
    with {:ok, index} =
           PortAudio.device_index_from_host_api(
             host_api.index,
             host_api.default_input_device
           ) do
      PortAudio.Device.new(index)
    end
  end

  @spec default_output_device(t) :: {:ok, PortAudio.Device.t()} | {:error, atom}

  @doc """
  Return the default output device for the given host API, if any.
  """
  def default_output_device(%HostAPI{} = host_api) do
    with {:ok, index} =
           PortAudio.device_index_from_host_api(
             host_api.index,
             host_api.default_output_device
           ) do
      PortAudio.Device.new(index)
    end
  end
end
