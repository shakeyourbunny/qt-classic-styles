/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Qt 6 port modifications for the qt-classic-styles Project.
// Copyright (C) 2026 qt-classic-styles project. Contact: qt-classic-styles-project@trinity2k.net
// License unchanged from the Qt original above: LGPL-2.1 (Digia Qt LGPL Exception).

#ifndef QMOTIFSTYLE_H
#define QMOTIFSTYLE_H

#include <QtWidgets/qcommonstyle.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qpointer.h>


QT_BEGIN_NAMESPACE

class QPalette;
class QFocusFrame;
class QProgressBar;

class QMotifStyle : public QCommonStyle
{
    Q_OBJECT

public:
    explicit QMotifStyle(bool useHighlightCols=false);
    virtual ~QMotifStyle();

    void setUseHighlightColors(bool);
    bool useHighlightColors() const;

    void polish(QPalette&) override;
    void polish(QWidget*) override;
    void unpolish(QWidget*) override;
    void polish(QApplication*) override;
    void unpolish(QApplication*) override;

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                        const QWidget *w = nullptr) const override;

    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                      const QWidget *w = nullptr) const override;

    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = nullptr) const override;

    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                         SubControl sc, const QWidget *widget = nullptr) const override;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr,
                     const QWidget *widget = nullptr) const override;

    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                           const QSize &contentsSize, const QWidget *widget = nullptr) const override;

    QRect subElementRect(SubElement r, const QStyleOption *opt,const QWidget *widget = nullptr) const override;

    int styleHint(StyleHint hint, const QStyleOption *opt = nullptr, const QWidget *widget = nullptr,
                  QStyleHintReturn *returnData = nullptr) const override;

    bool event(QEvent *) override;
    QPalette standardPalette() const override;
    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *opt = nullptr,
                       const QWidget *widget = nullptr) const override;

protected:
    QPointer<QFocusFrame> focus;
    void timerEvent(QTimerEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    void startProgressAnimation(QProgressBar *bar);
    void stopProgressAnimation(QProgressBar *bar);

private:
    bool highlightCols;
    QList<QProgressBar *> bars;
    int animationFps;
    int animateTimer;
    QElapsedTimer startTime;
    int animateStep;

protected:
    int spinboxHCoeff;
};

QT_END_NAMESPACE


#endif // QMOTIFSTYLE_H
