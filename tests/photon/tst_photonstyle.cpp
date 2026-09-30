// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#include "photonpainter.h"
#include "qphotonstyle.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QImage>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QStyleOption>
#include <QTabBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTest>
#include <QToolButton>
#include <QVBoxLayout>

#include <cstdlib>
#include <memory>

// Expected values are measured from the reference/qnx-photon/ screenshots.

namespace {

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

#define EXPECT_PX_NEAR(img, x, y, rgb, tol)                                             \
    QVERIFY2(near(px(img, x, y), (rgb), (tol)),                                       \
             qPrintable(QStringLiteral("(%1,%2) is %3, want %4 +/-%5")                \
                            .arg(x).arg(y).arg(hex(px(img, x, y))).arg(hex(rgb)).arg(tol)))

#define EXPECT_PX(img, x, y, rgb) EXPECT_PX_NEAR(img, x, y, rgb, 0)

} // namespace

class TstPhotonStyle : public QObject
{
    Q_OBJECT

private:
    std::unique_ptr<QPhotonStyle> m_style;

    // Styles the widget and its children with Photon and the standard palette.
    void apply(QWidget *w);
    QImage render(QWidget *w);

private slots:
    void init();
    void cleanup();

    void metrics_match_reference();
    void standard_palette_is_photon();

    void push_button_has_etch_outline_bevel_gradient();
    void push_button_pressed_reverses_bevel();
    void push_button_minimum_size();
    void line_edit_is_field_with_top_left_shade();
    void check_box_indicator_is_boxed_cross();
    void radio_button_checked_has_dark_centre();

    void combo_is_button_with_etched_separator();
    void editable_combo_has_field_and_arrow_button();
    void spin_box_buttons_are_stacked();
    void scroll_bar_trough_slider_and_disabled_arrow();
    void scroll_bar_rects();
    void slider_handle_is_gradient_on_groove();
    void slider_focus_ring_leaves_handle_outline();
    void progress_bar_fill_and_trough();

    void tabs_selected_unselected_and_strip();
    void tabs_neighbours_stack_left_over_right();
    void tabs_last_slant_is_not_clipped();
    void tabs_other_shapes_render();

    void menu_bar_is_gradient_panel();
    void menu_rows_gaps_and_hover();
    void tool_button_checked_is_sunken_white();
    void header_section_is_dark_on_top();
    void item_view_selection_colour();

    void painter_state_survives_every_entry_point();
    void menu_item_pitch_follows_font();
    void null_widget_sweep();
    void showcase_renders();
};

void TstPhotonStyle::init()
{
    m_style = std::make_unique<QPhotonStyle>();
}

void TstPhotonStyle::cleanup()
{
    m_style.reset();
}

void TstPhotonStyle::apply(QWidget *w)
{
    const QPalette pal = m_style->standardPalette();
    w->setStyle(m_style.get());
    w->setPalette(pal);
    QFont f(QStringLiteral("DejaVu Sans"));
    f.setPixelSize(12);
    w->setFont(f);
    for (QWidget *child : w->findChildren<QWidget *>()) {
        child->setStyle(m_style.get());
        child->setPalette(pal);
    }
}

QImage TstPhotonStyle::render(QWidget *w)
{
    // Shown, because QTabWidget and friends defer their layout while hidden.
    w->show();
    if (!QTest::qWaitForWindowExposed(w))
        qWarning("window not exposed");
    return w->grab().toImage().convertToFormat(QImage::Format_RGB32);
}

void TstPhotonStyle::metrics_match_reference()
{
    const QStyle &s = *m_style;
    QCOMPARE(s.pixelMetric(QStyle::PM_DefaultFrameWidth), 3);
    QCOMPARE(s.pixelMetric(QStyle::PM_ButtonMargin), 8);
    QCOMPARE(s.pixelMetric(QStyle::PM_ButtonDefaultIndicator), 0);
    QCOMPARE(s.pixelMetric(QStyle::PM_ButtonShiftHorizontal), 1);
    QCOMPARE(s.pixelMetric(QStyle::PM_ButtonShiftVertical), 1);
    QCOMPARE(s.pixelMetric(QStyle::PM_IndicatorWidth), 14);
    QCOMPARE(s.pixelMetric(QStyle::PM_IndicatorHeight), 14);
    QCOMPARE(s.pixelMetric(QStyle::PM_ExclusiveIndicatorWidth), 13);
    QCOMPARE(s.pixelMetric(QStyle::PM_ExclusiveIndicatorHeight), 13);
    QCOMPARE(s.pixelMetric(QStyle::PM_ScrollBarExtent), 17);
    QCOMPARE(s.pixelMetric(QStyle::PM_ScrollBarSliderMin), 10);
    QCOMPARE(s.pixelMetric(QStyle::PM_SliderThickness), 20);
    QCOMPARE(s.pixelMetric(QStyle::PM_SliderLength), 14);
    QCOMPARE(s.pixelMetric(QStyle::PM_SliderControlThickness), 20);
    QCOMPARE(s.pixelMetric(QStyle::PM_ComboBoxFrameWidth), 3);
    QCOMPARE(s.pixelMetric(QStyle::PM_SpinBoxFrameWidth), 3);
    QCOMPARE(s.pixelMetric(QStyle::PM_MenuBarPanelWidth), 1);
    QCOMPARE(s.pixelMetric(QStyle::PM_MenuPanelWidth), 2);
    QCOMPARE(s.pixelMetric(QStyle::PM_TabBarBaseOverlap), 2);
    QCOMPARE(s.pixelMetric(QStyle::PM_ToolBarSeparatorExtent), 6);
    QCOMPARE(s.pixelMetric(QStyle::PM_SplitterWidth), 6);
    QCOMPARE(s.pixelMetric(QStyle::PM_TitleBarHeight), 20);
}

