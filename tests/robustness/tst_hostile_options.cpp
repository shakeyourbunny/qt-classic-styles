// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

// Every style entry point with mismatched options, degenerate rects, extreme
// slider values and no widget, as the QML bridge or third-party code may send.
// Passing means no crash, no sanitizer report and no runaway loop.

#include <climits>
#include <functional>
#include <memory>
#include <vector>

#include <QElapsedTimer>
#include <QImage>
#include <QPainter>
#include <QStyle>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTest>

namespace {

// Past the last value each Qt 6.8 enum defines, so new values are covered too.
constexpr int kPrimitiveCount = 60;
constexpr int kControlCount = 50;
constexpr int kComplexCount = 12;
constexpr int kSubElementCount = 60;
constexpr int kPixelMetricCount = 110;
constexpr int kStyleHintCount = 130;
constexpr int kContentsCount = 30;

// A single call slower than this is treated as a runaway loop.
constexpr qint64 kCallBudgetMs = 2000;

const QList<QRect> &hostileRects()
{
    static const QList<QRect> rects{QRect(), QRect(0, 0, -8, -8), QRect(0, 0, 1, 1),
                                    QRect(3, 3, 40, 22)};
    return rects;
}

// shared_ptr keeps the concrete deleter: QStyleOption has no virtual destructor.
std::vector<std::shared_ptr<QStyleOption>> plainOptions()
{
    std::vector<std::shared_ptr<QStyleOption>> v;
    v.push_back(std::make_shared<QStyleOption>());
    v.push_back(std::make_shared<QStyleOptionButton>());
    v.push_back(std::make_shared<QStyleOptionFrame>());
    v.push_back(std::make_shared<QStyleOptionTabWidgetFrame>());
    v.push_back(std::make_shared<QStyleOptionTabBarBase>());
    v.push_back(std::make_shared<QStyleOptionTab>());
    v.push_back(std::make_shared<QStyleOptionToolBar>());
    v.push_back(std::make_shared<QStyleOptionProgressBar>());
    v.push_back(std::make_shared<QStyleOptionMenuItem>());
    v.push_back(std::make_shared<QStyleOptionDockWidget>());
    v.push_back(std::make_shared<QStyleOptionViewItem>());
    v.push_back(std::make_shared<QStyleOptionToolBox>());
    v.push_back(std::make_shared<QStyleOptionRubberBand>());
    v.push_back(std::make_shared<QStyleOptionHeaderV2>());
    v.push_back(std::make_shared<QStyleOptionFocusRect>());
    return v;
}

// Sliders get the values no QAbstractSlider would hold.
std::vector<std::shared_ptr<QStyleOptionComplex>> complexOptions()
{
    std::vector<std::shared_ptr<QStyleOptionComplex>> v;
    v.push_back(std::make_shared<QStyleOptionComplex>());
    v.push_back(std::make_shared<QStyleOptionSpinBox>());
    v.push_back(std::make_shared<QStyleOptionToolButton>());
    v.push_back(std::make_shared<QStyleOptionComboBox>());
    v.push_back(std::make_shared<QStyleOptionTitleBar>());
    v.push_back(std::make_shared<QStyleOptionGroupBox>());
    v.push_back(std::make_shared<QStyleOptionSizeGrip>());
    struct SliderValues
    {
        int minimum, maximum, position, pageStep;
    };
    for (const SliderValues &s : {SliderValues{0, 0, 0, 0}, SliderValues{10, 0, 5, 10},
                                  SliderValues{0, 10, 5, -10}, SliderValues{0, 10, 99, -3},
                                  SliderValues{INT_MIN, INT_MAX, 0, INT_MAX},
                                  SliderValues{INT_MIN, INT_MAX, INT_MAX, 1}}) {
        for (const Qt::Orientation o : {Qt::Horizontal, Qt::Vertical}) {
            auto slider = std::make_shared<QStyleOptionSlider>();
            slider->minimum = s.minimum;
            slider->maximum = s.maximum;
            slider->sliderPosition = s.position;
            slider->sliderValue = s.position;
            slider->pageStep = s.pageStep;
            slider->singleStep = 1;
            slider->orientation = o;
            slider->tickPosition = QSlider::NoTicks;
            v.push_back(std::move(slider));
        }
    }
    return v;
}

} // namespace

