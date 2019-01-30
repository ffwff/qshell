#include <QBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QIcon>
#include <QApplication>
#include <QScreen>
#include <QScrollArea>

#include <X11/Xatom.h>
#include <QX11Info>
#include <X11/Xlib.h>
#include <fixx11h.h>

#include <KF5/KConfigCore/KConfigGroup>
#include <KF5/KIOWidgets/KRun>
#include <KF5/KService/KService>
#include <KF5/KService/KSycoca>
#include <KF5/KService/KSycocaEntry>
#include <KF5/KService/KToolInvocation>
#include <KF5/KService/KServiceGroup>
#include <KF5/KWindowSystem/KWindowSystem>

#include "model.h"
#include "shell.h"
#include "dash.h"
#include "desktop.h"
#include "frame.h"

Q::DashLabelContainer::DashLabelContainer(QWidget *parent) : QWidget(parent) {
    setLayout(new QHBoxLayout(this));
}

// ----------
Q::DashAppsContainer::DashAppsContainer(QWidget *parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);
    setLayout(layout);
}

// ----------

Q::DashItem::DashItem(QWidget *parent, const QString &name, const QIcon &icon,
                      const QString &command, const QString &tooltip,
                      const bool isTerminal, Dash* dash)
    : QPushButton(parent),
      myDash(dash),
      icon(icon) {

    if(isTerminal) {
        myCommand = "x-terminal-emulator";
        myArguments.append("-e");
        myArguments.append(command);
    } else {
        QStringList args = command.split(" ");
        myCommand = args.first();
        QStringList arguments(args);
        arguments.removeFirst();
        myArguments = arguments;
    }

    QGridLayout *layout = new QGridLayout(this);
    layout->setColumnStretch(1, 1);
    setLayout(layout);
    iconLabel = new QLabel();
    layout->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop);
    QLabel *nameLabel = new QLabel(name);
    nameLabel->setProperty("class", "nameLabel");
    layout->addWidget(nameLabel, 0, 1);
    QLabel *descLabel = new QLabel(tooltip);
    descLabel->setProperty("class", "descLabel");
    layout->addWidget(descLabel, 1, 1);

    setToolTip(tooltip);
}

// configs
void Q::DashItem::load(KConfigGroup *grp) {
    int size = grp->readEntry("Size", 48);
    setSize(size);
}

// icons
void Q::DashItem::setSize(int size) {
    mySize = QSize(size, size);
//     setIconSize(mySize);
    setMinimumSize(mySize);
    updatePixmapLabels();
}

void Q::DashItem::updatePixmapLabels() {
    const QPixmap pixmap = icon.pixmap(mySize);
    iconLabel->setPixmap(pixmap);
    iconLabel->setMinimumSize(mySize);
}

// mouse
void Q::DashItem::mouseReleaseEvent(QMouseEvent *) {
    runCommand();
}

// command
void Q::DashItem::runCommand() {
    qDebug() << myCommand;
    myProcess.startDetached(myCommand, myArguments);
    myDash->hide();
}

// ----------

Q::Dash::Dash(Shell *parent) : Q::Frame(parent), Model("Q::Dash", parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    mySearchBarContainer = new QWidget(this);
    mySearchBarContainer->setObjectName("Q--Dash-Search");
    mySearchBarContainer->setLayout(new QVBoxLayout(mySearchBarContainer));

    mySearchBar = new QLineEdit(mySearchBarContainer);
    mySearchBar->setFocusPolicy(Qt::StrongFocus);
    mySearchBar->setPlaceholderText("search for applications and programs...");
    mySearchBarContainer->layout()->addWidget(mySearchBar);

    QScrollArea *scrollArea = new QScrollArea(this);
    boxLayout()->addWidget(scrollArea);

    appsContainer = new QWidget(scrollArea);
    appsContainer->setObjectName("Q--Dash-Apps");
    appsContainer->setLayout(new QVBoxLayout(appsContainer));
    scrollArea->setWidget(appsContainer);
    scrollArea->setWidgetResizable(true);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
    setParent(shell());

    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged,
            this, &Q::Dash::activeWindowChanged);
}