void TstPhotonStyle::standard_palette_is_photon()
{
    const QPalette pal = m_style->standardPalette();
    QCOMPARE(pal.color(QPalette::Window).rgb() & 0xFFFFFF, 0xD8D8D8u);
    QCOMPARE(pal.color(QPalette::Highlight).rgb() & 0xFFFFFF, 0x8EA29Bu);
}

void TstPhotonStyle::push_button_has_etch_outline_bevel_gradient()
{
    QPushButton b(QStringLiteral("Cancel"));
    apply(&b);
    b.setFixedSize(75, 27);
    const QImage img = render(&b);
    // qnx621disp2.png Cancel button, column 240, rows 343..369.
    EXPECT_PX_NEAR(img, 10, 0, 0xC7C7C7, 1);
    EXPECT_PX(img, 10, 1, 0x4B4B4B);
    EXPECT_PX(img, 10, 2, 0xFFFFFF);
    EXPECT_PX(img, 10, 3, 0xEBEBEB);
    EXPECT_PX(img, 10, 23, 0xC3C3C3);
    EXPECT_PX(img, 10, 24, 0xB0B0B0);
    EXPECT_PX(img, 10, 25, 0x4B4B4B);
    EXPECT_PX_NEAR(img, 10, 26, 0xDDDDDD, 1);
    EXPECT_PX(img, 1, 13, 0x4B4B4B);
    EXPECT_PX(img, 2, 13, 0xFFFFFF);
    EXPECT_PX(img, 72, 13, 0xB0B0B0);
    EXPECT_PX(img, 73, 13, 0x4B4B4B);
}

void TstPhotonStyle::push_button_pressed_reverses_bevel()
{
    QPushButton b(QStringLiteral("Ok"));
    apply(&b);
    b.setFixedSize(75, 27);
    b.setDown(true);
    const QImage img = render(&b);
    EXPECT_PX(img, 10, 2, 0xB0B0B0);
    EXPECT_PX(img, 10, 24, 0xFFFFFF);
    EXPECT_PX(img, 10, 3, 0xC3C3C3);
}

void TstPhotonStyle::push_button_minimum_size()
{
    QPushButton b(QStringLiteral("Ok"));
    apply(&b);
    const QSize hint = b.sizeHint();
    QVERIFY2(hint.width() >= 72, qPrintable(QString::number(hint.width())));
    QVERIFY2(hint.height() >= 27, qPrintable(QString::number(hint.height())));
}

void TstPhotonStyle::line_edit_is_field_with_top_left_shade()
{
    QLineEdit e;
    apply(&e);
    e.setFixedSize(100, 24);
    const QImage img = render(&e);
    // Line: field, qnx621calc.png.
    EXPECT_PX(img, 10, 1, 0x4B4B4B);
    EXPECT_PX(img, 10, 2, 0xC0C0C0);
    EXPECT_PX(img, 2, 10, 0xC0C0C0);
    EXPECT_PX(img, 10, 10, 0xF4F4F4);
    EXPECT_PX(img, 97, 10, 0xF4F4F4);
    EXPECT_PX(img, 98, 10, 0x4B4B4B);
    EXPECT_PX(img, 10, 22, 0x4B4B4B);
}

