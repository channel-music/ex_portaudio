defmodule PortAudio.Example.PlaySong do
  alias PortAudio.Native

  def play_song(filepath) do
    spawn(fn ->
      {:ok, out_stream} = Native.stream_open_default(0, 2, :int16, 44100.0)
      Native.stream_start(out_stream)

      File.stream!(filepath, [], 1024)
      |> Enum.each(fn chunk ->
        Native.stream_write(out_stream, chunk)
      end)

      Native.stream_stop(out_stream)
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
