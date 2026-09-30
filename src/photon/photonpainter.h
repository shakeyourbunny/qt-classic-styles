// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#ifndef PHOTONPAINTER_H
#define PHOTONPAINTER_H

#include <QColor>
#include <QPalette>
#include <QPolygon>
#include <QRect>

class QPainter;

// Drawing primitives for the Photon style. Every value in here is measured
// from the QNX 6.2.1 screenshots in reference/qnx-photon/.
//
// All functions take logical rects and draw in device pixels: a Photon pixel
// is floor(dpr) device pixels, measured from the device edges of the rect.
// At a fractional ratio such as Plasma's 1.0625 a 1 px line stays one device
// pixel and a slant steps one pixel per row. Pieces that share an outline
// are drawn by one call so the shared edge cannot round apart.
namespace photon {

// Photon's PtBasic fill contrast default; the button gradient spans +/- this.
inline constexpr int kFillContrast = 20;

struct Colors
{
    QColor window;
    QColor button;
    QColor base;
    QColor text;
    QColor disabledText;
    QColor light;
    QColor shade;
    QColor outline;
    QColor fieldShade;
    QColor midlight;
    QColor highlight;
    QColor highlightedText;

    QColor fieldFill;
    QColor gradientTop;
    QColor gradientBottom;
    QColor checkOutline;
    QColor checkEtchDark;
    QColor checkEtchLight;
    QColor etchedDark;
    QColor etchedLight;
    QColor stripDark;
    QColor stripLight;
    QColor tabInactive;
    QColor tabInactiveLight;
    QColor tabInactiveText;
    QColor tabSlantLight;
    QColor trough;
    QColor troughShade;
    QColor panelTop;
    QColor panelBottom;
    QColor panelShade;
    QColor menuItem;
    QColor menuItemOpen;
    QColor menuFrameLight;
    QColor menuFrameShade;
    QColor headerLight;
    QColor headerShade;
    QColor toolCheckedShade;
    QColor sliderGrooveLight;
    QColor sliderEtch;
    QColor progressTrough;
    QColor progressTroughShade;
    QColor progressFill;
    QColor progressFillLight;
    QColor progressFillShade;
    QColor progressEnd;
    QColor arrow;
    QColor arrowDisabled;
    QColor menuHover;
    QColor focus;
};

// The measured QNX 6.2.1 default scheme mapped onto Qt roles.
QPalette standardPalette();

// Ring colours (outline, bevel, shades) follow Window and Button with the
// measured offsets, not the palette's Light/Dark/Mid/Shadow, which Plasma
// computes with its own contrast formula.
Colors colorsFromPalette(const QPalette &pal);

// Adds delta to every channel, clamped; Photon derives its shades this way.
QColor shifted(const QColor &c, int delta);

// --- Basic rings and fills ---------------------------------------------

// The alpha etch ring: black a20 on top/left, white a35 on bottom/right.
void drawEtch(QPainter *p, const QRect &r);

// Ramp from base+contrast to base-contrast (top to bottom for Vertical, left
// to right for Horizontal), one step per device line. reversed swaps ends.
// inset is in Photon pixels from the edges of r.
void fillGradient(QPainter *p, const QRect &r, const QColor &base, int contrast,
                  Qt::Orientation orientation, bool reversed, int inset = 0);

void fillInset(QPainter *p, const QRect &r, int inset, const QColor &color);

// A ring of one Photon pixel at the given inset.
void drawRing(QPainter *p, const QRect &r, int inset, const QColor &color);

// One side of r at the given inset, full length. Invalid colour: nothing.
void drawSide(QPainter *p, const QRect &r, Qt::Edge side, int inset, const QColor &color);

// Bevel at the inset: topLeft on the top and left, bottomRight on the other
// two. The top line stops short of the right side, as Photon's does.
void drawBevel(QPainter *p, const QRect &r, int inset, const QColor &topLeft,
               const QColor &bottomRight);

// Outline ring at r, bevel ring inside it. sunken swaps light and shade.
void drawRaisedFrame(QPainter *p, const QRect &r, const Colors &c, bool sunken);

// Outline ring at r, field shade on the inner top row and left column only.
void drawFieldFrame(QPainter *p, const QRect &r, const Colors &c, const QColor &outline);

// A line of `dark` followed by a parallel line of `light` (etched groove).
void drawEtchedLine(QPainter *p, const QPoint &from, Qt::Orientation orientation, int length,
                    const QColor &dark, const QColor &light);

// Two nested rings, dark outside at the top-left and light offset by one.
void drawEtchedRect(QPainter *p, const QRect &r, const QColor &dark, const QColor &light);

// --- Composite elements ------------------------------------------------

// Push button body: optional etch, outline, bevel, gradient.
void drawButton(QPainter *p, const QRect &r, const Colors &c, bool sunken, bool etched);

// Text field: etch, outline, top/left shade, fill. An invalid fill leaves the
// inside alone; shadeColor defaults to the field shade.
void drawField(QPainter *p, const QRect &r, const Colors &c, const QColor &fill,
               const QColor &shadeColor = QColor());

// Menu bar and tool bar panel: outline, white top line, ramp, shade line.
void drawPanel(QPainter *p, const QRect &r, const Colors &c);

// The raised arrow button of an editable combo, sharing the field outline
// on the right of r. width is in Photon pixels.
void drawComboArrowButton(QPainter *p, const QRect &r, int width, bool pressed, bool enabled,
                          const Colors &c);

// Spin box buttons stacked on the right of r, sharing its field outline.
void drawSpinButtons(QPainter *p, const QRect &r, int width, bool upPressed, bool downPressed,
                     bool upEnabled, bool downEnabled, const Colors &c);

struct ScrollBarState
{
    Qt::Orientation orientation = Qt::Vertical;
    int buttonLength = 16;     // Photon pixels, outlines included
    double sliderStart = 0.0;  // fraction of the free travel, 0..1
    double sliderLength = 1.0; // fraction of the groove, 0..1
    int sliderMinimum = 10;    // Photon pixels
    bool showSlider = true;
    bool subPressed = false;
    bool addPressed = false;
    bool subEnabled = true;
    bool addEnabled = true;
};

// The whole scroll bar: trough, arrow buttons and slider sharing outlines.
void drawScrollBar(QPainter *p, const QRect &r, const ScrollBarState &s, const Colors &c);

// Slider handle with the etch line below (or right of) it.
void drawSliderHandle(QPainter *p, const QRect &r, Qt::Orientation orientation, const Colors &c);

void drawProgressGroove(QPainter *p, const QRect &r, const Colors &c);

// fraction 0..1 of the inner length; reversed fills from the far end.
void drawProgressBar(QPainter *p, const QRect &r, double fraction, Qt::Orientation orientation,
                     bool reversed, const Colors &c);

// Header section. Sections after the first reuse their left neighbour's
// outline column instead of drawing their own.
void drawHeaderSection(QPainter *p, const QRect &r, bool first, bool sunken, const Colors &c);

// Popup menu frame: rim, light top/left, shade bottom/right.
void drawMenuFrame(QPainter *p, const QRect &r, const QColor &rim, const Colors &c);

// --- Glyphs ----------------------------------------------------------------

struct ArrowShape
{
    int headBase;
    int headLength;
    int stemWidth;
    int stemLength;
};

// Combo box and menu arrows; scroll bar arrows; tree branch triangles.
inline constexpr ArrowShape kComboArrow{8, 4, 4, 2};
inline constexpr ArrowShape kScrollArrow{9, 5, 3, 2};
inline constexpr ArrowShape kBranchArrow{7, 4, 0, 0};

// Solid arrow centred in r. The head points in the given direction.
void drawArrow(QPainter *p, const QRect &r, Qt::ArrowType type, const QColor &color,
               const ArrowShape &shape);

// The check box X: two 1 px diagonals over 8x8 with softened side pixels.
void drawCross(QPainter *p, const QRect &r, const QColor &color, const QColor &background);

// Header sort chevron, 7 wide and 4 tall, outline only.
void drawChevron(QPainter *p, const QRect &r, bool up, const QColor &color);

// The 12 px shaded radio sphere plus its 1 px light etch below and right.
void drawRadio(QPainter *p, const QRect &r, bool checked, bool enabled, const Colors &c);

// The Photon check box in r (14x14 at 1x): strong etch, outline, box, X.
void drawCheckBox(QPainter *p, const QRect &r, bool on, bool enabled, bool noChange, const Colors &c);

// A north tab (other shapes go through the painter transform). Selected
// tabs run to the bottom of r and merge with the pane; unselected ones stop
// two rows short and leave the triangle their left neighbour owns.
void drawTab(QPainter *p, const QRect &r, bool selected, bool clipLeftNeighbour, const Colors &c);

// Outline polygon of a north tab: chamfered top-left corner, 45 degree
// right edge that starts h/2 inside the rect at the top and ends h/2 past it
// at the bottom, where h is the rect height.
QPolygon tabOutline(const QRect &r);

// Region the left neighbour's slant owns inside r; a later-painted tab
// leaves it alone so neighbours stack left over right.
QPolygon leftNeighbourSlant(const QRect &r);

} // namespace photon

#endif // PHOTONPAINTER_H