class TstHostileOptions : public QObject
{
    Q_OBJECT

private slots:
    void every_entry_point_survives_hostile_options_data();
    void every_entry_point_survives_hostile_options();

private:
    void timed(const char *what, int element, const std::function<void()> &call);
    QString m_style;
};

void TstHostileOptions::timed(const char *what, int element, const std::function<void()> &call)
{
    QElapsedTimer timer;
    timer.start();
    call();
    const qint64 ms = timer.elapsed();
    QVERIFY2(ms < kCallBudgetMs,
             qPrintable(QStringLiteral("%1 %2 %3 took %4 ms").arg(m_style, QLatin1String(what)).arg(element).arg(ms)));
}

void TstHostileOptions::every_entry_point_survives_hostile_options_data()
{
    QTest::addColumn<QString>("style");
    QTest::addColumn<qreal>("dpr");
    for (const char *name : {"motif", "cde", "plastique", "cleanlooks", "photon"}) {
        QTest::addRow("%s@1", name) << QString::fromLatin1(name) << qreal(1.0);
        QTest::addRow("%s@2", name) << QString::fromLatin1(name) << qreal(2.0);
    }
}

void TstHostileOptions::every_entry_point_survives_hostile_options()
{
    QFETCH(QString, style);
    QFETCH(qreal, dpr);
    m_style = style;
    const std::unique_ptr<QStyle> s(QStyleFactory::create(style));
    QVERIFY2(s, qPrintable(style));

    QImage image(QSize(96, 96) * dpr, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(dpr);
    image.fill(Qt::transparent);
    QPainter p(&image);

    for (const QRect &rect : hostileRects()) {
        for (const auto &opt : plainOptions()) {
            opt->rect = rect;
            opt->state = QStyle::State_Enabled | QStyle::State_HasFocus | QStyle::State_Sunken;
            for (int pe = 0; pe < kPrimitiveCount; ++pe)
                timed("drawPrimitive", pe, [&] {
                    s->drawPrimitive(QStyle::PrimitiveElement(pe), opt.get(), &p, nullptr);
                });
            for (int ce = 0; ce < kControlCount; ++ce)
                timed("drawControl", ce, [&] {
                    s->drawControl(QStyle::ControlElement(ce), opt.get(), &p, nullptr);
                });
            for (int se = 0; se < kSubElementCount; ++se)
                timed("subElementRect", se, [&] {
                    s->subElementRect(QStyle::SubElement(se), opt.get(), nullptr);
                });
            for (int ct = 0; ct < kContentsCount; ++ct)
                timed("sizeFromContents", ct, [&] {
                    s->sizeFromContents(QStyle::ContentsType(ct), opt.get(), QSize(-5, -5), nullptr);
                });
            for (int pm = 0; pm < kPixelMetricCount; ++pm)
                s->pixelMetric(QStyle::PixelMetric(pm), opt.get(), nullptr);
            for (int sh = 0; sh < kStyleHintCount; ++sh)
                s->styleHint(QStyle::StyleHint(sh), opt.get(), nullptr, nullptr);
        }
        for (const auto &opt : complexOptions()) {
            opt->rect = rect;
            opt->state = QStyle::State_Enabled | QStyle::State_Sunken;
            opt->subControls = QStyle::SC_All;
            opt->activeSubControls = QStyle::SC_All;
            for (int cc = 0; cc < kComplexCount; ++cc) {
                const auto control = QStyle::ComplexControl(cc);
                timed("drawComplexControl", cc, [&] { s->drawComplexControl(control, opt.get(), &p, nullptr); });
                for (quint32 bit = 1; bit != 0 && bit <= 0x10000; bit <<= 1) {
                    const auto sc = QStyle::SubControl(bit);
                    timed("subControlRect", cc, [&] { s->subControlRect(control, opt.get(), sc, nullptr); });
                    timed("hitTestComplexControl", cc, [&] {
                        s->hitTestComplexControl(control, opt.get(), QPoint(10, 10), nullptr);
                    });
                }
            }
        }
    }
    for (int pm = 0; pm < kPixelMetricCount; ++pm)
        s->pixelMetric(QStyle::PixelMetric(pm), nullptr, nullptr);
    for (int sh = 0; sh < kStyleHintCount; ++sh)
        s->styleHint(QStyle::StyleHint(sh), nullptr, nullptr, nullptr);
}

QTEST_MAIN(TstHostileOptions)
#include "tst_hostile_options.moc"
