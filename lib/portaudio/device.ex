defmodule PortAudio.Device do
  alias PortAudio.Device

  defstruct [
    :index,
    :name,
    :host_api_index,
    :max_input_channels,
    :max_output_channels,
    :default_input_latency,
    :default_output_latency,
    :default_sample_rate
  ]

  @type t :: %Device{}

  @doc """
  Fetch a device with the given device index.

  Will return `{:ok, %Device{}}` if found or `{:error, msg}` otherwise.
  """
  def new(index) do
    with {:ok, device} <- PortAudio.Native.device_info(index) do
      {:ok, from_native(device)}
    end
  end

  defp from_native(device) do
    # TODO: use struct() function
    %Device{
      index: device.index,
      name: device.name,
      host_api_index: device.host_api,
      max_input_channels: device.max_input_channels,
      max_output_channels: device.max_output_channels,
      default_input_latency: [
        low: device.default_low_input_latency,
        high: device.default_high_input_latency
      ],
      default_output_latency: [
        low: device.default_low_output_latency,
        high: device.default_high_output_latency
      ],
      default_sample_rate: device.default_sample_rate
    }
  end

  @doc """
  Return the HostAPI of the given device.
  """
  def host_api(%Device{host_api_index: host_idx}) do
    {:ok, host_api} = PortAudio.HostAPI.new(host_idx)
    host_api
  end

  @type stream_params :: %{
          channel_count: non_neg_integer,
          sample_format: PortAudio.Native.sample_format(),
          suggested_latency: float | :high | :low
        }

  @spec stream(device :: t, params :: params) :: {:ok, PortAudio.Stream.t()} | {:error, atom}
        when params: [
               input: stream_params | nil,
               output: stream_params | nil,
               sample_rate: float | nil
             ]

  @doc """
  Opens and starts a stream for the given device.

  ## Options

      * `sample_rate` - The sample rate to use while streaming. Defaults to the
      sample rate of the device if not set.
      * `input` - The input stream parameters or `nil` if using an output
      only stream.
      * `output` - The output stream parameters or `nil` if using an input
      only stream.

  If neither `input` or `output` are set an `ArgumentError` exception will
  be raised.
  """
  def stream(%Device{} = device, params) do
    sample_rate = Keyword.get(params, :sample_rate, device.default_sample_rate)

    input_params =
      add_defaults_to_params(
        device,
        Keyword.get(params, :input),
        device.default_input_latency
      )

    output_params =
      add_defaults_to_params(
        device,
        Keyword.get(params, :output),
        device.default_output_latency
      )

    if !input_params and !output_params do
      raise ArgumentError, "Either input or output parameters are expected"
    end

    with {:ok, s} <- PortAudio.Stream.new(input_params, output_params, sample_rate),
         {:ok, s} <- PortAudio.Stream.start(s),
         do: {:ok, s}
  end

  @doc """
  Same as `PortAudio.Device.stream/2`, but will raise an exception on failure
  instead of returning an error tuple.
  """
  def stream!(%Device{} = device, params) do
    case stream(device, params) do
      {:ok, s} -> s
      {:error, reason} -> raise reason
    end
  end

  defp add_defaults_to_params(_device, nil, _latencies) do
    nil
  end

  defp add_defaults_to_params(device, params, default_latencies) do
    # PortAudio defaults to high latency
    params
    |> Map.update(:suggested_latency, :high, fn
      :low -> default_latencies[:low]
      :high -> default_latencies[:high]
      val -> val
    end)
  end
end
