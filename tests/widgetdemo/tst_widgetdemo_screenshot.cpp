// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

// Runs the real widgetdemo binary, so the CLI contract and the images are
// tested together. WIDGETDEMO_PATH comes from CMake.

#include <memory>

#include <QFile>
#include <QImage>
#include <QProcess>
#include <QStyle>
#include <QStyleFactory>
#include <QTemporaryDir>
#include <QTest>

class TstWidgetDemoScreenshot : public QObject
{
    Q_OBJECT

private slots:
    void version_exits_zero_without_a_display();
    void unknown_option_exits_two();
    void unknown_style_exits_one();
    void each_image_uses_its_own_styles_palette();
    void qt_options_reach_qapplication();
    void style_that_fails_to_load_exits_one();
    void interactive_style_that_fails_to_load_exits_one();

private:
    static int run(const QStringList &args, QByteArray *out = nullptr, bool offscreen = false,
                   QByteArray *err = nullptr);
};

int TstWidgetDemoScreenshot::run(const QStringList &args, QByteArray *out, bool offscreen,
                                 QByteArray *err)
{
    QProcess p;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove(QStringLiteral("DISPLAY"));
    env.remove(QStringLiteral("WAYLAND_DISPLAY"));
    env.remove(QStringLiteral("QT_QPA_PLATFORM"));
    if (offscreen)
        env.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
    p.setProcessEnvironment(env);
    p.start(QStringLiteral(WIDGETDEMO_PATH), args);
    if (!p.waitForFinished(60000)) {
        p.kill();
        p.waitForFinished();
        return -1;
    }
    if (out)
        *out = p.readAllStandardOutput();
    if (err)
        *err = p.readAllStandardError();
    return p.exitStatus() == QProcess::NormalExit ? p.exitCode() : -1;
}

void TstWidgetDemoScreenshot::version_exits_zero_without_a_display()
{
    QByteArray out;
    QCOMPARE(run({QStringLiteral("--version")}, &out), 0);
    QVERIFY(out.startsWith("qt-classic-styles widgetdemo "));
}

void TstWidgetDemoScreenshot::unknown_option_exits_two()
{
    QCOMPARE(run({QStringLiteral("--frobnicate")}), 2);
}

void TstWidgetDemoScreenshot::unknown_style_exits_one()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QCOMPARE(run({QStringLiteral("--screenshot"), dir.path(), QStringLiteral("nosuchstyle")}), 1);
}

// Regression: the window used to keep the previous style's palette, and a
// style at combo index 0 (cde) was never applied at all.
void TstWidgetDemoScreenshot::each_image_uses_its_own_styles_palette()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QStringList styles{QStringLiteral("motif"), QStringLiteral("cde"),
                             QStringLiteral("plastique"), QStringLiteral("cleanlooks"),
                             QStringLiteral("photon")};
    QCOMPARE(run(QStringList{QStringLiteral("--screenshot"), dir.path()} + styles), 0);

    for (const QString &name : styles) {
        const std::unique_ptr<QStyle> style(QStyleFactory::create(name));
        QVERIFY2(style, qPrintable(name));
        const QImage image(dir.filePath(name + QStringLiteral(".png")));
        QVERIFY2(!image.isNull(), qPrintable(name));
        const QRgb expected = style->standardPalette().window().color().rgb();
        QVERIFY2(image.pixel(2, 2) == expected,
                 qPrintable(QStringLiteral("%1: %2 != %3")
                                .arg(name, QColor(image.pixel(2, 2)).name(),
                                     QColor(expected).name())));
    }
}

// -reverse is QApplication's; the gallery must come out mirrored, not rejected.
void TstWidgetDemoScreenshot::qt_options_reach_qapplication()
{
    QTemporaryDir ltr;
    QTemporaryDir rtl;
    QVERIFY(ltr.isValid() && rtl.isValid());
    QCOMPARE(run({QStringLiteral("--screenshot"), ltr.path(), QStringLiteral("motif")}), 0);
    QCOMPARE(run({QStringLiteral("-reverse"), QStringLiteral("--screenshot"), rtl.path(),
                  QStringLiteral("motif")}), 0);
    const QImage a(ltr.filePath(QStringLiteral("motif.png")));
    const QImage b(rtl.filePath(QStringLiteral("motif.png")));
    QVERIFY(!a.isNull() && !b.isNull());
    QVERIFY(a != b);
}

// Regression: a listed style whose plugin could not create it was rendered
// with the previous style under the requested name, exit 0.
void TstWidgetDemoScreenshot::style_that_fails_to_load_exits_one()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QByteArray err;
    QCOMPARE(run({QStringLiteral("--screenshot"), dir.path(), QStringLiteral("motif"),
                  QStringLiteral("brokenstyle")}, nullptr, false, &err), 1);
    QVERIFY2(err.contains("cannot create style"), err.constData());
    QVERIFY(!QFile::exists(dir.filePath(QStringLiteral("brokenstyle.png"))));
}

// Regression: interactive mode opened the gallery in the default style instead.
void TstWidgetDemoScreenshot::interactive_style_that_fails_to_load_exits_one()
{
    QByteArray err;
    QCOMPARE(run({QStringLiteral("brokenstyle")}, nullptr, true, &err), 1);
    QVERIFY2(err.contains("cannot create style"), err.constData());
}

QTEST_MAIN(TstWidgetDemoScreenshot)
#include "tst_widgetdemo_screenshot.moc"
