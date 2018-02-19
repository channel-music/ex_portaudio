defmodule PortAudio.Native do
  @compile {:autoload, false}
  @on_load {:init, 0}

  def init do
    case load_nif() do
      :ok -> :ok

      reason -> raise "An error occured when loading portaudio: #{inspect(reason)}"
    end
  end

  # TODO: generate these stubs
  def version, do: nif_error()

  # Native Host API's
  def default_host_api_index, do: nif_error()
  def device_index_from_host_api(_host_idx, _dev_idx),
    do: nif_error()
  def host_api_count, do: nif_error()
  def host_api_info(_index), do: nif_error()
  def host_api_index_from_type(_type), do: nif_error()

  # Devices
  def default_input_device_index, do: nif_error()
  def default_output_device_index, do: nif_error()
  def device_count, do: nif_error()
  def device_info(_index), do: nif_error()

  defp load_nif do
    path = :filename.join(:code.priv_dir(:portaudio), 'portaudio_nif')
    :erlang.load_nif(path, 0)
  end

  defp nif_error, do: :erlang.nif_error(:nif_not_loaded)
end
