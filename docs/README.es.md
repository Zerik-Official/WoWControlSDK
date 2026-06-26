# Documentación de WoWControlSDK

<p align="center"><a href="README.en.md">View in English</a></p>

## Índice

- [Arquitectura](./ES/01-arquitectura.md)
- [Inyección](./ES/02-inyeccion.md)
- [Métodos RPC](./ES/03-metodos/)
  - [lua.*](./ES/03-metodos/lua.md)
  - [unit.*](./ES/03-metodos/unit.md)
  - [group.*](./ES/03-metodos/group.md)
  - [player.*](./ES/03-metodos/player.md)
  - [world.*](./ES/03-metodos/world.md)
  - [object.*](./ES/03-metodos/object.md)
  - [client.*](./ES/03-metodos/client.md)
  - [events.*](./ES/03-metodos/events.md)
  - [dll.*](./ES/03-metodos/dll.md)

## Estructura

```
docs/
├── README.en.md        ← Índice en inglés
├── README.es.md        ← Este archivo (índice español)
├── EN/                 ← Documentación en inglés
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
└── ES/                 ← Documentación en español
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

## DeepWiki

El proyecto está indexado en [DeepWiki](https://deepwiki.com/Zerik-Official/WoWControlSDK) — puedes hacerle preguntas sobre el código para respuestas rápidas. El índice se actualiza cada 7 días.

## Convenciones

- Cada dominio RPC tiene su propio archivo dentro de `03-methods/` o `03-metodos/`
- Cada método dentro de un dominio se documenta con:
  - **Nombre** — nombre completo del método RPC
  - **Parámetros** — tabla con tipos y descripciones
  - **Respuesta** — estructura de la respuesta con tipos
  - **Ejemplo** — JSON de petición/respuesta
  - **Notas** — casos borde, limitaciones, detalles de implementación
- La documentación en español e inglés mantiene la misma estructura
