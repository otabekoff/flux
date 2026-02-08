# PHASE 1

1. Add sigstore and all details and publisher info and other info needed to make windows not to consider binaries as viruses. 

2. docker setup, is it good to use docker for github actions too? what about local and remote testing?

3. And note we've to consider windows(compatible with x86, x64, both and amd, arm like stuffs), linux(different distros, amd, arm like stuffs, different architectures, x86, x64, both), macos(different architectures, x86, x64, both, arm, amd, intel, apple silicon/bionic chip or other). By this I mean we need to release professionally, do the best. Also document about release, tagging or whatever about packaging and distributing, release should have some info message.

4. Also write appended message to readme.dev.md file about local building release and debug and testing files with it. Is it good to move to LLVM 18 on local too? 

5. make semantic versioning and conventional commit stuff, lintsaged and commitlint and husky. List places to edit before publishing new version. And tool to change those parts?





# PHASE 2
Below is a **production-grade, cross-platform CI/CD** setup similar in spirit to what modern language projects use (Rust, Zig, Nim style pipelines).

This provides:

* Unified LLVM setup across OSes
* Build caching (faster CI)
* Multi-platform builds
* Automatic release binaries
* Clean, minimal, stable workflow

---

# What this CI does

### On every push / PR

* Builds on:

  * Linux
  * Windows
  * macOS
* Uses LLVM + Ninja
* Uses caching for faster builds

### On tagged releases (e.g. `v0.1.0`)

* Builds binaries for all platforms
* Uploads them as release assets automatically

---

# Final production-grade `ci.yml`

Replace your current file with this.

```yaml
name: CI

on:
  push:
    branches: [main, master]
    tags: ["v*"]
  pull_request:
    branches: [main, master]

jobs:
  build:
    name: Build (${{ matrix.os }})
    runs-on: ${{ matrix.os }}
    strategy:
      fail-fast: false
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]

    steps:
    - uses: actions/checkout@v4

    # -------------------------
    # Install Ninja
    # -------------------------
    - name: Install Ninja (Linux)
      if: matrix.os == 'ubuntu-latest'
      run: |
        sudo apt-get update
        sudo apt-get install -y ninja-build clang llvm-dev

    - name: Install Ninja (macOS)
      if: matrix.os == 'macos-latest'
      run: brew install ninja llvm

    - name: Install LLVM + Ninja (Windows)
      if: matrix.os == 'windows-latest'
      run: |
        choco install llvm --version=18.1.8 -y
        choco install ninja -y

    # -------------------------
    # Export LLVM paths
    # -------------------------
    - name: Set LLVM env (macOS)
      if: matrix.os == 'macos-latest'
      run: |
        echo "LLVM_DIR=$(brew --prefix llvm)/lib/cmake/llvm" >> $GITHUB_ENV
        echo "$(brew --prefix llvm)/bin" >> $GITHUB_PATH

    - name: Set LLVM env (Windows)
      if: matrix.os == 'windows-latest'
      shell: pwsh
      run: |
        echo "LLVM_DIR=C:\Program Files\LLVM\lib\cmake\llvm" >> $env:GITHUB_ENV
        echo "C:\Program Files\LLVM\bin" >> $env:GITHUB_PATH

    # -------------------------
    # Cache build
    # -------------------------
    - name: Cache build
      uses: actions/cache@v4
      with:
        path: build
        key: ${{ runner.os }}-build-${{ hashFiles('**/CMakeLists.txt') }}
        restore-keys: |
          ${{ runner.os }}-build-

    # -------------------------
    # Configure
    # -------------------------
    - name: Configure
      run: cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

    # -------------------------
    # Build
    # -------------------------
    - name: Build
      run: cmake --build build

    # -------------------------
    # Test
    # -------------------------
    - name: Test
      run: ctest --test-dir build --output-on-failure

    # -------------------------
    # Package binaries
    # -------------------------
    - name: Package
      if: startsWith(github.ref, 'refs/tags/v')
      shell: bash
      run: |
        mkdir dist
        if [ "$RUNNER_OS" = "Windows" ]; then
          cp build/*.exe dist/
          cd dist
          7z a flux-windows.zip *.exe
        else
          cp build/* dist/ || true
          cd dist
          tar -czf flux-${RUNNER_OS}.tar.gz *
        fi

    # -------------------------
    # Upload artifacts
    # -------------------------
    - name: Upload artifact
      if: startsWith(github.ref, 'refs/tags/v')
      uses: actions/upload-artifact@v4
      with:
        name: flux-${{ matrix.os }}
        path: dist/*
```

