// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#include "photonpainter.h"

#include <QLinearGradient>
#include <QPaintDevice>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>

#include <algorithm>
#include <cmath>
#include <tuple>
#include <vector>

namespace photon {

namespace {

int clampChannel(int v)
{
    return std::clamp(v, 0, 255);
}

// t in [0,1]: 0 gives a, 1 gives b.
QColor mix(const QColor &a, const QColor &b, double t)
{
    const auto lerp = [t](int x, int y) {
        return clampChannel(static_cast<int>(std::lround(x + (y - x) * t)));
    };
    return QColor(lerp(a.red(), b.red()), lerp(a.green(), b.green()), lerp(a.blue(), b.blue()));
}

// Switches the painter to device pixels for one element. The world transform
// keeps its rotation and reflection; the device pixel ratio is taken out and
// the translation snapped to a whole device pixel. Rect edges are rounded in
// absolute coordinates, so two elements that meet at a logical edge meet at
// the same device column whatever transform each was drawn under.
class Canvas
{
public:
    Canvas(QPainter *p, const QRect &logical) : m_painter(p)
    {
        const QPaintDevice *device = p->device();
        m_dpr = device ? device->devicePixelRatio() : 1.0;
        if (!(m_dpr > 0.0))
            m_dpr = 1.0;
        m_unit = std::max(1, static_cast<int>(std::floor(m_dpr + 1e-6)));
        m_scaled = std::abs(m_dpr - 1.0) > 1e-6;
        if (m_scaled) {
            p->save();
            m_world = p->worldTransform();
            m_permutation = isSignedPermutation(m_world);
            m_origin = QPoint(static_cast<int>(std::lround(m_world.dx() * m_dpr)),
                              static_cast<int>(std::lround(m_world.dy() * m_dpr)));
            p->setWorldTransform(QTransform(m_world.m11() / m_dpr, m_world.m12() / m_dpr,
                                            m_world.m21() / m_dpr, m_world.m22() / m_dpr,
                                            m_origin.x() / m_dpr, m_origin.y() / m_dpr));
        }
        m_rect = map(logical);
    }
    ~Canvas()
    {
        if (m_scaled)
            m_painter->restore();
    }
    Canvas(const Canvas &) = delete;
    Canvas &operator=(const Canvas &) = delete;

    const QRect &rect() const { return m_rect; }
    int unit() const { return m_unit; }

    QRect map(const QRect &logical) const
    {
        if (!m_scaled)
            return logical;
        const QPoint a = corner(logical.left(), logical.top());
        const QPoint b = corner(logical.left() + logical.width(), logical.top() + logical.height());
        return QRect(QPoint(std::min(a.x(), b.x()), std::min(a.y(), b.y())),
                     QPoint(std::max(a.x(), b.x()) - 1, std::max(a.y(), b.y()) - 1));
    }

private:
    static bool isSignedPermutation(const QTransform &t)
    {
        const auto unitOrZero = [](qreal v) {
            return std::abs(v) < 1e-9 || std::abs(std::abs(v) - 1.0) < 1e-9;
        };
        return t.type() <= QTransform::TxRotate && unitOrZero(t.m11()) && unitOrZero(t.m12())
            && unitOrZero(t.m21()) && unitOrZero(t.m22())
            && std::abs(std::abs(t.m11() * t.m22() - t.m12() * t.m21()) - 1.0) < 1e-9;
    }

    // A logical corner in the element's coordinates, as a device corner in
    // the element's device coordinates.
    QPoint corner(int x, int y) const
    {
        if (!m_permutation) {
            return QPoint(static_cast<int>(std::lround(x * m_dpr)),
                          static_cast<int>(std::lround(y * m_dpr)));
        }
        const QPointF absolute = m_world.map(QPointF(x, y));
        const int ax = static_cast<int>(std::lround(absolute.x() * m_dpr)) - m_origin.x();
        const int ay = static_cast<int>(std::lround(absolute.y() * m_dpr)) - m_origin.y();
        // The inverse of a signed permutation is its transpose.
        return QPoint(static_cast<int>(std::lround(m_world.m11() * ax + m_world.m12() * ay)),
                      static_cast<int>(std::lround(m_world.m21() * ax + m_world.m22() * ay)));
    }

