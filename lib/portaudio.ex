defmodule PortAudio do
  alias PortAudio.Native

  @doc """
  Returns a list of audio devices available on the system.
  """
  def devices do
    for device_idx <- 0..(Native.device_count() - 1) do
      # TODO: check for nil
      Native.device_info(device_idx)
    end
  end

  def default_input_device do
    case Native.default_input_device_index() do
      {:error, reason} -> {:error, reason}
      idx -> Native.device_info(idx)
    end
  end

  def default_output_device do
    case Native.default_output_device_index() do
      {:error, reason} -> {:error, reason}
      idx -> Native.device_info(idx)
    end
  end

  @doc """
  Returns a list of native host API's available on the system.
  """
  def host_apis do
    for host_idx <- 0..(Native.host_api_count() - 1) do
      Native.host_api_info(host_idx)
    end
  end

  def default_host_api do
    case Native.default_host_api_index() do
      {:error, reason} -> {:error, reason}
      idx -> Native.host_api_info(idx)
    end
  end

  def host_apis_from_type(type) do
    case Native.host_api_from_type(type) do
      nil -> {:error, :not_found}
      idx -> Native.host_api_info(idx)
    end
  end
end