---

# Separate release workflow (auto-publish binaries)

Create:

```
.github/workflows/release.yml
```

```yaml
name: Release

on:
  workflow_run:
    workflows: ["CI"]
    types:
      - completed

jobs:
  release:
    if: >
      github.event.workflow_run.conclusion == 'success' &&
      startsWith(github.event.workflow_run.head_branch, 'refs/tags/v')
    runs-on: ubuntu-latest

    steps:
    - name: Download artifacts
      uses: actions/download-artifact@v4
      with:
        path: dist

    - name: Create GitHub Release
      uses: softprops/action-gh-release@v2
      with:
        files: dist/**/*
```

---

# How to trigger a release

Tag your repo:

```bash
git tag v0.1.0
git push origin v0.1.0
```

CI will:

1. Build on all platforms
2. Package binaries
3. Create GitHub release
4. Upload binaries automatically

---

# What you now have (production-level features)

| Feature               | Status |
| --------------------- | ------ |
| Cross-platform builds | ✅      |
| Unified LLVM setup    | ✅      |
| Build caching         | ✅      |
| Automatic testing     | ✅      |
| Release binaries      | ✅      |
| Artifact uploads      | ✅      |
| Tag-based releases    | ✅      |

---

# Optional next upgrades (used by serious language projects)

If you want to go fully professional:

### Performance

* ccache / sccache integration
* LTO builds for releases

### Tooling

* Static analyzers
* Fuzz testing
* Sanitizer builds (ASan/UBSan)

### Distribution

* Windows installer
* macOS notarized app
* Linux AppImage

---

If you want, I can next:

* Add **sccache** (builds become 5–10× faster)
* Add **nightly builds**
* Set up **package manager publishing**
* Create a **professional release strategy** like Rust or Zig.


Below is a **production-grade upgrade** of your pipeline adding:

1. **sccache** (fast incremental builds)
2. **Nightly builds**
3. **Package manager publishing structure**
4. **Professional release strategy** similar to Rust/Zig

Everything is designed to be **simple, reliable, and scalable**.

---

# 1) Add sccache (5–10× faster builds)

`sccache` caches compiled object files across CI runs.

## Update your CI build job

Add this step **before Configure**.

```yaml
    # -------------------------
    # Install sccache
    # -------------------------
    - name: Install sccache
      uses: mozilla-actions/sccache-action@v0.0.4
```

---

## Add cache for sccache

Add this after checkout:

```yaml
    - name: Cache sccache
      uses: actions/cache@v4
      with:
        path: ~/.cache/sccache
        key: ${{ runner.os }}-sccache-${{ hashFiles('**/CMakeLists.txt') }}
        restore-keys: |
          ${{ runner.os }}-sccache-
```

---

## Tell CMake to use sccache

Replace your configure step with:

```yaml
    - name: Configure
      run: |
        cmake -B build -G Ninja \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_C_COMPILER_LAUNCHER=sccache \
          -DCMAKE_CXX_COMPILER_LAUNCHER=sccache
```

---

## Show cache statistics (optional but useful)

Add after build:

```yaml
    - name: sccache stats
      run: sccache --show-stats || true
```

---

# 2) Nightly builds (automatic)

Create:

```
.github/workflows/nightly.yml
```

```yaml
name: Nightly Build

on:
  schedule:
    - cron: "0 2 * * *"   # every day at 02:00 UTC
  workflow_dispatch:

jobs:
  nightly:
    uses: ./.github/workflows/ci.yml
```

This:

* runs every day
* builds all platforms
* ensures the project never breaks

---

# 3) Package manager publishing strategy

Modern language projects use:

| Platform | Tool                     |
| -------- | ------------------------ |
| Windows  | Scoop or Winget          |
| macOS    | Homebrew                 |
| Linux    | APT, Pacman, or tarballs |