void TstPhotonStyle::check_box_indicator_is_boxed_cross()
{
    QCheckBox c(QStringLiteral("Full window dragging"));
    apply(&c);
    c.setChecked(true);
    c.resize(c.sizeHint());
    const QImage img = render(&c);
    QStyleOptionButton opt;
    opt.initFrom(&c);
    const QRect ind = m_style->subElementRect(QStyle::SE_CheckBoxIndicator, &opt, &c);
    QCOMPARE(ind.size(), QSize(14, 14));
    const int x = ind.left();
    const int y = ind.top();
    // qnx621disp2.png "Full window dragging" box at 267,222.
    EXPECT_PX_NEAR(img, x, y + 7, 0xAEAEAE, 1);
    QVERIFY(qRed(px(img, x + 1, y + 7)) < 0x50);
    EXPECT_PX(img, x + 2, y + 2, 0xFFFFFF);
    EXPECT_PX(img, x + 3, y + 3, 0x000000);
    EXPECT_PX(img, x + 6, y + 6, 0x000000);
    EXPECT_PX(img, x + 7, y + 7, 0x000000);
    EXPECT_PX(img, x + 6, y + 3, 0xFFFFFF);
}

void TstPhotonStyle::radio_button_checked_has_dark_centre()
{
    QRadioButton r(QStringLiteral("Center"));
    apply(&r);
    r.setChecked(true);
    r.resize(r.sizeHint());
    const QImage img = render(&r);
    QStyleOptionButton opt;
    opt.initFrom(&r);
    const QRect ind = m_style->subElementRect(QStyle::SE_RadioButtonIndicator, &opt, &r);
    QCOMPARE(ind.size(), QSize(13, 13));
    QVERIFY(qRed(px(img, ind.left() + 6, ind.top() + 6)) < 0x60);
    QVERIFY(qRed(px(img, ind.left() + 3, ind.top() + 3)) > 0xB0);
}

void TstPhotonStyle::combo_is_button_with_etched_separator()
{
    QComboBox c;
    c.addItem(QStringLiteral("Custom"));
    apply(&c);
    c.setFixedSize(218, 27);
    const QImage img = render(&c);
    // Scheme combo, qnx621disp2.png x 166..383, row 165.
    EXPECT_PX(img, 20, 1, 0x4B4B4B);
    EXPECT_PX(img, 20, 2, 0xFFFFFF);
    EXPECT_PX(img, 20, 3, 0xEBEBEB);
    EXPECT_PX(img, 218 - 17, 13, 0xA1A1A1);
    EXPECT_PX(img, 218 - 16, 13, 0xFFFFFF);
    // Arrow head row centred in 369..380 of the reference.
    EXPECT_PX(img, 218 - 9, 14, 0x000000);
}

void TstPhotonStyle::editable_combo_has_field_and_arrow_button()
{
    QComboBox c;
    c.setEditable(true);
    apply(&c);
    c.setFixedSize(150, 27);
    const QImage img = render(&c);
    // Device combo, qnx621network.png.
    EXPECT_PX(img, 4, 20, 0xF4F4F4);
    EXPECT_PX(img, 10, 2, 0xC0C0C0);
    QStyleOptionComboBox opt;
    opt.initFrom(&c);
    opt.editable = true;
    const QRect arrow =
        m_style->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, &c);
    QVERIFY(arrow.width() >= 14 && arrow.width() <= 18);
    EXPECT_PX(img, arrow.center().x(), arrow.top() + 1, 0xFFFFFF);
}

void TstPhotonStyle::spin_box_buttons_are_stacked()
{
    QSpinBox s;
    apply(&s);
    s.setFixedSize(80, 27);
    QStyleOptionSpinBox opt;
    opt.initFrom(&s);
    opt.subControls = QStyle::SC_All;
    // Mirror the widget; the option defaults to frame = false.
    opt.frame = s.hasFrame();
    opt.buttonSymbols = s.buttonSymbols();
    const QRect up = m_style->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxUp, &s);
    const QRect down =
        m_style->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxDown, &s);
    const QRect field =
        m_style->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxEditField, &s);
    QVERIFY(up.isValid() && down.isValid());
    QCOMPARE(up.width(), down.width());
    QCOMPARE(up.left(), down.left());
    QVERIFY(up.bottom() < down.top());
    QVERIFY(field.right() < up.left());
    const QImage img = render(&s);
    EXPECT_PX(img, up.left() + 1, up.top() + 1, 0xFFFFFF);
}

