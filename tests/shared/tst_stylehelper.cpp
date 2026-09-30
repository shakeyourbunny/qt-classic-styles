// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#include "qstylehelper_p.h"

#include <climits>
#include <memory>
#include <optional>

#include <QImage>
#include <QPainter>
#include <QPixmapCache>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTabBar>
#include <QTest>

class TstStyleHelper : public QObject
{
    Q_OBJECT

private slots:
    void init() { QPixmapCache::clear(); }
    void dial_knob_is_identical_on_cache_hit();
    void dial_survives_the_full_int_range_data();
    void dial_survives_the_full_int_range();
    void dial_at_an_offset_matches_the_dial_at_the_origin();
    void tab_bar_scroll_buttons_match_qcommonstyle_with_a_widget_data();
    void tab_bar_scroll_buttons_match_qcommonstyle_with_a_widget();
    void scroll_bar_option_is_left_alone_when_valid();
    void scroll_bar_option_clamps_negative_page_step();
    void scroll_bar_option_straightens_inverted_range();
    void scroll_bar_option_keeps_the_unsigned_divisor_from_wrapping();
    void tick_interval_keeps_sparse_ticks();
    void tick_interval_grows_to_one_tick_per_pixel();
    void tick_interval_stays_a_multiple_of_the_original();
    void tick_interval_survives_the_full_int_range();
    void tick_interval_with_no_space_gives_at_most_one_tick();
    void effective_tick_interval_follows_qcommonstyle_rule();
    void cache_key_depends_on_dpr();
    void cache_dpr_reads_the_paint_device();
    void cache_pixmap_has_device_size_and_ratio();
};