We’ll prepare the structure first.

---

## Step 1: Standard release artifact names

Modify your package step:

```bash
flux-v${{ github.ref_name }}-${RUNNER_OS}.tar.gz
```

Windows:

```bash
flux-v${{ github.ref_name }}-windows.zip
```

---

## Step 2: Directory structure for releases

Your releases should contain:

```
flux/
 ├── bin/
 │    └── flux
 ├── README.md
 └── LICENSE
```

Modify packaging step:

```yaml
- name: Package
  if: startsWith(github.ref, 'refs/tags/v')
  shell: bash
  run: |
    mkdir -p dist/flux/bin

    if [ "$RUNNER_OS" = "Windows" ]; then
      cp build/*.exe dist/flux/bin/flux.exe
      cd dist
      7z a flux-${{ runner.os }}.zip flux
    else
      cp build/* dist/flux/bin/flux || true
      cd dist
      tar -czf flux-${{ runner.os }}.tar.gz flux
    fi
```

---

# 4) Professional release strategy (Rust/Zig style)

## Versioning model

Use **semantic versioning**:

```
v0.1.0   ← first release
v0.2.0   ← new features
v0.2.1   ← bugfix
v1.0.0   ← stable language
```

---

## Branch model

```
main        → stable development
nightly     → experimental features
release/*   → release candidates
```

Example:

```
release/0.2.0
```

---

## Release pipeline stages

### Stage 1 — Development

* commits to `main`
* CI builds + tests

### Stage 2 — Nightly

* automatic builds
* no guarantees
* for early adopters

### Stage 3 — Release candidate

Tag:

```
v0.2.0-rc1
```

CI:

* builds
* tests
* produces binaries

### Stage 4 — Stable release

Tag:

```
v0.2.0
```

CI:

* builds
* signs binaries
* publishes release
* updates package managers

---

# 5) Optional: automatic Homebrew + Scoop publishing

Later you can add:

### Homebrew

Auto-update:

```
homebrew-flux/flux.rb
```

### Scoop

Auto-update:

```
scoop-bucket/flux.json
```

Triggered by release workflow.

---

# Final upgraded CI features

| Feature                   | Status |
| ------------------------- | ------ |
| Cross-platform builds     | ✅      |
| LLVM unified setup        | ✅      |
| sccache acceleration      | ✅      |
| Nightly builds            | ✅      |
| Release automation        | ✅      |
| Package-ready artifacts   | ✅      |
| Semantic versioning model | ✅      |
| Professional release flow | ✅      |

---

# What serious language projects additionally do

| Feature               | Rust | Zig | You (possible next) |
| --------------------- | ---- | --- | ------------------- |
| sccache               | ✅    | ✅   | Now                 |
| Nightly channel       | ✅    | ✅   | Now                 |
| Self-hosting compiler | ✅    | 🚧  | Future              |
| Fuzz testing          | ✅    | ✅   | Next step           |
| Benchmark CI          | ✅    | ✅   | Optional            |
| Installer scripts     | ✅    | ✅   | Easy to add         |


1. Add **fuzz testing CI**
2. Add **self-hosting milestone plan**
3. Create **official installer scripts** (curl | sh, PowerShell installer)

Those are the steps that move a language from “project” to “ecosystem.”

## Full check

Full Professional Release Asset Matrix
1) Windows
x86_64 (mainstream)
flux-0.1.0-windows-x86_64.zip
flux-0.1.0-windows-x86_64-installer.exe
flux-0.1.0-windows-x86_64.msi

ARM64 (modern laptops, Surface, etc.)
flux-0.1.0-windows-arm64.zip
flux-0.1.0-windows-arm64-installer.exe
flux-0.1.0-windows-arm64.msi

x86 (32-bit legacy)
flux-0.1.0-windows-x86.zip
flux-0.1.0-windows-x86-installer.exe

2) macOS
Universal build (recommended primary)
flux-0.1.0-macos-universal.dmg

Separate architecture builds
flux-0.1.0-macos-x86_64.dmg
flux-0.1.0-macos-aarch64.dmg

Portable tarballs
flux-0.1.0-macos-x86_64.tar.gz
flux-0.1.0-macos-aarch64.tar.gz

