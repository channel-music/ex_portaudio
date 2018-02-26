defmodule PortAudio do
  alias PortAudio.Native

  def devices do
    # TODO: convert to struct
    for device_idx <- 0 .. Native.device_count() - 1 do
      Native.device_info(device_idx)
    end
  end

  def host_apis do
    for host_idx <- 0 .. Native.host_api_count() - 1 do
      Native.host_api_info(host_idx)
    end
  end
end
