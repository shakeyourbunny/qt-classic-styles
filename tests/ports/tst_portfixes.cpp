// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

// Regression tests for the ported styles, loaded as plugins.

#include <climits>
#include <memory>

#include <QElapsedTimer>
#include <QImage>
#include <QPainter>
#include <QPixmapCache>
#include <QProgressBar>
#include <QRegularExpression>
#include <QSlider>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTabBar>
#include <QTest>

#include "qstylehelper_p.h"

namespace {

std::unique_ptr<QStyle> style(const char *name)
{
    return std::unique_ptr<QStyle>(QStyleFactory::create(QString::fromLatin1(name)));
}

} // namespace

class TstPortFixes : public QObject
{
    Q_OBJECT

private slots:
    void busy_progress_bar_survives_zero_width_data();
    void busy_progress_bar_survives_zero_width();
    void busy_progress_bar_leaves_painter_state_alone();
    void busy_progress_bar_survives_a_huge_width_data();
    void busy_progress_bar_survives_a_huge_width();
    void busy_progress_bar_can_be_destroyed_data();
    void busy_progress_bar_can_be_destroyed();
    void tab_bar_scroll_buttons_without_widget_data();
    void tab_bar_scroll_buttons_without_widget();
    void scroll_bar_survives_hostile_page_step_data();
    void scroll_bar_survives_hostile_page_step();
    void dense_ticks_paint_quickly_data();
    void dense_ticks_paint_quickly();
    void ticks_survive_int_max_range_data();
    void ticks_survive_int_max_range();
    void ticks_are_drawn_at_int_max_data();
    void ticks_are_drawn_at_int_max();
    void ordinary_ticks_are_all_drawn_data();
    void ordinary_ticks_are_all_drawn();
    void cached_parts_are_cached_at_the_device_ratio();
    void cache_entries_are_per_device_ratio();
    void empty_rects_never_open_a_painter_data();
    void empty_rects_never_open_a_painter();
    void plastique_title_label_is_indented();
    void cleanlooks_grid_colour_without_option_matches_the_base_style();
};

void TstPortFixes::busy_progress_bar_survives_zero_width_data()
{
    QTest::addColumn<QString>("name");
    QTest::newRow("motif") << QStringLiteral("motif");
    QTest::newRow("cde") << QStringLiteral("cde");
}

// Regression: a busy bar narrower than its frame divided by zero (SIGFPE).
void TstPortFixes::busy_progress_bar_survives_zero_width()
{
    QFETCH(QString, name);
    const std::unique_ptr<QStyle> s(QStyleFactory::create(name));
    QVERIFY(s);
    for (int width : {8, 6, 4, 3, 2, 1}) {
        QProgressBar bar;
        bar.setStyle(s.get());
        bar.setRange(0, 0);
        bar.setTextVisible(false);
        bar.setMinimumSize(0, 0);
        bar.resize(width, 20);
        bar.grab();
    }
}

void TstPortFixes::busy_progress_bar_leaves_painter_state_alone()
{
    const auto s = style("motif");
    QVERIFY(s);
    QStyleOptionProgressBar opt;
    opt.rect = QRect(0, 0, 20, 100);
    opt.state = QStyle::State_Enabled; // no State_Horizontal: vertical
    opt.minimum = 0;
    opt.maximum = 0;
    QImage img(40, 120, QImage::Format_ARGB32);
    QPainter p(&img);
    const QPen pen(Qt::red, 1);
    p.setPen(pen);
    s->drawControl(QStyle::CE_ProgressBarContents, &opt, &p, nullptr);
    QVERIFY(p.worldTransform().isIdentity());
    QCOMPARE(p.pen(), pen);
}

void TstPortFixes::busy_progress_bar_survives_a_huge_width_data()
{
    busy_progress_bar_survives_zero_width_data();
}