    QPainter *m_painter;
    qreal m_dpr = 1.0;
    int m_unit = 1;
    bool m_scaled = false;
    bool m_permutation = false;
    QTransform m_world;
    QPoint m_origin;
    QRect m_rect;
};

// --- Device-space drawing; d is in device pixels, u the Photon pixel size.

void fill(QPainter *p, const QRect &r, const QColor &color)
{
    if (color.isValid() && r.width() > 0 && r.height() > 0)
        p->fillRect(r, color);
}

QRect inset(const QRect &d, int n, int u)
{
    return d.adjusted(n * u, n * u, -n * u, -n * u);
}

void side(QPainter *p, const QRect &a, Qt::Edge edge, int u, const QColor &color)
{
    switch (edge) {
    case Qt::TopEdge:
        fill(p, QRect(a.left(), a.top(), a.width(), u), color);
        break;
    case Qt::BottomEdge:
        fill(p, QRect(a.left(), a.bottom() - u + 1, a.width(), u), color);
        break;
    case Qt::LeftEdge:
        fill(p, QRect(a.left(), a.top(), u, a.height()), color);
        break;
    case Qt::RightEdge:
        fill(p, QRect(a.right() - u + 1, a.top(), u, a.height()), color);
        break;
    }
}

void ring(QPainter *p, const QRect &a, int u, const QColor &color)
{
    if (a.width() < 2 * u || a.height() < 2 * u)
        return;
    fill(p, QRect(a.left(), a.top(), a.width(), u), color);
    fill(p, QRect(a.left(), a.bottom() - u + 1, a.width(), u), color);
    fill(p, QRect(a.left(), a.top() + u, u, a.height() - 2 * u), color);
    fill(p, QRect(a.right() - u + 1, a.top() + u, u, a.height() - 2 * u), color);
}

void bevel(QPainter *p, const QRect &a, int u, const QColor &topLeft, const QColor &bottomRight)
{
    if (a.width() < 2 * u || a.height() < 2 * u)
        return;
    fill(p, QRect(a.left(), a.top(), a.width() - u, u), topLeft);
    fill(p, QRect(a.left(), a.top() + u, u, a.height() - 2 * u), topLeft);
    fill(p, QRect(a.left(), a.bottom() - u + 1, a.width(), u), bottomRight);
    fill(p, QRect(a.right() - u + 1, a.top(), u, a.height() - u), bottomRight);
}

void etch(QPainter *p, const QRect &d, int u)
{
    if (d.width() < 2 * u || d.height() < 2 * u)
        return;
    const QColor dark(0, 0, 0, 20);
    const QColor light(255, 255, 255, 35);
    fill(p, QRect(d.left(), d.top(), d.width(), u), dark);
    fill(p, QRect(d.left(), d.top() + u, u, d.height() - 2 * u), dark);
    fill(p, QRect(d.left(), d.bottom() - u + 1, d.width(), u), light);
    fill(p, QRect(d.right() - u + 1, d.top() + u, u, d.height() - 2 * u), light);
}

// Integer step per device line; QLinearGradient dithers differently.
void gradient(QPainter *p, const QRect &a, const QColor &base, int contrast,
              Qt::Orientation orientation, bool reversed)
{
    if (a.width() <= 0 || a.height() <= 0)
        return;
    QColor first = shifted(base, contrast);
    QColor last = shifted(base, -contrast);
    if (reversed)
        std::swap(first, last);
    const int steps = orientation == Qt::Vertical ? a.height() : a.width();
    if (steps == 1) {
        fill(p, a, base);
        return;
    }
    for (int i = 0; i < steps; ++i) {
        const QColor c = mix(first, last, static_cast<double>(i) / (steps - 1));
        if (orientation == Qt::Vertical)
            fill(p, QRect(a.left(), a.top() + i, a.width(), 1), c);
        else
            fill(p, QRect(a.left() + i, a.top(), 1, a.height()), c);
    }
}

void raised(QPainter *p, const QRect &a, int u, const Colors &c, bool sunken)
{
    if (a.width() < 4 * u || a.height() < 4 * u)
        return;
    ring(p, a, u, c.outline);
    bevel(p, inset(a, 1, u), u, sunken ? c.shade : c.light, sunken ? c.light : c.shade);
}

void fieldFrame(QPainter *p, const QRect &a, int u, const QColor &outline, const QColor &shade)
{
    if (a.width() < 3 * u || a.height() < 3 * u)
        return;
    ring(p, a, u, outline);
    const QRect in = inset(a, 1, u);
    fill(p, QRect(in.left(), in.top(), in.width(), u), shade);
    fill(p, QRect(in.left(), in.top() + u, u, in.height() - u), shade);
}

void button(QPainter *p, const QRect &a, int u, const Colors &c, bool sunken)
{
    gradient(p, inset(a, 2, u), c.button, kFillContrast, Qt::Vertical, sunken);
    raised(p, a, u, c, sunken);
}

// Pixel rows of an arrow pointing down: stem first, then the head narrowing
// by one pixel per side per row. Each entry is (row, first column, width).
struct Span
{
    int along;
    int across;
    int width;
};

std::vector<Span> arrowSpans(const ArrowShape &s)
{
    std::vector<Span> spans;
    const int stemOffset = (s.headBase - s.stemWidth) / 2;
    for (int v = 0; v < s.stemLength; ++v)
        spans.push_back({v, stemOffset, s.stemWidth});
    for (int i = 0; i < s.headLength; ++i) {
        const int width = s.headBase - 2 * i;
        if (width <= 0)
            break;
        spans.push_back({s.stemLength + i, i, width});
    }
    return spans;
}

void arrow(QPainter *p, const QRect &d, int u, Qt::ArrowType type, const QColor &color,
           const ArrowShape &shape)
{
    if (type == Qt::NoArrow || shape.headBase <= 0 || shape.headLength <= 0)
        return;
    const int length = (shape.stemLength + shape.headLength) * u;
    const int base = shape.headBase * u;
    const bool vertical = type == Qt::UpArrow || type == Qt::DownArrow;
    const int acrossSpace = vertical ? d.width() : d.height();
    const int alongSpace = vertical ? d.height() : d.width();
    if (acrossSpace < base || alongSpace < length)
        return;

    const int acrossOrigin = (vertical ? d.left() : d.top()) + (acrossSpace - base) / 2;
    const int alongOrigin = (vertical ? d.top() : d.left()) + (alongSpace - length) / 2;
    const bool flipped = type == Qt::UpArrow || type == Qt::LeftArrow;
    const int rows = shape.stemLength + shape.headLength;

    for (const Span &s : arrowSpans(shape)) {
        const int row = flipped ? rows - 1 - s.along : s.along;
        const int along = alongOrigin + row * u;
        const int across = acrossOrigin + s.across * u;
        if (vertical)
            fill(p, QRect(across, along, s.width * u, u), color);
        else
            fill(p, QRect(along, across, u, s.width * u), color);
    }
}

void cross(QPainter *p, const QRect &d, int u, const QColor &color, const QColor &background)
{
    constexpr int kSize = 8;
    if (d.width() < kSize * u || d.height() < kSize * u)
        return;
    const int x0 = d.left() + (d.width() - kSize * u) / 2;
    const int y0 = d.top() + (d.height() - kSize * u) / 2;
    // The side pixels are 40% ink over the background (#999999 on white).
    const QColor soft = mix(background, color, 0.4);
    for (int y = 0; y < kSize; ++y) {
        const int a = y;
        const int b = kSize - 1 - y;
        for (const int x : {a - 1, a + 1, b - 1, b + 1}) {
            if (x >= 0 && x < kSize && x != a && x != b)
                fill(p, QRect(x0 + x * u, y0 + y * u, u, u), soft);
        }
        fill(p, QRect(x0 + a * u, y0 + y * u, u, u), color);
        fill(p, QRect(x0 + b * u, y0 + y * u, u, u), color);
    }
}

} // namespace

QColor shifted(const QColor &c, int delta)
{
    return QColor(clampChannel(c.red() + delta), clampChannel(c.green() + delta),
                  clampChannel(c.blue() + delta), c.alpha());
}

QPalette standardPalette()
{
    const QColor window(0xD8, 0xD8, 0xD8);
    const QColor button(0xD7, 0xD7, 0xD7);
    const QColor black(0x00, 0x00, 0x00);
    const QColor white(0xFF, 0xFF, 0xFF);
    const QColor disabledText(0x85, 0x85, 0x85);

    QPalette pal(black, button, white, QColor(0xB0, 0xB0, 0xB0), QColor(0xC0, 0xC0, 0xC0),
                 black, white, white, window);
    pal.setColor(QPalette::Midlight, QColor(0xEC, 0xEC, 0xEC));
    pal.setColor(QPalette::Shadow, QColor(0x4B, 0x4B, 0x4B));
    pal.setColor(QPalette::AlternateBase, QColor(0xF4, 0xF4, 0xF4));
    pal.setColor(QPalette::Highlight, QColor(0x8E, 0xA2, 0x9B));
    pal.setColor(QPalette::HighlightedText, black);
    pal.setColor(QPalette::ToolTipBase, QColor(0xFF, 0xFF, 0xE0));
    pal.setColor(QPalette::ToolTipText, black);
    pal.setColor(QPalette::PlaceholderText, disabledText);
    pal.setColor(QPalette::Link, QColor(0x00, 0x00, 0xEE));
    pal.setColor(QPalette::LinkVisited, QColor(0x55, 0x1A, 0x8B));

    for (const QPalette::ColorRole role :
         {QPalette::WindowText, QPalette::Text, QPalette::ButtonText}) {
        pal.setColor(QPalette::Disabled, role, disabledText);
    }
    pal.setColor(QPalette::Disabled, QPalette::Base, window);
    pal.setColor(QPalette::Disabled, QPalette::Highlight, QColor(0xB0, 0xB0, 0xB0));
    return pal;
}

Colors colorsFromPalette(const QPalette &pal)
{
    Colors c;
    c.window = pal.color(QPalette::Window);
    c.button = pal.color(QPalette::Button);
    c.base = pal.color(QPalette::Base);
    c.text = pal.color(QPalette::Text);
    c.disabledText = pal.color(QPalette::Disabled, QPalette::Text);
    c.highlight = pal.color(QPalette::Highlight);
    c.highlightedText = pal.color(QPalette::HighlightedText);

    // Offsets are the measured distance from Window (or Button) in the
    // default scheme, so any palette keeps Photon's relationships.
    c.light = shifted(c.window, 39);
    c.shade = shifted(c.button, -39);
    c.outline = shifted(c.window, -141);
    c.fieldShade = shifted(c.window, -24);
    c.midlight = shifted(c.window, 20);
    c.fieldFill = shifted(c.base, -11);
    c.gradientTop = shifted(c.button, kFillContrast);
    c.gradientBottom = shifted(c.button, -kFillContrast);
    c.checkOutline = shifted(c.outline, -16);
    c.checkEtchDark = shifted(c.window, -42);
    c.checkEtchLight = shifted(c.window, 28);
    c.etchedDark = shifted(c.window, -20);
    c.etchedLight = c.midlight;
    c.stripDark = shifted(c.window, -55);
    c.stripLight = c.light;
    c.tabInactive = shifted(c.window, -17);
    c.tabInactiveLight = shifted(c.window, 3);
    // Grey on purpose: QNX 6.2.1 draws the unselected tab label #636363
    // (Display settings dialog), lighter than text but without disabled's etch.
    c.tabInactiveText = mix(pal.color(QPalette::WindowText), c.window, 0.46);
    c.tabSlantLight = shifted(c.window, 10);
    c.trough = shifted(c.window, -35);
    c.troughShade = shifted(c.window, -55);
    c.panelTop = shifted(c.window, -19);
    c.panelBottom = shifted(c.window, 1);
    c.panelShade = shifted(c.window, -63);
    c.menuItem = shifted(c.window, -12);
    c.menuItemOpen = shifted(c.window, -37);
    c.menuFrameLight = shifted(c.window, 25);
    c.menuFrameShade = shifted(c.window, -25);
    c.headerLight = shifted(c.button, 31);
    c.headerShade = shifted(c.button, -29);
    c.toolCheckedShade = shifted(c.window, -50);
    c.sliderGrooveLight = shifted(c.window, -76);
    c.sliderEtch = shifted(c.window, -54);
    c.progressTrough = shifted(c.window, -28);
    c.progressTroughShade = shifted(c.window, -75);
    c.progressEnd = shifted(c.outline, -12);
    c.arrow = shifted(pal.color(QPalette::ButtonText), 0x33);
    c.arrowDisabled = pal.color(QPalette::Disabled, QPalette::ButtonText);

    // Photon has no palette role for these; they are the scheme's own.
    c.progressFill = QColor(0xB5, 0xC4, 0xB0);
    c.progressFillLight = QColor(0xDD, 0xEC, 0xD8);
    c.progressFillShade = QColor(0x8D, 0x9C, 0x88);
    c.menuHover = QColor(0x9B, 0xA9, 0xC9);
    c.focus = QColor(0x90, 0x98, 0xF8);
    return c;
}

void drawEtch(QPainter *p, const QRect &r)
{
    if (!p)
        return;
    const Canvas cv(p, r);
    etch(p, cv.rect(), cv.unit());
}

void fillGradient(QPainter *p, const QRect &r, const QColor &base, int contrast,
                  Qt::Orientation orientation, bool reversed, int insetBy)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    gradient(p, inset(cv.rect(), insetBy, cv.unit()), base, contrast, orientation, reversed);
}

