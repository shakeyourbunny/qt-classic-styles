// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#include "photonpainter.h"

#include <QImage>
#include <QPainter>
#include <QTest>

#include <cstdlib>

// Expected values are pixel runs measured from the QNX 6.2.1 screenshots in
// reference/qnx-photon/.

namespace {

QImage canvas(int w, int h, QRgb fill)
{
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(fill);
    return img;
}

QRgb px(const QImage &img, int x, int y)
{
    return img.pixel(x, y) & 0xFFFFFF;
}

bool near(QRgb actual, QRgb expected, int tolerance)
{
    return std::abs(qRed(actual) - qRed(expected)) <= tolerance
        && std::abs(qGreen(actual) - qGreen(expected)) <= tolerance
        && std::abs(qBlue(actual) - qBlue(expected)) <= tolerance;
}

QString hex(QRgb c)
{
    return QStringLiteral("#%1").arg(c & 0xFFFFFF, 6, 16, QLatin1Char('0')).toUpper();
}

#define EXPECT_PX(img, x, y, rgb)                                                       \
    QVERIFY2(px(img, x, y) == ((rgb) & 0xFFFFFF),                                     \
             qPrintable(QStringLiteral("(%1,%2) is %3, want %4")                      \
                            .arg(x).arg(y).arg(hex(px(img, x, y))).arg(hex(rgb))))

#define EXPECT_PX_NEAR(img, x, y, rgb, tol)                                             \
    QVERIFY2(near(px(img, x, y), (rgb), (tol)),                                       \
             qPrintable(QStringLiteral("(%1,%2) is %3, want %4 +/-%5")                \
                            .arg(x).arg(y).arg(hex(px(img, x, y))).arg(hex(rgb)).arg(tol)))

// Renders a glyph into a bitmap of '#' (== colour) and '.' for comparison.
QStringList bitmap(const QImage &img, const QRect &r, QRgb ink)
{
    QStringList rows;
    for (int y = r.top(); y <= r.bottom(); ++y) {
        QString row;
        for (int x = r.left(); x <= r.right(); ++x)
            row += px(img, x, y) == (ink & 0xFFFFFF) ? u'#' : u'.';
        rows << row;
    }
    return rows;
}

} // namespace

class TstPhotonPainter : public QObject
{
    Q_OBJECT

private slots:
    void standard_palette_matches_measurements();
    void colors_derive_from_standard_palette();
    void colors_follow_custom_button_colour();
    void shifted_clamps_channels();

    void etch_on_dialog_body();
    void etch_on_button_strip();

    void gradient_vertical_matches_button_ramp();
    void gradient_reversed_swaps_ends();
    void gradient_horizontal_ramps_along_x();
    void gradient_single_row_uses_base();

    void raised_frame_has_outline_and_bevel();
    void sunken_frame_swaps_bevel();
    void field_frame_shades_top_left_only();
    void etched_line_draws_dark_then_light();

    void combo_arrow_down_matches_reference();
    void combo_arrow_right_matches_submenu_reference();
    void scroll_arrow_up_matches_reference();
    void branch_arrow_right_is_plain_triangle();
    void arrow_in_too_small_rect_draws_nothing();

    void cross_matches_reference();

    void radio_is_lit_from_top_left();
    void radio_checked_has_dark_centre();

    void tab_outline_geometry();
    void left_neighbour_slant_covers_bottom_left_only();

    void ring_colours_derive_from_window_not_kde_shades();
    void frame_lines_stay_one_device_pixel_at_17_16();
    void frame_lines_are_two_device_pixels_at_2x();
    void cross_keeps_its_pattern_at_17_16();
    void tab_slant_is_straight_at_17_16();
    void check_box_outline_is_even_at_17_16();
    void selected_slant_stops_on_the_pane_line();
    void neighbour_slant_survives_the_next_tab();
};

namespace {

// Plasma's ScaleFactor=1.0625 gives Qt a device pixel ratio of 17/16.
constexpr qreal kPlasmaDpr = 17.0 / 16.0;

QImage scaledCanvas(int logicalW, int logicalH, qreal dpr, QRgb fill)
{
    QImage img(qCeil(logicalW * dpr), qCeil(logicalH * dpr), QImage::Format_RGB32);
    img.fill(fill);
    img.setDevicePixelRatio(dpr);
    return img;
}

// Width of the run of `ink` starting at (x,y) going in (dx,dy).
int runLength(const QImage &img, int x, int y, int dx, int dy, QRgb ink)
{
    int n = 0;
    while (x >= 0 && y >= 0 && x < img.width() && y < img.height() && px(img, x, y) == ink) {
        ++n;
        x += dx;
        y += dy;
    }
    return n;
}

} // namespace

