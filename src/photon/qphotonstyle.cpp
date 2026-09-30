// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

#include "qphotonstyle.h"

#include "photonpainter.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QComboBox>
#include <QMenuBar>
#include <QPainter>
#include <QScrollBar>
#include <QSlider>
#include <QStyleOption>
#include <QTabBar>
#include <QToolButton>

#include <algorithm>

using photon::Colors;

namespace {

// Sizes measured from the reference/qnx-photon/ screenshots.
constexpr int kFrameWidth = 3;
constexpr int kButtonMinWidth = 72;
constexpr int kButtonMinHeight = 27;
constexpr int kComboMinHeight = 26;
constexpr int kComboArrowArea = 17;
constexpr int kScrollBarExtent = 17;
constexpr int kScrollButtonLength = 16;
constexpr int kSpinButtonWidth = 17;
constexpr int kMenuItemHeight = 19;
constexpr int kMenuSeparatorHeight = 7;
constexpr int kMenuCheckColumn = 14;
constexpr int kTabBaseOverlap = 2;

// Callers keep drawing with the pen they set before calling the style, so
// nothing the style changes on the painter may outlive the call.
class PainterStateGuard
{
public:
    explicit PainterStateGuard(QPainter *p) : m_painter(p) { m_painter->save(); }
    ~PainterStateGuard() { m_painter->restore(); }
    PainterStateGuard(const PainterStateGuard &) = delete;
    PainterStateGuard &operator=(const PainterStateGuard &) = delete;

private:
    QPainter *m_painter;
};

Colors colorsOf(const QStyleOption *opt)
{
    return photon::colorsFromPalette(opt->palette);
}

bool isEnabled(const QStyleOption *opt)
{
    return opt->state & QStyle::State_Enabled;
}

// Maps a north-shaped tab drawing onto the real tab shape. Rn is the tab in
// north coordinates with its origin at (0,0).
QTransform tabTransform(const QRect &r, QTabBar::Shape shape, QRect *northRect)
{
    switch (shape) {
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        *northRect = QRect(0, 0, r.width(), r.height());
        return QTransform(1, 0, 0, -1, r.left(), r.bottom() + 1);
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        *northRect = QRect(0, 0, r.height(), r.width());
        return QTransform(0, 1, 1, 0, r.left(), r.top());
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        *northRect = QRect(0, 0, r.height(), r.width());
        return QTransform(0, 1, -1, 0, r.right() + 1, r.top());
    default:
        *northRect = QRect(0, 0, r.width(), r.height());
        return QTransform::fromTranslate(r.left(), r.top());
    }
}

bool isVerticalTab(QTabBar::Shape shape)
{
    return shape == QTabBar::RoundedWest || shape == QTabBar::RoundedEast
        || shape == QTabBar::TriangularWest || shape == QTabBar::TriangularEast;
}

// A slant overhangs its tab by half the tab height into the next tab. The
// last tab has no next tab, only the edge of the tab bar, so it reserves
// that overhang inside its own rect.
bool isLastTab(const QStyleOptionTab *tab)
{
    return tab->position == QStyleOptionTab::End || tab->position == QStyleOptionTab::OnlyOneTab;
}

void drawTabShape(QPainter *p, const QStyleOptionTab *tab)
{
    QRect n;
    const QTransform xf = tabTransform(tab->rect, tab->shape, &n);
    if (isLastTab(tab))
        n.setWidth(n.width() - n.height() / 2);
    p->setTransform(xf, true);
    p->setRenderHint(QPainter::Antialiasing, false);
    // Neighbours stack left over right; the selected tab is painted last and
    // is left unclipped so its slant covers the next tab.
    const bool hasLeftNeighbour = tab->position != QStyleOptionTab::Beginning
        && tab->position != QStyleOptionTab::OnlyOneTab
        && tab->position != QStyleOptionTab::Moving;
    photon::drawTab(p, n, tab->state & QStyle::State_Selected, hasLeftNeighbour, colorsOf(tab));
}

QRect scrollBarGroove(const QStyleOptionSlider *sb)
{
    const QRect r = sb->rect;
    if (sb->orientation == Qt::Horizontal)
        return QRect(r.left() + kScrollButtonLength - 1, r.top(),
                     r.width() - 2 * (kScrollButtonLength - 1), r.height());
    return QRect(r.left(), r.top() + kScrollButtonLength - 1, r.width(),
                 r.height() - 2 * (kScrollButtonLength - 1));
}

} // namespace

QPhotonStyle::QPhotonStyle() = default;

QPhotonStyle::~QPhotonStyle() = default;

QPalette QPhotonStyle::standardPalette() const
{
    return photon::standardPalette();
}

