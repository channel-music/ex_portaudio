# Assumes native library can be loaded properly and that
# portaudio returns some device drivers
#
# TODO: add a setup program to check that audio drivers
# TODO: exist before running tests
defmodule PortAudio.NativeTest do
  use ExUnit.Case

  alias PortAudio.Native

  describe "version/0" do
    test "returns the version number and text" do
      {version_num, version_str} = Native.version()

      assert is_integer(version_num)
      assert is_binary(version_str)
    end
  end

  describe "default_host_api_index/0" do
    test "returns the default host API index" do
      {:ok, index} = Native.default_host_api_index()
      assert is_integer(index)
      assert index >= 0 && Native.host_api_count()
    end
  end

  describe "host_api_count/0" do
    test "returns an integer of available host API's" do
      n = Native.host_api_count()
      assert is_integer(n)
      assert n >= 0
    end
  end

  describe "host_api_info/1" do
    test "returns a keyword list" do
      {:ok, info} =
        with {:ok, idx} <- Native.default_host_api_index() do
          Native.host_api_info(idx)
        end

      expected_fields = [
        :index,
        :name,
        :type,
        :device_count,
        :default_input_device,
        :default_output_device
      ]

      for f <- expected_fields, do: assert(info[f])
    end

    test "returns an error when host is not found" do
      assert {:error, :not_found} = Native.host_api_info(Native.host_api_count() + 1)
    end
  end

  describe "device_index_from_host_api/2" do
    test "returns the device index when parameters are valid" do
      {:ok, host_index} = Native.default_host_api_index()
      {:ok, host_info} = Native.host_api_info(host_index)
      device_index = host_info[:default_input_device]

      {:ok, index} = Native.device_index_from_host_api(host_index, device_index)
      assert is_integer(index)
      assert index >= 0 && index < Native.device_count()
    end

    test "returns an error with an invalid host index" do
      host_index = with {:ok, n} <- Native.host_api_count(), do: n + 1

      assert {:error, :invalid_host_api} = Native.device_index_from_host_api(host_index, 8)
    end

    test "returns an error with an invalid device index" do
      {:ok, host_index} = Native.default_host_api_index()
      device_index = 10_000

      assert {:error, :invalid_device} =
               Native.device_index_from_host_api(host_index, device_index)
    end
  end

  describe "host_api_index_from_type/1" do
    # FIXME: this assumes these types exist
    test "returns an index when given a correct type as an atom" do
      assert {:ok, _} = Native.host_api_index_from_type(:alsa)
    end

    test "returns an index when given a correct type as an integer" do
      {:ok, host_info} =
        with {:ok, idx} <- Native.default_host_api_index() do
          Native.host_api_info(idx)
        end

      {:ok, api_index} = Native.host_api_index_from_type(host_info[:type])
      {:ok, api_info} = Native.host_api_info(api_index)
      assert api_info == host_info
    end

    test "returns an error when given an invalid type ID" do
      assert {:error, :no_host_api} = Native.host_api_index_from_type(10_000)
      assert {:error, :no_host_api} = Native.host_api_index_from_type(:missing)
    end
  end

  describe "default_input_device_index/0" do
    def valid_index?({:ok, index}) do
      index >= 0 && index < Native.device_count()
    end

    def valid_index?(_) do
      false
    end

    test "returns the index of the default input device" do
      assert Native.default_input_device_index() |> valid_index?()
    end
  end

  describe "default_output_device_index/0" do
    test "returns the index of the default output device" do
      assert Native.default_output_device_index() |> valid_index?()
    end
  end

  describe "device_count/0" do
    test "returns the total number of devices" do
      n = Native.device_count()
      assert n >= 0
    end
  end

  describe "device_info/1" do
    test "returns a keyword list when device is found" do
      {:ok, info} =
        with {:ok, idx} <- Native.default_input_device_index() do
          Native.device_info(idx)
        end

      expected_fields = [
        :index,
        :name,
        :host_api,
        :max_input_channels,
        :max_output_channels,
        :default_low_input_latency,
        :default_low_output_latency,
        :default_high_input_latency,
        :default_high_output_latency,
        :default_sample_rate
      ]

      for f <- expected_fields, do: assert(info[f])
    end

    test "returns nil when device doesn't exist" do
      assert Native.device_info(10_000)
    end
  end

  # TODO: find a way to write this. It can be quite difficult to validate
  # describe "stream_format_supported/3" do

  # end

  describe "garbage collection" do
    test "resources released properly" do
      spawn(fn ->
        {:ok, s} = PortAudio.Native.stream_open_default(0, 2, :int16, 44100.0)
        PortAudio.Native.stream_start(s)
      end)

      # Garbage collected here
    end
  end
end
