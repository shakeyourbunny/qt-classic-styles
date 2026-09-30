// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#include "qphotonstyle.h"

#include <QStylePlugin>

class QPhotonStylePlugin : public QStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QStyleFactoryInterface" FILE "photon.json")

public:
    QStyle *create(const QString &key) override;
};

QStyle *QPhotonStylePlugin::create(const QString &key)
{
    if (key.compare(QLatin1String("photon"), Qt::CaseInsensitive) == 0)
        return new QPhotonStyle;
    return nullptr;
}

#include "plugin.moc"
