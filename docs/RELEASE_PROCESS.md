# Flux Professional Release Process

This document outlines the professional standards and steps for packaging and distributing the Flux compiler.

## 1. Versioning Protocol

Flux follows **Semantic Versioning (SemVer)**: `major.minor.patch`.

- **Major**: Breaking language changes.
- **Minor**: New features, non-breaking.
- **Patch**: Bug fixes, performance improvements.

### Unified Version Update

To update the version across all project metadata (CMake, Windows Resource, package.json):

```bash
./scripts/version.sh <new_version>
```

### 📋 Places to Edit (Managed by script)

The `scripts/version.sh` tool handles these files. If adding new components, update the script accordingly:

1.  **`CMakeLists.txt`**: `project(Flux VERSION ...)`
2.  **`package.json`**: Root `"version"` field.
3.  **`editors/vscode/package.json`**: Extension `"version"` field.
4.  **`tools/flux/flux.rc`**: Windows binary metadata (FileVersion, ProductVersion).

## 2. Supported Platforms & Architectures

We aim for maximum reach by targeting the most common CPU architectures and operating systems.

| OS          | Architecture         | Target Label   | Notes                                        |
| ----------- | -------------------- | -------------- | -------------------------------------------- |
| **Windows** | x86_64               | `x64`          | Standard desktop (Win 10/11)                 |
| **Linux**   | x86_64, AArch64      | `x64`, `arm64` | Compatible with Ubuntu, Debian, Fedora, etc. |
| **macOS**   | Intel, Apple Silicon | `Universal`    | Supports both Intel and M1/M2/M3 chips       |

> [!NOTE]
> **Docker for CI**: We use Docker in GitHub Actions for Linux builds to ensure the binary is linked against a stable `glibc` version (Ubuntu 24.04), preventing "version not found" errors on older distros.

## 3. Commit Standards

All contributions must use **Conventional Commits**.

- `feat:` for new features
- `fix:` for bug fixes
- `docs:` for documentation changes
- `chore:` for maintenance

Use `npm run commit` (via `commitizen`) for a guided wizard. **Husky** will prevent non-compliant commits.

## 4. Pre-release Checklist

- [ ] Ensure all unit tests pass (`make test`).
- [ ] Verify Linux build inside Docker (`./scripts/docker-test.sh`).
- [ ] Update `CHANGELOG.md` with notes from conventional commits.
- [ ] Run `./scripts/version.sh X.Y.Z` to sync versions.
- [ ] Tag the release: `git tag -a vX.Y.Z -m "Release vX.Y.Z"`.

## 5. CI/CD & Signing

Pushing a tag triggers the `Release Build` workflow.

### Binary Attestation & Keyless Signing

- **GitHub Attestation**: Automatically generated for all release artifacts.
- **Keyless Sigstore (Cosign)**: Binaries are signed using **Keyless Signing**. This uses your GitHub identity (OIDC) to prove the origin without requiring manual private keys or secrets.
- **Windows Metadata**: `flux.exe` contains full publisher info in its resource block.
  +> [!IMPORTANT]
  +> **Windows Signing**: We will use **Keyless Sigstore** (via GitHub Actions). This is the modern, open standard that uses your GitHub identity to prove the binary's origin. It eliminates the need for manual private keys. However, note that while this provides high-quality proof of origin, Windows SmartScreen may still show an "Unknown Publisher" warning until the certificate gains reputation.

## 5. Supported Platforms Matrix

| OS          | Architecture               | Package Format      |
| ----------- | -------------------------- | ------------------- |
| **Windows** | x64                        | ZIP, NSIS Installer |
| **Linux**   | x64, ARM64                 | .deb, .tar.gz       |
| **macOS**   | Universal (Intel/M-series) | DMG, .tar.gz        |

## 6. Distribution Message

Professional releases should include a clear identification message. The compiler will display:
`Flux Compiler vX.Y.Z (built for <target> on <date>)`
This info is derived from the CMake project version and the Windows `.rc` metadata.
