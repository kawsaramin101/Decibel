# Decibel
### Simple music player app for desktop

---

## Features

- 🎵 **Playlist** 
- 🔁 **Repeat**
- 🔀 **Shuffle**
- 🔍 **Search** 
- ✅ **Simple**
- 🪶 **Not bloated** 

---

## Requirements

- Qt 6.8 or later
- CMake 3.16 or later
- A C++17-compatible compiler (GCC, Clang, MSVC)
- Qt modules: `Quick`, `Sql`, `Multimedia`, `Widgets`

---

## Build Instructions

### 1. Clone the repository

```bash
git clone https://github.com/kawsaramin101/Decibel.git
cd Decibel
```

### 2. Configure with CMake

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

### 3. Build

```bash
cmake --build build --parallel
```

### 4. Run

```bash
./build/appDecibel
```

---

## Build a .deb Package

Make sure you have completed the build steps above, then run CPack from the build directory:

```bash
cd build
cpack -G DEB
```

This produces a `.deb` package in the build directory, e.g.:

```
decibel-0.1-Linux.deb
```

### Install the package

```bash
sudo dpkg -i decibel-0.1-Linux.deb
```

If there are missing dependencies, resolve them with:

```bash
sudo apt-get install -f
```

---

## License

MIT License. See [LICENSE](LICENSE) for details.