# Dev Containers

This directory defines the build environment using the open
[Development Containers](https://containers.dev/) specification. It is not tied
to one editor: VS Code (Dev Containers extension), JetBrains IDEs, GitHub
Codespaces, DevPod and the [`devcontainer` CLI](https://github.com/devcontainers/cli)
can all use it ([full list](https://containers.dev/supporting)).

```text
.devcontainer/
├── Dockerfile                 # Shared image (build tools, Qt6, SDL2, X11 libs)
├── default/
│   └── devcontainer.json      # Standard configuration (committed)
└── personal/                  # Your own configuration (git-ignored, optional)
    └── devcontainer.json
```

## Using the default configuration

- **VS Code:** open the repository and run **Dev Containers: Reopen in Container**.
  If more than one configuration exists, VS Code asks which one to use.
- **devcontainer CLI:**

  ```sh
  devcontainer up --workspace-folder . --config .devcontainer/default/devcontainer.json
  devcontainer exec --workspace-folder . --config .devcontainer/default/devcontainer.json bash
  ```

- **Other tools:** point them at `.devcontainer/default/devcontainer.json`
  (or `personal/`) when asked for a configuration.

The default configuration forwards X11 (GUI apps), `/dev/input` and the udev
database so that SDL can see controllers. It runs privileged with host networking.

> If GUI apps fail with `Can't open display`, run `xhost +local:` on the host.

## Personal configuration

To add your own settings (extra mounts, extensions, environment variables),
create `.devcontainer/personal/devcontainer.json`. The directory is listed in
`.gitignore`, so it is never committed.

Dev container configs cannot extend each other, so copy
`default/devcontainer.json` as a starting point and keep it in sync when the
default changes. Paths are relative to the config file, so keep:

```jsonc
"build": {
    "dockerfile": "../Dockerfile",
    "context": ".."
}
```

Example: additional bind mount from your home directory:

```jsonc
"mounts": [
    // ...mounts copied from default/devcontainer.json...
    "source=${localEnv:HOME}/.my-tool,target=/home/vscode/.my-tool,type=bind,consistency=cached"
]
```

If you use VS Code, the `dev.containers.defaultExtensions` user setting installs
your favourite extensions in every dev container without touching the repository.