// Regression: the bounce period w * 2 overflowed an int (UBSan in the ASan build).
void TstPortFixes::busy_progress_bar_survives_a_huge_width()
{
    QFETCH(QString, name);
    const std::unique_ptr<QStyle> s(QStyleFactory::create(name));
    QVERIFY(s);
    QStyleOptionProgressBar opt;
    opt.rect = QRect(0, 0, INT_MAX / 2 + 2, 20);
    opt.state = QStyle::State_Enabled | QStyle::State_Horizontal;
    opt.minimum = 0;
    opt.maximum = 0;
    QImage img(40, 40, QImage::Format_ARGB32);
    QPainter p(&img);
    s->drawControl(QStyle::CE_ProgressBarContents, &opt, &p, nullptr);
}

void TstPortFixes::busy_progress_bar_can_be_destroyed_data()
{
    QTest::addColumn<QString>("name");
    for (const char *n : {"motif", "cde", "plastique", "cleanlooks"})
        QTest::newRow(n) << QString::fromLatin1(n);
}

// Found by UBSan in the ASan tree: QEvent::Destroy comes from ~QWidget, when
// the bar is no longer a QProgressBar, and Cleanlooks static_cast it.
void TstPortFixes::busy_progress_bar_can_be_destroyed()
{
    QFETCH(QString, name);
    const std::unique_ptr<QStyle> s(QStyleFactory::create(name));
    QVERIFY(s);
    auto bar = std::make_unique<QProgressBar>();
    bar->setStyle(s.get());
    bar->setRange(0, 0);
    bar->show();
    QVERIFY(QTest::qWaitForWindowExposed(bar.get()));
    bar.reset();
}

void TstPortFixes::tab_bar_scroll_buttons_without_widget_data()
{
    QTest::addColumn<QString>("name");
    for (const char *n : {"motif", "cde", "plastique", "cleanlooks", "photon"})
        QTest::newRow(n) << QString::fromLatin1(n);
}

// Regression: SIGSEGV inside QCommonStyle with widget == nullptr. Without a
// widget the rects must be the ones the style gives with one (the two
// buttons share a column: PM_TabBar_ScrollButtonOverlap).
void TstPortFixes::tab_bar_scroll_buttons_without_widget()
{
    QFETCH(QString, name);
    const std::unique_ptr<QStyle> s(QStyleFactory::create(name));
    QVERIFY(s);
    QTabBar bar;
    QStyleOption opt;
    opt.rect = QRect(0, 0, 200, 30);
    for (auto se : {QStyle::SE_TabBarScrollLeftButton, QStyle::SE_TabBarScrollRightButton})
        QCOMPARE(s->subElementRect(se, &opt, nullptr), s->subElementRect(se, &opt, &bar));
}

void TstPortFixes::scroll_bar_survives_hostile_page_step_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("minimum");
    QTest::addColumn<int>("maximum");
    QTest::addColumn<int>("pageStep");
    for (const char *n : {"motif", "cde", "plastique", "cleanlooks", "photon"}) {
        QTest::addRow("%s-negative", n) << QString::fromLatin1(n) << 0 << 10 << -10;
        QTest::addRow("%s-inverted", n) << QString::fromLatin1(n) << 10 << 0 << 10;
        QTest::addRow("%s-full-range", n) << QString::fromLatin1(n) << INT_MIN << INT_MAX << 1;
        QTest::addRow("%s-huge-page", n) << QString::fromLatin1(n) << 0 << 10 << 1'000'000'000;
    }
}

// Regression: SIGFPE from dividing by range + pageStep.
void TstPortFixes::scroll_bar_survives_hostile_page_step()
{
    QFETCH(QString, name);
    QFETCH(int, minimum);
    QFETCH(int, maximum);
    QFETCH(int, pageStep);
    const std::unique_ptr<QStyle> s(QStyleFactory::create(name));
    QVERIFY(s);
    QStyleOptionSlider opt;
    opt.rect = QRect(0, 0, 200, 17);
    opt.orientation = Qt::Horizontal;
    opt.state = QStyle::State_Enabled | QStyle::State_Horizontal;
    opt.minimum = minimum;
    opt.maximum = maximum;
    opt.pageStep = pageStep;
    opt.subControls = QStyle::SC_All;
    for (quint32 bit = 1; bit <= 0x80; bit <<= 1)
        s->subControlRect(QStyle::CC_ScrollBar, &opt, QStyle::SubControl(bit), nullptr);
    s->hitTestComplexControl(QStyle::CC_ScrollBar, &opt, QPoint(100, 8), nullptr);
    QImage img(opt.rect.size(), QImage::Format_ARGB32);
    QPainter p(&img);
    s->drawComplexControl(QStyle::CC_ScrollBar, &opt, &p, nullptr);
}

