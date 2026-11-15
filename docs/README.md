# WoWControlSDK documentation

Technical documentation is available in English and Spanish. It is also published as a site with MkDocs Material, at <https://zerik-official.github.io/WoWControlSDK/docs/> (or locally with `mkdocs serve`).

## English

| Document | Content |
| --- | --- |
| [Overview](en/index.md) | Purpose of the SDK and how to navigate the docs |
| [Getting started](en/getting-started.md) | Tools, build, injection and first request |
| [Architecture](en/architecture.md) | Layers, data flow, threads, offsets and events |
| [Injection](en/injection.md) | Freeze-sync injector, arguments and alternative injectors |
| [RPC protocol](en/protocol.md) | Named pipes, JSON-RPC envelope, errors and client examples |
| [Methods](en/methods/index.md) | Every RPC domain and method with parameters and responses |
| [Operations](en/operations/building.md) | Building, desktop client, releases and the documentation site |
| [Glossary](en/glossary.md) | Transport, game and engineering terms |

## Español

| Documento | Contenido |
| --- | --- |
| [Introducción](es/index.md) | Propósito del SDK y cómo navegar la documentación |
| [Primeros pasos](es/getting-started.md) | Herramientas, compilación, inyección y primera petición |
| [Arquitectura](es/architecture.md) | Capas, flujo de datos, hilos, offsets y eventos |
| [Inyección](es/injection.md) | Inyector congelar-sincronizar, argumentos e inyectores alternativos |
| [Protocolo RPC](es/protocol.md) | Named pipes, sobre JSON-RPC, errores y ejemplos de cliente |
| [Métodos](es/methods/index.md) | Cada dominio y método RPC con parámetros y respuestas |
| [Operación](es/operations/building.md) | Compilación, cliente de escritorio, versiones y sitio de documentación |
| [Glosario](es/glossary.md) | Términos de transporte, del juego y de ingeniería |

## Local preview

Install the documentation dependencies and start the development server:

```bash
pip install -r requirements-docs.txt
mkdocs serve
```

The English site is served at `http://127.0.0.1:8001/WoWControlSDK/docs/` and the Spanish site at `http://127.0.0.1:8001/WoWControlSDK/docs/es/`.