void fillInset(QPainter *p, const QRect &r, int insetBy, const QColor &color)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    fill(p, inset(cv.rect(), insetBy, cv.unit()), color);
}

void drawRing(QPainter *p, const QRect &r, int insetBy, const QColor &color)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    ring(p, inset(cv.rect(), insetBy, cv.unit()), cv.unit(), color);
}

void drawSide(QPainter *p, const QRect &r, Qt::Edge edge, int insetBy, const QColor &color)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    side(p, inset(cv.rect(), insetBy, cv.unit()), edge, cv.unit(), color);
}

void drawBevel(QPainter *p, const QRect &r, int insetBy, const QColor &topLeft,
               const QColor &bottomRight)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    bevel(p, inset(cv.rect(), insetBy, cv.unit()), cv.unit(), topLeft, bottomRight);
}

void drawRaisedFrame(QPainter *p, const QRect &r, const Colors &c, bool sunken)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    raised(p, cv.rect(), cv.unit(), c, sunken);
}

void drawFieldFrame(QPainter *p, const QRect &r, const Colors &c, const QColor &outline)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    fieldFrame(p, cv.rect(), cv.unit(), outline, c.fieldShade);
}

void drawEtchedLine(QPainter *p, const QPoint &from, Qt::Orientation orientation, int length,
                    const QColor &dark, const QColor &light)
{
    if (!p || length <= 0)
        return;
    const QRect logical = orientation == Qt::Horizontal ? QRect(from, QSize(length, 1))
                                                        : QRect(from, QSize(1, length));
    const Canvas cv(p, logical);
    const QRect d = cv.rect();
    const int u = cv.unit();
    if (orientation == Qt::Horizontal) {
        fill(p, QRect(d.left(), d.top(), d.width(), u), dark);
        fill(p, QRect(d.left(), d.top() + u, d.width(), u), light);
    } else {
        fill(p, QRect(d.left(), d.top(), u, d.height()), dark);
        fill(p, QRect(d.left() + u, d.top(), u, d.height()), light);
    }
}