namespace {

QImage paintDial(const QStyleOptionSlider &opt)
{
    QImage img(opt.rect.size(), QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    ClassicStyleHelper::drawDial(&opt, &p);
    return img;
}

} // namespace

// Regression: the knob colour was adjusted inside the cache block, so a
// cache hit painted the unadjusted colour.
void TstStyleHelper::dial_knob_is_identical_on_cache_hit()
{
    QStyleOptionSlider opt;
    opt.rect = QRect(0, 0, 80, 80);
    opt.state = QStyle::State_Enabled;
    opt.minimum = 0;
    opt.maximum = 99;
    opt.sliderPosition = 30;
    opt.sliderValue = 30;
    opt.subControls = QStyle::SC_DialGroove | QStyle::SC_DialHandle;
    // Dark and saturated, so the setHsv clamp really changes the colour.
    opt.palette.setColor(QPalette::Button, QColor(0x40, 0x60, 0xA0));

    const QImage miss = paintDial(opt);
    const QImage hit = paintDial(opt);
    QCOMPARE(hit, miss);
}

void TstStyleHelper::dial_survives_the_full_int_range_data()
{
    QTest::addColumn<int>("position");
    QTest::addColumn<bool>("upsideDown");
    QTest::addColumn<bool>("wrapping");
    for (int position : {INT_MIN, 0, INT_MAX})
        for (bool upsideDown : {false, true})
            for (bool wrapping : {false, true})
                QTest::addRow("%d-%d-%d", position, int(upsideDown), int(wrapping))
                    << position << upsideDown << wrapping;
}

// Found by tst_hostile_options under UBSan: the knob angle was computed in
// int, and range, position and their products overflow for these values.
void TstStyleHelper::dial_survives_the_full_int_range()
{
    QFETCH(int, position);
    QFETCH(bool, upsideDown);
    QFETCH(bool, wrapping);
    QStyleOptionSlider opt;
    opt.rect = QRect(0, 0, 60, 60);
    opt.state = QStyle::State_Enabled;
    opt.minimum = INT_MIN;
    opt.maximum = INT_MAX;
    opt.sliderPosition = position;
    opt.sliderValue = position;
    opt.upsideDown = upsideDown;
    opt.dialWrapping = wrapping;
    opt.tickInterval = INT_MAX / 4;
    opt.pageStep = INT_MAX;
    opt.subControls = QStyle::SC_DialGroove | QStyle::SC_DialHandle | QStyle::SC_DialTickmarks;
    const QImage img = paintDial(opt);
    QVERIFY(!img.isNull());
}

// Regression: the cached background was painted at option->rect's offset
// into a pixmap whose origin is (0,0), and the knob ignored the offset.
void TstStyleHelper::dial_at_an_offset_matches_the_dial_at_the_origin()
{
    QStyleOptionSlider opt;
    opt.rect = QRect(0, 0, 60, 60);
    opt.state = QStyle::State_Enabled;
    opt.minimum = 0;
    opt.maximum = 99;
    opt.sliderPosition = 30;
    opt.sliderValue = 30;
    opt.subControls = QStyle::SC_DialGroove | QStyle::SC_DialHandle | QStyle::SC_DialTickmarks;
    const auto render = [&](QPoint at) {
        opt.rect.moveTopLeft(at);
        QImage img(QSize(120, 120), QImage::Format_ARGB32);
        img.fill(Qt::white);
        QPainter p(&img);
        ClassicStyleHelper::drawDial(&opt, &p);
        p.end();
        return img.copy(QRect(at, QSize(60, 60)));
    };
    const QImage origin = render(QPoint(0, 0));
    QPixmapCache::clear();
    QCOMPARE(render(QPoint(50, 50)), origin);
}

void TstStyleHelper::tab_bar_scroll_buttons_match_qcommonstyle_with_a_widget_data()
{
    QTest::addColumn<int>("element");
    QTest::addColumn<int>("direction");
    for (int e : {int(QStyle::SE_TabBarScrollLeftButton), int(QStyle::SE_TabBarScrollRightButton)}) {
        QTest::addRow("%d-ltr", e) << e << int(Qt::LeftToRight);
        QTest::addRow("%d-rtl", e) << e << int(Qt::RightToLeft);
    }
}

// The helper must give what QCommonStyle gives when it has a widget.
void TstStyleHelper::tab_bar_scroll_buttons_match_qcommonstyle_with_a_widget()
{
    QFETCH(int, element);
    QFETCH(int, direction);
    const std::unique_ptr<QStyle> windows(QStyleFactory::create(QStringLiteral("Windows")));
    QVERIFY(windows);
    QTabBar bar;
    bar.setLayoutDirection(Qt::LayoutDirection(direction));
    QStyleOption opt;
    opt.rect = QRect(0, 0, 200, 30);
    opt.direction = Qt::LayoutDirection(direction);
    const auto se = QStyle::SubElement(element);
    QCOMPARE(ClassicStyleHelper::tabBarScrollButtonRect(windows.get(), se, &opt, nullptr),
             windows->subElementRect(se, &opt, &bar));
}

void TstStyleHelper::scroll_bar_option_is_left_alone_when_valid()
{
    QStyleOptionSlider opt;
    opt.minimum = 0;
    opt.maximum = 10;
    opt.pageStep = 3;
    QVERIFY(!ClassicStyleHelper::sanitizedScrollBar(QStyle::CC_ScrollBar, &opt));
    opt.pageStep = -3;
    QVERIFY(!ClassicStyleHelper::sanitizedScrollBar(QStyle::CC_Slider, &opt));
}

void TstStyleHelper::scroll_bar_option_clamps_negative_page_step()
{
    QStyleOptionSlider opt;
    opt.minimum = 0;
    opt.maximum = 10;
    opt.pageStep = -10;
    const std::optional<QStyleOptionSlider> fixed =
        ClassicStyleHelper::sanitizedScrollBar(QStyle::CC_ScrollBar, &opt);
    QVERIFY(fixed);
    QCOMPARE(fixed->pageStep, 0);
    QCOMPARE(fixed->maximum, 10);
}

void TstStyleHelper::scroll_bar_option_straightens_inverted_range()
{
    QStyleOptionSlider opt;
    opt.minimum = 10;
    opt.maximum = 0;
    opt.pageStep = 10;
    const std::optional<QStyleOptionSlider> fixed =
        ClassicStyleHelper::sanitizedScrollBar(QStyle::CC_ScrollBar, &opt);
    QVERIFY(fixed);
    QCOMPARE(fixed->maximum, 10);
    QCOMPARE(fixed->pageStep, 10);
}

// Found by tst_hostile_options: Qt sums uint range + pageStep, which wraps
// to zero for the full int range with a page step of 1.
void TstStyleHelper::scroll_bar_option_keeps_the_unsigned_divisor_from_wrapping()
{
    QStyleOptionSlider opt;
    opt.minimum = INT_MIN;
    opt.maximum = INT_MAX;
    opt.pageStep = 1;
    const std::optional<QStyleOptionSlider> fixed =
        ClassicStyleHelper::sanitizedScrollBar(QStyle::CC_ScrollBar, &opt);
    QVERIFY(fixed);
    const quint64 sum = quint64(qint64(fixed->maximum) - fixed->minimum) + quint64(fixed->pageStep);
    QVERIFY(sum <= UINT_MAX);
    QCOMPARE(fixed->minimum, INT_MIN);
    QCOMPARE(fixed->maximum, INT_MAX);
}

void TstStyleHelper::tick_interval_keeps_sparse_ticks()
{
    QCOMPARE(ClassicStyleHelper::boundedTickInterval(0, 100, 10, 200), 10);
}

void TstStyleHelper::tick_interval_grows_to_one_tick_per_pixel()
{
    const int interval = ClassicStyleHelper::boundedTickInterval(0, 5'000'000, 1, 200);
    QCOMPARE(interval, 25'000);
}

void TstStyleHelper::tick_interval_stays_a_multiple_of_the_original()
{
    const int interval = ClassicStyleHelper::boundedTickInterval(0, 1'000'000, 3, 100);
    QCOMPARE(interval % 3, 0);
    QVERIFY(1'000'000 / interval <= 100);
}

void TstStyleHelper::tick_interval_survives_the_full_int_range()
{
    const int interval = ClassicStyleHelper::boundedTickInterval(INT_MIN, INT_MAX, 1, 100);
    QVERIFY(interval > 0);
    QVERIFY((qint64(INT_MAX) - INT_MIN) / interval <= 100);
}

void TstStyleHelper::tick_interval_with_no_space_gives_at_most_one_tick()
{
    const int interval = ClassicStyleHelper::boundedTickInterval(0, 1000, 1, 0);
    QVERIFY(1000 / interval <= 1);
}

void TstStyleHelper::effective_tick_interval_follows_qcommonstyle_rule()
{
    QStyleOptionSlider opt;
    opt.minimum = 0;
    opt.maximum = 100;
    opt.singleStep = 1;
    opt.pageStep = 10;
    opt.tickInterval = 0;
    // One single step is under 3 px of 20 px travel: QCommonStyle uses the page step.
    QCOMPARE(ClassicStyleHelper::effectiveTickInterval(opt, 20, 20), 10);
    opt.tickInterval = 7;
    QCOMPARE(ClassicStyleHelper::effectiveTickInterval(opt, 1000, 1000), 7);
    // The density bound follows travel, not available (Plastique: 16 vs 280).
    opt.tickInterval = 5;
    QCOMPARE(ClassicStyleHelper::effectiveTickInterval(opt, 16, 280), 5);
}

void TstStyleHelper::cache_key_depends_on_dpr()
{
    QStyleOption opt;
    const QSize size(20, 20);
    QVERIFY(ClassicStyleHelper::uniqueName(QStringLiteral("k"), &opt, size, 1.0)
            != ClassicStyleHelper::uniqueName(QStringLiteral("k"), &opt, size, 2.0));
}

void TstStyleHelper::cache_dpr_reads_the_paint_device()
{
    QImage img(QSize(40, 40), QImage::Format_ARGB32);
    img.setDevicePixelRatio(2.0);
    QPainter p(&img);
    QCOMPARE(ClassicStyleHelper::cacheDpr(&p), 2.0);
    QCOMPARE(ClassicStyleHelper::cacheDpr(nullptr), 1.0);
}

void TstStyleHelper::cache_pixmap_has_device_size_and_ratio()
{
    const QPixmap pm = ClassicStyleHelper::styleCachePixmap(QSize(16, 10), 2.0);
    QCOMPARE(pm.size(), QSize(32, 20));
    QCOMPARE(pm.devicePixelRatio(), 2.0);
    QCOMPARE(pm.deviceIndependentSize().toSize(), QSize(16, 10));
}

QTEST_MAIN(TstStyleHelper)
#include "tst_stylehelper.moc"
