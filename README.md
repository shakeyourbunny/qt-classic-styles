<!-- This file is part of the qt-classic-styles Project.
     License: LGPL-2.1-only (inherited from Qt). Contact: qt-classic-styles-project@trinity2k.net -->

# Qt Classic Styles

Classic Qt widget styles for Qt 6: Motif, CDE, Plastique and Cleanlooks,
ported from Qt, plus Photon, the look of the QNX 6 Photon microGUI.

These styles were part of Qt until they were moved to the
qtstyleplugins repository and left unmaintained. This project brings
them back as Qt 6 style plugins.

## Building

Requires Qt 6.5 or later and CMake 3.24+.

```
mkdir build && cd build
cmake ..
cmake --build .
```

## Installation

Copy the built `.so` (or `.dll`) files into your Qt installation's
`plugins/styles/` directory, or point `QT_PLUGIN_PATH` to a directory
containing a `styles/` subfolder with the plugins.

## Usage

Set the style via environment variable:

```
QT_STYLE_OVERRIDE=motif ./your-app
```

Or in code:

```cpp
QApplication::setStyle("motif");   // or "cde", "plastique", "cleanlooks", "photon"
```

## Styles

- **Motif** - the classic Motif/X11 beveled look
- **CDE** - Common Desktop Environment variation of Motif
- **Plastique** - translucent gradients, the default KDE 3 style
- **Cleanlooks** - clean flat-ish style from early GNOME
- **Photon** - the grey, etched look of QNX 6.2 Photon

## License

Motif, CDE, Plastique and Cleanlooks are LGPL-2.1, inherited from the
original Qt source. Photon is new code under LGPL-2.1-or-later. See LICENSE
and THIRD_PARTY_LICENSES.txt.

SJ-PKG-0010

---
qt-classic-styles has been written with the help of tooled assistance,
but has been designed, reviewed and thoroughly tested and signed off by
its developer.
