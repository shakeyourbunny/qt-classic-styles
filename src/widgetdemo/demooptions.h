// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#ifndef QTCLASSICSTYLES_WIDGETDEMO_DEMOOPTIONS_H
#define QTCLASSICSTYLES_WIDGETDEMO_DEMOOPTIONS_H

#include <QString>
#include <QStringList>

struct DemoOptions
{
    enum class Mode { Interactive, Help, Version, Screenshot, UsageError };

    Mode mode = Mode::Interactive;
    // Interactive: at most one style to preselect. Screenshot: styles to render.
    QStringList styles;
    QString outputDir;
    QString error;
};

// The five styles this project ships, in README order.
QStringList classicStyleKeys();

// args includes the program name, as QCoreApplication::arguments() does.
// Needs no QCoreApplication, so it can run before a display is opened.
DemoOptions parseDemoOptions(const QStringList &args);

QString demoHelpText();
QString demoVersionText();

#endif // QTCLASSICSTYLES_WIDGETDEMO_DEMOOPTIONS_H