void drawEtchedRect(QPainter *p, const QRect &r, const QColor &dark, const QColor &light)
{
    if (!p || r.width() < 3 || r.height() < 3)
        return;
    const Canvas cv(p, r);
    const QRect d = cv.rect();
    const int u = cv.unit();
    ring(p, QRect(d.left() + u, d.top() + u, d.width() - u, d.height() - u), u, light);
    ring(p, QRect(d.left(), d.top(), d.width() - u, d.height() - u), u, dark);
}

void drawButton(QPainter *p, const QRect &r, const Colors &c, bool sunken, bool etched)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    const int u = cv.unit();
    if (etched)
        etch(p, cv.rect(), u);
    button(p, etched ? inset(cv.rect(), 1, u) : cv.rect(), u, c, sunken);
}

void drawField(QPainter *p, const QRect &r, const Colors &c, const QColor &fillColor,
               const QColor &shadeColor)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    const int u = cv.unit();
    etch(p, cv.rect(), u);
    fill(p, inset(cv.rect(), 2, u), fillColor);
    fieldFrame(p, inset(cv.rect(), 1, u), u, c.outline,
               shadeColor.isValid() ? shadeColor : c.fieldShade);
}

void drawPanel(QPainter *p, const QRect &r, const Colors &c)
{
    if (!p || r.width() < 3 || r.height() < 5)
        return;
    const Canvas cv(p, r);
    const QRect d = cv.rect();
    const int u = cv.unit();
    const QRect ramp(d.left() + u, d.top() + 2 * u, d.width() - 2 * u, d.height() - 4 * u);
    for (int i = 0; i < ramp.height(); ++i) {
        const double t = ramp.height() == 1 ? 0.0 : static_cast<double>(i) / (ramp.height() - 1);
        fill(p, QRect(ramp.left(), ramp.top() + i, ramp.width(), 1), mix(c.panelTop, c.panelBottom, t));
    }
    fill(p, QRect(d.left() + u, d.top() + u, d.width() - 2 * u, u), c.light);
    fill(p, QRect(d.left() + u, d.bottom() - 2 * u + 1, d.width() - 2 * u, u), c.panelShade);
    ring(p, d, u, c.outline);
}