void TstPhotonPainter::standard_palette_matches_measurements()
{
    const QPalette pal = photon::standardPalette();
    QCOMPARE(pal.color(QPalette::Active, QPalette::Window).rgb() & 0xFFFFFF, 0xD8D8D8u);
    QCOMPARE(pal.color(QPalette::Active, QPalette::Button).rgb() & 0xFFFFFF, 0xD7D7D7u);
    QCOMPARE(pal.color(QPalette::Active, QPalette::Base).rgb() & 0xFFFFFF, 0xFFFFFFu);
    QCOMPARE(pal.color(QPalette::Active, QPalette::Shadow).rgb() & 0xFFFFFF, 0x4B4B4Bu);
    QCOMPARE(pal.color(QPalette::Active, QPalette::Dark).rgb() & 0xFFFFFF, 0xB0B0B0u);
    QCOMPARE(pal.color(QPalette::Active, QPalette::Mid).rgb() & 0xFFFFFF, 0xC0C0C0u);
    QCOMPARE(pal.color(QPalette::Active, QPalette::Midlight).rgb() & 0xFFFFFF, 0xECECECu);
    QCOMPARE(pal.color(QPalette::Active, QPalette::Highlight).rgb() & 0xFFFFFF, 0x8EA29Bu);
    QCOMPARE(pal.color(QPalette::Active, QPalette::HighlightedText).rgb() & 0xFFFFFF, 0x000000u);
    QCOMPARE(pal.color(QPalette::Disabled, QPalette::ButtonText).rgb() & 0xFFFFFF, 0x858585u);
    QCOMPARE(pal.color(QPalette::Disabled, QPalette::Text).rgb() & 0xFFFFFF, 0x858585u);
}

void TstPhotonPainter::colors_derive_from_standard_palette()
{
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    QCOMPARE(c.outline.rgb() & 0xFFFFFF, 0x4B4B4Bu);
    QCOMPARE(c.light.rgb() & 0xFFFFFF, 0xFFFFFFu);
    QCOMPARE(c.shade.rgb() & 0xFFFFFF, 0xB0B0B0u);
    QCOMPARE(c.fieldShade.rgb() & 0xFFFFFF, 0xC0C0C0u);
    QCOMPARE(c.gradientTop.rgb() & 0xFFFFFF, 0xEBEBEBu);
    QCOMPARE(c.gradientBottom.rgb() & 0xFFFFFF, 0xC3C3C3u);
    QCOMPARE(c.fieldFill.rgb() & 0xFFFFFF, 0xF4F4F4u);
    QCOMPARE(c.menuItem.rgb() & 0xFFFFFF, 0xCCCCCCu);
    QCOMPARE(c.menuItemOpen.rgb() & 0xFFFFFF, 0xB3B3B3u);
    QCOMPARE(c.menuFrameLight.rgb() & 0xFFFFFF, 0xF1F1F1u);
    QCOMPARE(c.menuFrameShade.rgb() & 0xFFFFFF, 0xBFBFBFu);
    QCOMPARE(c.tabInactive.rgb() & 0xFFFFFF, 0xC7C7C7u);
    QCOMPARE(c.tabInactiveLight.rgb() & 0xFFFFFF, 0xDBDBDBu);
    QCOMPARE(c.tabInactiveText.rgb() & 0xFFFFFF, 0x636363u);
    QCOMPARE(c.trough.rgb() & 0xFFFFFF, 0xB5B5B5u);
    QCOMPARE(c.troughShade.rgb() & 0xFFFFFF, 0xA1A1A1u);
    QCOMPARE(c.panelTop.rgb() & 0xFFFFFF, 0xC5C5C5u);
    QCOMPARE(c.panelBottom.rgb() & 0xFFFFFF, 0xD9D9D9u);
    QCOMPARE(c.panelShade.rgb() & 0xFFFFFF, 0x999999u);
    QCOMPARE(c.headerLight.rgb() & 0xFFFFFF, 0xF6F6F6u);
    QCOMPARE(c.headerShade.rgb() & 0xFFFFFF, 0xBABABAu);
    QCOMPARE(c.toolCheckedShade.rgb() & 0xFFFFFF, 0xA6A6A6u);
    QCOMPARE(c.checkEtchDark.rgb() & 0xFFFFFF, 0xAEAEAEu);
    QCOMPARE(c.checkEtchLight.rgb() & 0xFFFFFF, 0xF4F4F4u);
    QCOMPARE(c.stripDark.rgb() & 0xFFFFFF, 0xA1A1A1u);
    QCOMPARE(c.etchedDark.rgb() & 0xFFFFFF, 0xC4C4C4u);
    QCOMPARE(c.etchedLight.rgb() & 0xFFFFFF, 0xECECECu);
    QCOMPARE(c.sliderGrooveLight.rgb() & 0xFFFFFF, 0x8C8C8Cu);
    QCOMPARE(c.sliderEtch.rgb() & 0xFFFFFF, 0xA2A2A2u);
    QCOMPARE(c.progressFill.rgb() & 0xFFFFFF, 0xB5C4B0u);
    QCOMPARE(c.progressFillLight.rgb() & 0xFFFFFF, 0xDDECD8u);
    QCOMPARE(c.progressFillShade.rgb() & 0xFFFFFF, 0x8D9C88u);
    QCOMPARE(c.menuHover.rgb() & 0xFFFFFF, 0x9BA9C9u);
    QCOMPARE(c.focus.rgb() & 0xFFFFFF, 0x9098F8u);
}