void TstPhotonStyle::scroll_bar_trough_slider_and_disabled_arrow()
{
    QScrollBar sb(Qt::Vertical);
    apply(&sb);
    sb.setRange(0, 100);
    sb.setPageStep(10);
    sb.setValue(0);
    sb.setFixedSize(17, 200);
    const QImage img = render(&sb);
    // Help viewer scroll bar, qnx621help.png x 624..640.
    EXPECT_PX(img, 8, 150, 0xB5B5B5);
    EXPECT_PX(img, 1, 150, 0xA1A1A1);
    EXPECT_PX(img, 0, 150, 0x4B4B4B);
    EXPECT_PX(img, 16, 150, 0x4B4B4B);
    // Slider sits right below the 16 px up button; gradient runs across.
    QVERIFY(qRed(px(img, 3, 22)) < qRed(px(img, 13, 22)));
    // At the minimum the up arrow is drawn disabled.
    bool sawDisabled = false;
    for (int y = 2; y < 15; ++y)
        sawDisabled |= px(img, 8, y) == 0x858585;
    QVERIFY(sawDisabled);
}

void TstPhotonStyle::scroll_bar_rects()
{
    QScrollBar sb(Qt::Vertical);
    apply(&sb);
    sb.setRange(0, 10000);
    sb.setPageStep(1);
    sb.setFixedSize(17, 200);
    QStyleOptionSlider opt;
    opt.initFrom(&sb);
    opt.orientation = Qt::Vertical;
    opt.minimum = 0;
    opt.maximum = 10000;
    opt.pageStep = 1;
    opt.rect = QRect(0, 0, 17, 200);
    const QRect sub = m_style->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSubLine, &sb);
    const QRect add = m_style->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarAddLine, &sb);
    const QRect slider = m_style->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SC_ScrollBarSlider, &sb);
    QCOMPARE(sub.height(), 16);
    QCOMPARE(add.height(), 16);
    QCOMPARE(slider.height(), 10);
}

void TstPhotonStyle::slider_handle_is_gradient_on_groove()
{
    QSlider s(Qt::Horizontal);
    apply(&s);
    s.setRange(0, 100);
    s.setValue(0);
    s.setFixedSize(200, 24);
    QStyleOptionSlider opt;
    opt.initFrom(&s);
    opt.orientation = Qt::Horizontal;
    opt.minimum = 0;
    opt.maximum = 100;
    opt.rect = s.rect();
    const QRect handle = m_style->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, &s);
    QCOMPARE(handle.width(), 14);
    const QImage img = render(&s);
    const int cx = handle.center().x();
    // Mouse speed slider, qnx621disp2.png x 477..490.
    EXPECT_PX(img, cx, handle.top(), 0x4B4B4B);
    EXPECT_PX_NEAR(img, cx, handle.top() + 1, 0xEDEDED, 2);
    QVERIFY(qRed(px(img, cx, handle.bottom() - 2)) < 0xD0);
    bool sawGroove = false;
    for (int y = 0; y < 24; ++y)
        sawGroove |= px(img, 150, y) == 0x4B4B4B;
    QVERIFY(sawGroove);
}

void TstPhotonStyle::slider_focus_ring_leaves_handle_outline()
{
    QSlider s(Qt::Horizontal);
    apply(&s);
    s.setRange(0, 100);
    s.setValue(50);
    s.setFixedSize(200, 24);
    s.setFocusPolicy(Qt::StrongFocus);
    render(&s);
    s.setFocus(Qt::OtherFocusReason);
    QVERIFY(QTest::qWaitFor([&] { return s.hasFocus(); }));
    const QImage img = render(&s);
    QStyleOptionSlider opt;
    opt.initFrom(&s);
    opt.orientation = Qt::Horizontal;
    opt.minimum = 0;
    opt.maximum = 100;
    opt.sliderPosition = 50;
    opt.sliderValue = 50;
    const QRect handle = m_style->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, &s);
    QVERIFY(opt.state & QStyle::State_HasFocus);
    // The handle touches the top edge; its outline must survive the ring.
    EXPECT_PX(img, handle.center().x(), handle.top(), 0x4B4B4B);
    EXPECT_PX(img, handle.left() - 2, handle.center().y(), 0x9098F8);
}

void TstPhotonStyle::progress_bar_fill_and_trough()
{
    QProgressBar p;
    apply(&p);
    p.setRange(0, 100);
    p.setValue(50);
    p.setTextVisible(false);
    p.setFixedSize(200, 20);
    const QImage img = render(&p);
    // Shelf system monitor, qnx621disp2.png x 694..797.
    EXPECT_PX(img, 30, 10, 0xB5C4B0);
    EXPECT_PX(img, 170, 10, 0xBCBCBC);
    EXPECT_PX(img, 0, 10, 0x4B4B4B);
    bool sawLight = false;
    for (int x = 0; x < 8; ++x)
        sawLight |= px(img, x, 10) == 0xDDECD8;
    QVERIFY(sawLight);
}

