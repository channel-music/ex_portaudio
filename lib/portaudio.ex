defmodule PortAudio do
  @moduledoc """
  Low level PortAudio bindings for elixir.

  ## Querying devices and host API's

      iex> PortAudio.devices
      # list of devices
      iex> PortAudio.host_apis
      # list of host API's
      iex> PortAudio.device(0)
      {:ok, ...}
      iex> PortAudio.device(10_000)
      {:error, :not_found}

  ## Getting default devices

  These are default input and output devices used by the system.
  These functions may return an error if an input or output device
  doesn't exist on the host system.

      iex> PortAudio.default_input_device()
      {:ok, device}
      iex> PortAudio.default_output_device()
      {:ok, device}

  ## Starting an audio stream

  To write to the default output device, the stream must first be opened
  and then started. The returned number of bytes can vary.

      iex> {:ok, s} = PortAudio.open_default_stream(0, 2, :int16, 44100.0)
      {:ok, <REF>}
      iex> PortAudio.start_stream(s)
      iex> PortAudio.read(s)
      {:ok, <<binary>>}

  To read from the default input device, the process is similar.

      iex> {:ok, s} = PortAudio.open_default_stream(2, 0, :int16, 44100.0)
      {:ok, <REF>}
      iex> PortAudio.start_stream(s)
      iex> PortAudio.write(s, <<some PCM data>>)
      :ok

  Check the examples directory for more thorough examples.
  """

  alias PortAudio.Native

  @spec devices :: [PortAudio.Device.t()]

  @doc """
  Returns a list of audio devices available on the system.
  """
  def devices do
    for device_idx <- 0..(Native.device_count() - 1) do
      {:ok, dev} = PortAudio.Device.new(device_idx)
      dev
    end
  end

  @spec device(non_neg_integer) :: {:ok, PortAudio.Device.t()} | {:error, atom}

  @doc """
  Returns the device with the given index or an error if not found.
  """
  def device(idx) when is_integer(idx) do
    PortAudio.Device.new(idx)
  end

  @spec default_input_device :: {:ok, PortAudio.Device.t()} | {:error, atom}

  @doc """
  Returns the default input device or an error if it is unavailable.

  Will return an error if there is no default input device  was an internal
  error in PortAudio.
  """
  def default_input_device do
    with {:ok, idx} <- Native.default_input_device_index() do
      PortAudio.Device.new(idx)
    end
  end

  @spec default_output_device :: {:ok, PortAudio.Device.t()} | {:error, atom}

  @doc """
  Returns the default output device or an error if it is unavailable.

  Will return an error if there is no default output device  was an internal
  error in PortAudio.
  """
  def default_output_device do
    with {:ok, idx} <- Native.default_output_device_index() do
      PortAudio.Device.new(idx)
    end
  end

  @spec host_apis :: [PortAudio.HostAPI.t()]

  @doc """
  Returns a list of native host API's available on the system.
  """
  def host_apis do
    for host_idx <- 0..(Native.host_api_count() - 1) do
      # Should never fail
      {:ok, host} = PortAudio.HostAPI.new(host_idx)
      host
    end
  end

  @spec host_api(non_neg_integer) :: {:ok, PortAudio.HostAPI.t()} | {:error, atom}

  @doc """
  Returns the host API with the given index or an error if not found.
  """
  def host_api(idx) do
    PortAudio.HostAPI.new(idx)
  end

  @spec default_host_api :: {:ok, PortAudio.HostAPI.t()} | {:error, atom}

  @doc """
  Returns the default host API used by the system.

  Will return an error if the API is unavailable or if there was an internal
  error in PortAudio.
  """
  def default_host_api do
    with {:ok, idx} <- Native.default_host_api_index() do
      PortAudio.HostAPI.new(idx)
    end
  end
end