void TstPhotonPainter::colors_follow_custom_button_colour()
{
    QPalette pal = photon::standardPalette();
    pal.setColor(QPalette::Button, QColor(0xA0, 0xA0, 0xA0));
    const photon::Colors c = photon::colorsFromPalette(pal);
    QCOMPARE(c.gradientTop.rgb() & 0xFFFFFF, 0xB4B4B4u);
    QCOMPARE(c.gradientBottom.rgb() & 0xFFFFFF, 0x8C8C8Cu);
}

void TstPhotonPainter::shifted_clamps_channels()
{
    QCOMPARE(photon::shifted(QColor(250, 10, 128), 20).rgb() & 0xFFFFFF, 0xFF1E94u);
    QCOMPARE(photon::shifted(QColor(250, 10, 128), -20).rgb() & 0xFFFFFF, 0xE6006Cu);
}

void TstPhotonPainter::etch_on_dialog_body()
{
    QImage img = canvas(20, 10, 0xD8D8D8);
    {
        QPainter p(&img);
        photon::drawEtch(&p, QRect(0, 0, 20, 10));
    }
    EXPECT_PX_NEAR(img, 10, 0, 0xC7C7C7, 1);
    EXPECT_PX_NEAR(img, 0, 5, 0xC7C7C7, 1);
    EXPECT_PX_NEAR(img, 10, 9, 0xDDDDDD, 1);
    EXPECT_PX_NEAR(img, 19, 5, 0xDDDDDD, 1);
    EXPECT_PX(img, 10, 5, 0xD8D8D8);
}

void TstPhotonPainter::etch_on_button_strip()
{
    QImage img = canvas(20, 10, 0xC0C0C0);
    {
        QPainter p(&img);
        photon::drawEtch(&p, QRect(0, 0, 20, 10));
    }
    EXPECT_PX_NEAR(img, 10, 0, 0xB1B1B1, 1);
    EXPECT_PX_NEAR(img, 10, 9, 0xC9C9C9, 1);
}

void TstPhotonPainter::gradient_vertical_matches_button_ramp()
{
    QImage img = canvas(4, 21, 0xFF00FF);
    {
        QPainter p(&img);
        photon::fillGradient(&p, img.rect(), QColor(0xD7, 0xD7, 0xD7), photon::kFillContrast,
                             Qt::Vertical, false);
    }
    // Cancel button, qnx621disp2.png column 240, rows 346..366.
    EXPECT_PX(img, 1, 0, 0xEBEBEB);
    EXPECT_PX(img, 1, 1, 0xE9E9E9);
    EXPECT_PX(img, 1, 10, 0xD7D7D7);
    EXPECT_PX(img, 1, 20, 0xC3C3C3);
    for (int y = 1; y < 21; ++y)
        QVERIFY(qRed(px(img, 1, y)) < qRed(px(img, 1, y - 1)));
}

