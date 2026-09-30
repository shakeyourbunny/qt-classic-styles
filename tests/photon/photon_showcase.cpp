// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

// Renders scenes shaped like the QNX 6.2.1 reference screenshots with the
// Photon style, for side-by-side comparison. Usage: photon_showcase OUTDIR

#include "qphotonstyle.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTest>
#include <QTextEdit>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <cstdio>
#include <memory>

namespace {

// The Cancel/Apply/Done strip at the bottom of Photon dialogs.
QWidget *buttonStrip(const QStringList &labels, int disabledIndex)
{
    auto *strip = new QFrame;
    strip->setAutoFillBackground(true);
    QPalette pal = strip->palette();
    pal.setColor(QPalette::Window, pal.color(QPalette::Mid));
    strip->setPalette(pal);
    auto *row = new QHBoxLayout(strip);
    row->setContentsMargins(6, 6, 6, 6);
    row->setSpacing(2);
    row->addStretch();
    for (int i = 0; i < labels.size(); ++i) {
        auto *b = new QPushButton(labels.at(i));
        b->setEnabled(i != disabledIndex);
        row->addWidget(b);
    }
    return strip;
}

QWidget *displaySettings()
{
    auto *dlg = new QWidget;
    auto *outer = new QVBoxLayout(dlg);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *tabs = new QTabWidget;
    auto *page = new QWidget;
    auto *grid = new QGridLayout(page);
    auto *scheme = new QComboBox;
    scheme->addItems({QStringLiteral("Custom"), QStringLiteral("Default")});
    grid->addWidget(new QLabel(QStringLiteral("Scheme:")), 0, 0, Qt::AlignRight);
    grid->addWidget(scheme, 0, 1, 1, 2);

    auto *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    grid->addWidget(line, 1, 0, 1, 3);

    auto *left = new QVBoxLayout;
    left->addWidget(new QLabel(QStringLiteral("Title Alignment:")));
    auto *l = new QRadioButton(QStringLiteral("Left"));
    auto *c = new QRadioButton(QStringLiteral("Center"));
    auto *r = new QRadioButton(QStringLiteral("Right"));
    c->setChecked(true);
    left->addWidget(l);
    left->addWidget(c);
    left->addWidget(r);
    grid->addLayout(left, 2, 0, 1, 2);

    auto *right = new QVBoxLayout;
    auto *drag = new QCheckBox(QStringLiteral("Full window dragging"));
    drag->setChecked(true);
    auto *focus = new QCheckBox(QStringLiteral("Cursor focus"));
    auto *front = new QCheckBox(QStringLiteral("Click to front"));
    front->setChecked(true);
    right->addWidget(drag);
    right->addWidget(focus);
    right->addWidget(front);
    right->addWidget(new QCheckBox(QStringLiteral("Multi-monitor placement")));
    grid->addLayout(right, 2, 2);

    tabs->addTab(page, QStringLiteral("Window"));
    tabs->addTab(new QWidget, QStringLiteral("Background"));
    outer->addWidget(tabs);
    outer->addWidget(buttonStrip({QStringLiteral("Cancel"), QStringLiteral("Apply"),
                                  QStringLiteral("Done")},
                                 1));
    dlg->resize(400, 330);
    return dlg;
}

QWidget *editor()
{
    auto *win = new QMainWindow;
    for (const char *m : {"File", "Edit", "Search", "Type", "Buffer", "Marker", "Help"})
        win->menuBar()->addMenu(QString::fromLatin1(m));

    auto *tb = win->addToolBar(QStringLiteral("main"));
    tb->setMovable(false);
    auto *back = tb->addAction(QStringLiteral("<"));
    back->setEnabled(false);
    tb->addAction(QStringLiteral(">"));
    auto *font = new QComboBox;
    font->addItem(QStringLiteral("fantasy"));
    font->setMinimumWidth(150);
    tb->addWidget(font);
    auto *size = new QComboBox;
    size->addItem(QStringLiteral("20"));
    tb->addWidget(size);
    tb->addSeparator();
    for (const char *t : {"Aa", "i", "B"}) {
        auto *b = new QToolButton;
        b->setText(QString::fromLatin1(t));
        b->setCheckable(true);
        b->setChecked(t[0] == 'i');
        b->setAutoRaise(true);
        tb->addWidget(b);
    }

    auto *text = new QTextEdit;
    text->setPlainText(QStringLiteral("QNX IS COOL"));
    win->setCentralWidget(text);

    auto *status = win->statusBar();
    status->addWidget(new QLabel(QStringLiteral("Line:")));
    auto *lineNo = new QLineEdit(QStringLiteral("1"));
    lineNo->setFixedWidth(60);
    lineNo->setAlignment(Qt::AlignRight);
    status->addWidget(lineNo);
    status->addWidget(new QLabel(QStringLiteral("Column: 12")));
    status->addWidget(new QLabel(QStringLiteral("Buffer: 1")));
    win->resize(520, 220);
    return win;
}

QWidget *fileManager()
{
    auto *w = new QWidget;
    auto *v = new QVBoxLayout(w);
    auto *path = new QHBoxLayout;
    path->addWidget(new QLabel(QStringLiteral("Path:")));
    auto *combo = new QComboBox;
    combo->setEditable(true);
    combo->setEditText(QStringLiteral("/root"));
    path->addWidget(combo, 1);
    v->addLayout(path);

    auto *row = new QHBoxLayout;
    auto *tree = new QTreeWidget;
    tree->setHeaderLabel(QStringLiteral("Bookmarks"));
    auto *root = new QTreeWidgetItem(tree, {QStringLiteral("/")});
    for (const char *n : {"lib", "local", "man"})
        new QTreeWidgetItem(root, {QString::fromLatin1(n)});
    auto *closed = new QTreeWidgetItem(tree, {QStringLiteral("usr")});
    new QTreeWidgetItem(closed, {QStringLiteral("share")});
    tree->expandItem(root);
    row->addWidget(tree);

    auto *list = new QTreeWidget;
    list->setRootIsDecorated(false);
    list->setHeaderLabels({QStringLiteral("Filename"), QStringLiteral("Size"),
                           QStringLiteral("Date"), QStringLiteral("Owner")});
    for (int i = 0; i < 30; ++i)
        new QTreeWidgetItem(list, {QStringLiteral("snap%1.png").arg(i), QStringLiteral("41,094"),
                                   QStringLiteral("01/11/2004"), QStringLiteral("root")});
    list->setCurrentItem(list->topLevelItem(0));
    row->addWidget(list, 2);
    v->addLayout(row);

    auto *bottom = new QHBoxLayout;
    auto *progress = new QProgressBar;
    progress->setValue(35);
    progress->setTextVisible(false);
    bottom->addWidget(progress);
    auto *slider = new QSlider(Qt::Horizontal);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(10);
    bottom->addWidget(slider);
    auto *spin = new QSpinBox;
    spin->setValue(5);
    bottom->addWidget(spin);
    v->addLayout(bottom);
    w->resize(520, 300);
    return w;
}

QWidget *tabRows()
{
    auto *w = new QWidget;
    auto *grid = new QGridLayout(w);
    const QStringList names = {QStringLiteral("Folders && Tabs"), QStringLiteral("Previews"),
                               QStringLiteral("Confirmations"), QStringLiteral("Panels"),
                               QStringLiteral("Status && Location bars")};
    const QTabWidget::TabPosition positions[] = {QTabWidget::North, QTabWidget::South,
                                                 QTabWidget::West, QTabWidget::East};
    for (int i = 0; i < 4; ++i) {
        auto *tabs = new QTabWidget;
        tabs->setTabPosition(positions[i]);
        for (const QString &n : names.mid(0, i < 2 ? 5 : 3))
            tabs->addTab(new QWidget, n);
        tabs->setCurrentIndex(1);
        grid->addWidget(tabs, i < 2 ? i : 2, i < 2 ? 0 : i - 2, 1, i < 2 ? 2 : 1);
    }
    w->resize(560, 520);
    return w;
}

QWidget *launchMenu()
{
    auto *menu = new QMenu;
    for (const char *t : {"MultiMedia", "Editors", "Utilities", "Internet", "Development"})
        menu->addMenu(QString::fromLatin1(t));
    menu->addSeparator();
    menu->addMenu(QStringLiteral("Software"));
    menu->addMenu(QStringLiteral("Configure"));
    menu->addAction(QStringLiteral("Help"));
    menu->addSeparator();
    auto *check = menu->addAction(QStringLiteral("Show clock"));
    check->setCheckable(true);
    check->setChecked(true);
    menu->addAction(QStringLiteral("Shutdown...\tCtrl+Q"));
    menu->setActiveAction(menu->actions().at(3));
    return menu;
}

bool save(QWidget *w, const QString &path)
{
    w->show();
    if (!QTest::qWaitForWindowExposed(w))
        std::fprintf(stderr, "not exposed: %s\n", qPrintable(path));
    QApplication::processEvents();
    return w->grab().save(path);
}

} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s OUTDIR\n", argv[0]);
        return 2;
    }
    const QDir out(QString::fromLocal8Bit(argv[1]));
    if (!out.exists()) {
        std::fprintf(stderr, "no such directory: %s\n", argv[1]);
        return 1;
    }
    QApplication::setStyle(new QPhotonStyle);
    QApplication::setPalette(QApplication::style()->standardPalette());
    QFont font(QStringLiteral("DejaVu Sans"));
    font.setPixelSize(12);
    QApplication::setFont(font);

    const struct
    {
        QWidget *(*make)();
        const char *name;
    } scenes[] = {
        {displaySettings, "photon-display.png"},
        {editor, "photon-editor.png"},
        {fileManager, "photon-fileman.png"},
        {launchMenu, "photon-menu.png"},
        {tabRows, "photon-tabs.png"},
    };
    int failures = 0;
    for (const auto &scene : scenes) {
        std::unique_ptr<QWidget> w(scene.make());
        if (!save(w.get(), out.filePath(QString::fromLatin1(scene.name)))) {
            std::fprintf(stderr, "could not save %s\n", scene.name);
            ++failures;
        }
    }
    return failures == 0 ? 0 : 1;
}