void QPhotonStyle::polish(QWidget *widget)
{
    QCommonStyle::polish(widget);
    if (!widget)
        return;
    if (qobject_cast<QAbstractButton *>(widget) || qobject_cast<QComboBox *>(widget)
        || qobject_cast<QScrollBar *>(widget) || qobject_cast<QTabBar *>(widget)
        || qobject_cast<QMenuBar *>(widget) || qobject_cast<QSlider *>(widget)) {
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void QPhotonStyle::unpolish(QWidget *widget)
{
    if (widget
        && (qobject_cast<QAbstractButton *>(widget) || qobject_cast<QComboBox *>(widget)
            || qobject_cast<QScrollBar *>(widget) || qobject_cast<QTabBar *>(widget)
            || qobject_cast<QMenuBar *>(widget) || qobject_cast<QSlider *>(widget))) {
        widget->setAttribute(Qt::WA_Hover, false);
    }
    QCommonStyle::unpolish(widget);
}

void QPhotonStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                 const QWidget *widget) const
{
    if (!opt || !p)
        return;
    const PainterStateGuard guard(p);
    const Colors c = colorsOf(opt);
    const QRect r = opt->rect;
    const bool sunken = opt->state & (State_Sunken | State_On);

    switch (pe) {
    case PE_PanelButtonCommand:
    case PE_PanelButtonBevel:
        photon::drawButton(p, r, c, sunken, true);
        return;

    case PE_FrameDefaultButton:
        return;

    case PE_FrameButtonBevel:
        photon::drawRaisedFrame(p, r, c, sunken);
        return;

    case PE_PanelButtonTool: {
        const bool hover = (opt->state & State_MouseOver) && isEnabled(opt);
        const bool raised = opt->state & State_Raised;
        if (opt->state & State_On) {
            // Checked toggle: sunken, white, one shade line top/left.
            photon::drawField(p, r, c, c.base, c.toolCheckedShade);
        } else if (opt->state & State_Sunken) {
            photon::drawButton(p, r, c, true, true);
        } else if (hover || raised) {
            photon::drawButton(p, r, c, false, true);
        }
        return;
    }

    case PE_PanelLineEdit:
        if (const auto *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            const bool readOnly = frame->state & State_ReadOnly;
            const QColor fill = isEnabled(opt) && !readOnly ? c.fieldFill : c.window;
            if (frame->lineWidth > 0)
                photon::drawField(p, r, c, fill);
            else
                p->fillRect(r, fill);
        }
        return;

    case PE_FrameLineEdit:
        photon::drawField(p, r, c, QColor());
        return;

    case PE_Frame:
        if (r.width() >= 4 && r.height() >= 4)
            photon::drawField(p, r, c, QColor());
        return;

    case PE_FrameGroupBox:
    case PE_FrameDockWidget:
        photon::drawEtchedRect(p, r, c.etchedDark, c.etchedLight);
        return;

    case PE_FrameStatusBarItem:
        photon::drawEtchedLine(p, QPoint(r.left(), r.top() + 1), Qt::Vertical,
                               std::max(0, r.height() - 2), c.stripDark, c.stripLight);
        return;

    case PE_FrameTabWidget:
        if (const auto *twf = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
            // The #C0C0C0 strip the tab row sits on, above the pane.
            if (twf->shape == QTabBar::RoundedNorth || twf->shape == QTabBar::TriangularNorth) {
                const int stripTop = r.top() - twf->tabBarSize.height() + kTabBaseOverlap;
                p->fillRect(QRect(r.left(), stripTop, r.width(), r.top() - stripTop), c.fieldShade);
            }
        }
        if (r.width() >= 4 && r.height() >= 4) {
            photon::drawRing(p, r, 0, c.outline);
            photon::drawBevel(p, r, 1, c.midlight, c.shade);
        }
        return;

    case PE_FrameTabBarBase:
        photon::drawSide(p, r.adjusted(0, 0, 0, -1), Qt::BottomEdge, 0, c.outline);
        photon::drawSide(p, r, Qt::BottomEdge, 0, c.midlight);
        return;

    case PE_FrameWindow:
        photon::drawRaisedFrame(p, r, c, false);
        return;

    case PE_PanelMenuBar:
    case PE_PanelToolBar:
        photon::drawPanel(p, r, c);
        return;

    case PE_PanelMenu:
        p->fillRect(r, c.window);
        return;

    case PE_FrameMenu:
        photon::drawMenuFrame(p, r, opt->palette.color(QPalette::WindowText), c);
        return;

    case PE_PanelTipLabel:
        p->fillRect(r, opt->palette.color(QPalette::ToolTipBase));
        photon::drawRing(p, r, 0, c.outline);
        return;

    case PE_FrameFocusRect:
        if (r.width() >= 2 && r.height() >= 2) {
            if (qobject_cast<const QAbstractItemView *>(widget)) {
                // Item views mark the current row with a line above and below.
                photon::drawSide(p, r, Qt::TopEdge, 0, c.focus);
                photon::drawSide(p, r, Qt::BottomEdge, 0, c.focus);
            } else {
                photon::drawRing(p, r, 0, c.focus);
            }
        }
        return;

    case PE_IndicatorCheckBox:
    case PE_IndicatorItemViewItemCheck:
        photon::drawCheckBox(p, r, opt->state & State_On, isEnabled(opt),
                             opt->state & State_NoChange, c);
        return;

    case PE_IndicatorRadioButton:
        photon::drawRadio(p, r, opt->state & State_On, isEnabled(opt), c);
        return;

    case PE_IndicatorMenuCheckMark:
        if (r.width() >= 8 && r.height() >= 8)
            photon::drawCross(p, r, isEnabled(opt) ? c.text : c.disabledText, c.menuItem);
        return;

    case PE_IndicatorArrowUp:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight:
    case PE_IndicatorSpinUp:
    case PE_IndicatorSpinDown:
    case PE_IndicatorSpinPlus:
    case PE_IndicatorSpinMinus: {
        Qt::ArrowType type = Qt::DownArrow;
        if (pe == PE_IndicatorArrowUp || pe == PE_IndicatorSpinUp || pe == PE_IndicatorSpinPlus)
            type = Qt::UpArrow;
        else if (pe == PE_IndicatorArrowLeft)
            type = Qt::LeftArrow;
        else if (pe == PE_IndicatorArrowRight)
            type = Qt::RightArrow;
        const QColor ink = isEnabled(opt) ? c.text : c.disabledText;
        const photon::ArrowShape shape = (pe == PE_IndicatorSpinUp || pe == PE_IndicatorSpinDown
                                          || pe == PE_IndicatorSpinPlus || pe == PE_IndicatorSpinMinus)
            ? photon::kBranchArrow
            : photon::kComboArrow;
        photon::drawArrow(p, r, type, ink, shape);
        return;
    }

    case PE_IndicatorButtonDropDown:
        photon::drawButton(p, r, c, sunken, false);
        return;

    case PE_IndicatorHeaderArrow:
        if (const auto *header = qstyleoption_cast<const QStyleOptionHeader *>(opt)) {
            photon::drawChevron(p, r, header->sortIndicator == QStyleOptionHeader::SortUp, c.text);
        }
        return;

    case PE_IndicatorBranch:
        if (opt->state & State_Children) {
            const QColor ink = photon::shifted(c.text, 0x32);
            photon::drawArrow(p, r, (opt->state & State_Open) ? Qt::DownArrow : Qt::RightArrow, ink,
                              photon::kBranchArrow);
        }
        return;

    case PE_IndicatorToolBarSeparator:
        if (opt->state & State_Horizontal)
            photon::drawEtchedLine(p, QPoint(r.center().x(), r.top() + 2), Qt::Vertical,
                                   std::max(0, r.height() - 4), c.stripDark, c.stripLight);
        else
            photon::drawEtchedLine(p, QPoint(r.left() + 2, r.center().y()), Qt::Horizontal,
                                   std::max(0, r.width() - 4), c.stripDark, c.stripLight);
        return;

    case PE_IndicatorToolBarHandle:
        if (opt->state & State_Horizontal) {
            for (int x : {r.center().x() - 2, r.center().x() + 1})
                photon::drawEtchedLine(p, QPoint(x, r.top() + 3), Qt::Vertical,
                                       std::max(0, r.height() - 6), c.light, c.stripDark);
        } else {
            for (int y : {r.center().y() - 2, r.center().y() + 1})
                photon::drawEtchedLine(p, QPoint(r.left() + 3, y), Qt::Horizontal,
                                       std::max(0, r.width() - 6), c.light, c.stripDark);
        }
        return;

    case PE_IndicatorDockWidgetResizeHandle:
        if (opt->state & State_Horizontal)
            photon::drawEtchedLine(p, QPoint(r.left(), r.center().y()), Qt::Horizontal, r.width(),
                                   c.etchedDark, c.etchedLight);
        else
            photon::drawEtchedLine(p, QPoint(r.center().x(), r.top()), Qt::Vertical, r.height(),
                                   c.etchedDark, c.etchedLight);
        return;

    default:
        break;
    }
    QCommonStyle::drawPrimitive(pe, opt, p, widget);
}

void QPhotonStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                               const QWidget *widget) const
{
    if (!opt || !p)
        return;
    const PainterStateGuard guard(p);
    const Colors c = colorsOf(opt);
    const QRect r = opt->rect;

    switch (ce) {
    case CE_MenuBarEmptyArea:
        photon::drawPanel(p, widget ? widget->rect() : r, c);
        return;

    case CE_MenuBarItem:
        if (const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            photon::drawPanel(p, widget ? widget->rect() : r, c);
            const bool active = (mi->state & State_Selected) || (mi->state & State_Sunken);
            if (active && isEnabled(mi))
                p->fillRect(r.adjusted(0, 1, 0, -1), c.menuHover);
            QStyleOptionMenuItem label = *mi;
            label.state &= ~(State_Selected | State_Sunken);
            label.palette.setColor(QPalette::ButtonText, mi->palette.color(QPalette::WindowText));
            const int flags = Qt::AlignCenter | Qt::TextShowMnemonic | Qt::TextDontClip
                | Qt::TextSingleLine;
            drawItemText(p, r, flags, label.palette, isEnabled(mi), mi->text, QPalette::WindowText);
        }
        return;

    case CE_MenuEmptyArea:
        p->fillRect(r, c.window);
        return;

    case CE_MenuItem:
        if (const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            p->fillRect(r, c.window);
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                // The frame edge repeated across the menu.
                const QRect band(r.left() - 2, r.top() + (r.height() - 3) / 2, r.width() + 4, 3);
                photon::drawSide(p, band, Qt::TopEdge, 0, c.menuFrameShade);
                photon::drawSide(p, band, Qt::BottomEdge, 0, c.menuFrameLight);
                photon::drawSide(p, band.adjusted(-1, 0, 1, 0), Qt::TopEdge, 1,
                                 mi->palette.color(QPalette::WindowText));
                return;
            }
            const bool enabled = isEnabled(mi);
            const bool selected = (mi->state & State_Selected) && enabled;
            const QRect row(r.left(), r.top() + 1, r.width(), std::max(0, r.height() - 2));
            p->fillRect(row, selected ? c.menuHover : c.menuItem);

            int x = row.left() + 4;
            const bool reserveCheck = mi->menuHasCheckableItems;
            if (mi->checkType != QStyleOptionMenuItem::NotCheckable && mi->checked) {
                const QRect box(x, row.top() + (row.height() - 10) / 2, 10, 10);
                photon::drawCross(p, box, enabled ? c.text : c.disabledText,
                                  selected ? c.menuHover : c.menuItem);
            }
            if (reserveCheck)
                x += kMenuCheckColumn;
            if (!mi->icon.isNull()) {
                const int iconSize = pixelMetric(PM_SmallIconSize, opt, widget);
                const QIcon::Mode mode = enabled ? QIcon::Normal : QIcon::Disabled;
                const QPixmap pm = mi->icon.pixmap(QSize(iconSize, iconSize), p->device()->devicePixelRatio(), mode);
                p->drawPixmap(x, row.top() + (row.height() - iconSize) / 2, pm);
            }
            x += std::max(mi->maxIconWidth, 0) + (mi->maxIconWidth > 0 ? 4 : 0);

            const QColor ink = enabled ? c.text : c.disabledText;
            QString text = mi->text;
            QString shortcut;
            if (const qsizetype tab = text.indexOf(u'\t'); tab >= 0) {
                shortcut = text.mid(tab + 1);
                text = text.left(tab);
            }
            const int arrowSpace = mi->menuItemType == QStyleOptionMenuItem::SubMenu ? 16 : 4;
            const QRect textRect(x, row.top(), row.right() - arrowSpace - x, row.height());
            p->save();
            p->setPen(ink);
            p->setFont(mi->font);
            const int flags = Qt::AlignVCenter | Qt::TextShowMnemonic | Qt::TextDontClip
                | Qt::TextSingleLine;
            p->drawText(textRect, flags | Qt::AlignLeft, text);
            if (!shortcut.isEmpty())
                p->drawText(textRect, flags | Qt::AlignRight, shortcut);
            p->restore();
            if (mi->menuItemType == QStyleOptionMenuItem::SubMenu) {
                const QRect arrow(row.right() - 12, row.top(), 10, row.height());
                photon::drawArrow(p, arrow, Qt::RightArrow, ink, photon::kComboArrow);
            }
        }
        return;

    case CE_ToolBar:
        photon::drawPanel(p, r, c);
        return;

    case CE_HeaderSection:
        {
            const auto *header = qstyleoption_cast<const QStyleOptionHeader *>(opt);
            const bool first = !header || header->position == QStyleOptionHeader::Beginning
                || header->position == QStyleOptionHeader::OnlyOneSection;
            photon::drawHeaderSection(p, r, first, opt->state & State_Sunken, c);
        }
        return;

    case CE_ProgressBarGroove:
        photon::drawProgressGroove(p, r, c);
        return;

    case CE_ProgressBarContents:
        if (const auto *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
            const qint64 range = qint64(pb->maximum) - pb->minimum;
            double fraction = 0.25; // busy indicator: a quarter-length block, no animation
            if (range > 0) {
                const qint64 done = std::clamp<qint64>(qint64(pb->progress) - pb->minimum, 0, range);
                fraction = static_cast<double>(done) / static_cast<double>(range);
            }
            const Qt::Orientation orientation =
                (pb->state & State_Horizontal) ? Qt::Horizontal : Qt::Vertical;
            const bool reversed = pb->invertedAppearance != (pb->direction == Qt::RightToLeft);
            photon::drawProgressBar(p, r, fraction, orientation, reversed, c);
        }
        return;

    case CE_TabBarTabShape:
        if (const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(opt))
            drawTabShape(p, tab);
        return;

    case CE_TabBarTabLabel:
        if (const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            QStyleOptionTab label = *tab;
            int half = (isVerticalTab(tab->shape) ? r.width() : r.height()) / 2;
            if (isLastTab(tab))
                half *= 2;
            if (isVerticalTab(tab->shape))
                label.rect.adjust(0, 0, 0, -half);
            else
                label.rect.adjust(0, 0, -half, 0);
            if (!(tab->state & State_Selected))
                label.palette.setColor(QPalette::WindowText, c.tabInactiveText);
            QCommonStyle::drawControl(ce, &label, p, widget);
        }
        return;

    case CE_ShapedFrame:
        if (const auto *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (frame->frameShape == QFrame::HLine) {
                photon::drawEtchedLine(p, QPoint(r.left(), r.center().y()), Qt::Horizontal,
                                       r.width(), c.etchedDark, c.etchedLight);
                return;
            }
            if (frame->frameShape == QFrame::VLine) {
                photon::drawEtchedLine(p, QPoint(r.center().x(), r.top()), Qt::Vertical,
                                       r.height(), c.etchedDark, c.etchedLight);
                return;
            }
        }
        break;

    case CE_Splitter:
        if (opt->state & State_Horizontal)
            photon::drawEtchedLine(p, QPoint(r.center().x(), r.top()), Qt::Vertical, r.height(),
                                   c.etchedDark, c.etchedLight);
        else
            photon::drawEtchedLine(p, QPoint(r.left(), r.center().y()), Qt::Horizontal, r.width(),
                                   c.etchedDark, c.etchedLight);
        return;

    case CE_ToolBoxTabShape:
        photon::drawButton(p, r, c, opt->state & State_Sunken, false);
        return;

    case CE_DockWidgetTitle:
        if (qstyleoption_cast<const QStyleOptionDockWidget *>(opt))
            photon::drawPanel(p, r, c);
        break;

    default:
        break;
    }
    QCommonStyle::drawControl(ce, opt, p, widget);
}

void QPhotonStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                      const QWidget *widget) const
{
    if (!opt || !p)
        return;
    const PainterStateGuard guard(p);
    const Colors c = colorsOf(opt);

    switch (cc) {
    case CC_ComboBox:
        if (const auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            const QRect r = cb->rect;
            const QRect arrow = subControlRect(cc, cb, SC_ComboBoxArrow, widget);
            const bool pressed = (cb->state & State_Sunken) && (cb->activeSubControls & SC_ComboBoxArrow);
            const QColor ink = isEnabled(cb) ? c.text : c.disabledText;
            if (cb->editable) {
                photon::drawField(p, r, c, isEnabled(cb) ? c.fieldFill : c.window);
                photon::drawComboArrowButton(p, r, kComboArrowArea, pressed, isEnabled(cb), c);
            } else {
                photon::drawButton(p, r, c, pressed, true);
                if (r.width() > kComboArrowArea + 8) {
                    const int sepX = r.right() - 16;
                    photon::drawEtchedLine(p, QPoint(sepX, r.top() + 4), Qt::Vertical,
                                           std::max(0, r.height() - 8), c.stripDark, c.stripLight);
                }
                photon::drawArrow(p, arrow, Qt::DownArrow, ink, photon::kComboArrow);
                if (cb->state & State_HasFocus) {
                    QStyleOptionFocusRect focus;
                    focus.QStyleOption::operator=(*cb);
                    focus.rect = subControlRect(cc, cb, SC_ComboBoxEditField, widget).adjusted(-1, 0, 1, 0);
                    drawPrimitive(PE_FrameFocusRect, &focus, p, widget);
                }
            }
        }
        return;

    case CC_SpinBox:
        if (const auto *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            if (sb->frame)
                photon::drawField(p, sb->rect, c, isEnabled(sb) ? c.fieldFill : c.window);
            if (sb->buttonSymbols != QAbstractSpinBox::NoButtons
                && (sb->subControls & (SC_SpinBoxUp | SC_SpinBoxDown))) {
                const bool sunken = sb->state & State_Sunken;
                const bool enabled = isEnabled(sb);
                photon::drawSpinButtons(
                    p, sb->rect, kSpinButtonWidth, sunken && (sb->activeSubControls & SC_SpinBoxUp),
                    sunken && (sb->activeSubControls & SC_SpinBoxDown),
                    enabled && (sb->stepEnabled & QAbstractSpinBox::StepUpEnabled),
                    enabled && (sb->stepEnabled & QAbstractSpinBox::StepDownEnabled), c);
            }
        }
        return;

    case CC_ScrollBar:
        if (const auto *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            const bool enabled = isEnabled(sb);
            const bool sunken = sb->state & State_Sunken;
            const qint64 range = qint64(sb->maximum) - sb->minimum;
            photon::ScrollBarState state;
            state.orientation = sb->orientation;
            state.buttonLength = kScrollButtonLength;
            state.sliderMinimum = pixelMetric(PM_ScrollBarSliderMin, sb, widget);
            state.showSlider = enabled && range > 0;
            if (range > 0) {
                const double pos =
                    static_cast<double>(qint64(sb->sliderPosition) - sb->minimum) / static_cast<double>(range);
                state.sliderStart = sb->upsideDown ? 1.0 - pos : pos;
                state.sliderLength = static_cast<double>(sb->pageStep)
                    / static_cast<double>(range + std::max(sb->pageStep, 0));
            }
            state.subPressed = sunken && (sb->activeSubControls & SC_ScrollBarSubLine);
            state.addPressed = sunken && (sb->activeSubControls & SC_ScrollBarAddLine);
            state.subEnabled = enabled && sb->sliderValue > sb->minimum;
            state.addEnabled = enabled && sb->sliderValue < sb->maximum;
            photon::drawScrollBar(p, sb->rect, state, c);
        }
        return;

    case CC_Slider:
        if (const auto *sl = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            const bool horizontal = sl->orientation == Qt::Horizontal;
            const QRect groove = subControlRect(cc, sl, SC_SliderGroove, widget);
            const QRect handle = subControlRect(cc, sl, SC_SliderHandle, widget);
            if (sl->subControls & SC_SliderGroove) {
                if (horizontal)
                    photon::drawEtchedLine(p, QPoint(groove.left(), handle.center().y() - 1),
                                           Qt::Horizontal, groove.width(), c.sliderGrooveLight, c.outline);
                else
                    photon::drawEtchedLine(p, QPoint(handle.center().x() - 1, groove.top()),
                                           Qt::Vertical, groove.height(), c.sliderGrooveLight, c.outline);
            }
            if ((sl->subControls & SC_SliderTickmarks) && sl->tickPosition != QSlider::NoTicks) {
                const int interval = sl->tickInterval > 0 ? sl->tickInterval
                                                          : std::max(sl->pageStep, 1);
                const int available = pixelMetric(PM_SliderSpaceAvailable, sl, widget);
                const int len = pixelMetric(PM_SliderLength, sl, widget);
                for (qint64 v = sl->minimum; v <= sl->maximum; v += interval) {
                    const int pos = sliderPositionFromValue(sl->minimum, sl->maximum, int(v), available,
                                                            sl->upsideDown) + len / 2;
                    if (horizontal) {
                        const int x = sl->rect.left() + pos;
                        if (sl->tickPosition & QSlider::TicksBelow)
                            p->fillRect(QRect(x, sl->rect.bottom() - 2, 1, 2), c.outline);
                        if (sl->tickPosition & QSlider::TicksAbove)
                            p->fillRect(QRect(x, sl->rect.top(), 1, 2), c.outline);
                    } else {
                        const int y = sl->rect.top() + pos;
                        if (sl->tickPosition & QSlider::TicksRight)
                            p->fillRect(QRect(sl->rect.right() - 2, y, 2, 1), c.outline);
                        if (sl->tickPosition & QSlider::TicksLeft)
                            p->fillRect(QRect(sl->rect.left(), y, 2, 1), c.outline);
                    }
                    if (interval <= 0)
                        break;
                }
            }
            if ((sl->subControls & SC_SliderHandle) && handle.width() >= 4 && handle.height() >= 4) {
                photon::drawSliderHandle(p, handle, sl->orientation, c);
                if (sl->state & State_HasFocus) {
                    QStyleOptionFocusRect focus;
                    focus.QStyleOption::operator=(*sl);
                    // Not clamped to the widget: a clamped ring lands on the
                    // handle outline when the handle touches the edge.
                    focus.rect = handle.adjusted(-2, -2, 2, 2);
                    drawPrimitive(PE_FrameFocusRect, &focus, p, widget);
                }
            }
        }
        return;

    default:
        break;
    }
    QCommonStyle::drawComplexControl(cc, opt, p, widget);
}

