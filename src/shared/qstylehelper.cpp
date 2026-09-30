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

#include <qstyleoption.h>
#include <qpainter.h>
#include <qpaintdevice.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qmath.h>
#include <qscrollbar.h>
#include <qabstractscrollarea.h>
#include <qwindow.h>

#include "qstylecache_p.h"
#include "qstylehelper_p.h"
#include <qstringbuilder.h>

#include <algorithm>
#include <climits>

QT_BEGIN_NAMESPACE

static const qreal Q_PI   = qreal(3.14159265358979323846);   // pi

namespace ClassicStyleHelper {

QString uniqueName(const QString &key, const QStyleOption *option, const QSize &size, qreal dpr)
{
    const QStyleOptionComplex *complexOption = qstyleoption_cast<const QStyleOptionComplex *>(option);
    QString tmp = key % HexString<uint>(option->state)
                      % HexString<uint>(option->direction)
                      % HexString<uint>(complexOption ? uint(complexOption->activeSubControls) : 0u)
                      % HexString<quint64>(option->palette.cacheKey())
                      % HexString<uint>(size.width())
                      % HexString<uint>(size.height())
                      % HexString<qreal>(dpr);

#ifndef QT_NO_SPINBOX
    if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
        tmp = tmp % HexString<uint>(spinBox->buttonSymbols)
                  % HexString<uint>(spinBox->stepEnabled)
                  % QLatin1Char(spinBox->frame ? '1' : '0'); ;
    }
#endif // QT_NO_SPINBOX
    return tmp;
}

#ifndef QT_NO_ACCESSIBILITY
bool isInstanceOf(QObject *obj, QAccessible::Role role)
{
    bool match = false;
    QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(obj);
    match = iface && iface->role() == role;
    return match;
}

// Searches for an ancestor of a particular accessible role
bool hasAncestor(QObject *obj, QAccessible::Role role)
{
    bool found = false;
    QObject *parent = obj ? obj->parent() : 0;
    while (parent && !found) {
        if (isInstanceOf(parent, role))
            found = true;
        parent = parent->parent();
    }
    return found;
}
#endif // QT_NO_ACCESSIBILITY


#ifndef QT_NO_DIAL

int calcBigLineSize(int radius)
{
    int bigLineSize = radius / 6;
    if (bigLineSize < 4)
        bigLineSize = 4;
    if (bigLineSize > radius / 2)
        bigLineSize = radius / 2;
    return bigLineSize;
}

static QPointF calcRadialPos(const QStyleOptionSlider *dial, qreal offset)
{
    const int width = dial->rect.width();
    const int height = dial->rect.height();
    const int r = qMin(width, height) / 2;
    // qreal, not int: range, position and their products overflow an int for
    // hostile options, and every int value here is exact in a double.
    const qreal currentSliderPosition = dial->upsideDown
        ? qreal(dial->sliderPosition) : qreal(dial->maximum) - dial->sliderPosition;
    const qreal range = qreal(dial->maximum) - dial->minimum;
    qreal a = 0;
    if (dial->maximum == dial->minimum)
        a = Q_PI / 2;
    else if (dial->dialWrapping)
        a = Q_PI * 3 / 2 - (currentSliderPosition - dial->minimum) * 2 * Q_PI / range;
    else
        a = (Q_PI * 8 - (currentSliderPosition - dial->minimum) * 10 * Q_PI / range) / 6;
    qreal xc = width / 2.0;
    qreal yc = height / 2.0;
    qreal len = r - ClassicStyleHelper::calcBigLineSize(r) - 3;
    qreal back = offset * len;
    QPointF pos(QPointF(xc + back * qCos(a), yc - back * qSin(a)));
    return pos;
}

qreal angle(const QPointF &p1, const QPointF &p2)
{
    static const qreal rad_factor = 180 / Q_PI;
    qreal _angle = 0;

    if (p1.x() == p2.x()) {
        if (p1.y() < p2.y())
            _angle = 270;
        else
            _angle = 90;
    } else  {
        qreal x1, x2, y1, y2;

        if (p1.x() <= p2.x()) {
            x1 = p1.x(); y1 = p1.y();
            x2 = p2.x(); y2 = p2.y();
        } else {
            x2 = p1.x(); y2 = p1.y();
            x1 = p2.x(); y1 = p2.y();
        }

        qreal m = -(y2 - y1) / (x2 - x1);
        _angle = qAtan(m) *  rad_factor;

        if (p1.x() < p2.x())
            _angle = 180 - _angle;
        else
            _angle = -_angle;
    }
    return _angle;
}

