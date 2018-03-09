defmodule PortAudio.Example.PlaySong do
  alias PortAudio.{Device, Stream}

  def play_song(filepath) do
    spawn(fn ->
      {:ok, dev} = PortAudio.default_output_device()
      out_stream = Device.stream!(dev,
        output: %{channel_count: 2, sample_format: :int16},
        sample_rate: 44100.0
      )

      File.stream!(filepath, [], 1024)
      |> Enum.each(fn chunk ->
        Stream.write!(out_stream, chunk)
      end)
      Stream.stop(out_stream)

      IO.puts "Done"
    end)
  end

  def print_usage do
    IO.puts """
    ERROR: Invalid command line arguments.

    Usage: mix run --no-halt examples/play_song.exs FILE
    """
  end
end

case System.argv() do
  [file] -> PortAudio.Example.PlaySong.play_song(file)
  _      -> PortAudio.Example.PlaySong.print_usage()
end