int QPhotonStyle::pixelMetric(PixelMetric pm, const QStyleOption *opt, const QWidget *widget) const
{
    switch (pm) {
    case PM_DefaultFrameWidth:
    case PM_ComboBoxFrameWidth:
    case PM_SpinBoxFrameWidth:
        return kFrameWidth;
    case PM_ButtonMargin:
        return 8;
    case PM_ButtonDefaultIndicator:
        return 0;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        return 1;
    case PM_IndicatorWidth:
    case PM_IndicatorHeight:
        return 14;
    case PM_ExclusiveIndicatorWidth:
    case PM_ExclusiveIndicatorHeight:
        return 13;
    case PM_ScrollBarExtent:
        return kScrollBarExtent;
    case PM_ScrollBarSliderMin:
        return 10;
    case PM_SliderThickness:
    case PM_SliderControlThickness:
        return 20;
    case PM_SliderLength:
        return 14;
    case PM_MenuBarPanelWidth:
        return 1;
    case PM_MenuBarItemSpacing:
        return 0;
    case PM_MenuBarHMargin:
        return 2;
    case PM_MenuBarVMargin:
        return 2;
    case PM_MenuPanelWidth:
        return 2;
    case PM_MenuHMargin:
    case PM_MenuVMargin:
        return 2;
    case PM_TabBarTabHSpace:
        return 16;
    case PM_TabBarTabVSpace:
        return 6;
    case PM_TabBarBaseOverlap:
        return kTabBaseOverlap;
    case PM_TabBarTabShiftVertical:
    case PM_TabBarTabShiftHorizontal:
        return 0;
    case PM_ToolBarFrameWidth:
        return 2;
    case PM_ToolBarItemSpacing:
        return 2;
    case PM_ToolBarSeparatorExtent:
        return 6;
    case PM_ToolBarHandleExtent:
        return 8;
    case PM_SplitterWidth:
        return 6;
    case PM_ProgressBarChunkWidth:
        return 1;
    case PM_TitleBarHeight:
        return 20;
    case PM_HeaderMargin:
        return 4;
    case PM_DockWidgetSeparatorExtent:
    case PM_DockWidgetHandleExtent:
        return 6;
    case PM_MdiSubWindowFrameWidth:
        return 4;
    default:
        break;
    }
    return QCommonStyle::pixelMetric(pm, opt, widget);
}