QPolygonF calcLines(const QStyleOptionSlider *dial)
{
    QPolygonF poly;
    int width = dial->rect.width();
    int height = dial->rect.height();
    qreal r = qMin(width, height) / 2;
    int bigLineSize = calcBigLineSize(int(r));

    qreal xc = width / 2 + 0.5;
    qreal yc = height / 2 + 0.5;
    const int ns = dial->tickInterval;
    if (!ns) // Invalid values may be set by Qt Designer.
        return poly;
    // qint64: the span overflows an int for the full range.
    const qint64 span = qint64(dial->maximum) - dial->minimum;
    qint64 notches = (span + ns - 1) / ns;
    if (notches <= 0)
        return poly;
    if (span < 0 || span > 1000)
        notches = (1000 + qint64(ns) - 1) / ns;
    if (notches <= 0)
        return poly;

    poly.resize(2 + 2 * notches);
    int smallLineSize = bigLineSize / 2;
    for (int i = 0; i <= notches; ++i) {
        qreal angle = dial->dialWrapping ? Q_PI * 3 / 2 - i * 2 * Q_PI / notches
                  : (Q_PI * 8 - i * 10 * Q_PI / notches) / 6;
        qreal s = qSin(angle);
        qreal c = qCos(angle);
        if (i == 0 || (((qint64(ns) * i) % (dial->pageStep ? dial->pageStep : 1)) == 0)) {
            poly[2 * i] = QPointF(xc + (r - bigLineSize) * c,
                                  yc - (r - bigLineSize) * s);
            poly[2 * i + 1] = QPointF(xc + r * c, yc - r * s);
        } else {
            poly[2 * i] = QPointF(xc + (r - 1 - smallLineSize) * c,
                                  yc - (r - 1 - smallLineSize) * s);
            poly[2 * i + 1] = QPointF(xc + (r - 1) * c, yc -(r - 1) * s);
        }
    }
    return poly;
}

// This will draw a nice and shiny QDial for us. We don't want
// all the shinyness in QWindowsStyle, hence we place it here

