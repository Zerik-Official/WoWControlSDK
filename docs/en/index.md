# WoWControlSDK

WoWControlSDK is a lightweight injectable DLL and SDK for **World of Warcraft 3.3.5a (build 12340)**. It exposes the internal methods of the game through **JSON-RPC 2.0** over Windows named pipes, so launchers, overlays, automation tools and other external applications can read game state and control the client from any language.

The DLL runs inside `Wow.exe`. A client connects to `\\.\pipe\WowGameCommand`, sends one JSON-RPC request and receives one JSON response. A second pipe, `\\.\pipe\WowGameEvent`, streams events such as screen changes, loading progress and subscribed WoW UI events.

## At a glance

| Item | Value |
| --- | --- |
| Supported client | World of Warcraft 3.3.5a, build 12340 |
| Platform | Windows 10 or 11, x64 |
| Transport | Windows named pipes |
| Protocol | JSON-RPC 2.0, one request per connection |
| Command pipe | `\\.\pipe\WowGameCommand` |
| Event pipe | `\\.\pipe\WowGameEvent` |
| RPC domains | `lua`, `unit`, `group`, `player`, `world`, `object`, `client`, `events`, `dll`, `window` |
| Language support | Any language that can open a named pipe |

## Where to start

| If you want to | Read |
| --- | --- |
| Build the DLL, inject it and send a first request | [Getting started](getting-started.md) |
| Understand how the layers and threads fit together | [Architecture](architecture.md) |
| Know exactly how injection works | [Injection](injection.md) |
| Connect from Python, C# or another language | [RPC protocol](protocol.md) |
| Look up a method and its response | [Methods](methods/index.md) |
| Build a release or read the CI | [Operations](operations/building.md) |
| Look up a term | [Glossary](glossary.md) |

## Contents

**Guides**

- [Getting started](getting-started.md): prerequisites, build, injection, first request and local documentation.
- [Architecture](architecture.md): components, layered design, request and event flow, threading and offsets.
- [Injection](injection.md): the freeze-sync injector, its arguments, alternative injectors and startup commands.
- [RPC protocol](protocol.md): pipe behavior, JSON-RPC envelope, errors, method catalog and client examples.

**Reference**

- [Methods](methods/index.md): conventions shared by every domain and links to each method page.
- [lua.*](methods/lua.md), [unit.*](methods/unit.md), [group.*](methods/group.md), [player.*](methods/player.md), [world.*](methods/world.md), [object.*](methods/object.md), [client.*](methods/client.md), [events.*](methods/events.md), [dll.*](methods/dll.md) and [window.*](methods/window.md).
- [Glossary](glossary.md): transport, game and engineering terms.

**Operations**

- [Building](operations/building.md): requirements, CMake targets and output layout.
- [CLI tool](operations/cli.md): the interactive Python CLI and the live event listener.
- [Releases](operations/releases.md): tags, artifacts, manifest and GitHub Releases.
- [Documentation site](operations/documentation.md): MkDocs, translations, validation and deployment.

## Scope

The SDK is intended for legitimate integrations such as launchers, stream overlays, Discord Rich Presence and UI automation. It is not a hacking, cheating or botting tool, although many methods can support botting. Use it responsibly.

Only an unmodified or lightly modified **3.3.5a (build 12340)** client is supported. Other builds, including later expansions, do not share the same memory layout or Lua environment.