3) Linux (major distros)
Generic portable builds
flux-0.1.0-linux-x86_64.tar.gz
flux-0.1.0-linux-aarch64.tar.gz
flux-0.1.0-linux-armv7.tar.gz

Debian / Ubuntu
flux-0.1.0-linux-x86_64.deb
flux-0.1.0-linux-aarch64.deb

Red Hat / Fedora / Rocky / Alma
flux-0.1.0-linux-x86_64.rpm
flux-0.1.0-linux-aarch64.rpm

Arch Linux (binary package)
flux-0.1.0-linux-x86_64.pkg.tar.zst

Alpine Linux (musl)
flux-0.1.0-linux-x86_64-musl.tar.gz
flux-0.1.0-linux-aarch64-musl.tar.gz

4) BSD systems (advanced/professional coverage)
flux-0.1.0-freebsd-x86_64.tar.gz
flux-0.1.0-openbsd-x86_64.tar.gz
flux-0.1.0-netbsd-x86_64.tar.gz

5) Source distributions
flux-0.1.0-src.tar.gz
flux-0.1.0-src.zip


(Separate from GitHub auto-generated source archives)

6) Editor and tooling
VS Code extension
flux-0.1.0.vsix

Language server (if separate)
flux-lsp-0.1.0-windows-x86_64.zip
flux-lsp-0.1.0-linux-x86_64.tar.gz
flux-lsp-0.1.0-macos-universal.tar.gz

7) Documentation bundle

Offline docs:

flux-0.1.0-docs.zip
flux-0.1.0-docs.tar.gz

8) Checksums and signatures (security-critical)
Checksums
flux-0.1.0-checksums.txt

GPG signature
flux-0.1.0-checksums.txt.asc


This is mandatory for serious language projects.

Complete professional asset list (summary)
Windows
flux-0.1.0-windows-x86_64.zip
flux-0.1.0-windows-x86_64-installer.exe
flux-0.1.0-windows-x86_64.msi
flux-0.1.0-windows-arm64.zip
flux-0.1.0-windows-arm64-installer.exe
flux-0.1.0-windows-arm64.msi
flux-0.1.0-windows-x86.zip
flux-0.1.0-windows-x86-installer.exe

macOS
flux-0.1.0-macos-universal.dmg
flux-0.1.0-macos-x86_64.dmg
flux-0.1.0-macos-aarch64.dmg
flux-0.1.0-macos-x86_64.tar.gz
flux-0.1.0-macos-aarch64.tar.gz

Linux
flux-0.1.0-linux-x86_64.tar.gz
flux-0.1.0-linux-aarch64.tar.gz
flux-0.1.0-linux-armv7.tar.gz
flux-0.1.0-linux-x86_64.deb
flux-0.1.0-linux-aarch64.deb
flux-0.1.0-linux-x86_64.rpm
flux-0.1.0-linux-aarch64.rpm
flux-0.1.0-linux-x86_64.pkg.tar.zst
flux-0.1.0-linux-x86_64-musl.tar.gz
flux-0.1.0-linux-aarch64-musl.tar.gz

BSD
flux-0.1.0-freebsd-x86_64.tar.gz
flux-0.1.0-openbsd-x86_64.tar.gz
flux-0.1.0-netbsd-x86_64.tar.gz

Tooling & source
flux-0.1.0-src.tar.gz
flux-0.1.0-src.zip
flux-0.1.0.vsix
flux-0.1.0-docs.zip
flux-0.1.0-docs.tar.gz
flux-0.1.0-checksums.txt
flux-0.1.0-checksums.txt.asc


# Make better

Create official installer scripts (curl | sh, PowerShell installer) for all windows, linux, macos.

1s
Run cd build
CMake Error at D:/a/flux/flux/build/CPackConfig.cmake:26 (set):
  Syntax error in cmake code at
    D:/a/flux/flux/build/CPackConfig.cmake:27
  when parsing string
    
          CreateShortCut '$DESKTOP\Flux Compiler.lnk' '$INSTDIR\bin\flux.exe'
      
  Invalid character escape '\F'.
CPack Error: CPack project name not specified
CPack Error: CPack project name not specified
Error: Process completed with exit code 1.