void TstPhotonStyle::tabs_selected_unselected_and_strip()
{
    QTabWidget t;
    t.addTab(new QWidget, QStringLiteral("Window"));
    t.addTab(new QWidget, QStringLiteral("B"));
    apply(&t);
    t.resize(320, 200);
    t.setCurrentIndex(0);
    const QImage img = render(&t);
    QTabBar *bar = t.findChild<QTabBar *>();
    QVERIFY(bar);
    const QPoint off = bar->mapTo(&t, QPoint(0, 0));
    const QRect r0 = bar->tabRect(0).translated(off);
    const QRect r1 = bar->tabRect(1).translated(off);
    // qnx621disp2.png Window/Background tabs.
    EXPECT_PX(img, r0.left() + 3, r0.top() + 3, 0xD8D8D8);
    EXPECT_PX(img, r0.left() + 1, r0.top() + 3, 0xECECEC);
    EXPECT_PX(img, r0.left(), r0.top() + 3, 0x4B4B4B);
    // Tab 1 is the last tab: its slant is reserved inside its rect, so the
    // body's top edge ends a full tab height before the right edge.
    EXPECT_PX(img, r1.right() - r1.height() - 3, r1.top() + 3, 0xC7C7C7);
    EXPECT_PX(img, r1.right() + 30, r1.top() + 3, 0xC0C0C0);
    // The selected tab's slant reaches past its rect over the neighbour.
    const QRgb slant = px(img, r0.right() + 3, r0.bottom() - 2);
    QVERIFY2(slant == 0xD8D8D8 || slant == 0x4B4B4B || slant == 0xE2E2E2,
             qPrintable(hex(slant)));
}

void TstPhotonStyle::tabs_neighbours_stack_left_over_right()
{
    QTabWidget t;
    t.addTab(new QWidget, QStringLiteral("Devices"));
    t.addTab(new QWidget, QStringLiteral("Connections"));
    t.addTab(new QWidget, QStringLiteral("Network"));
    apply(&t);
    t.resize(400, 200);
    t.setCurrentIndex(0);
    const QImage img = render(&t);
    QTabBar *bar = t.findChild<QTabBar *>();
    const QPoint off = bar->mapTo(&t, QPoint(0, 0));
    const QRect r2 = bar->tabRect(2).translated(off);
    // Tab 1's slant covers tab 2's bottom-left corner.
    EXPECT_PX(img, r2.left(), r2.bottom() - 2, 0xC7C7C7);
}

void TstPhotonStyle::tabs_last_slant_is_not_clipped()
{
    QTabWidget t;
    t.addTab(new QWidget, QStringLiteral("Window"));
    t.addTab(new QWidget, QStringLiteral("Background"));
    apply(&t);
    t.resize(400, 200);
    t.setCurrentIndex(0);
    const QImage img = render(&t);
    QTabBar *bar = t.findChild<QTabBar *>();
    const QPoint off = bar->mapTo(&t, QPoint(0, 0));
    const QRect last = bar->tabRect(1).translated(off);
    const QRect barRect = bar->geometry();
    // The last tab's slant must reach its lowest unselected row inside the
    // tab bar; nothing past the bar's edge is ever painted.
    const int y = last.bottom() - 2;
    bool sawSlant = false;
    for (int x = last.center().x(); x < barRect.right(); ++x)
        sawSlant |= px(img, x, y) == 0x4B4B4B;
    QVERIFY2(sawSlant, qPrintable(QStringLiteral("no slant on row %1 before x=%2").arg(y).arg(barRect.right())));
}

void TstPhotonStyle::tabs_other_shapes_render()
{
    for (const QTabWidget::TabPosition pos :
         {QTabWidget::South, QTabWidget::West, QTabWidget::East}) {
        QTabWidget t;
        t.addTab(new QWidget, QStringLiteral("One"));
        t.addTab(new QWidget, QStringLiteral("Two"));
        t.setTabPosition(pos);
        apply(&t);
        t.resize(300, 200);
        const QImage img = render(&t);
        QVERIFY(!img.isNull());
    }
}

void TstPhotonStyle::menu_bar_is_gradient_panel()
{
    QMenuBar mb;
    mb.addMenu(QStringLiteral("File"));
    mb.addMenu(QStringLiteral("Edit"));
    apply(&mb);
    mb.setFixedSize(300, 24);
    const QImage img = render(&mb);
    // Editor menu bar, qnx621calc.png column 300, rows 365..391.
    const int x = 250;
    EXPECT_PX(img, x, 0, 0x4B4B4B);
    EXPECT_PX(img, x, 1, 0xFFFFFF);
    EXPECT_PX_NEAR(img, x, 2, 0xC5C5C5, 1);
    EXPECT_PX_NEAR(img, x, 21, 0xD9D9D9, 1);
    EXPECT_PX(img, x, 22, 0x999999);
    EXPECT_PX(img, x, 23, 0x4B4B4B);
}