void TstPhotonPainter::gradient_reversed_swaps_ends()
{
    QImage img = canvas(4, 21, 0xFF00FF);
    {
        QPainter p(&img);
        photon::fillGradient(&p, img.rect(), QColor(0xD7, 0xD7, 0xD7), 20, Qt::Vertical, true);
    }
    EXPECT_PX(img, 1, 0, 0xC3C3C3);
    EXPECT_PX(img, 1, 20, 0xEBEBEB);
}

void TstPhotonPainter::gradient_horizontal_ramps_along_x()
{
    QImage img = canvas(21, 4, 0xFF00FF);
    {
        QPainter p(&img);
        photon::fillGradient(&p, img.rect(), QColor(0xD7, 0xD7, 0xD7), 20, Qt::Horizontal, true);
    }
    // Vertical scroll bar slider, qnx621help.png row 162: dark left, light right.
    EXPECT_PX(img, 0, 2, 0xC3C3C3);
    EXPECT_PX(img, 20, 2, 0xEBEBEB);
}

void TstPhotonPainter::gradient_single_row_uses_base()
{
    QImage img = canvas(4, 1, 0xFF00FF);
    {
        QPainter p(&img);
        photon::fillGradient(&p, img.rect(), QColor(0xD7, 0xD7, 0xD7), 20, Qt::Vertical, false);
    }
    EXPECT_PX(img, 0, 0, 0xD7D7D7);
}

void TstPhotonPainter::raised_frame_has_outline_and_bevel()
{
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    QImage img = canvas(20, 12, 0x123456);
    {
        QPainter p(&img);
        photon::drawRaisedFrame(&p, img.rect(), c, false);
    }
    EXPECT_PX(img, 10, 0, 0x4B4B4B);
    EXPECT_PX(img, 0, 6, 0x4B4B4B);
    EXPECT_PX(img, 10, 11, 0x4B4B4B);
    EXPECT_PX(img, 19, 6, 0x4B4B4B);
    EXPECT_PX(img, 10, 1, 0xFFFFFF);
    EXPECT_PX(img, 1, 6, 0xFFFFFF);
    EXPECT_PX(img, 10, 10, 0xB0B0B0);
    EXPECT_PX(img, 18, 6, 0xB0B0B0);
    EXPECT_PX(img, 10, 6, 0x123456);
}

void TstPhotonPainter::sunken_frame_swaps_bevel()
{
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    QImage img = canvas(20, 12, 0x123456);
    {
        QPainter p(&img);
        photon::drawRaisedFrame(&p, img.rect(), c, true);
    }
    EXPECT_PX(img, 10, 1, 0xB0B0B0);
    EXPECT_PX(img, 1, 6, 0xB0B0B0);
    EXPECT_PX(img, 10, 10, 0xFFFFFF);
    EXPECT_PX(img, 18, 6, 0xFFFFFF);
}

void TstPhotonPainter::field_frame_shades_top_left_only()
{
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    QImage img = canvas(20, 12, 0xF4F4F4);
    {
        QPainter p(&img);
        photon::drawFieldFrame(&p, img.rect(), c, c.outline);
    }
    // Line: field, qnx621calc.png column 200 rows 529..551.
    EXPECT_PX(img, 10, 0, 0x4B4B4B);
    EXPECT_PX(img, 10, 1, 0xC0C0C0);
    EXPECT_PX(img, 1, 6, 0xC0C0C0);
    EXPECT_PX(img, 10, 10, 0xF4F4F4);
    EXPECT_PX(img, 18, 6, 0xF4F4F4);
    EXPECT_PX(img, 10, 11, 0x4B4B4B);
    EXPECT_PX(img, 19, 6, 0x4B4B4B);
}

void TstPhotonPainter::etched_line_draws_dark_then_light()
{
    QImage img = canvas(10, 10, 0xD8D8D8);
    {
        QPainter p(&img);
        photon::drawEtchedLine(&p, QPoint(2, 4), Qt::Horizontal, 6, QColor(0xC4C4C4),
                               QColor(0xECECEC));
        photon::drawEtchedLine(&p, QPoint(0, 0), Qt::Vertical, 3, QColor(0xA1A1A1),
                               QColor(0xFFFFFF));
    }
    EXPECT_PX(img, 2, 4, 0xC4C4C4);
    EXPECT_PX(img, 7, 4, 0xC4C4C4);
    EXPECT_PX(img, 8, 4, 0xD8D8D8);
    EXPECT_PX(img, 2, 5, 0xECECEC);
    EXPECT_PX(img, 0, 2, 0xA1A1A1);
    EXPECT_PX(img, 1, 2, 0xFFFFFF);
    EXPECT_PX(img, 0, 3, 0xD8D8D8);
}

