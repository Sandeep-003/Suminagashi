# 🌊 Suminagashi - Digital Paper Marbling

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17%2B-blue.svg)](https://isocpp.org/)
[![Raylib](https://img.shields.io/badge/Raylib-5.0-orange.svg)](https://www.raylib.com/)
[![WebAssembly](https://img.shields.io/badge/WebAssembly-Emscripten-purple.svg)](https://emscripten.org/)

A modern, interactive web application that brings the ancient Japanese art of **Suminagashi** (paper marbling) to your browser. Create beautiful, flowing patterns with dynamic color palettes using cutting-edge web technologies.

![Suminagashi Demo](docs/demo.gif)

## 🎨 What is Suminagashi?

Suminagashi (墨流し, "floating ink") is a traditional Japanese paper marbling technique that dates back to the 12th century. Artists drop ink onto water's surface, manipulate the patterns with brushes or breath, then transfer the design onto paper. This digital recreation captures the essence of this meditative art form.

## ✨ Features

### 🎯 Core Features
- **Interactive Canvas**: Click anywhere to create marbling drops
- **15 Beautiful Color Palettes**: Carefully curated themes including:
  - Traditional Japanese Suminagashi (indigo & gray)
  - Turkish Ebru Classic (vibrant traditional colors)
  - Venetian Marble (warm stone tones)
  - Cherry Blossom (soft pink gradients)
  - Ocean, Forest, Sunset, and more!
- **Dynamic Color Selection**: Each drop randomly selects from themed palettes
- **Real-time Marbling Simulation**: Drops interact to create organic patterns
- **Screenshot Capture**: Save your artwork as PNG images
- **Responsive Design**: Works on desktop, tablet, and mobile

### 🛠️ Technical Features
- **WebAssembly Performance**: Compiled C++ code for near-native speed
- **WebGL Rendering**: Hardware-accelerated graphics
- **Modern Web Interface**: Dark theme with developer-friendly design
- **Cross-platform**: Runs in any modern web browser

## 🚀 Live Demo

**[Try it online →](https://sandeep-003.github.io/Suminagashi/)**

## 🏗️ Architecture

```
suminasashi_web_final/
├── 📁 src/
│   ├── suminasashi.cpp      # Main application loop
│   ├── Drops.cpp            # Drop physics and rendering
│   ├── drops.h              # Drop class declaration
│   ├── colors.cpp           # Color palette management
│   └── colors.h             # ColorPalette class declaration
├── 📁 docs/                 # Web deployment
│   ├── index.html           # Modern web interface
│   ├── suminasashi.js       # Compiled WebAssembly
│   └── suminasashi.wasm     # Binary WebAssembly module
├── 📁 build/                # Build artifacts
├── CMakeLists.txt           # Build configuration
└── README.md               # This file
```

## 🛠️ Building from Source

### Prerequisites

1. **Emscripten SDK**
   ```bash
   git clone https://github.com/Sandeep-003/Suminagashi.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh
   ```

2. **CMake** (3.10+)
3. **Modern C++ Compiler** (C++17 support)

### Build Steps

1. **Clone the repository**
   ```bash
   git clone https://github.com/Sandeep-003/Suminagashi.git
   cd Suminagashi
   ```

2. **Activate Emscripten environment**
   ```bash
   source path/to/emsdk/emsdk_env.sh  # Linux/macOS
   # or
   path\to\emsdk\emsdk_env.bat        # Windows
   ```

3. **Configure and build**
   ```bash
   mkdir build && cd build
   emcmake cmake ..
   emmake cmake --build .
   ```

4. **Deploy to web**
   ```bash
   cp web/suminasashi.js ../docs/
   cp web/suminasashi.wasm ../docs/
   ```

### Development Build (Native)

For faster development cycles, you can build natively:

```bash
mkdir build-native && cd build-native
cmake ..
make
./suminasashi
```

## 🎮 Usage

### Web Interface
1. **Visit the live demo** or serve locally
2. **Click on canvas** to create drops
3. **Watch patterns emerge** as drops interact
4. **Save screenshots** using the button
5. **Experiment** with different clicking patterns

### Controls
- **Left Click**: Create a new drop with random color
- **Screenshot Button**: Save current canvas as PNG
- **Clear Button**: Reset the canvas (if implemented)
- **Fullscreen Button**: Toggle fullscreen mode

## 🎨 Color Palettes

The application features 15 carefully designed color palettes:

| Palette | Description | Colors |
|---------|-------------|--------|
| **Sunset** | Warm coral, orange, golden tones | 🟠🟡🟤 |
| **Ocean** | Cool blues and teals | 🔵🟦💙 |
| **Traditional Japanese** | Classic indigo and gray | ⚫🔵⚪ |
| **Turkish Ebru** | Vibrant traditional marbling colors | 🔴🟡🟢🔵 |
| **Venetian Marble** | Warm stone and earth tones | 🟤🟫⚪ |
| **Cherry Blossom** | Soft pink gradients | 🌸💗🩷 |
| **Jade Garden** | Teal and aquamarine tones | 🟢💚🔷 |
| **Royal Purple** | Luxurious purple and gold | 🟣🟡👑 |
| **Midnight Blues** | Deep blue gradients | 🌌🔵💙 |
| **Earth Tones** | Natural browns and beiges | 🟫🟤🤎 |
| **Copper Verdigris** | Metallic copper with oxidized green | 🟫🟢💚 |
| **Wine & Cream** | Deep reds with creamy whites | 🍷⚪🔴 |

## 🧪 Technical Implementation

### Core Classes

#### `ColorPalette`
```cpp
class ColorPalette {
    using ColorValue = std::array<int, 4>; // RGBA
    using Palette = std::vector<ColorValue>;
    
public:
    ColorPalette();                    // Random palette selection
    ColorValue getColor();             // Random color from current palette
    static void addPalette(const Palette& palette);
    static const std::vector<Palette>& getAllPalettes();
};
```

#### `Drop`
```cpp
class Drop {
    Vector2 center;
    double radius;
    color clr;
    std::vector<Vector2> vertices;
    
public:
    Drop(float x, float y, color clr, double radius, int n);
    void Draw_drops();
    void marble(const Drop& other);    // Interaction with other drops
    void wavy_transformation();       // Visual effects
};
```

### Rendering Pipeline
1. **Initialization**: Setup Raylib window and WebGL context
2. **Input Handling**: Detect mouse clicks for drop creation
3. **Physics Update**: Calculate drop interactions and marbling effects
4. **Rendering**: Draw all drops with WebGL acceleration
5. **Screenshot**: Capture WebGL framebuffer for image export

### WebAssembly Integration
- **Emscripten**: Compiles C++ to WebAssembly
- **WebGL**: Hardware-accelerated graphics rendering
- **Memory Management**: Efficient handling of drop collections
- **JavaScript Bridge**: Seamless integration with web interface

## 🔮 Future Enhancements

### Phase 1: Enhanced Visuals
- [ ] **Particle Systems**: Add floating particles for atmosphere
- [ ] **Advanced Shaders**: Custom GLSL shaders for more realistic effects
- [ ] **Animation Curves**: Smooth transitions and easing
- [ ] **Multiple Brush Types**: Different drop styles and sizes

### Phase 2: Fluid Simulation Integration 🌊
The next major evolution will incorporate **realistic fluid dynamics**:

#### **Computational Fluid Dynamics (CFD)**
```cpp
class FluidSimulator {
    // Navier-Stokes equations for fluid motion
    void updateVelocityField(float dt);
    void advectColor(float dt);
    void applyViscosity(float dt);
    void projectDivergenceFree();
    
    // Surface tension effects
    void calculateSurfaceTension();
    void applyCapillaryWaves();
};
```

#### **Planned Fluid Features**
- **🌊 Realistic Flow**: Implement Navier-Stokes equations for authentic fluid motion
- **💧 Surface Tension**: Accurate droplet behavior and coalescence
- **🌀 Vorticity**: Swirling patterns and turbulence
- **⚗️ Viscosity**: Different ink viscosity properties
- **🎯 Advection**: Color transport through fluid medium
- **📐 Grid-based Solver**: High-performance fluid grid simulation
- **🔬 Multi-phase Flow**: Oil-water interactions
- **📊 Real-time Visualization**: Velocity field visualization

#### **Technical Implementation**
- **Eulerian Grid**: Uniform grid for velocity and pressure fields
- **FLIP/PIC Hybrid**: Particle-in-Cell method for detail preservation
- **GPU Acceleration**: Compute shaders for parallel processing
- **Adaptive Time Steps**: Stable simulation with variable timesteps

### Phase 3: Advanced Features
- [ ] **3D Marbling**: Three-dimensional fluid simulation
- [ ] **VR/AR Support**: Immersive marbling experiences
- [ ] **AI Pattern Generation**: Machine learning for pattern suggestions
- [ ] **Collaborative Mode**: Multi-user real-time marbling
- [ ] **Physical Simulation**: Paper absorption and texture effects

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

1. **Fork the repository**
2. **Create a feature branch** (`git checkout -b feature/amazing-feature`)
3. **Commit your changes** (`git commit -m 'Add amazing feature'`)
4. **Push to the branch** (`git push origin feature/amazing-feature`)
5. **Open a Pull Request**

### Development Guidelines
- Follow C++17 standards
- Maintain consistent code style
- Add comments for complex algorithms
- Test on multiple browsers
- Update documentation

## 🐛 Known Issues

- [ ] Screenshot may not work on some older browsers
- [ ] Mobile touch performance could be optimized
- [ ] Clear canvas function needs C++ implementation
- [ ] Memory usage grows with many drops (need cleanup)

## 📚 References

### Academic Papers
- **"Fluid Simulation for Computer Graphics"** - Robert Bridson
- **"Real-time Fluid Dynamics for Games"** - Jos Stam (GDC 2003)
- **"Particle-based Viscoelastic Fluid Simulation"** - Clavet et al.

### Traditional Art
- **Suminagashi History**: [Wikipedia](https://en.wikipedia.org/wiki/Paper_marbling)
- **Turkish Ebru Techniques**: Traditional marbling methods
- **Modern Paper Marbling**: Contemporary artistic approaches

### Technical Resources
- **[Raylib Documentation](https://www.raylib.com/)**
- **[Emscripten Guide](https://emscripten.org/docs/)**
- **[WebGL Specifications](https://www.khronos.org/webgl/)**

## 👨‍💻 Author

**Sandeep Saurav**
- GitHub: [@Sandeep-003](https://github.com/Sandeep-003)
- Portfolio: [Your Portfolio Link]
- Email: [Your Email]

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Raylib Community** for the amazing graphics library
- **Emscripten Team** for WebAssembly compilation
- **Traditional Suminagashi Artists** for the inspiration
- **Open Source Community** for tools and libraries

---

<div align="center">

**⭐ Star this repository if you found it interesting! ⭐**

Made with ❤️ and lots of ☕ by [Sandeep](https://github.com/Sandeep-003)

</div>
