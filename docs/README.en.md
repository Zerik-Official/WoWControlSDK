# WoWControlSDK Documentation

## Index

- [Architecture](./EN/01-architecture.md)
- [Injection](./EN/02-injection.md)
- [RPC Methods](./EN/03-methods/)
  - [lua.*](./EN/03-methods/lua.md)
  - [unit.*](./EN/03-methods/unit.md)
  - [group.*](./EN/03-methods/group.md)
  - [player.*](./EN/03-methods/player.md)
  - [world.*](./EN/03-methods/world.md)
  - [object.*](./EN/03-methods/object.md)
  - [client.*](./EN/03-methods/client.md)
  - [events.*](./EN/03-methods/events.md)
  - [dll.*](./EN/03-methods/dll.md)

## Structure

```
docs/
├── README.en.md        ← This file (english index)
├── README.es.md        ← Spanish index
├── EN/                 ← English docs
│   ├── 01-architecture.md
│   ├── 02-injection.md
│   └── 03-methods/
│       ├── lua.md
│       ├── unit.md
│       ├── group.md
│       ├── player.md
│       ├── world.md
│       ├── object.md
│       ├── client.md
│       ├── events.md
│       └── dll.md
└── ES/                 ← Spanish docs (mirrors EN/)
    ├── 01-arquitectura.md
    ├── 02-inyeccion.md
    └── 03-metodos/
        ├── lua.md
        ├── unit.md
        ├── group.md
        ├── player.md
        ├── world.md
        ├── object.md
        ├── client.md
        ├── events.md
        └── dll.md
```

## Conventions

- Each RPC domain gets its own file under `03-methods/` (or `03-metodos/`)
- Every method within a domain is documented with:
  - **Name** — full RPC method name
  - **Params** — parameter table with types and descriptions
  - **Response** — response structure with types
  - **Example** — request/response JSON
  - **Notes** — edge cases, limitations, implementation details
- Spanish and English docs mirror the same structure