void TstPhotonPainter::combo_arrow_down_matches_reference()
{
    QImage img = canvas(16, 16, 0xFFFFFF);
    {
        QPainter p(&img);
        photon::drawArrow(&p, img.rect(), Qt::DownArrow, Qt::black, photon::kComboArrow);
    }
    // Scheme combo, qnx621disp2.png x 371..378, y 162..167.
    const QStringList want = {
        "..####..",
        "..####..",
        "########",
        ".######.",
        "..####..",
        "...##...",
    };
    QCOMPARE(bitmap(img, QRect(4, 5, 8, 6), 0x000000), want);
    QCOMPARE(bitmap(img, QRect(0, 0, 16, 5), 0x000000).join(QString()).count(u'#'), 0);
    QCOMPARE(bitmap(img, QRect(0, 11, 16, 5), 0x000000).join(QString()).count(u'#'), 0);
}

void TstPhotonPainter::combo_arrow_right_matches_submenu_reference()
{
    QImage img = canvas(16, 16, 0xFFFFFF);
    {
        QPainter p(&img);
        photon::drawArrow(&p, img.rect(), Qt::RightArrow, Qt::black, photon::kComboArrow);
    }
    // Launch menu submenu arrow, qnx621about2.png x 143..148, y 447..454.
    const QStringList want = {
        "..#...",
        "..##..",
        "#####.",
        "######",
        "######",
        "#####.",
        "..##..",
        "..#...",
    };
    QCOMPARE(bitmap(img, QRect(5, 4, 6, 8), 0x000000), want);
}

void TstPhotonPainter::scroll_arrow_up_matches_reference()
{
    QImage img = canvas(15, 15, 0xFFFFFF);
    {
        QPainter p(&img);
        photon::drawArrow(&p, img.rect(), Qt::UpArrow, QColor(0x333333), photon::kScrollArrow);
    }
    // Help viewer scroll bar, qnx621help.png x 628..636, y 141..147.
    const QStringList want = {
        "....#....",
        "...###...",
        "..#####..",
        ".#######.",
        "#########",
        "...###...",
        "...###...",
    };
    QCOMPARE(bitmap(img, QRect(3, 4, 9, 7), 0x333333), want);
}

void TstPhotonPainter::branch_arrow_right_is_plain_triangle()
{
    QImage img = canvas(9, 9, 0xFFFFFF);
    {
        QPainter p(&img);
        photon::drawArrow(&p, img.rect(), Qt::RightArrow, QColor(0x323232), photon::kBranchArrow);
    }
    // File manager tree, qnx621fileman.png x 17..20, y 154..160.
    const QStringList want = {
        "#...",
        "##..",
        "###.",
        "####",
        "###.",
        "##..",
        "#...",
    };
    QCOMPARE(bitmap(img, QRect(2, 1, 4, 7), 0x323232), want);
}

void TstPhotonPainter::arrow_in_too_small_rect_draws_nothing()
{
    QImage img = canvas(5, 5, 0xFFFFFF);
    {
        QPainter p(&img);
        photon::drawArrow(&p, img.rect(), Qt::DownArrow, Qt::black, photon::kScrollArrow);
        photon::drawArrow(&p, QRect(), Qt::DownArrow, Qt::black, photon::kComboArrow);
    }
    QCOMPARE(bitmap(img, img.rect(), 0x000000).join(QString()).count(u'#'), 0);
}