void drawDial(const QStyleOptionSlider *option, QPainter *painter)
{
    QPalette pal = option->palette;
    QColor buttonColor = pal.button().color();
    const int width = option->rect.width();
    const int height = option->rect.height();
    const bool enabled = option->state & QStyle::State_Enabled;
    qreal r = qMin(width, height) / 2;
    r -= r/50;
    const qreal penSize = r/20.0;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    // Draw notches
    if (option->subControls & QStyle::SC_DialTickmarks) {
        painter->setPen(option->palette.dark().color().darker(120));
        painter->drawLines(ClassicStyleHelper::calcLines(option).translated(option->rect.topLeft()));
    }

    // Before the cache block: the knob below uses buttonColor on a cache hit too.
    buttonColor.setHsv(buttonColor .hue(),
                       qMin(140, buttonColor .saturation()),
                       qMax(180, buttonColor.value()));
    // Cache dial background
    BEGIN_STYLE_PIXMAPCACHE(QString::fromLatin1("qdial"));
    p->setRenderHint(QPainter::Antialiasing);

    const qreal d_ = r / 6;
    // rect, not option->rect: the macro sets it to the cache pixmap's origin.
    const qreal dx = rect.x() + d_ + (width - 2 * r) / 2 + 1;
    const qreal dy = rect.y() + d_ + (height - 2 * r) / 2 + 1;

    QRectF br = QRectF(dx + 0.5, dy + 0.5,
                       int(r * 2 - 2 * d_ - 2),
                       int(r * 2 - 2 * d_ - 2));
    QColor shadowColor(0, 0, 0, 20);

    if (enabled) {
        // Drop shadow
        qreal shadowSize = qMax(1.0, penSize/2.0);
        QRectF shadowRect= br.adjusted(-2*shadowSize, -2*shadowSize,
                                       2*shadowSize, 2*shadowSize);
        QRadialGradient shadowGradient(shadowRect.center().x(),
                                       shadowRect.center().y(), shadowRect.width()/2.0,
                                       shadowRect.center().x(), shadowRect.center().y());
        shadowGradient.setColorAt(qreal(0.91), QColor(0, 0, 0, 40));
        shadowGradient.setColorAt(qreal(1.0), Qt::transparent);
        p->setBrush(shadowGradient);
        p->setPen(Qt::NoPen);
        p->translate(shadowSize, shadowSize);
        p->drawEllipse(shadowRect);
        p->translate(-shadowSize, -shadowSize);

        // Main gradient
        QRadialGradient gradient(br.center().x() - br.width()/3, dy,
                                 br.width()*1.3, br.center().x(),
                                 br.center().y() - br.height()/2);
        gradient.setColorAt(0, buttonColor.lighter(110));
        gradient.setColorAt(qreal(0.5), buttonColor);
        gradient.setColorAt(qreal(0.501), buttonColor.darker(102));
        gradient.setColorAt(1, buttonColor.darker(115));
        p->setBrush(gradient);
    } else {
        p->setBrush(Qt::NoBrush);
    }

    p->setPen(QPen(buttonColor.darker(280)));
    p->drawEllipse(br);
    p->setBrush(Qt::NoBrush);
    p->setPen(buttonColor.lighter(110));
    p->drawEllipse(br.adjusted(1, 1, -1, -1));

    if (option->state & QStyle::State_HasFocus) {
        QColor highlight = pal.highlight().color();
        highlight.setHsv(highlight.hue(),
                         qMin(160, highlight.saturation()),
                         qMax(230, highlight.value()));
        highlight.setAlpha(127);
        p->setPen(QPen(highlight, 2.0));
        p->setBrush(Qt::NoBrush);
        p->drawEllipse(br.adjusted(-1, -1, 1, 1));
    }

    END_STYLE_PIXMAPCACHE

    const QPointF origin = option->rect.topLeft();
    QPointF dp = origin + calcRadialPos(option, qreal(0.70));
    buttonColor = buttonColor.lighter(104);
    buttonColor.setAlphaF(qreal(0.8));
    const qreal ds = r/qreal(7.0);
    QRectF dialRect(dp.x() - ds, dp.y() - ds, 2*ds, 2*ds);
    QRadialGradient dialGradient(dialRect.center().x() + dialRect.width()/2,
                                 dialRect.center().y() + dialRect.width(),
                                 dialRect.width()*2,
                                 dialRect.center().x(), dialRect.center().y());
    dialGradient.setColorAt(1, buttonColor.darker(140));
    dialGradient.setColorAt(qreal(0.4), buttonColor.darker(120));
    dialGradient.setColorAt(0, buttonColor.darker(110));
    if (penSize > 3.0) {
        painter->setPen(QPen(QColor(0, 0, 0, 25), penSize));
        painter->drawLine(origin + calcRadialPos(option, qreal(0.90)), origin + calcRadialPos(option, qreal(0.96)));
    }

    painter->setBrush(dialGradient);
    painter->setPen(QColor(255, 255, 255, 150));
    painter->drawEllipse(dialRect.adjusted(-1, -1, 1, 1));
    painter->setPen(QColor(0, 0, 0, 80));
    painter->drawEllipse(dialRect);
    painter->restore();
}
#endif //QT_NO_DIAL

void drawBorderPixmap(const QPixmap &pixmap, QPainter *painter, const QRect &rect,
                     int left, int top, int right,
                     int bottom)
{
    QSize size = pixmap.size();
    //painter->setRenderHint(QPainter::SmoothPixmapTransform);

    //top
    if (top > 0) {
        painter->drawPixmap(QRect(rect.left() + left, rect.top(), rect.width() -right - left, top), pixmap,
                            QRect(left, 0, size.width() -right - left, top));

        //top-left
        if (left > 0)
            painter->drawPixmap(QRect(rect.left(), rect.top(), left, top), pixmap,
                                QRect(0, 0, left, top));

        //top-right
        if (right > 0)
            painter->drawPixmap(QRect(rect.left() + rect.width() - right, rect.top(), right, top), pixmap,
                                QRect(size.width() - right, 0, right, top));
    }

    //left
    if (left > 0)
        painter->drawPixmap(QRect(rect.left(), rect.top()+top, left, rect.height() - top - bottom), pixmap,
                            QRect(0, top, left, size.height() - bottom - top));

    //center
    painter->drawPixmap(QRect(rect.left() + left, rect.top()+top, rect.width() -right - left,
                             rect.height() - bottom - top), pixmap,
                       QRect(left, top, size.width() -right -left,
                             size.height() - bottom - top));
    //right
    if (right > 0)
        painter->drawPixmap(QRect(rect.left() +rect.width() - right, rect.top()+top, right, rect.height() - top - bottom), pixmap,
                            QRect(size.width() - right, top, right, size.height() - bottom - top));

    //bottom
    if (bottom > 0) {
        painter->drawPixmap(QRect(rect.left() +left, rect.top() + rect.height() - bottom,
                                 rect.width() - right - left, bottom), pixmap,
                            QRect(left, size.height() - bottom,
                                 size.width() - right - left, bottom));
        //bottom-left
        if (left > 0)
            painter->drawPixmap(QRect(rect.left(), rect.top() + rect.height() - bottom, left, bottom), pixmap,
                                QRect(0, size.height() - bottom, left, bottom));

        //bottom-right
        if (right > 0)
            painter->drawPixmap(QRect(rect.left() + rect.width() - right, rect.top() + rect.height() - bottom, right, bottom), pixmap,
                                QRect(size.width() - right, size.height() - bottom, right, bottom));

    }
}

