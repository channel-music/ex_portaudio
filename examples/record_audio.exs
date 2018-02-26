defmodule PortAudio.Example.RecordAudio do
  alias PortAudio.Native

  def record_audio(filepath, duration \\ 10_000) do
    p = spawn(fn ->
      {:ok, in_stream} = Native.stream_open_default(1, 0, :int16, 44100.0)
      Native.stream_start(in_stream)
      record_loop(in_stream, filepath)
    end)

    :timer.send_after(duration, p, :stop)
  end

  defp record_loop(stream, filepath, pcmdata \\ "") do
    receive do
      :stop ->
        File.write!(filepath, pcmdata)
        IO.puts "Done"
    after 0 ->
        case Native.stream_read(stream) do
          {:ok, data} -> record_loop(stream, filepath, pcmdata <> data)
          _           -> record_loop(stream, filepath, pcmdata)
        end
    end
  end

  def print_usage do
    IO.puts """
    ERROR: Invalid command line arguments.

    Usage: mix run --no-halt examples/record_audio.exs FILE [DURATION]
    """
  end
end

case System.argv() do
  [file] ->
    PortAudio.Example.RecordAudio.record_audio(file)

  [file, duration] ->
    {duration, _} = Integer.parse(duration)
    PortAudio.Example.RecordAudio.record_audio(file, duration)

  _ ->
    PortAudio.Example.RecordAudio.print_usage()
end