// Misc
static QString &normalize(QString cmd) {
    return cmd
        .replace("%f", "", Qt::CaseInsensitive)
        .replace("%u", "", Qt::CaseInsensitive)
        .replace("%d", "", Qt::CaseInsensitive)
        .replace("%n", "", Qt::CaseInsensitive)
        .replace("%k", "")
        .replace("%v", "");
}

// Events
void Q::Dash::activeWindowChanged(WId id) {
    if(isVisible() && id != shell()->desktop()->winId())
        hide();
}

void Q::Dash::showEvent(QShowEvent *) {
    QSize geometry = QGuiApplication::primaryScreen()->size();
    if(myWidth && myHeight)
        resize(geometry.width() * (myWidth / 100), geometry.height() * (myHeight / 100));
    if(myPosition == DashPosition::TopLeft)
        move(shell()->getStrutLeft(), shell()->getStrutTop());
    else if (myPosition == DashPosition::TopRight)
        move(geometry.width() - shell()->getStrutRight() - sizeHint().width(), shell()->getStrutTop());
    else if (myPosition == DashPosition::BottomLeft)
        move(shell()->getStrutLeft(), geometry.height() - shell()->getStrutBottom() - height());
    else
        move(geometry.width() - shell()->getStrutRight() - sizeHint().width(), geometry.height() - shell()->getStrutBottom() - sizeHint().height());

    shell()->desktop()->activateWindow(); // HACK to activate
    activateWindow();
    KWindowSystem::setState(winId(), NET::SkipTaskbar);

    Display *display = QX11Info::display();
    Atom atom = XInternAtom(display, "_KDE_SLIDE", false);

    QVarLengthArray<long, 1024> data(4);

    if(myPosition == DashPosition::TopLeft || myPosition == DashPosition::BottomLeft)
        data[0] = shell()->getStrutLeft();
    else
        data[0] = shell()->getStrutRight();
    data[1] = mySlidePosition;
    data[2] = 200;
    data[3] = 200;

    XChangeProperty(display, winId(), atom, atom, 32, PropModeReplace,
             reinterpret_cast<unsigned char *>(data.data()), data.size());

    mySearchBar->setFocus();
}

// Configurations
void Q::Dash::load( KConfigGroup *grp ) {
    iconSize = grp->readEntry("IconSize", 48);
    mySearchBar->setVisible(grp->readEntry("ShowSearch", true));
    myPosition = (DashPosition)grp->readEntry("Position", 0);
    myWidth = grp->readEntry("Width", 0);
    myHeight = grp->readEntry("Height", 0);
    mySlidePosition = grp->readEntry("SlidePosition", 0);
    bool searchBelow = grp->readEntry("SearchBelow", false);
    if(searchBelow)
        boxLayout()->addWidget(mySearchBarContainer);
    else
        boxLayout()->insertWidget(0, mySearchBarContainer);

    connect ( mySearchBar, &QLineEdit::returnPressed, [this](){
        if(!items.isEmpty()) items.first()->runCommand();
    });
    connect ( mySearchBar, SIGNAL(textChanged(QString)), this, SLOT(setSearch(QString)));
    connect ( KSycoca::self(), SIGNAL(databaseChanged()), this, SLOT(slotRepopulate()));
    connect ( KSycoca::self(), SIGNAL(databaseChanged(const QStringList&)), this, SLOT(slotRepopulate()));

    slotRepopulate();
}

void Q::Dash::save(KConfigGroup *) {
}