void drawComboArrowButton(QPainter *p, const QRect &r, int width, bool pressed, bool enabled,
                          const Colors &c, bool mirrored)
{
    if (!p || width <= 0)
        return;
    const Canvas cv(p, r);
    const QRect d = cv.rect();
    const int u = cv.unit();
    // Shares the field outline on the top, bottom and right.
    const int x = mirrored ? d.left() + u : d.right() - u + 1 - width * u;
    const QRect b(x, d.top() + u, width * u, d.height() - 2 * u);
    if (b.width() < 4 * u || b.height() < 4 * u)
        return;
    button(p, b, u, c, pressed);
    arrow(p, inset(b, 2, u), u, Qt::DownArrow, enabled ? c.text : c.disabledText, kComboArrow);
}

void drawSpinButtons(QPainter *p, const QRect &r, int width, bool upPressed, bool downPressed,
                     bool upEnabled, bool downEnabled, const Colors &c, bool mirrored)
{
    if (!p || width <= 0)
        return;
    const Canvas cv(p, r);
    const QRect d = cv.rect();
    const int u = cv.unit();
    const int x = mirrored ? d.left() + u : d.right() - u + 1 - width * u;
    const int top = d.top() + u;
    const int height = d.height() - 2 * u;
    const int upHeight = ((height / u + 1) / 2) * u;
    const QRect up(x, top, width * u, upHeight);
    const QRect down(x, top + upHeight, width * u, height - upHeight);
    for (const auto &[b, pressed, enabled, type] :
         {std::tuple{up, upPressed, upEnabled, Qt::UpArrow},
          std::tuple{down, downPressed, downEnabled, Qt::DownArrow}}) {
        if (b.width() < 4 * u || b.height() < 4 * u)
            continue;
        button(p, b, u, c, pressed);
        arrow(p, inset(b, 2, u), u, type, enabled ? c.text : c.disabledText, kBranchArrow);
    }
}