void TstPhotonPainter::cross_matches_reference()
{
    QImage img = canvas(10, 10, 0xFFFFFF);
    {
        QPainter p(&img);
        photon::drawCross(&p, img.rect(), Qt::black, Qt::white);
    }
    // Checked box, qnx621disp2.png x 270..277, y 225..232: D dark, g softened.
    const char *want[] = {
        "Dg....gD",
        "gDg..gDg",
        ".gDggDg.",
        "..gDDg..",
        "..gDDg..",
        ".gDggDg.",
        "gDg..gDg",
        "Dg....gD",
    };
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            const QRgb c = px(img, x + 1, y + 1);
            const char w = want[y][x];
            const QRgb expected = w == 'D' ? 0x000000 : w == 'g' ? 0x999999 : 0xFFFFFF;
            QVERIFY2(near(c, expected, 2),
                     qPrintable(QStringLiteral("(%1,%2) is %3, want %4")
                                    .arg(x).arg(y).arg(hex(c)).arg(hex(expected))));
        }
    }
    EXPECT_PX(img, 0, 0, 0xFFFFFF);
    EXPECT_PX(img, 9, 9, 0xFFFFFF);
}

void TstPhotonPainter::radio_is_lit_from_top_left()
{
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    QImage img = canvas(13, 13, 0xD8D8D8);
    {
        QPainter p(&img);
        photon::drawRadio(&p, img.rect(), false, true, c);
    }
    // Unchecked "Left", qnx621disp2.png: E0 upper left, B5 lower right.
    QVERIFY(qRed(px(img, 3, 3)) > 0xD4);
    QVERIFY(qRed(px(img, 9, 9)) < 0xC4);
    QVERIFY(qRed(px(img, 0, 6)) < 0x70);
    QVERIFY(qRed(px(img, 6, 0)) < 0x70);
    QVERIFY(qRed(px(img, 6, 6)) > 0xB8);
}

void TstPhotonPainter::radio_checked_has_dark_centre()
{
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    QImage img = canvas(13, 13, 0xD8D8D8);
    {
        QPainter p(&img);
        photon::drawRadio(&p, img.rect(), true, true, c);
    }
    QVERIFY(qRed(px(img, 6, 6)) < 0x60);
    QVERIFY(qRed(px(img, 3, 3)) > 0xB0);
}

void TstPhotonPainter::tab_outline_geometry()
{
    // Tab 22 high at x 10..70: slant top at 70-11=59, bottom 59+21=80.
    const QPolygon poly = photon::tabOutline(QRect(10, 5, 61, 22));
    QCOMPARE(poly.size(), 5);
    QCOMPARE(poly.at(0), QPoint(10, 26));
    QCOMPARE(poly.at(1), QPoint(10, 6));
    QCOMPARE(poly.at(2), QPoint(11, 5));
    QCOMPARE(poly.at(3), QPoint(59, 5));
    QCOMPARE(poly.at(4), QPoint(80, 26));
}

void TstPhotonPainter::left_neighbour_slant_covers_bottom_left_only()
{
    const QRect r(71, 5, 61, 22);
    const QPolygon owned = photon::leftNeighbourSlant(r);
    // The neighbour at 10..70 slants from (59,5) to (80,26).
    QVERIFY(owned.containsPoint(QPoint(71, 26), Qt::OddEvenFill));
    QVERIFY(owned.containsPoint(QPoint(78, 25), Qt::OddEvenFill));
    QVERIFY(!owned.containsPoint(QPoint(71, 5), Qt::OddEvenFill));
    QVERIFY(!owned.containsPoint(QPoint(82, 26), Qt::OddEvenFill));
}

void TstPhotonPainter::ring_colours_derive_from_window_not_kde_shades()
{
    // KDE computes Light/Dark/Mid/Shadow with its own contrast formula
    // (#3B3B3B shadow on a #CECECE scheme); Photon's rings follow Window.
    QPalette pal = photon::standardPalette();
    pal.setColor(QPalette::Shadow, Qt::black);
    pal.setColor(QPalette::Dark, QColor(0x80, 0x80, 0x80));
    pal.setColor(QPalette::Mid, QColor(0xA0, 0xA0, 0xA0));
    pal.setColor(QPalette::Light, QColor(0xF0, 0xF0, 0xF0));
    pal.setColor(QPalette::Midlight, QColor(0xE0, 0xE0, 0xE0));
    const photon::Colors c = photon::colorsFromPalette(pal);
    QCOMPARE(c.outline.rgb() & 0xFFFFFF, 0x4B4B4Bu);
    QCOMPARE(c.shade.rgb() & 0xFFFFFF, 0xB0B0B0u);
    QCOMPARE(c.fieldShade.rgb() & 0xFFFFFF, 0xC0C0C0u);
    QCOMPARE(c.light.rgb() & 0xFFFFFF, 0xFFFFFFu);
    QCOMPARE(c.midlight.rgb() & 0xFFFFFF, 0xECECECu);
}