// Populate apps
void Q::Dash::slotRepopulate() {
    QLayoutItem* item;
    while ( ( item = appsLayout()->takeAt( 0 ) ) != NULL )
    {
        delete item->widget();
        delete item;
    }
    items.clear();
    if(search.isEmpty())
        repopulate(KServiceGroup::root());
    else
        repopulate(KServiceGroup::root(), 0, search);
    static_cast<QBoxLayout*>(appsLayout())->addStretch(1);
}

bool Q::Dash::repopulate( KServiceGroup::Ptr group, QLayout *layout, const QString &filter ) {
    if (!group || !group->isValid())
        return 0;

    bool ret = false;
    KServiceGroup::List list = group->entries(true /* sorted */, true /* excludeNoDisplay */,
            true /* allowSeparators */, false /* sortByGenericName */);

    for (KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);
        if (p->isType(KST_KService)) {
            KService::Ptr a(static_cast<KService*>(p.data()));
            if( a->isApplication() && layout ) {
                if(filter != 0 && !a->name().contains(filter, Qt::CaseInsensitive)) {
                    continue;
                }
                DashItem *item = new DashItem(
                    layout->parentWidget(),
                    a->name(),
                    QIcon::fromTheme(a->icon()),
                    !a->actions().isEmpty() ?
                    a->actions().first().exec() :
                    !a->property("TryExec").toString().isEmpty() ?
                    a->property("TryExec").toString() :
                    normalize(a->exec()).simplified(),
                    a->comment().isEmpty() ? a->name() : a->name() + ": " + a->comment(),
                    a->terminal(),
                    this
                );
                item->setSize(iconSize);
                layout->addWidget(item);
                items << item;
                ret = true;
            }
            else
                qDebug() << "Dunno here" << p->entryPath();
        } else if (p->isType(KST_KServiceGroup)) {
            KServiceGroup::Ptr g(static_cast<KServiceGroup*>(p.data()));
            g->setShowEmptyMenu(false);
            if( g->entries(true,true).count() != 0 ) {
                DashLabelContainer *container = new DashLabelContainer(appsLayout()->parentWidget());
                appsLayout()->addWidget(container);

                QLabel *icon = new QLabel(container);
                icon->setPixmap(QIcon::fromTheme(g->icon()).pixmap(24));
                container->layout()->addWidget(icon);
                container->layout()->setAlignment(icon, Qt::AlignVCenter);
                QLabel *item = new QLabel(g->caption(), container);
                container->layout()->addWidget(item);
                container->layout()->setAlignment(item, Qt::AlignVCenter);
                static_cast<QHBoxLayout*>(container->layout())->addStretch(1);

                DashAppsContainer *widget;
                if(layout)
                    widget = new DashAppsContainer(layout->parentWidget());
                else
                    widget = new DashAppsContainer(appsLayout()->parentWidget());

                appsLayout()->addWidget(widget);
                if (repopulate(g, static_cast<QLayout*>(widget->layout()), search))
                    ret = true;
                else {
                    delete container;
                    delete widget;
                }
            }
        }
        else
            qDebug() << "Dunno" << p->entryPath();
    }
    return ret;
}

// Search
void Q::Dash::setSearch(const QString &s) {
    search = s;
    slotRepopulate();
}

// ----------

Q::DashButton::DashButton(const QString &name, Shell *parent)
    : QPushButton(static_cast<QWidget *>(parent)),
      Q::Model(name, parent) {
}

void Q::DashButton::load(KConfigGroup *grp) {
    const QString &icon = grp->readEntry("Icon", "kde");
    if(!icon.isEmpty())
        setIcon(QIcon::fromTheme(icon));
    const QString &text = grp->readEntry("Text", "");
    if(!text.isEmpty())
        setText(text);
    const int s = grp->readEntry("Size", 0);
    if(s) {
        size = QSize(s, s);
        setIconSize(size);
        setMinimumSize(size);
    }
}

void Q::DashButton::mouseReleaseEvent(QMouseEvent *) {
    shell()->dash()->setVisible(!shell()->dash()->isVisible());
}
