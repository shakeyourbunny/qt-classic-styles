<!-- This file is part of the qt-classic-styles Project.
     License: LGPL-2.1-only (inherited from Qt). Contact: qt-classic-styles-project@trinity2k.net -->

# Qt Classic Styles

A collection of classic Qt widget styles packaged as Qt 6 style plugins. Four
of them (Motif, CDE, Plastique, Cleanlooks) shipped with Qt for years before
Qt dropped them into the qtstyleplugins repository and stopped maintaining it.
This project ports them forward so they build and run against Qt 6. The fifth,
Photon, is new code that recreates the look of the QNX 6.2 Photon microGUI.
Each style installs as a plugin you can select per application or set as the
default for your whole desktop.

## Styles

- **Motif** is the classic X11 look: beveled edges, sharp corners, and the
  chunky button relief that defined Unix workstations in the early 1990s.
- **CDE** is the Common Desktop Environment variation of Motif. Same bones,
  a flatter and more restrained treatment.
- **Plastique** brings the translucent gradients and rounded highlights that
  were the default style in KDE 3.
- **Cleanlooks** is the clean, near-flat style that came out of the early
  GNOME look, with soft gradients and thin borders.
- **Photon** recreates the grey, etched panel look of the QNX 6.2 Photon
  microGUI. It was measured from public QNX 6.2.1 screenshots and written from
  scratch. It contains no QNX source code and is not a port of anything.

## Building

You need Qt 6.5 or later, a C++17 compiler, and CMake 3.24 or later.

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

The build produces one plugin file per style (`.so` on Linux, `.dll` on
Windows). To build and run the Photon test suite, turn on `BUILD_TESTING`:

```bash
cmake -DBUILD_TESTING=ON ..
cmake --build .
ctest
```

## Installation

There are three ways to make the plugins available to Qt.

**Install into the Qt prefix (system-wide).** From your build directory,
`cmake --install` copies the plugins into Qt's own `plugins/styles/`
directory. Every Qt application on the system finds them there.

```bash
cmake --install .
```

**Copy the plugin files by hand.** The style plugins are ordinary shared
objects. Put them in any directory that has a `styles/` subfolder, for example
`~/.local/qtplugins/styles/`, and point Qt at the parent directory (see the
next method). Copy the `.so` files (or `.dll` on Windows) from the build
output.

**Point `QT_PLUGIN_PATH` at your own directory.** Qt searches every path in
`QT_PLUGIN_PATH` for a `styles/` subfolder. If you placed the plugins in
`~/.local/qtplugins/styles/`, set:

```bash
export QT_PLUGIN_PATH=~/.local/qtplugins
```

## Usage

Once a plugin is installed, activate the style by its name: `motif`, `cde`,
`plastique`, `cleanlooks`, or `photon`.

**One application, environment variable.** Set `QT_STYLE_OVERRIDE` for a single
launch:

```bash
QT_STYLE_OVERRIDE=photon ./your-app
```

**In code.** Call `setStyle` before you create widgets:

```cpp
QApplication::setStyle("photon");
```

**All Qt applications with qt6ct.** On a desktop that is not KDE or GNOME, use
qt6ct to pick a style once for every Qt application. Install qt6ct from your
distribution, then tell Qt to use it as the platform theme:

```bash
export QT_QPA_PLATFORMTHEME=qt6ct
```

Put that line in `~/.profile` or `/etc/environment` so it applies to every
session. Launch `qt6ct`, open the Appearance tab, and pick the style from the
dropdown. qt6ct writes the choice to its config, and every Qt application picks
it up on next start.

**KDE Plasma.** Open System Settings, go to Appearance, then Application Style,
and select the style from the list.

For the full Photon look on KDE, two extras ship in `extras/kde/`:

- **Aurorae title bars.** Copy `extras/kde/aurorae/Photon/` into
  `~/.local/share/aurorae/themes/`, then choose it under System Settings,
  Appearance, Window Decorations.
- **Color scheme.** Copy `extras/kde/Photon.colors` into
  `~/.local/share/color-schemes/`, then select it under System Settings,
  Appearance, Colors.

## Environment variables

| Variable | What it does |
|---|---|
| `QT_STYLE_OVERRIDE` | Forces a style for one application. Set it to a style name (`photon`, `motif`, and so on) on the command line. |
| `QT_QPA_PLATFORMTHEME` | Selects the platform theme. Set it to `qt6ct` on desktops that are not KDE or GNOME so qt6ct controls the style. |
| `QT_PLUGIN_PATH` | Adds directories to Qt's plugin search. Each entry is scanned for a `styles/` subfolder. |
| `QT_QPA_PLATFORM` | Selects the windowing backend. Set it to `offscreen` to run Qt without a display, which is what the test suite uses for headless runs. |

## License

The four ported styles (Motif, CDE, Plastique, Cleanlooks) are LGPL-2.1,
inherited from the original Qt source they came from. Photon is new code under
LGPL-2.1-or-later. See `LICENSE` for the full text and `THIRD_PARTY_LICENSES.txt`
for the attribution of the ported code.

SJ-PKG-0010

---
qt-classic-styles has been written with the help of tooled assistance, but has
been designed, reviewed and thoroughly tested and signed off by its developer.