void TstPhotonPainter::frame_lines_stay_one_device_pixel_at_17_16()
{
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    // Every logical offset 0..16 so some edge lands on a rounding boundary.
    for (int off = 0; off <= 16; ++off) {
        QImage img = scaledCanvas(60, 40, kPlasmaDpr, 0x123456);
        {
            QPainter p(&img);
            photon::drawRaisedFrame(&p, QRect(off, 3, 30, 20), c, false);
        }
        const int midY = qRound(13 * kPlasmaDpr);
        int x = 0;
        while (x < img.width() && px(img, x, midY) != 0x4B4B4B)
            ++x;
        QVERIFY2(x < img.width(), qPrintable(QString::number(off)));
        QVERIFY2(runLength(img, x, midY, 1, 0, 0x4B4B4B) == 1,
                 qPrintable(QStringLiteral("left outline at offset %1").arg(off)));
        QVERIFY2(runLength(img, x + 1, midY, 1, 0, 0xFFFFFF) == 1,
                 qPrintable(QStringLiteral("left bevel at offset %1").arg(off)));
        int right = img.width() - 1;
        while (right > 0 && px(img, right, midY) != 0x4B4B4B)
            --right;
        QVERIFY2(runLength(img, right, midY, -1, 0, 0x4B4B4B) == 1,
                 qPrintable(QStringLiteral("right outline at offset %1").arg(off)));
        QVERIFY2(runLength(img, right - 1, midY, -1, 0, 0xB0B0B0) == 1,
                 qPrintable(QStringLiteral("right bevel at offset %1").arg(off)));
    }
}

void TstPhotonPainter::frame_lines_are_two_device_pixels_at_2x()
{
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    QImage img = scaledCanvas(40, 30, 2.0, 0x123456);
    {
        QPainter p(&img);
        photon::drawRaisedFrame(&p, QRect(3, 3, 30, 20), c, false);
    }
    QCOMPARE(runLength(img, 6, 26, 1, 0, 0x4B4B4B), 2);
    QCOMPARE(runLength(img, 8, 26, 1, 0, 0xFFFFFF), 2);
}

void TstPhotonPainter::cross_keeps_its_pattern_at_17_16()
{
    for (int off = 0; off <= 16; ++off) {
        QImage img = scaledCanvas(40, 20, kPlasmaDpr, 0xFFFFFF);
        {
            QPainter p(&img);
            photon::drawCross(&p, QRect(off, 2, 10, 10), Qt::black, Qt::white);
        }
        // Each diagonal is exactly one dark pixel per row: 16 dark pixels,
        // the two centre rows sharing their pair.
        int dark = 0;
        for (int y = 0; y < img.height(); ++y)
            for (int x = 0; x < img.width(); ++x)
                dark += px(img, x, y) == 0x000000;
        QVERIFY2(dark == 16, qPrintable(QStringLiteral("offset %1: %2 dark").arg(off).arg(dark)));
    }
}

void TstPhotonPainter::tab_slant_is_straight_at_17_16()
{
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    for (int off = 0; off <= 16; ++off) {
        QImage img = scaledCanvas(140, 40, kPlasmaDpr, 0xC0C0C0);
        {
            QPainter p(&img);
            photon::drawTab(&p, QRect(off + 5, 5, 61, 23), true, false, c);
        }
        // Walk the slant: exactly one outline pixel per row, one column right
        // of the row above, from the top edge down to the pane line row. The
        // last row holds only the left edge (see the pane line test).
        int prev = -1;
        int rows = 0;
        const int leftEdge = qRound((off + 5) * kPlasmaDpr);
        for (int y = 0; y < img.height(); ++y) {
            int last = -1;
            for (int x = 0; x < img.width(); ++x) {
                if (px(img, x, y) == 0x4B4B4B)
                    last = x;
            }
            if (last <= leftEdge)
                continue;
            if (prev >= 0 && rows > 0) {
                QVERIFY2(last == prev + 1,
                         qPrintable(QStringLiteral("offset %1 row %2: %3 after %4")
                                        .arg(off).arg(y).arg(last).arg(prev)));
            }
            prev = last;
            ++rows;
        }
        QVERIFY(rows > 20);
    }
}

