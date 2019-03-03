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

Q::DashLabelContainer::DashLabelContainer(const QString &iconName, const QString &caption,
                                        DashAppsContainer *appsContainer, QWidget *parent)
    : QWidget(parent), appsContainer(appsContainer) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    setLayout(layout);

    icon.setPixmap(QIcon::fromTheme(iconName).pixmap(24));
    layout->addWidget(&icon);
    item.setText(caption);
    layout->addWidget(&item);
    layout->addStretch();
}

void Q::DashLabelContainer::mouseReleaseEvent(QMouseEvent *) {
    appsContainer->setVisible(!appsContainer->isVisible());
}

// ----------
Q::DashAppsContainer::DashAppsContainer(QWidget *parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);
    setLayout(layout);
}

// ----------

Q::DashItem::DashItem(QWidget *parent, const QString &name, const QIcon &icon,
                      QStringList arguments, const QString &tooltip,
                      Dash* dash)
    : QPushButton(parent),
      myDash(dash),
      icon(icon) {

    myCommand = arguments.first();
    myArguments = arguments;
    myArguments.removeFirst();

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

// Events
void Q::Dash::activeWindowChanged(WId id) {
    if(noCheckActive) {
        noCheckActive = false;
        return;
    }
    if(isVisible() && id != shell()->desktop()->winId())
        hide();
}

void Q::Dash::showEvent(QShowEvent *) {
    const QSize geo = QGuiApplication::primaryScreen()->size();
    if(myWidth && myHeight)
        resize(geo.width() * (myWidth / 100), geo.height() * (myHeight / 100));\
    if(offsetTop != -1 && offsetLeft != -1) { // TODO
    	move(offsetLeft, offsetTop);
    } else {
        if(myPosition == DashPosition::TopLeft)
            move(shell()->getStrutLeft(), shell()->getStrutTop());
        else if (myPosition == DashPosition::TopRight)
            move(geo.width() - shell()->getStrutRight() - sizeHint().width(), shell()->getStrutTop());
        else if (myPosition == DashPosition::BottomLeft)
            move(shell()->getStrutLeft(), geo.height() - shell()->getStrutBottom() - height());
        else
            move(geo.width() - shell()->getStrutRight() - sizeHint().width(), geo.height() - shell()->getStrutBottom() - sizeHint().height());
	}

    shell()->desktop()->activateWindow(); // HACK to activate for kwin
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
    noCheckActive = true;
}

void Q::Dash::hideEvent(QHideEvent *) {
    KWindowSystem::forceActiveWindow(0);
    noCheckActive = true;
}

// Configurations
void Q::Dash::load( KConfigGroup *grp ) {
    iconSize = grp->readEntry("IconSize", 48);
    mySearchBar->setVisible(grp->readEntry("ShowSearch", true));
    myPosition = (DashPosition)grp->readEntry("Position", 0);
    mySlidePosition = grp->readEntry("SlidePosition", 0);
    myWidth = grp->readEntry("Width", 0);
    myHeight = grp->readEntry("Height", 0);
    offsetTop = grp->readEntry("OffsetTop", -1);
    offsetLeft = grp->readEntry("OffsetLeft", -1);
    bool searchBelow = grp->readEntry("SearchBelow", false);
    if(searchBelow)
        boxLayout()->addWidget(mySearchBarContainer);
    else
        boxLayout()->insertWidget(0, mySearchBarContainer);
    bool wmManage = grp->readEntry("WmManage", true);
    if(!wmManage)
	    setWindowFlags(windowFlags() | Qt::X11BypassWindowManagerHint);

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

    KServiceGroup::List list = group->entries(true /* sorted */, true /* excludeNoDisplay */,
            false /* allowSeparators */, false /* sortByGenericName */);

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
                    QStringList({ "dex", p->entryPath() }),
                    a->comment().isEmpty() ? a->name() : a->name() + ": " + a->comment(),
                    this
                );
                item->setSize(iconSize);
                layout->addWidget(item);
                items << item;
            }
            else
                qDebug() << "Dunno here" << p->entryPath();
        } else if (p->isType(KST_KServiceGroup)) {
            KServiceGroup::Ptr g(static_cast<KServiceGroup*>(p.data()));
            g->setShowEmptyMenu(false);
            if( g->entries(true,true).count() != 0 ) {
                DashAppsContainer *widget;
                if(layout)
                    widget = new DashAppsContainer(layout->parentWidget());
                else
                    widget = new DashAppsContainer(appsLayout()->parentWidget());

                DashLabelContainer *container = new DashLabelContainer(
                    g->icon(), g->caption(),
                    widget, appsLayout()->parentWidget());
                appsLayout()->addWidget(container);

                appsLayout()->addWidget(widget);
                if (!repopulate(g, static_cast<QLayout*>(widget->layout()), search)) {
                    delete container;
                    delete widget;
                }
            }
        }
        else
            qDebug() << "Dunno" << p->entryPath();
    }
    if(layout && !layout->count())
        return false;
    return true;
}

// Search
void Q::Dash::setSearch(const QString &s) {
    search = s;
    slotRepopulate();
}

// ----------

Q::DashButton::DashButton(const QString &name, Shell *parent)
    : QPushButton(), Q::Model(name, parent) {
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
    qDebug() << KWindowSystem::activeWindow() << shell()->dash()->winId() << shell()->desktop()->winId() << parentWidget()->winId();
    if(shell()->dash()->deactive()) {
        // HACK: the dash will automatically hide when a panel is activated
        shell()->dash()->hide();
        shell()->dash()->activeWindowChanged(0); // force deactivate
    }
    else shell()->dash()->show();
}