QSize QPhotonStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &size,
                                     const QWidget *widget) const
{
    if (!opt)
        return QCommonStyle::sizeFromContents(ct, opt, size, widget);

    switch (ct) {
    case CT_PushButton:
        if (const auto *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            int w = size.width() + 2 * kFrameWidth + 2 * 8 + 2;
            int h = size.height() + 2 * kFrameWidth + 4;
            if (!btn->text.isEmpty() || !btn->icon.isNull()) {
                w = std::max(w, kButtonMinWidth);
                h = std::max(h, kButtonMinHeight);
            }
            return QSize(w, h);
        }
        break;
    case CT_ComboBox:
        if (qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            const int w = size.width() + 2 * kFrameWidth + kComboArrowArea + 8;
            const int h = std::max(size.height() + 2 * kFrameWidth + 4, kComboMinHeight);
            return QSize(w, h);
        }
        break;
    case CT_SpinBox:
        return QSize(size.width() + 2 * kFrameWidth + kSpinButtonWidth + 2,
                     std::max(size.height() + 2 * kFrameWidth, 24));
    case CT_LineEdit:
        return QSize(size.width() + 2 * kFrameWidth + 2, std::max(size.height() + 2 * kFrameWidth, 22));
    case CT_MenuItem:
        if (const auto *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            if (mi->menuItemType == QStyleOptionMenuItem::Separator)
                return QSize(size.width(), kMenuSeparatorHeight);
            QSize s = QCommonStyle::sizeFromContents(ct, opt, size, widget);
            s.setWidth(s.width() + 8 + (mi->menuHasCheckableItems ? kMenuCheckColumn : 0) + 16);
            // A row is the text plus 3 px of fill, then the 2 px gap.
            s.setHeight(std::max(size.height() + 5, kMenuItemHeight));
            return s;
        }
        break;
    case CT_MenuBarItem:
        return QSize(size.width() + 16, size.height() + 6);
    case CT_TabBarTab:
        if (const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            QSize s = QCommonStyle::sizeFromContents(ct, opt, size, widget);
            // Room for the slant, plus the two rows shared with the pane.
            const int slants = isLastTab(tab) ? 2 : 1;
            if (isVerticalTab(tab->shape)) {
                s.setWidth(s.width() + kTabBaseOverlap);
                s.setHeight(s.height() + slants * (s.width() / 2));
            } else {
                s.setHeight(s.height() + kTabBaseOverlap);
                s.setWidth(s.width() + slants * (s.height() / 2));
            }
            return s;
        }
        break;
    case CT_HeaderSection: {
        QSize s = QCommonStyle::sizeFromContents(ct, opt, size, widget);
        s.setHeight(std::max(s.height(), 22));
        return s;
    }
    default:
        break;
    }
    return QCommonStyle::sizeFromContents(ct, opt, size, widget);
}

