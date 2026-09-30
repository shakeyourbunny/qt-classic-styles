// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

// Test-only: listed in QStyleFactory::keys() but create() fails, as a
// plugin with a missing dependency would.

#include <QStylePlugin>

class BrokenStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "brokenstyle.json")

public:
    QStyle *create(const QString &) override { return nullptr; }
};

#include "brokenstyleplugin.moc"