QColor backgroundColor(const QPalette &pal, const QWidget* widget)
{
    if (qobject_cast<const QScrollBar *>(widget) && widget->parent() &&
            qobject_cast<const QAbstractScrollArea *>(widget->parent()->parent()))
        return widget->parentWidget()->parentWidget()->palette().color(QPalette::Base);
    return pal.color(QPalette::Base);
}

QWindow *styleObjectWindow(QObject *so)
{
    if (so)
        return so->property("_q_styleObjectWindow").value<QWindow *>();

    return nullptr;
}

qreal cacheDpr(const QPainter *painter)
{
    const QPaintDevice *device = painter ? painter->device() : nullptr;
    const qreal dpr = device ? device->devicePixelRatio() : 1.0;
    return dpr > 0.0 ? dpr : 1.0;
}

QPixmap styleCachePixmap(const QSize &size, qreal dpr)
{
    if (size.isEmpty())
        return QPixmap();
    QPixmap pixmap(size * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);
    return pixmap;
}

QRect tabBarScrollButtonRect(const QStyle *style, QStyle::SubElement element,
                             const QStyleOption *option, const QWidget *widget)
{
    const QRect r = option->rect;
    const bool vertical = r.width() < r.height();
    const int width = style->pixelMetric(QStyle::PM_TabBarScrollButtonWidth, nullptr, widget);
    const int overlap = style->pixelMetric(QStyle::PM_TabBar_ScrollButtonOverlap, nullptr, widget);
    if (element == QStyle::SE_TabBarScrollLeftButton) {
        if (vertical)
            return QRect(0, r.height() - width * 2 + overlap, r.width(), width);
        return QStyle::visualRect(option->direction, r,
                                  QRect(r.width() - width * 2 + overlap, 0, width, r.height()));
    }
    if (vertical)
        return QRect(0, r.height() - width, r.width(), width);
    return QStyle::visualRect(option->direction, r, QRect(r.width() - width, 0, width, r.height()));
}

std::optional<QStyleOptionSlider> sanitizedScrollBar(QStyle::ComplexControl control,
                                                     const QStyleOptionComplex *option)
{
    if (control != QStyle::CC_ScrollBar)
        return std::nullopt;
    const auto *slider = qstyleoption_cast<const QStyleOptionSlider *>(option);
    if (!slider)
        return std::nullopt;
    const qint64 span = std::max<qint64>(qint64(slider->maximum) - slider->minimum, 0);
    const qint64 maxPageStep = qint64(UINT_MAX) - span;
    if (slider->pageStep >= 0 && slider->maximum >= slider->minimum && slider->pageStep <= maxPageStep)
        return std::nullopt;
    QStyleOptionSlider fixed = *slider;
    fixed.maximum = std::max(fixed.maximum, fixed.minimum);
    fixed.pageStep = int(std::clamp<qint64>(fixed.pageStep, 0, maxPageStep));
    return fixed;
}

int boundedTickInterval(int minimum, int maximum, int interval, int available)
{
    if (interval <= 0)
        interval = 1;
    const qint64 span = qint64(maximum) - qint64(minimum);
    if (span <= 0)
        return interval;
    const qint64 limit = std::max(available, 1);
    const qint64 ticks = span / interval;
    if (ticks <= limit)
        return interval;
    const qint64 factor = (ticks + limit - 1) / limit;
    return int(std::min<qint64>(qint64(interval) * factor, INT_MAX));
}

int effectiveTickInterval(const QStyleOptionSlider &slider, int available, int travel)
{
    int interval = slider.tickInterval;
    if (interval <= 0) {
        interval = slider.singleStep;
        if (QStyle::sliderPositionFromValue(slider.minimum, slider.maximum, interval, available)
                - QStyle::sliderPositionFromValue(slider.minimum, slider.maximum, 0, available)
            < 3)
            interval = slider.pageStep;
    }
    return boundedTickInterval(slider.minimum, slider.maximum, interval, travel);
}

}
QT_END_NAMESPACE
