// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#ifndef QPHOTONSTYLE_H
#define QPHOTONSTYLE_H

#include <QCommonStyle>

// QNX 6.2.1 Photon microGUI look, measured from reference/qnx-photon/.
//
// Every entry point accepts a null widget; the QML desktop bridge passes one.
class QPhotonStyle : public QCommonStyle
{
    Q_OBJECT

public:
    QPhotonStyle();
    ~QPhotonStyle() override;

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                       const QWidget *widget = nullptr) const override;
    void drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                     const QWidget *widget = nullptr) const override;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *widget = nullptr) const override;

    int pixelMetric(PixelMetric pm, const QStyleOption *opt = nullptr,
                    const QWidget *widget = nullptr) const override;
    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &contentsSize,
                           const QWidget *widget = nullptr) const override;
    QRect subElementRect(SubElement se, const QStyleOption *opt,
                         const QWidget *widget = nullptr) const override;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                         const QWidget *widget = nullptr) const override;
    int styleHint(StyleHint sh, const QStyleOption *opt = nullptr, const QWidget *widget = nullptr,
                  QStyleHintReturn *returnData = nullptr) const override;

    QPalette standardPalette() const override;
    void polish(QWidget *widget) override;
    void unpolish(QWidget *widget) override;
    using QCommonStyle::polish;
    using QCommonStyle::unpolish;
};

#endif // QPHOTONSTYLE_H
