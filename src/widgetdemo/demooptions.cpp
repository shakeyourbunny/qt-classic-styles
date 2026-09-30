// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#include "demooptions.h"

#include <qtclassicstyles/version.h>

#include <QCommandLineOption>
#include <QCommandLineParser>

namespace {

QString fromView(std::string_view v)
{
    return QString::fromLatin1(v.data(), qsizetype(v.size()));
}

DemoOptions usageError(const QString &message)
{
    DemoOptions o;
    o.mode = DemoOptions::Mode::UsageError;
    o.error = message;
    return o;
}

} // namespace

QStringList classicStyleKeys()
{
    return {QStringLiteral("motif"), QStringLiteral("cde"), QStringLiteral("plastique"),
            QStringLiteral("cleanlooks"), QStringLiteral("photon")};
}

DemoOptions parseDemoOptions(const QStringList &args)
{
    QCommandLineParser parser;
    const QCommandLineOption help({QStringLiteral("h"), QStringLiteral("help")}, QString());
    const QCommandLineOption version({QStringLiteral("v"), QStringLiteral("version")}, QString());
    const QCommandLineOption screenshot(QStringLiteral("screenshot"), QString(),
                                        QStringLiteral("dir"));
    parser.addOption(help);
    parser.addOption(version);
    parser.addOption(screenshot);

    // QApplication's own options, as listed in the Qt 6.8 QGuiApplication and
    // QApplication constructor docs. Accepted and ignored here; QApplication
    // takes them out of argv itself.
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    for (const char *name : {"platform", "platformpluginpath", "platformtheme", "plugin",
                             "qmljsdebugger", "qwindowgeometry", "qwindowicon", "qwindowtitle",
                             "session", "display", "geometry", "style", "stylesheet"}) {
        QCommandLineOption option(QString::fromLatin1(name), QString(), QStringLiteral("value"));
        option.setFlags(QCommandLineOption::HiddenFromHelp);
        parser.addOption(option);
    }
    for (const char *name : {"reverse", "widgetcount"}) {
        QCommandLineOption option(QString::fromLatin1(name));
        option.setFlags(QCommandLineOption::HiddenFromHelp);
        parser.addOption(option);
    }

    if (!parser.parse(args))
        return usageError(parser.errorText());

    DemoOptions o;
    if (parser.isSet(help)) {
        o.mode = DemoOptions::Mode::Help;
        return o;
    }
    if (parser.isSet(version)) {
        o.mode = DemoOptions::Mode::Version;
        return o;
    }

    o.styles = parser.positionalArguments();
    if (parser.isSet(screenshot)) {
        o.mode = DemoOptions::Mode::Screenshot;
        o.outputDir = parser.value(screenshot);
        if (o.outputDir.isEmpty())
            return usageError(QStringLiteral("--screenshot needs an output directory."));
        if (o.styles.isEmpty())
            o.styles = classicStyleKeys();
        return o;
    }

    if (o.styles.size() > 1)
        return usageError(QStringLiteral("Give at most one style to start with."));
    return o;
}

QString demoHelpText()
{
    return QStringLiteral(
        "Usage: widgetdemo [style]\n"
        "       widgetdemo --screenshot <dir> [style ...]\n"
        "\n"
        "Shows a gallery of widgets in the classic styles. Point QT_PLUGIN_PATH at\n"
        "the build's plugins directory to see the styles from this tree.\n"
        "\n"
        "  style                  Style to start with, for example photon.\n"
        "  --screenshot <dir>     Render the gallery in each style (default: all five)\n"
        "                         with the style's own palette and write <dir>/<style>.png.\n"
        "                         Runs without a display.\n"
        "  -h, --help             Show this help and exit.\n"
        "  -v, --version          Show the version and part number and exit.\n");
}

QString demoVersionText()
{
    return QStringLiteral("qt-classic-styles widgetdemo %1 (%2)\n")
        .arg(fromView(qtclassicstyles::version), fromView(qtclassicstyles::part_number));
}
