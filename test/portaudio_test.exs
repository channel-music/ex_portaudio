defmodule PortaudioTest do
  use ExUnit.Case
  doctest Portaudio

  test "greets the world" do
    assert Portaudio.hello() == :world
  end
end