void TstPortFixes::dense_ticks_paint_quickly_data()
{
    QTest::addColumn<QString>("name");
    for (const char *n : {"motif", "cde", "plastique", "cleanlooks", "photon"})
        QTest::newRow(n) << QString::fromLatin1(n);
}

// Regression: one loop pass per tick value, 0.6 to 1.3 s at range 5e6.
void TstPortFixes::dense_ticks_paint_quickly()
{
    QFETCH(QString, name);
    const std::unique_ptr<QStyle> s(QStyleFactory::create(name));
    QVERIFY(s);
    QStyleOptionSlider opt;
    opt.rect = QRect(0, 0, 200, 30);
    opt.orientation = Qt::Horizontal;
    opt.state = QStyle::State_Enabled | QStyle::State_Horizontal;
    opt.minimum = 0;
    opt.maximum = 5'000'000;
    opt.tickInterval = 1;
    opt.singleStep = 1;
    opt.pageStep = 1;
    opt.tickPosition = QSlider::TicksBelow;
    opt.subControls = QStyle::SC_All;
    QImage img(opt.rect.size(), QImage::Format_ARGB32);
    QPainter p(&img);
    QElapsedTimer timer;
    timer.start();
    s->drawComplexControl(QStyle::CC_Slider, &opt, &p, nullptr);
    // Before the fix: 580 to 1300 ms in the release tree.
    QVERIFY2(timer.elapsed() < 200, qPrintable(QString::number(timer.elapsed())));
}

void TstPortFixes::ticks_survive_int_max_range_data()
{
    dense_ticks_paint_quickly_data();
}

// Regression: maximum + 1 overflowed at INT_MAX (UBSan in the ASan build).
void TstPortFixes::ticks_survive_int_max_range()
{
    QFETCH(QString, name);
    const std::unique_ptr<QStyle> s(QStyleFactory::create(name));
    QVERIFY(s);
    QStyleOptionSlider opt;
    opt.rect = QRect(0, 0, 200, 30);
    opt.orientation = Qt::Horizontal;
    opt.state = QStyle::State_Enabled | QStyle::State_Horizontal;
    opt.minimum = INT_MIN;
    opt.maximum = INT_MAX;
    opt.tickInterval = 1;
    opt.tickPosition = QSlider::TicksBothSides;
    opt.subControls = QStyle::SC_All;
    QImage img(opt.rect.size(), QImage::Format_ARGB32);
    QPainter p(&img);
    s->drawComplexControl(QStyle::CC_Slider, &opt, &p, nullptr);
}

namespace {

QStyleOptionButton checkBoxOption()
{
    QStyleOptionButton opt;
    opt.rect = QRect(0, 0, 13, 13);
    opt.state = QStyle::State_Enabled | QStyle::State_On;
    return opt;
}

void paintAt(QStyle *s, const QStyleOption &opt, qreal dpr)
{
    QImage img(QSize(40, 40) * dpr, QImage::Format_ARGB32_Premultiplied);
    img.setDevicePixelRatio(dpr);
    QPainter p(&img);
    s->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &p, nullptr);
}

} // namespace

