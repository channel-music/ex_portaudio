defmodule PortAudio do
  alias PortAudio.Native

  @doc """
  Returns a list of audio devices available on the system.
  """
  def devices do
    for device_idx <- 0..(Native.device_count() - 1) do
      # Should never fail
      {:ok, device} = Native.device_info(device_idx)
      device
    end
  end

  def default_input_device do
    with {:ok, idx} <- Native.default_input_device_index() do
      Native.device_info(idx)
    end
  end

  def default_output_device do
    with {:ok, idx} <- Native.default_output_device_index() do
      Native.device_info(idx)
    end
  end

  @doc """
  Returns a list of native host API's available on the system.
  """
  def host_apis do
    for host_idx <- 0..(Native.host_api_count() - 1) do
      # Should never fail
      {:ok, host} = Native.host_api_info(host_idx)
      host
    end
  end

  def default_host_api do
    with {:ok, idx} <- Native.default_host_api_index() do
      Native.host_api_info(idx)
    end
  end

  def host_api_from_type(type) do
    with {:ok, idx} <- Native.host_api_index_from_type(type) do
      Native.host_api_info(idx)
    end
  end
end