void TstPhotonStyle::menu_rows_gaps_and_hover()
{
    QMenu m;
    QAction *a = m.addAction(QStringLiteral("MultiMedia"));
    QAction *b = m.addAction(QStringLiteral("Editors"));
    m.addSeparator();
    m.addAction(QStringLiteral("Help"));
    apply(&m);
    m.resize(m.sizeHint());
    m.setActiveAction(b);
    const QImage img = render(&m);
    const QRect ra = m.actionGeometry(a);
    const QRect rb = m.actionGeometry(b);
    // Launch menu, qnx621about2.png.
    EXPECT_PX(img, 0, 5, 0x000000);
    EXPECT_PX(img, 1, 5, 0xF1F1F1);
    EXPECT_PX(img, ra.right() - 4, ra.top() + 2, 0xCCCCCC);
    EXPECT_PX(img, rb.right() - 4, rb.top() + 2, 0x9BA9C9);
    EXPECT_PX(img, ra.right() - 4, ra.bottom(), 0xD8D8D8);
}

void TstPhotonStyle::tool_button_checked_is_sunken_white()
{
    QToolButton b;
    b.setText(QStringLiteral("i"));
    b.setCheckable(true);
    b.setChecked(true);
    b.setAutoRaise(true);
    apply(&b);
    b.setFixedSize(26, 26);
    const QImage img = render(&b);
    // Italic toggle, qnx621calc.png x 492..526.
    EXPECT_PX(img, 5, 5, 0xFFFFFF);
    EXPECT_PX(img, 2, 10, 0xA6A6A6);
}

void TstPhotonStyle::header_section_is_dark_on_top()
{
    QTableWidget t(2, 2);
    t.setHorizontalHeaderLabels({QStringLiteral("Filename"), QStringLiteral("Size")});
    apply(&t);
    t.resize(300, 150);
    QHeaderView *h = t.horizontalHeader();
    h->resize(280, h->sizeHint().height());
    const QImage img = h->grab().toImage().convertToFormat(QImage::Format_RGB32);
    const int x = h->sectionSize(0) - 10;
    // File manager header, qnx621fileman.png column 250.
    QVERIFY(qRed(px(img, x, 3)) + 20 < qRed(px(img, x, img.height() - 4)));
}

void TstPhotonStyle::item_view_selection_colour()
{
    QListWidget l;
    l.addItems({QStringLiteral(".."), QStringLiteral("snap1.png"), QStringLiteral("snap2.png")});
    apply(&l);
    l.resize(200, 120);
    l.setCurrentRow(0);
    const QImage img = render(&l);
    const QRect r = l.visualItemRect(l.item(0)).translated(l.viewport()->pos());
    EXPECT_PX(img, r.right() - 5, r.center().y(), 0x8EA29B);
}