QRect QPhotonStyle::subElementRect(SubElement se, const QStyleOption *opt, const QWidget *widget) const
{
    if (!opt)
        return QCommonStyle::subElementRect(se, opt, widget);

    switch (se) {
    case SE_TabBarScrollLeftButton:
    case SE_TabBarScrollRightButton: {
        // QCommonStyle reads widget->layoutDirection() without a null check
        // (Qt 6.8 qcommonstyle.cpp:2967); this uses the option's direction.
        const QRect r = opt->rect;
        const bool vertical = r.width() < r.height();
        const int width = pixelMetric(PM_TabBarScrollButtonWidth, nullptr, widget);
        const int overlap = pixelMetric(PM_TabBar_ScrollButtonOverlap, nullptr, widget);
        if (se == SE_TabBarScrollLeftButton) {
            if (vertical)
                return QRect(0, r.height() - width * 2 + overlap, r.width(), width);
            return visualRect(opt->direction, r,
                              QRect(r.width() - width * 2 + overlap, 0, width, r.height()));
        }
        if (vertical)
            return QRect(0, r.height() - width, r.width(), width);
        return visualRect(opt->direction, r, QRect(r.width() - width, 0, width, r.height()));
    }
    case SE_ProgressBarGroove:
    case SE_ProgressBarContents:
    case SE_ProgressBarLabel:
        // Photon draws the label over the middle of the bar, never beside it.
        return opt->rect;
    default:
        break;
    }
    return QCommonStyle::subElementRect(se, opt, widget);
}

