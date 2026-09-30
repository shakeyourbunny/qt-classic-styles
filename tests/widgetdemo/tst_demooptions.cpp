// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#include "demooptions.h"

#include <qtclassicstyles/version.h>

#include <QRegularExpression>
#include <QTest>

using Mode = DemoOptions::Mode;

class TstDemoOptions : public QObject
{
    Q_OBJECT

private slots:
    void no_arguments_starts_interactive();
    void positional_style_is_preselected();
    void two_positionals_without_screenshot_are_a_usage_error();
    void version_flag_selects_version_mode();
    void help_flag_selects_help_mode();
    void screenshot_takes_directory_and_styles();
    void screenshot_without_styles_means_all_styles();
    void screenshot_without_directory_is_a_usage_error();
    void screenshot_with_empty_directory_is_a_usage_error();
    void unknown_option_is_a_usage_error();
    void qt_application_options_are_left_for_qapplication();
    void version_is_semver();
    void part_number_is_the_dated_house_number();
    void version_text_names_version_and_part_number();
    void help_text_mentions_every_option();
};

static DemoOptions parse(const QStringList &args)
{
    return parseDemoOptions(QStringList{QStringLiteral("widgetdemo")} + args);
}

void TstDemoOptions::no_arguments_starts_interactive()
{
    const DemoOptions o = parse({});
    QCOMPARE(o.mode, Mode::Interactive);
    QVERIFY(o.styles.isEmpty());
    QVERIFY(o.error.isEmpty());
}

void TstDemoOptions::positional_style_is_preselected()
{
    const DemoOptions o = parse({QStringLiteral("photon")});
    QCOMPARE(o.mode, Mode::Interactive);
    QCOMPARE(o.styles, QStringList{QStringLiteral("photon")});
}

void TstDemoOptions::two_positionals_without_screenshot_are_a_usage_error()
{
    const DemoOptions o = parse({QStringLiteral("photon"), QStringLiteral("motif")});
    QCOMPARE(o.mode, Mode::UsageError);
    QVERIFY(!o.error.isEmpty());
}

void TstDemoOptions::version_flag_selects_version_mode()
{
    QCOMPARE(parse({QStringLiteral("--version")}).mode, Mode::Version);
    QCOMPARE(parse({QStringLiteral("-v")}).mode, Mode::Version);
}

void TstDemoOptions::help_flag_selects_help_mode()
{
    QCOMPARE(parse({QStringLiteral("--help")}).mode, Mode::Help);
    QCOMPARE(parse({QStringLiteral("-h")}).mode, Mode::Help);
}

void TstDemoOptions::screenshot_takes_directory_and_styles()
{
    const DemoOptions o = parse({QStringLiteral("--screenshot"), QStringLiteral("shots"),
                                 QStringLiteral("cde"), QStringLiteral("photon")});
    QCOMPARE(o.mode, Mode::Screenshot);
    QCOMPARE(o.outputDir, QStringLiteral("shots"));
    QCOMPARE(o.styles, (QStringList{QStringLiteral("cde"), QStringLiteral("photon")}));
}

void TstDemoOptions::screenshot_without_styles_means_all_styles()
{
    const DemoOptions o = parse({QStringLiteral("--screenshot"), QStringLiteral("shots")});
    QCOMPARE(o.mode, Mode::Screenshot);
    QCOMPARE(o.styles, classicStyleKeys());
}

void TstDemoOptions::screenshot_without_directory_is_a_usage_error()
{
    const DemoOptions o = parse({QStringLiteral("--screenshot")});
    QCOMPARE(o.mode, Mode::UsageError);
    QVERIFY(!o.error.isEmpty());
}

void TstDemoOptions::screenshot_with_empty_directory_is_a_usage_error()
{
    const DemoOptions o = parse({QStringLiteral("--screenshot"), QString()});
    QCOMPARE(o.mode, Mode::UsageError);
}

void TstDemoOptions::unknown_option_is_a_usage_error()
{
    const DemoOptions o = parse({QStringLiteral("--frobnicate")});
    QCOMPARE(o.mode, Mode::UsageError);
    QVERIFY(o.error.contains(QStringLiteral("frobnicate")));
}

void TstDemoOptions::version_is_semver()
{
    static const QRegularExpression semver(QStringLiteral(R"(^\d+\.\d+\.\d+$)"));
    const QString v = QString::fromLatin1(qtclassicstyles::version.data(),
                                          qsizetype(qtclassicstyles::version.size()));
    QVERIFY2(semver.match(v).hasMatch(), qPrintable(v));
}

void TstDemoOptions::part_number_is_the_dated_house_number()
{
    static const QRegularExpression dated(QStringLiteral(R"(^SJ-PKG-0010-(\d{4})(\d{2})(\d{2})$)"));
    const QString pn = QString::fromLatin1(qtclassicstyles::part_number.data(),
                                           qsizetype(qtclassicstyles::part_number.size()));
    const QRegularExpressionMatch m = dated.match(pn);
    QVERIFY2(m.hasMatch(), qPrintable(pn));
    const QDate date(m.captured(1).toInt(), m.captured(2).toInt(), m.captured(3).toInt());
    QVERIFY2(date.isValid(), qPrintable(pn));
}

void TstDemoOptions::version_text_names_version_and_part_number()
{
    const QString text = demoVersionText();
    QVERIFY(text.startsWith(QStringLiteral("qt-classic-styles widgetdemo ")));
    QVERIFY(text.contains(QString::fromLatin1(qtclassicstyles::version.data(),
                                              qsizetype(qtclassicstyles::version.size()))));
    QVERIFY(text.contains(QString::fromLatin1(qtclassicstyles::part_number.data(),
                                              qsizetype(qtclassicstyles::part_number.size()))));
}

void TstDemoOptions::help_text_mentions_every_option()
{
    const QString text = demoHelpText();
    for (const char *opt : {"--help", "--version", "--screenshot"})
        QVERIFY2(text.contains(QLatin1String(opt)), opt);
}

// QApplication consumes its own options (-reverse, -style motif, ...), but
// the demo parses argv before it exists and must not reject them.
void TstDemoOptions::qt_application_options_are_left_for_qapplication()
{
    DemoOptions o = parse({QStringLiteral("-reverse"), QStringLiteral("--screenshot"),
                           QStringLiteral("shots"), QStringLiteral("photon")});
    QCOMPARE(o.mode, Mode::Screenshot);
    QCOMPARE(o.styles, QStringList{QStringLiteral("photon")});

    o = parse({QStringLiteral("-style"), QStringLiteral("motif"), QStringLiteral("-platform"),
               QStringLiteral("offscreen"), QStringLiteral("-widgetcount"), QStringLiteral("cde")});
    QCOMPARE(o.mode, Mode::Interactive);
    QCOMPARE(o.styles, QStringList{QStringLiteral("cde")});

    o = parse({QStringLiteral("-qmljsdebugger=port:1234"), QStringLiteral("-stylesheet=a.qss")});
    QCOMPARE(o.mode, Mode::Interactive);
    QVERIFY(o.styles.isEmpty());

    QCOMPARE(parse({QStringLiteral("-frobnicate")}).mode, Mode::UsageError);
}

QTEST_APPLESS_MAIN(TstDemoOptions)
#include "tst_demooptions.moc"
