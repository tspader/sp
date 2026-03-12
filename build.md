# Build

## Linux

Build a test target:

```sh
spn-2026-01-14 build -t fs -p debug -f
```

Run it:

```sh
./build/debug/store/bin/fs
```

Replace `fs` with any other test target, like `str`.

## Windows

Start a `tmux` session and `ssh spader@piotr`. Then, from `C:/Users/spader/source/sp`, load the MSVC environment:

```powershell
. .\tools\windows\devenv.ps1
```

Build and run fs tests:

```powershell
msbuild .\tools\windows\sp\fs.vcxproj /t:Run
```

Build and run the main amalgamated test app:

```powershell
msbuild .\tools\windows\sp\sp.vcxproj /t:Run
```
