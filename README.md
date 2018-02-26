# ex_portaudio

PortAudio bindings for elixir.

**WARNING: This project is heavily WIP, don't expect it to work at all.**

## Prerequisites

- Working C compiler along with Make build tools
- Erlang >= 20
- PortAudio

## Installation

There is currently no hex package, so installation can be done using the git
repository directly.

```elixir
def deps do 
  [{:ex_portaudio, git: "https://github.com/channel-music/ex_portaudio"}]
end
```

## TODO

- Ensure that stream resources are properly deallocated

## License

This project is licensed under BSDv3 to Antonis Kalou.  

See the [license file](LICENSE) for more details.