QRect QPhotonStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                                   const QWidget *widget) const
{
    if (!opt)
        return QCommonStyle::subControlRect(cc, opt, sc, widget);

    switch (cc) {
    case CC_ComboBox:
        if (const auto *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            const QRect r = cb->rect;
            QRect ret;
            switch (sc) {
            case SC_ComboBoxFrame:
            case SC_ComboBoxListBoxPopup:
                ret = r;
                break;
            case SC_ComboBoxArrow:
                // Editable: a raised button sharing the field's right outline.
                // Plain: the area right of the etched separator.
                ret = cb->editable ? QRect(r.right() - kComboArrowArea, r.top() + 1, kComboArrowArea,
                                           r.height() - 2)
                                   : QRect(r.right() - 14, r.top() + 3, 12, r.height() - 6);
                break;
            case SC_ComboBoxEditField:
                ret = cb->editable
                    ? QRect(r.left() + 4, r.top() + 4, r.width() - kComboArrowArea - 6, r.height() - 8)
                    : QRect(r.left() + 6, r.top() + 4, r.width() - kComboArrowArea - 10, r.height() - 8);
                break;
            default:
                return QCommonStyle::subControlRect(cc, opt, sc, widget);
            }
            return visualRect(cb->direction, r, ret);
        }
        break;

    case CC_SpinBox:
        if (const auto *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            const QRect r = sb->rect;
            const int inset = sb->frame ? 1 : 0;
            const int h = r.height() - 2 * inset;
            const int upHeight = (h + 1) / 2;
            const int x = r.right() - inset - kSpinButtonWidth + 1;
            QRect ret;
            switch (sc) {
            case SC_SpinBoxUp:
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return {};
                ret = QRect(x, r.top() + inset, kSpinButtonWidth, upHeight);
                break;
            case SC_SpinBoxDown:
                if (sb->buttonSymbols == QAbstractSpinBox::NoButtons)
                    return {};
                ret = QRect(x, r.top() + inset + upHeight, kSpinButtonWidth, h - upHeight);
                break;
            case SC_SpinBoxEditField: {
                const int fw = sb->frame ? kFrameWidth + 1 : 0;
                const int right = sb->buttonSymbols == QAbstractSpinBox::NoButtons ? r.right() - fw : x - 1;
                ret = QRect(r.left() + fw, r.top() + fw, right - r.left() - fw + 1, r.height() - 2 * fw);
                break;
            }
            case SC_SpinBoxFrame:
                ret = r;
                break;
            default:
                return QCommonStyle::subControlRect(cc, opt, sc, widget);
            }
            return visualRect(sb->direction, r, ret);
        }
        break;

    case CC_ScrollBar:
        if (const auto *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            const QRect r = sb->rect;
            const bool horizontal = sb->orientation == Qt::Horizontal;
            const int length = horizontal ? r.width() : r.height();
            // Buttons are 16 long and share an outline row with the slider.
            const int button = std::min(kScrollButtonLength, length / 2);
            const QRect groove = scrollBarGroove(sb);
            const int grooveLength = std::max(0, horizontal ? groove.width() : groove.height());

            int sliderLength = grooveLength;
            if (sb->maximum > sb->minimum) {
                const qint64 range = qint64(sb->maximum) - sb->minimum;
                sliderLength = int((qint64(sb->pageStep) * grooveLength) / (range + sb->pageStep));
                const int minimum = pixelMetric(PM_ScrollBarSliderMin, sb, widget);
                if (sliderLength < minimum || range > INT_MAX / 2)
                    sliderLength = minimum;
                sliderLength = std::min(sliderLength, grooveLength);
            }
            const int start = sliderPositionFromValue(sb->minimum, sb->maximum, sb->sliderPosition,
                                                      grooveLength - sliderLength, sb->upsideDown);
            const int grooveStart = horizontal ? groove.left() : groove.top();

            QRect ret;
            switch (sc) {
            case SC_ScrollBarSubLine:
                ret = horizontal ? QRect(r.left(), r.top(), button, r.height())
                                 : QRect(r.left(), r.top(), r.width(), button);
                break;
            case SC_ScrollBarAddLine:
                ret = horizontal ? QRect(r.right() - button + 1, r.top(), button, r.height())
                                 : QRect(r.left(), r.bottom() - button + 1, r.width(), button);
                break;
            case SC_ScrollBarSubPage: {
                const int from = grooveStart + 1;
                const int to = grooveStart + start;
                ret = horizontal ? QRect(from, r.top(), to - from, r.height())
                                 : QRect(r.left(), from, r.width(), to - from);
                break;
            }
            case SC_ScrollBarAddPage: {
                const int from = grooveStart + start + sliderLength;
                const int to = grooveStart + grooveLength - 1;
                ret = horizontal ? QRect(from, r.top(), to - from, r.height())
                                 : QRect(r.left(), from, r.width(), to - from);
                break;
            }
            case SC_ScrollBarSlider:
                ret = horizontal ? QRect(grooveStart + start, r.top(), sliderLength, r.height())
                                 : QRect(r.left(), grooveStart + start, r.width(), sliderLength);
                break;
            case SC_ScrollBarGroove:
                ret = groove;
                break;
            default:
                return QCommonStyle::subControlRect(cc, opt, sc, widget);
            }
            return visualRect(sb->direction, r, ret);
        }
        break;

    default:
        break;
    }
    return QCommonStyle::subControlRect(cc, opt, sc, widget);
}

int QPhotonStyle::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *widget,
                            QStyleHintReturn *returnData) const
{
    switch (sh) {
    case SH_DialogButtonBox_ButtonsHaveIcons:
        return 0;
    case SH_Menu_SubMenuPopupDelay:
        return 256;
    case SH_ItemView_ShowDecorationSelected:
        return 1;
    case SH_ComboBox_Popup:
        return 0;
    case SH_Slider_SnapToValue:
        return 1;
    case SH_ScrollBar_ContextMenu:
        return 1;
    case SH_TitleBar_AutoRaise:
        return 1;
    case SH_EtchDisabledText:
        return 0;
    case SH_Menu_AllowActiveAndDisabled:
        return 0;
    case SH_MainWindow_SpaceBelowMenuBar:
        return 0;
    default:
        break;
    }
    return QCommonStyle::styleHint(sh, opt, widget, returnData);
}