// Regression: the cache held a 1x pixmap that was scaled up on a 2x screen.
void TstPortFixes::cached_parts_are_cached_at_the_device_ratio()
{
    QPixmapCache::clear();
    const auto s = style("plastique");
    QVERIFY(s);
    const QStyleOptionButton opt = checkBoxOption();
    paintAt(s.get(), opt, 2.0);
    QPixmap cached;
    QVERIFY(QPixmapCache::find(
        ClassicStyleHelper::uniqueName(QLatin1String("checkbox"), &opt, opt.rect.size(), 2.0), &cached));
    QCOMPARE(cached.devicePixelRatio(), 2.0);
    QCOMPARE(cached.size(), QSize(26, 26));
}

void TstPortFixes::cache_entries_are_per_device_ratio()
{
    QPixmapCache::clear();
    const auto s = style("plastique");
    QVERIFY(s);
    const QStyleOptionButton opt = checkBoxOption();
    paintAt(s.get(), opt, 1.0);
    paintAt(s.get(), opt, 2.0);
    QPixmap one, two;
    QVERIFY(QPixmapCache::find(
        ClassicStyleHelper::uniqueName(QLatin1String("checkbox"), &opt, opt.rect.size(), 1.0), &one));
    QVERIFY(QPixmapCache::find(
        ClassicStyleHelper::uniqueName(QLatin1String("checkbox"), &opt, opt.rect.size(), 2.0), &two));
    QCOMPARE(one.size(), QSize(13, 13));
    QCOMPARE(two.size(), QSize(26, 26));
}

void TstPortFixes::empty_rects_never_open_a_painter_data()
{
    QTest::addColumn<QString>("name");
    QTest::newRow("plastique") << QStringLiteral("plastique");
    QTest::newRow("cleanlooks") << QStringLiteral("cleanlooks");
}

// Regression: an empty rect opened a painter on a null cache pixmap.
void TstPortFixes::empty_rects_never_open_a_painter()
{
    QFETCH(QString, name);
    QTest::failOnWarning(QRegularExpression(QStringLiteral("QPainter")));
    const std::unique_ptr<QStyle> s(QStyleFactory::create(name));
    QVERIFY(s);
    QImage img(40, 40, QImage::Format_ARGB32);
    QPainter p(&img);
    QStyleOptionButton button;
    button.state = QStyle::State_Enabled;
    for (auto pe : {QStyle::PE_IndicatorCheckBox, QStyle::PE_IndicatorRadioButton,
                    QStyle::PE_PanelButtonCommand, QStyle::PE_IndicatorToolBarHandle,
                    QStyle::PE_PanelButtonTool})
        s->drawPrimitive(pe, &button, &p, nullptr);
    QStyleOptionHeader header;
    s->drawControl(QStyle::CE_HeaderSection, &header, &p, nullptr);
    QStyleOptionMenuItem menuBarItem;
    s->drawControl(QStyle::CE_MenuBarItem, &menuBarItem, &p, nullptr);
    QStyleOptionSlider slider;
    slider.state = QStyle::State_Enabled | QStyle::State_Horizontal;
    slider.orientation = Qt::Horizontal;
    slider.maximum = 100;
    slider.pageStep = 10;
    slider.subControls = QStyle::SC_All;
    for (auto cc : {QStyle::CC_Slider, QStyle::CC_ScrollBar})
        s->drawComplexControl(cc, &slider, &p, nullptr);
    QStyleOptionSpinBox spin;
    spin.subControls = QStyle::SC_All;
    s->drawComplexControl(QStyle::CC_SpinBox, &spin, &p, nullptr);
    QStyleOptionComboBox combo;
    combo.subControls = QStyle::SC_All;
    s->drawComplexControl(QStyle::CC_ComboBox, &combo, &p, nullptr);
}

// Regression: the indent was computed and thrown away.
void TstPortFixes::plastique_title_label_is_indented()
{
    const auto s = style("plastique");
    QVERIFY(s);
    QStyleOptionTitleBar o;
    o.rect = QRect(0, 0, 200, 20);
    o.titleBarFlags = Qt::WindowTitleHint | Qt::WindowSystemMenuHint;
    o.subControls = QStyle::SC_All;
    const QRect label = s->subControlRect(QStyle::CC_TitleBar, &o, QStyle::SC_TitleBarLabel, nullptr);
    // delta = (20 - 4 - 3) + 1 = 14 for the menu button, then 3 px indent.
    QCOMPARE(label.left(), 14 + 3);
    QCOMPARE(label.right(), 199 - 14 - 3);
}