void drawScrollBar(QPainter *p, const QRect &r, const ScrollBarState &s, const Colors &c)
{
    if (!p || r.width() < 4 || r.height() < 4)
        return;
    const Canvas cv(p, r);
    const QRect d = cv.rect();
    const int u = cv.unit();
    const bool horizontal = s.orientation == Qt::Horizontal;
    const int start = horizontal ? d.left() : d.top();
    const int end = horizontal ? d.right() : d.bottom();
    const int length = end - start + 1;
    const int buttonLength = std::min(s.buttonLength * u, length / 2);

    // The slider shares an outline line with each button.
    const int grooveStart = start + buttonLength - u;
    const int grooveEnd = end - buttonLength + u;
    const QRect sliderRect = s.slider.isEmpty() ? QRect() : cv.map(s.slider);
    const int sliderPos = horizontal ? sliderRect.left() : sliderRect.top();
    const int sliderLength = horizontal ? sliderRect.width() : sliderRect.height();
    const bool showSlider = !sliderRect.isEmpty();

    auto span = [&](int from, int count) {
        return horizontal ? QRect(from, d.top(), count, d.height())
                          : QRect(d.left(), from, d.width(), count);
    };
    auto shadeAt = [&](int pos) {
        if (horizontal)
            fill(p, QRect(pos, d.top() + u, u, d.height() - 2 * u), c.troughShade);
        else
            fill(p, QRect(d.left() + u, pos, d.width() - 2 * u, u), c.troughShade);
    };

    fill(p, d, c.trough);
    if (horizontal)
        fill(p, QRect(d.left() + u, d.top() + u, d.width() - 2 * u, u), c.troughShade);
    else
        fill(p, QRect(d.left() + u, d.top() + u, u, d.height() - 2 * u), c.troughShade);
    // Light comes from the top left whatever the direction: the shade falls
    // after the left-hand button and after the slider.
    shadeAt(grooveStart + u);
    if (showSlider && sliderPos + sliderLength < grooveEnd)
        shadeAt(sliderPos + sliderLength);
    ring(p, d, u, c.outline);

    const QRect startButton = span(start, buttonLength);
    const QRect endButton = span(end - buttonLength + 1, buttonLength);
    const QRect sub = s.mirrored ? endButton : startButton;
    const QRect add = s.mirrored ? startButton : endButton;
    const Qt::ArrowType subArrow =
        horizontal ? (s.mirrored ? Qt::RightArrow : Qt::LeftArrow) : Qt::UpArrow;
    const Qt::ArrowType addArrow =
        horizontal ? (s.mirrored ? Qt::LeftArrow : Qt::RightArrow) : Qt::DownArrow;
    for (const auto &[b, pressed, enabled, type] :
         {std::tuple{sub, s.subPressed, s.subEnabled, subArrow},
          std::tuple{add, s.addPressed, s.addEnabled, addArrow}}) {
        if (b.width() < 6 * u || b.height() < 6 * u)
            continue;
        button(p, b, u, c, pressed);
        arrow(p, inset(b, 1, u), u, type, enabled ? c.arrow : c.arrowDisabled, kScrollArrow);
    }

    if (showSlider && sliderLength >= 4 * u) {
        const QRect slider = span(sliderPos, sliderLength);
        // The slider's ramp runs across the bar, dark side first.
        gradient(p, inset(slider, 2, u), c.button, kFillContrast,
                 horizontal ? Qt::Vertical : Qt::Horizontal, true);
        raised(p, slider, u, c, false);
    }
}

void drawSliderHandle(QPainter *p, const QRect &r, Qt::Orientation orientation, const Colors &c)
{
    if (!p || r.width() < 4 || r.height() < 4)
        return;
    const Canvas cv(p, r);
    const QRect d = cv.rect();
    const int u = cv.unit();
    const bool horizontal = orientation == Qt::Horizontal;
    // Handle ramp: window +/-21 (#EDEDED to #C4C4C4 in the reference).
    gradient(p, inset(d, 1, u), c.window, 21, horizontal ? Qt::Vertical : Qt::Horizontal, false);
    ring(p, d, u, c.outline);
    if (horizontal)
        fill(p, QRect(d.left(), d.bottom() + 1, d.width(), u), c.sliderEtch);
    else
        fill(p, QRect(d.right() + 1, d.top(), u, d.height()), c.sliderEtch);
}

void drawProgressGroove(QPainter *p, const QRect &r, const Colors &c)
{
    if (!p || r.width() < 3 || r.height() < 3)
        return;
    const Canvas cv(p, r);
    const QRect d = cv.rect();
    const int u = cv.unit();
    const QRect in = inset(d, 1, u);
    fill(p, in, c.progressTrough);
    fill(p, QRect(in.left(), in.top(), in.width(), u), c.progressTroughShade);
    fill(p, QRect(in.left(), in.top() + u, u, in.height() - u), c.progressTroughShade);
    ring(p, d, u, c.outline);
}

void drawProgressBar(QPainter *p, const QRect &r, double fraction, Qt::Orientation orientation,
                     bool reversed, const Colors &c)
{
    if (!p || r.width() < 3 || r.height() < 3)
        return;
    const Canvas cv(p, r);
    const int u = cv.unit();
    const QRect in = inset(cv.rect(), 1, u);
    const bool vertical = orientation == Qt::Vertical;
    const int length = vertical ? in.height() : in.width();
    const int filled = static_cast<int>(std::lround(std::clamp(fraction, 0.0, 1.0) * length));
    if (filled < 2 * u)
        return;
    // Vertical bars grow upwards; reversed ones grow down from the top.
    QRect bar = vertical ? QRect(in.left(), in.bottom() - filled + 1, in.width(), filled)
                         : QRect(in.left(), in.top(), filled, in.height());
    if (reversed) {
        if (vertical)
            bar.moveTop(in.top());
        else
            bar.moveRight(in.right());
    }
    fill(p, bar, c.progressFill);
    fill(p, QRect(bar.left(), bar.top(), bar.width(), u), c.progressFillLight);
    fill(p, QRect(bar.left(), bar.top() + u, u, bar.height() - u), c.progressFillLight);
    fill(p, QRect(bar.left() + u, bar.bottom() - u + 1, bar.width() - u, u), c.progressFillShade);
    fill(p, QRect(bar.right() - u + 1, bar.top() + u, u, bar.height() - u), c.progressFillShade);
    // Closing line and the trough's own shade after it.
    if (!vertical && !reversed && bar.right() + 2 * u <= in.right()) {
        fill(p, QRect(bar.right() + 1, in.top(), u, in.height()), c.progressEnd);
        fill(p, QRect(bar.right() + 1 + u, in.top(), u, in.height()), c.progressTroughShade);
    }
}