void TstPhotonStyle::painter_state_survives_every_entry_point()
{
    // Callers set the pen once and draw several elements with it (QComboBox
    // draws CE_ComboBoxLabel with the pen it set before CC_ComboBox).
    QImage img(120, 40, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    const QPen pen(QColor(0x12, 0x34, 0x56));
    const QBrush brush(QColor(0x65, 0x43, 0x21));
    const QPalette pal = m_style->standardPalette();

    auto check = [&](const char *what) {
        QVERIFY2(p.pen() == pen, what);
        QVERIFY2(p.brush() == brush, what);
    };
    p.setPen(pen);
    p.setBrush(brush);

    QStyleOptionFocusRect focus;
    focus.rect = QRect(0, 0, 100, 20);
    focus.palette = pal;
    m_style->drawPrimitive(QStyle::PE_FrameFocusRect, &focus, &p, nullptr);
    check("PE_FrameFocusRect");

    QStyleOptionFrame frame;
    frame.rect = focus.rect;
    frame.palette = pal;
    for (const auto pe : {QStyle::PE_FrameGroupBox, QStyle::PE_FrameMenu, QStyle::PE_PanelTipLabel,
                          QStyle::PE_FrameTabWidget, QStyle::PE_IndicatorRadioButton,
                          QStyle::PE_IndicatorCheckBox, QStyle::PE_IndicatorHeaderArrow}) {
        m_style->drawPrimitive(pe, &frame, &p, nullptr);
        check("primitive");
    }

    QStyleOptionMenuItem item;
    item.rect = focus.rect;
    item.palette = pal;
    item.text = QStringLiteral("Help\tF1");
    item.state = QStyle::State_Enabled | QStyle::State_Selected;
    m_style->drawControl(QStyle::CE_MenuItem, &item, &p, nullptr);
    check("CE_MenuItem");
    m_style->drawControl(QStyle::CE_MenuBarEmptyArea, &item, &p, nullptr);
    check("CE_MenuBarEmptyArea");

    QStyleOptionHeader header;
    header.rect = focus.rect;
    header.palette = pal;
    m_style->drawControl(QStyle::CE_HeaderSection, &header, &p, nullptr);
    check("CE_HeaderSection");

    QStyleOptionComboBox combo;
    combo.rect = QRect(0, 0, 120, 26);
    combo.palette = pal;
    combo.state = QStyle::State_Enabled | QStyle::State_HasFocus;
    combo.subControls = QStyle::SC_All;
    m_style->drawComplexControl(QStyle::CC_ComboBox, &combo, &p, nullptr);
    check("CC_ComboBox");

    QStyleOptionSlider slider;
    slider.rect = QRect(0, 0, 120, 24);
    slider.palette = pal;
    slider.maximum = 100;
    slider.orientation = Qt::Horizontal;
    slider.state = QStyle::State_Enabled | QStyle::State_Horizontal | QStyle::State_HasFocus;
    slider.subControls = QStyle::SC_All;
    m_style->drawComplexControl(QStyle::CC_Slider, &slider, &p, nullptr);
    check("CC_Slider");
    slider.orientation = Qt::Vertical;
    slider.rect = QRect(0, 0, 17, 40);
    m_style->drawComplexControl(QStyle::CC_ScrollBar, &slider, &p, nullptr);
    check("CC_ScrollBar");
}

void TstPhotonStyle::menu_item_pitch_follows_font()
{
    QMenu m;
    QAction *a = m.addAction(QStringLiteral("MultiMedia"));
    QAction *b = m.addAction(QStringLiteral("Editors"));
    apply(&m);
    m.resize(m.sizeHint());
    // Launch menu: 17 rows of fill plus a 2 row gap, with a ~12 px font.
    const int pitch = m.actionGeometry(b).top() - m.actionGeometry(a).top();
    const int fontHeight = QFontMetrics(m.font()).height();
    QVERIFY2(pitch <= std::max(19, fontHeight + 5), qPrintable(QString::number(pitch)));
    QVERIFY2(pitch >= 19, qPrintable(QString::number(pitch)));
}

void TstPhotonStyle::null_widget_sweep()
{
    QImage img(80, 40, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    const QPalette pal = m_style->standardPalette();
    const QRect r(0, 0, 80, 40);

    auto prime = [&](QStyleOption &o) {
        o.rect = r;
        o.palette = pal;
        o.state = QStyle::State_Enabled | QStyle::State_Raised;
    };

    for (int pe = QStyle::PE_Frame; pe <= QStyle::PE_IndicatorTabTearRight; ++pe) {
        QStyleOption o;
        prime(o);
        m_style->drawPrimitive(QStyle::PrimitiveElement(pe), &o, &p, nullptr);
    }

    QStyleOptionButton button;
    prime(button);
    button.text = QStringLiteral("x");
    QStyleOptionTab tab;
    prime(tab);
    tab.text = QStringLiteral("x");
    QStyleOptionMenuItem menu;
    prime(menu);
    menu.text = QStringLiteral("x\tCtrl+X");
    QStyleOptionProgressBar progress;
    prime(progress);
    progress.maximum = 100;
    progress.progress = 40;
    QStyleOptionHeader header;
    prime(header);
    QStyleOptionComboBox combo;
    prime(combo);
    QStyleOptionDockWidget dock;
    prime(dock);
    QStyleOptionToolBox toolbox;
    prime(toolbox);
    QStyleOptionFrame frame;
    prime(frame);
    QStyleOptionToolBar toolbar;
    prime(toolbar);
    QStyleOptionRubberBand rubber;
    prime(rubber);

    const std::pair<QStyle::ControlElement, const QStyleOption *> controls[] = {
        {QStyle::CE_PushButton, &button},       {QStyle::CE_PushButtonBevel, &button},
        {QStyle::CE_PushButtonLabel, &button},  {QStyle::CE_CheckBox, &button},
        {QStyle::CE_RadioButton, &button},      {QStyle::CE_TabBarTab, &tab},
        {QStyle::CE_TabBarTabShape, &tab},      {QStyle::CE_TabBarTabLabel, &tab},
        {QStyle::CE_MenuItem, &menu},           {QStyle::CE_MenuBarItem, &menu},
        {QStyle::CE_MenuBarEmptyArea, &menu},   {QStyle::CE_MenuEmptyArea, &menu},
        {QStyle::CE_ProgressBar, &progress},    {QStyle::CE_ProgressBarGroove, &progress},
        {QStyle::CE_ProgressBarContents, &progress}, {QStyle::CE_ProgressBarLabel, &progress},
        {QStyle::CE_Header, &header},           {QStyle::CE_HeaderSection, &header},
        {QStyle::CE_HeaderLabel, &header},      {QStyle::CE_ComboBoxLabel, &combo},
        {QStyle::CE_DockWidgetTitle, &dock},    {QStyle::CE_ToolBoxTab, &toolbox},
        {QStyle::CE_ToolBoxTabShape, &toolbox}, {QStyle::CE_ShapedFrame, &frame},
        {QStyle::CE_ToolBar, &toolbar},         {QStyle::CE_Splitter, &button},
        {QStyle::CE_SizeGrip, &button},         {QStyle::CE_RubberBand, &rubber},
        {QStyle::CE_FocusFrame, &button},       {QStyle::CE_ScrollBarSlider, &button},
    };
    for (const auto &[ce, opt] : controls)
        m_style->drawControl(ce, opt, &p, nullptr);

    QStyleOptionSlider slider;
    prime(slider);
    slider.maximum = 100;
    slider.pageStep = 10;
    slider.subControls = QStyle::SC_All;
    QStyleOptionSpinBox spin;
    prime(spin);
    spin.subControls = QStyle::SC_All;
    combo.subControls = QStyle::SC_All;
    QStyleOptionTitleBar title;
    prime(title);
    title.subControls = QStyle::SC_All;
    title.titleBarFlags = Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint
                        | Qt::WindowMinMaxButtonsHint;
    QStyleOptionToolButton tool;
    prime(tool);
    tool.subControls = QStyle::SC_All;

    const std::pair<QStyle::ComplexControl, const QStyleOptionComplex *> complex[] = {
        {QStyle::CC_ScrollBar, &slider}, {QStyle::CC_Slider, &slider},
        {QStyle::CC_Dial, &slider},      {QStyle::CC_SpinBox, &spin},
        {QStyle::CC_ComboBox, &combo},   {QStyle::CC_TitleBar, &title},
        {QStyle::CC_ToolButton, &tool},
    };
    for (const auto &[cc, opt] : complex) {
        m_style->drawComplexControl(cc, opt, &p, nullptr);
        for (int bit = 0; bit < 16; ++bit) {
            const auto sc = QStyle::SubControl(1u << bit);
            m_style->subControlRect(cc, opt, sc, nullptr);
        }
        m_style->hitTestComplexControl(cc, opt, QPoint(5, 5), nullptr);
    }

    for (int se = QStyle::SE_PushButtonContents; se <= QStyle::SE_TabBarScrollRightButton; ++se)
        m_style->subElementRect(QStyle::SubElement(se), &button, nullptr);

    for (int ct = QStyle::CT_PushButton; ct <= QStyle::CT_ItemViewItem; ++ct)
        m_style->sizeFromContents(QStyle::ContentsType(ct), &button, QSize(40, 16), nullptr);

    for (int pm = QStyle::PM_ButtonMargin; pm <= QStyle::PM_LineEditIconMargin; ++pm)
        m_style->pixelMetric(QStyle::PixelMetric(pm), nullptr, nullptr);

    for (int sh = QStyle::SH_EtchDisabledText; sh <= QStyle::SH_Table_AlwaysDrawLeftTopGridLines; ++sh)
        m_style->styleHint(QStyle::StyleHint(sh), nullptr, nullptr, nullptr);
}

void TstPhotonStyle::showcase_renders()
{
    QWidget w;
    auto *layout = new QVBoxLayout(&w);
    layout->addWidget(new QPushButton(QStringLiteral("Done")));
    layout->addWidget(new QCheckBox(QStringLiteral("Cursor focus")));
    layout->addWidget(new QRadioButton(QStringLiteral("Right")));
    auto *combo = new QComboBox;
    combo->addItems({QStringLiteral("Custom"), QStringLiteral("Default")});
    layout->addWidget(combo);
    layout->addWidget(new QSpinBox);
    layout->addWidget(new QLineEdit(QStringLiteral("/root")));
    layout->addWidget(new QSlider(Qt::Horizontal));
    auto *progress = new QProgressBar;
    progress->setValue(30);
    layout->addWidget(progress);
    auto *tabs = new QTabWidget;
    tabs->addTab(new QListWidget, QStringLiteral("Devices"));
    tabs->addTab(new QWidget, QStringLiteral("Network"));
    layout->addWidget(tabs);
    layout->addWidget(new QTableWidget(3, 3));
    apply(&w);
    w.resize(400, 600);
    const QImage img = render(&w);
    QVERIFY(!img.isNull());
}

QTEST_MAIN(TstPhotonStyle)
#include "tst_photonstyle.moc"