// Regression: fell through into SH_ComboBox_Popup and returned 0.
void TstPortFixes::cleanlooks_grid_colour_without_option_matches_the_base_style()
{
    const auto s = style("cleanlooks");
    const std::unique_ptr<QStyle> base(QStyleFactory::create(QStringLiteral("Windows")));
    QVERIFY(s);
    QVERIFY(base);
    QCOMPARE(s->styleHint(QStyle::SH_Table_GridLineColor, nullptr),
             base->styleHint(QStyle::SH_Table_GridLineColor, nullptr));
}

void TstPortFixes::ticks_are_drawn_at_int_max_data()
{
    dense_ticks_paint_quickly_data();
}

// Regression: Motif handed ticks to QCommonStyle, whose loop condition
// maximum + 1 overflows at INT_MAX and draws no tick at all.
void TstPortFixes::ticks_are_drawn_at_int_max()
{
    QFETCH(QString, name);
    const std::unique_ptr<QStyle> s(QStyleFactory::create(name));
    QVERIFY(s);
    QStyleOptionSlider opt;
    opt.rect = QRect(0, 0, 200, 30);
    opt.orientation = Qt::Horizontal;
    opt.state = QStyle::State_Enabled | QStyle::State_Horizontal;
    opt.minimum = 0;
    opt.maximum = INT_MAX;
    opt.tickInterval = INT_MAX / 8;
    opt.tickPosition = QSlider::TicksBelow;
    opt.subControls = QStyle::SC_SliderTickmarks;
    QImage img(opt.rect.size(), QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    s->drawComplexControl(QStyle::CC_Slider, &opt, &p, nullptr);
    p.end();
    int inked = 0;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x)
            inked += img.pixel(x, y) != qRgb(255, 255, 255);
    QVERIFY(inked > 0);
}

void TstPortFixes::ordinary_ticks_are_all_drawn_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("interval");
    QTest::addColumn<int>("columns");
    for (const char *n : {"motif", "cde", "plastique", "cleanlooks", "photon"}) {
        QTest::addRow("%s-1", n) << QString::fromLatin1(n) << 1 << 101;
        QTest::addRow("%s-5", n) << QString::fromLatin1(n) << 5 << 21;
    }
}

// Regression: Plastique bounded the tick count by PM_SliderSpaceAvailable,
// which it reports as the slider thickness, and thinned ordinary ticks.
void TstPortFixes::ordinary_ticks_are_all_drawn()
{
    QFETCH(QString, name);
    QFETCH(int, interval);
    QFETCH(int, columns);
    const std::unique_ptr<QStyle> s(QStyleFactory::create(name));
    QVERIFY(s);
    QStyleOptionSlider opt;
    opt.rect = QRect(0, 0, 300, 30);
    opt.orientation = Qt::Horizontal;
    opt.state = QStyle::State_Enabled | QStyle::State_Horizontal;
    opt.minimum = 0;
    opt.maximum = 100;
    opt.tickInterval = interval;
    opt.tickPosition = QSlider::TicksBelow;
    opt.subControls = QStyle::SC_SliderTickmarks;
    QImage img(opt.rect.size(), QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    s->drawComplexControl(QStyle::CC_Slider, &opt, &p, nullptr);
    p.end();
    int inkedColumns = 0;
    for (int x = 0; x < img.width(); ++x) {
        bool inked = false;
        for (int y = 0; y < img.height() && !inked; ++y)
            inked = img.pixel(x, y) != qRgb(255, 255, 255);
        inkedColumns += inked;
    }
    QCOMPARE(inkedColumns, columns);
}

QTEST_MAIN(TstPortFixes)
#include "tst_portfixes.moc"