void drawHeaderSection(QPainter *p, const QRect &r, bool first, bool sunken, const Colors &c)
{
    if (!p || r.width() < 4 || r.height() < 4)
        return;
    const Canvas cv(p, r);
    const QRect d = cv.rect();
    const int u = cv.unit();
    // Inside the outline; later sections start at their left edge because
    // the neighbour's right outline is theirs too.
    const QRect in(d.left() + (first ? u : 0), d.top() + u,
                   d.width() - u - (first ? u : 0), d.height() - 2 * u);
    gradient(p, in, c.button, kFillContrast, Qt::Vertical, !sunken);
    bevel(p, in, u, c.headerLight, c.headerShade);
    side(p, d, Qt::TopEdge, u, c.outline);
    side(p, d, Qt::BottomEdge, u, c.outline);
    side(p, d, Qt::RightEdge, u, c.outline);
    if (first)
        side(p, d, Qt::LeftEdge, u, c.outline);
}

void drawMenuFrame(QPainter *p, const QRect &r, const QColor &rim, const Colors &c)
{
    if (!p || r.width() < 4 || r.height() < 4)
        return;
    const Canvas cv(p, r);
    const int u = cv.unit();
    ring(p, cv.rect(), u, rim);
    bevel(p, inset(cv.rect(), 1, u), u, c.menuFrameLight, c.menuFrameShade);
}

void drawArrow(QPainter *p, const QRect &r, Qt::ArrowType type, const QColor &color,
               const ArrowShape &shape)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    arrow(p, cv.rect(), cv.unit(), type, color, shape);
}

void drawCross(QPainter *p, const QRect &r, const QColor &color, const QColor &background)
{
    if (!p || r.isEmpty())
        return;
    const Canvas cv(p, r);
    cross(p, cv.rect(), cv.unit(), color, background);
}

void drawChevron(QPainter *p, const QRect &r, bool up, const QColor &color)
{
    if (!p || r.width() < 7 || r.height() < 4)
        return;
    const Canvas cv(p, r);
    const QRect d = cv.rect();
    const int u = cv.unit();
    const QPoint mid = d.center();
    for (int i = 0; i < 4; ++i) {
        const int y = up ? mid.y() - 2 * u + i * u : mid.y() + u - i * u;
        fill(p, QRect(mid.x() - i * u, y, u, u), color);
        fill(p, QRect(mid.x() + i * u, y, u, u), color);
    }
}

void drawRadio(QPainter *p, const QRect &r, bool checked, bool enabled, const Colors &c)
{
    // Anti-aliased in logical coordinates; a sphere has no pixel grid to keep.
    if (!p || r.width() < 13 || r.height() < 13)
        return;
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);
    const QPointF origin(r.left() + (r.width() - 13) / 2, r.top() + (r.height() - 13) / 2);
    const QRectF sphere(origin.x() + 0.5, origin.y() + 0.5, 11.0, 11.0);

    p->setPen(QPen(QColor(255, 255, 255, 90), 1.0));
    p->setBrush(Qt::NoBrush);
    p->drawEllipse(sphere.translated(0.7, 0.7));

    QLinearGradient body(sphere.topLeft(), sphere.bottomRight());
    if (enabled) {
        body.setColorAt(0.0, shifted(c.window, 8));
        body.setColorAt(0.25, shifted(c.window, 8));
        body.setColorAt(1.0, shifted(c.window, -40));
    } else {
        body.setColorAt(0.0, c.window);
        body.setColorAt(1.0, c.window);
    }
    p->setPen(QPen(enabled ? shifted(c.outline, -8) : c.disabledText, 1.0));
    p->setBrush(body);
    p->drawEllipse(sphere);

    if (checked) {
        const QPointF centre = sphere.center();
        QRadialGradient dot(centre, 3.2);
        const QColor ink = enabled ? c.outline : c.disabledText;
        dot.setColorAt(0.0, ink);
        dot.setColorAt(0.55, ink);
        dot.setColorAt(1.0, shifted(ink, 60));
        p->setPen(Qt::NoPen);
        p->setBrush(dot);
        p->drawEllipse(centre, 3.2, 3.2);
    }
    p->restore();
}

