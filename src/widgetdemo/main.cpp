// This file is part of the qt-classic-styles Project.
// License: LGPL-2.1-or-later. Contact: qt-classic-styles-project@trinity2k.net
//

// Interactive widget gallery for visual testing. Loads style plugins at
// runtime via QStyleFactory, so the build directory just needs to be on
// QT_PLUGIN_PATH.

#include "demooptions.h"

#include <cstdio>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDir>
#include <QDial>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSaveFile>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStyleFactory>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

// Without standardPalette the palette stays unset, so the new style can polish it.
static bool applyDemoStyle(const QString &name, bool standardPalette)
{
    QStyle *style = name.isEmpty() ? nullptr : QStyleFactory::create(name);
    if (!style)
        return false;
    QApplication::setStyle(style);
    if (standardPalette)
        QApplication::setPalette(QApplication::style()->standardPalette());
    return true;
}

// The initial style and palette are applied by the caller before this is
// constructed: an unshown top-level window has no QWindow, misses
// ApplicationPaletteChange and would keep the palette it was created with.
class StylesDemo : public QWidget
{
    Q_OBJECT

public:
    StylesDemo(const QPalette &originalPalette, bool standardPalette)
        : m_originalPalette(originalPalette), m_useStandardPalette(standardPalette)
    {
        setWindowTitle(QStringLiteral("Qt Classic Styles - %1").arg(QApplication::style()->name()));
        setMinimumSize(640, 480);

        auto *main = new QVBoxLayout(this);

        auto *top = new QHBoxLayout;
        top->addWidget(new QLabel(QStringLiteral("Style:")));
        m_styleCombo = new QComboBox;
        m_styleCombo->addItems(QStyleFactory::keys());
        connect(m_styleCombo, &QComboBox::currentTextChanged,
                this, &StylesDemo::changeStyle);
        top->addWidget(m_styleCombo);
        top->addSpacing(16);

        m_paletteCheck = new QCheckBox(QStringLiteral("Use style's standard palette"));
        connect(m_paletteCheck, &QCheckBox::toggled, this, &StylesDemo::changePalette);
        top->addWidget(m_paletteCheck);

        auto *disableCheck = new QCheckBox(QStringLiteral("Disable widgets"));
        connect(disableCheck, &QCheckBox::toggled, this, &StylesDemo::toggleDisabled);
        top->addWidget(disableCheck);

        top->addStretch();
        main->addLayout(top);

        auto *mid = new QHBoxLayout;

        auto *leftCol = new QVBoxLayout;

        m_group1 = new QGroupBox(QStringLiteral("Group 1"));
        auto *g1 = new QVBoxLayout(m_group1);
        auto *rb1 = new QRadioButton(QStringLiteral("Radio button 1"));
        rb1->setChecked(true);
        g1->addWidget(rb1);
        g1->addWidget(new QRadioButton(QStringLiteral("Radio button 2")));
        g1->addWidget(new QRadioButton(QStringLiteral("Radio button 3")));
        auto *tristate = new QCheckBox(QStringLiteral("Tri-state check box"));
        tristate->setTristate(true);
        tristate->setCheckState(Qt::PartiallyChecked);
        g1->addWidget(tristate);
        leftCol->addWidget(m_group1);

        m_tabs = new QTabWidget;
        auto *table = new QTableWidget(4, 3);
        table->setHorizontalHeaderLabels({QStringLiteral("A"), QStringLiteral("B"),
                                          QStringLiteral("C")});
        m_tabs->addTab(table, QStringLiteral("Table"));

        auto *text = new QTextEdit;
        text->setPlainText(QStringLiteral(
            "The white-noise floor stretches out around you, "
            "silent and vast. Somewhere beyond the edge, the "
            "real world waits with its keyboards and its coffee."));
        m_tabs->addTab(text, QStringLiteral("Text Edit"));
        leftCol->addWidget(m_tabs);

        mid->addLayout(leftCol);

        auto *rightCol = new QVBoxLayout;

        m_group2 = new QGroupBox(QStringLiteral("Group 2"));
        auto *g2 = new QVBoxLayout(m_group2);
        auto *defaultBtn = new QPushButton(QStringLiteral("Default Push Button"));
        defaultBtn->setDefault(true);
        g2->addWidget(defaultBtn);
        auto *toggleBtn = new QPushButton(QStringLiteral("Toggle Push Button"));
        toggleBtn->setCheckable(true);
        toggleBtn->setChecked(true);
        g2->addWidget(toggleBtn);
        auto *flatBtn = new QPushButton(QStringLiteral("Flat Push Button"));
        flatBtn->setFlat(true);
        g2->addWidget(flatBtn);
        rightCol->addWidget(m_group2);

        m_group3 = new QGroupBox(QStringLiteral("Group 3"));
        auto *g3 = new QVBoxLayout(m_group3);
        auto *password = new QLineEdit(QStringLiteral("secret"));
        password->setEchoMode(QLineEdit::Password);
        g3->addWidget(password);
        auto *spin = new QSpinBox;
        spin->setValue(50);
        g3->addWidget(spin);
        g3->addWidget(new QDateTimeEdit);

        auto *sliderRow = new QHBoxLayout;
        auto *hSlider = new QSlider(Qt::Horizontal);
        hSlider->setValue(40);
        sliderRow->addWidget(hSlider);
        auto *hScroll = new QScrollBar(Qt::Horizontal);
        hScroll->setRange(0, 100);
        hScroll->setValue(60);
        sliderRow->addWidget(hScroll);
        auto *dial = new QDial;
        dial->setNotchesVisible(true);
        dial->setValue(30);
        dial->setMaximumSize(60, 60);
        sliderRow->addWidget(dial);
        g3->addLayout(sliderRow);
        rightCol->addWidget(m_group3);

        rightCol->addStretch();
        mid->addLayout(rightCol);

        main->addLayout(mid);

        m_progress = new QProgressBar;
        m_progress->setValue(61);
        m_progress->setFormat(QStringLiteral("%p%"));
        main->addWidget(m_progress);

        // Show what is active; setting the controls must not apply anything again.
        const QSignalBlocker comboBlocker(m_styleCombo);
        const QSignalBlocker checkBlocker(m_paletteCheck);
        m_styleCombo->setCurrentIndex(
            m_styleCombo->findText(QApplication::style()->name(), Qt::MatchFixedString));
        m_paletteCheck->setChecked(m_useStandardPalette);
    }

private slots:
    void changeStyle(const QString &name)
    {
        if (applyDemoStyle(name, m_useStandardPalette)) {
            setWindowTitle(QStringLiteral("Qt Classic Styles - %1").arg(name));
            return;
        }
        qWarning("widgetdemo: cannot create style \"%s\"", qPrintable(name));
        // Keep the combo on the style that is still active.
        const QSignalBlocker blocker(m_styleCombo);
        m_styleCombo->setCurrentIndex(
            m_styleCombo->findText(QApplication::style()->name(), Qt::MatchFixedString));
    }