void TstPhotonPainter::check_box_outline_is_even_at_17_16()
{
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    for (int off = 0; off <= 16; ++off) {
        QImage img = scaledCanvas(40, 20, kPlasmaDpr, 0xD8D8D8);
        {
            QPainter p(&img);
            photon::drawCheckBox(&p, QRect(off, 2, 14, 14), true, true, false, c);
        }
        const int midY = qRound(9 * kPlasmaDpr);
        int x = 0;
        while (x < img.width() && px(img, x, midY) != (c.checkOutline.rgb() & 0xFFFFFF))
            x += 1;
        QVERIFY2(x < img.width(), qPrintable(QString::number(off)));
        QVERIFY2(runLength(img, x, midY, 1, 0, c.checkOutline.rgb() & 0xFFFFFF) == 1,
                 qPrintable(QStringLiteral("left outline at offset %1").arg(off)));
        int right = img.width() - 1;
        while (right > 0 && px(img, right, midY) != (c.checkOutline.rgb() & 0xFFFFFF))
            --right;
        QVERIFY2(runLength(img, right, midY, -1, 0, c.checkOutline.rgb() & 0xFFFFFF) == 1,
                 qPrintable(QStringLiteral("right outline at offset %1").arg(off)));
    }
}

void TstPhotonPainter::selected_slant_stops_on_the_pane_line()
{
    // qnx621disp2.png: the selected slant ends on the pane's top line row
    // (the tab's second to last row); the last row is the pane highlight.
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    for (const qreal dpr : {1.0, kPlasmaDpr}) {
        for (int off = 0; off <= 16; ++off) {
            QImage img = scaledCanvas(140, 40, dpr, 0xC0C0C0);
            const QRect tab(off + 5, 5, 61, 23);
            {
                QPainter p(&img);
                p.translate(tab.topLeft());
                photon::drawTab(&p, QRect(QPoint(0, 0), tab.size()), true, false, c);
            }
            const int lastRow = qRound((tab.top() + tab.height()) * dpr) - 1;
            const int leftEdge = qRound(tab.left() * dpr);
            for (int x = leftEdge + 1; x < img.width(); ++x) {
                QVERIFY2(px(img, x, lastRow) != 0x4B4B4B,
                         qPrintable(QStringLiteral("dpr %1 offset %2: outline at %3,%4")
                                        .arg(dpr).arg(off).arg(x).arg(lastRow)));
            }
        }
    }
}

void TstPhotonPainter::neighbour_slant_survives_the_next_tab()
{
    // Two unselected neighbours: the right one is painted later and must
    // leave the left one's slant outline intact on every row down to its
    // lowest (the tab's third to last row).
    const photon::Colors c = photon::colorsFromPalette(photon::standardPalette());
    for (const qreal dpr : {1.0, kPlasmaDpr}) {
        for (int off = 0; off <= 16; ++off) {
            QImage img = scaledCanvas(200, 40, dpr, 0xC0C0C0);
            const QRect left(off + 5, 5, 61, 23);
            const QRect right(left.right() + 1, 5, 61, 23);
            {
                // The style translates to each tab's position first, which at
                // 17/16 puts the device origin on a fraction of a pixel.
                QPainter p(&img);
                p.save();
                p.translate(left.topLeft());
                photon::drawTab(&p, QRect(QPoint(0, 0), left.size()), false, false, c);
                p.restore();
                p.translate(right.topLeft());
                photon::drawTab(&p, QRect(QPoint(0, 0), right.size()), false, true, c);
            }
            // Follow the slant from where the top edge ends; positions are not
            // predicted, so the check holds for any device rounding.
            const int top = qRound(left.top() * dpr);
            const int lowest = qRound((left.top() + left.height() - 2) * dpr) - 1;
            int slantTop = qRound(left.left() * dpr) + 1;
            while (slantTop + 1 < img.width() && px(img, slantTop + 1, top) == 0x4B4B4B)
                ++slantTop;
            for (int y = top; y <= lowest; ++y) {
                const int x = slantTop + (y - top);
                QVERIFY2(px(img, x, y) == 0x4B4B4B,
                         qPrintable(QStringLiteral("dpr %1 offset %2 row %3: slant at %4 is %5")
                                        .arg(dpr).arg(off).arg(y).arg(x).arg(hex(px(img, x, y)))));
            }
        }
    }
}

QTEST_MAIN(TstPhotonPainter)
#include "tst_photonpainter.moc"