void drawCheckBox(QPainter *p, const QRect &r, bool on, bool enabled, bool noChange, const Colors &c)
{
    if (!p || r.width() < 6 || r.height() < 6)
        return;
    const Canvas cv(p, r);
    const QRect d = cv.rect();
    const int u = cv.unit();
    fill(p, QRect(d.left(), d.top(), d.width(), u), c.checkEtchDark);
    fill(p, QRect(d.left(), d.top() + u, u, d.height() - u), c.checkEtchDark);
    fill(p, QRect(d.left() + u, d.bottom() - u + 1, d.width() - u, u), c.checkEtchLight);
    fill(p, QRect(d.right() - u + 1, d.top() + u, u, d.height() - 2 * u), c.checkEtchLight);

    const QColor box = enabled ? c.base : c.window;
    const QRect inner = inset(d, 2, u);
    fill(p, inner, box);
    ring(p, inset(d, 1, u), u, enabled ? c.checkOutline : c.disabledText);

    const QColor ink = enabled ? c.text : c.disabledText;
    if (on)
        cross(p, inner, u, ink, box);
    else if (noChange)
        fill(p, QRect(inner.left() + 2 * u, inner.center().y(), inner.width() - 4 * u, 2 * u), ink);
}

void drawTab(QPainter *p, const QRect &r, bool selected, bool clipLeftNeighbour, const Colors &c)
{
    if (!p || r.width() < 8 || r.height() < 8)
        return;
    p->save();
    {
        const Canvas cv(p, r);
        const QRect n = cv.rect();
        const int u = cv.unit();
        // Unselected tabs leave their last two rows to the pane edge.
        const QRect logicalBody(r.left(), r.top(), r.width(), r.height() - 2);
        const int bottom = selected ? n.bottom() : cv.map(logicalBody).bottom();
        const int half = n.height() / 2;
        const int slantTop = n.right() - half;

        if (!selected && clipLeftNeighbour) {
            QPainterPath keep;
            keep.addRect(QRectF(n.left(), n.top() - 1, n.width() + n.height(), n.height() + 2));
            QPainterPath owned;
            owned.addPolygon(QPolygonF(leftNeighbourSlant(n)));
            owned.closeSubpath();
            p->setClipPath(keep.subtracted(owned), Qt::IntersectClip);
        }

        // The selected slant ends on the pane's top line row (qnx621disp2.png);
        // below it only the body continues, merging with the pane.
        const QRect logicalSlant(r.left(), r.top(), r.width(), r.height() - 1);
        const int slantBottom = selected ? cv.map(logicalSlant).bottom() : bottom;

        const QColor body = selected ? c.window : c.tabInactive;
        const QColor light = selected ? c.midlight : c.tabInactiveLight;
        for (int y = n.top(); y <= bottom; ++y) {
            const int dy = std::min(y, slantBottom) - n.top();
            const int left = n.left() + (y - n.top() < u ? u : 0);
            fill(p, QRect(left, y, slantTop + dy - left + 1, 1), body);
        }
        // Highlight inside the top and left edges, a lighter line inside the slant.
        fill(p, QRect(n.left() + 2 * u, n.top() + u, slantTop - n.left() - 2 * u + 1, u), light);
        fill(p, QRect(n.left() + u, n.top() + 2 * u, u, bottom - n.top() - 2 * u + 1), light);
        if (selected) {
            for (int y = n.top() + 2 * u; y <= slantBottom; ++y)
                fill(p, QRect(slantTop + (y - n.top()) - u - (u - 1), y, u, 1), c.tabSlantLight);
        }
        // Outline: left edge, chamfer, top edge, slant.
        fill(p, QRect(n.left(), n.top() + u, u, bottom - n.top() - u + 1), c.outline);
        fill(p, QRect(n.left() + u, n.top(), slantTop - n.left() - u + 1, u), c.outline);
        for (int y = n.top(); y <= slantBottom; ++y)
            fill(p, QRect(slantTop + (y - n.top()) - (u - 1), y, u, 1), c.outline);
    }
    p->restore();
}

QPolygon tabOutline(const QRect &r)
{
    if (r.width() < 4 || r.height() < 4)
        return {};
    const int half = r.height() / 2;
    const int slantTop = r.right() - half;
    return QPolygon({QPoint(r.left(), r.bottom()), QPoint(r.left(), r.top() + 1),
                     QPoint(r.left() + 1, r.top()), QPoint(slantTop, r.top()),
                     QPoint(slantTop + r.bottom() - r.top(), r.bottom())});
}

QPolygon leftNeighbourSlant(const QRect &r)
{
    if (r.width() < 4 || r.height() < 4)
        return {};
    // The neighbour ends at r.left()-1; extend one pixel above and below so
    // its own outline pixels count as the neighbour's.
    const int half = r.height() / 2;
    const int topX = r.left() - 1 - half;
    const int far = r.left() - 2 * r.height() - 2;
    return QPolygon({QPoint(topX - 1, r.top() - 1),
                     QPoint(topX + r.bottom() - r.top() + 1, r.bottom() + 1),
                     QPoint(far, r.bottom() + 1), QPoint(far, r.top() - 1)});
}

} // namespace photon