    void changePalette(bool useStandard)
    {
        m_useStandardPalette = useStandard;
        QApplication::setPalette(useStandard ? QApplication::style()->standardPalette()
                                             : m_originalPalette);
    }

    void toggleDisabled(bool disabled)
    {
        for (QWidget *w : {static_cast<QWidget *>(m_group1), static_cast<QWidget *>(m_group2),
                           static_cast<QWidget *>(m_group3), static_cast<QWidget *>(m_tabs),
                           static_cast<QWidget *>(m_progress)})
            w->setEnabled(!disabled);
    }

private:
    QComboBox *m_styleCombo = nullptr;
    QCheckBox *m_paletteCheck = nullptr;
    QGroupBox *m_group1 = nullptr;
    QGroupBox *m_group2 = nullptr;
    QGroupBox *m_group3 = nullptr;
    QTabWidget *m_tabs = nullptr;
    QProgressBar *m_progress = nullptr;
    QPalette m_originalPalette;
    bool m_useStandardPalette = false;
};

namespace {

constexpr int kExitError = 1;
constexpr int kExitUsage = 2;

void printTo(std::FILE *stream, const QString &text)
{
    std::fputs(text.toLocal8Bit().constData(), stream);
}

int takeScreenshots(const DemoOptions &options)
{
    if (!QDir().mkpath(options.outputDir)) {
        printTo(stderr, QStringLiteral("widgetdemo: cannot create %1\n").arg(options.outputDir));
        return kExitError;
    }
    const QDir dir(options.outputDir);
    const QPalette originalPalette = QApplication::palette();
    for (const QString &style : options.styles) {
        if (!applyDemoStyle(style, true)) {
            printTo(stderr, QStringLiteral("widgetdemo: cannot create style \"%1\"\n").arg(style));
            return kExitError;
        }
        StylesDemo demo(originalPalette, true);
        demo.resize(demo.sizeHint().expandedTo(demo.minimumSize()));
        const QImage image = demo.grab().toImage();

        const QString path = dir.filePath(style.toLower() + QStringLiteral(".png"));
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly) || !image.save(&file, "PNG") || !file.commit()) {
            printTo(stderr, QStringLiteral("widgetdemo: cannot write %1: %2\n")
                                .arg(path, file.errorString()));
            return kExitError;
        }
        printTo(stdout, QStringLiteral("wrote %1\n").arg(path));
    }
    return 0;
}

} // namespace

int main(int argc, char **argv)
{
    QStringList args;
    for (int i = 0; i < argc; ++i)
        args << QString::fromLocal8Bit(argv[i]);

    const DemoOptions options = parseDemoOptions(args);
    switch (options.mode) {
    case DemoOptions::Mode::Help:
        printTo(stdout, demoHelpText());
        return 0;
    case DemoOptions::Mode::Version:
        printTo(stdout, demoVersionText());
        return 0;
    case DemoOptions::Mode::UsageError:
        printTo(stderr, QStringLiteral("widgetdemo: %1\n\n").arg(options.error) + demoHelpText());
        return kExitUsage;
    case DemoOptions::Mode::Screenshot:
        // Screenshots never need a window; a set platform wins so tests can pick one.
        if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
            qputenv("QT_QPA_PLATFORM", "offscreen");
        break;
    case DemoOptions::Mode::Interactive:
        break;
    }

    QApplication app(argc, argv);
    const QStringList available = QStyleFactory::keys();
    for (const QString &style : options.styles) {
        if (!available.contains(style, Qt::CaseInsensitive)) {
            printTo(stderr, QStringLiteral("widgetdemo: no style \"%1\" (available: %2)\n")
                                .arg(style, available.join(QStringLiteral(", "))));
            return kExitError;
        }
    }
    if (options.mode == DemoOptions::Mode::Screenshot)
        return takeScreenshots(options);

    const QPalette originalPalette = QApplication::palette();
    if (!options.styles.isEmpty() && !applyDemoStyle(options.styles.first(), false)) {
        printTo(stderr, QStringLiteral("widgetdemo: cannot create style \"%1\"\n").arg(options.styles.first()));
        return kExitError;
    }
    StylesDemo demo(originalPalette, false);
    demo.show();
    return app.exec();
}

#include "main.moc"
