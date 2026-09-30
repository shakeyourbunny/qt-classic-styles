// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#include <memory>

#include <QStyle>
#include <QStyleFactory>
#include <QTest>

class TstPhotonPlugin : public QObject
{
    Q_OBJECT

private slots:
    void factory_lists_photon_key();
    void factory_creates_photon_style();
};

void TstPhotonPlugin::factory_lists_photon_key()
{
    const QStringList keys = QStyleFactory::keys();
    QVERIFY2(keys.contains(QStringLiteral("photon"), Qt::CaseInsensitive),
             qPrintable(keys.join(u',')));
}

void TstPhotonPlugin::factory_creates_photon_style()
{
    std::unique_ptr<QStyle> style(QStyleFactory::create(QStringLiteral("photon")));
    QVERIFY(style);
    QCOMPARE(style->metaObject()->className(), "QPhotonStyle");
}

QTEST_MAIN(TstPhotonPlugin)
#include "tst_photonplugin.moc"
