/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// Qt 6 port modifications for the qt-classic-styles Project.
// Copyright (C) 2026 qt-classic-styles project. Contact: qt-classic-styles-project@trinity2k.net
// License unchanged from the Qt original above: LGPL-2.1 (Digia Qt LGPL Exception).

#include <QtCore/qglobal.h>
#include <QtCore/qpoint.h>
#include <QtCore/qstring.h>
#include <QtGui/qpolygon.h>
#include <QtCore/qstringbuilder.h>
#include <QtGui/qaccessible.h>
#include <QtWidgets/qstyle.h>
#include <QtWidgets/qstyleoption.h>

#include <optional>

#ifndef QSTYLEHELPER_P_H
#define QSTYLEHELPER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qhexstring_p.h"

QT_BEGIN_NAMESPACE

class QPainter;
class QPixmap;
class QStyleOptionSlider;
class QStyleOption;
class QWindow;

// Our own namespace: QtWidgets exports a private QStyleHelper with the same
// function names, and a plugin's calls bound to those instead of these.
namespace ClassicStyleHelper
{
    // The key includes the device pixel ratio: a pixmap cached for one
    // screen must not be reused on a screen with another ratio.
    QString uniqueName(const QString &key, const QStyleOption *option, const QSize &size, qreal dpr);

    // Device pixel ratio of the painter's device; 1 when there is none.
    qreal cacheDpr(const QPainter *painter);

    // A transparent cache pixmap of logical size `size` at ratio `dpr`.
    // Null for an empty size, so callers can skip painting into it.
    QPixmap styleCachePixmap(const QSize &size, qreal dpr);
#ifndef QT_NO_DIAL
    qreal angle(const QPointF &p1, const QPointF &p2);
    QPolygonF calcLines(const QStyleOptionSlider *dial);
    int calcBigLineSize(int radius);
    void drawDial(const QStyleOptionSlider *dial, QPainter *painter);
#endif //QT_NO_DIAL
    void drawBorderPixmap(const QPixmap &pixmap, QPainter *painter, const QRect &rect,
                     int left = 0, int top = 0, int right = 0,
                     int bottom = 0);
#ifndef QT_NO_ACCESSIBILITY
    bool isInstanceOf(QObject *obj, QAccessible::Role role);
    bool hasAncestor(QObject *obj, QAccessible::Role role);
#endif
    QColor backgroundColor(const QPalette &pal, const QWidget* widget = nullptr);
    QWindow *styleObjectWindow(QObject *so);

    // SE_TabBarScrollLeftButton / RightButton from the option alone. Qt 6.8's
    // QCommonStyle reads widget->layoutDirection() for these without a null
    // check (qcommonstyle.cpp:2967); the QML desktop bridge passes no widget.
    QRect tabBarScrollButtonRect(const QStyle *style, QStyle::SubElement element,
                                 const QStyleOption *option, const QWidget *widget);

    // A copy of a CC_ScrollBar option whose uint (range + pageStep) divisor is not
    // zero, or nullopt when it already is. A page step clamped for a huge range
    // changes nothing: Qt uses the minimum slider length above INT_MAX / 2.
    std::optional<QStyleOptionSlider> sanitizedScrollBar(QStyle::ComplexControl control,
                                                         const QStyleOptionComplex *option);

    // The smallest multiple of interval that puts at most one tick on each
    // pixel of travel; denser ticks are invisible and cost a loop pass each.
    int boundedTickInterval(int minimum, int maximum, int interval, int available);

    // QCommonStyle's tick interval rule over available (tickInterval, else
    // singleStep, else pageStep when single steps are under 3 px), then
    // bounded to one tick per pixel of travel, the span the ticks are spaced over.
    int effectiveTickInterval(const QStyleOptionSlider &slider, int available, int travel);
}


QT_END_NAMESPACE

#endif // QSTYLEHELPER_P_H
