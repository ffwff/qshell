// shamelessly reused my own code
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

Q::DashItem::DashItem(QWidget *parent, QString name, QIcon icon, QString command, QString tooltip, bool isTerminal, Dash* dash) :
QPushButton(parent),
myDash(dash)
{
    if(isTerminal)
    {
        myCommand = "x-terminal-emulator";
        myArguments.append("-e");
        myArguments.append(command);
    }
    else
    {
        QStringList args = command.split(" ");
        myCommand = args.first();
        QStringList arguments(args);
        arguments.removeFirst();
        myArguments = arguments;
    }
    
    setIcon(icon);
    setToolTip(tooltip);
};

// configs
void Q::DashItem::load(KConfigGroup *grp)
{
    int size = grp->readEntry("Size", 48);
    mySize = QSize(size, size);
    setIconSize(mySize);
    setMinimumSize(mySize);
};

// icons
void Q::DashItem::setSize(int size)
{
    mySize = QSize(size, size);
    setIconSize(mySize);
    setMinimumSize(mySize);
};

// mouse
void Q::DashItem::mouseReleaseEvent(QMouseEvent *)
{
    runCommand();
    myDash->hide();
};

// command
void Q::DashItem::runCommand()
{
    qDebug() << myCommand;
    myProcess.startDetached(myCommand, myArguments);
};

// ----------

Q::Dash::Dash(Shell *parent) : Q::Frame(parent), Model("Q::Dash", parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
    
    searchBarContainer = new QWidget();
    searchBarContainer->setObjectName("Q--Dash-Search");
    searchBarContainer->setLayout(new QVBoxLayout(searchBarContainer));
    boxLayout()->addWidget(searchBarContainer);
    
    searchBar = new QLineEdit(searchBarContainer);
    searchBar->setPlaceholderText("search for applications and programs...");
    searchBarContainer->layout()->addWidget(searchBar);
    
    QScrollArea *scrollArea = new QScrollArea();
    boxLayout()->addWidget(scrollArea);
    
    appsContainer = new QWidget(scrollArea);
    appsContainer->setObjectName("Q--Dash-Apps");
    appsContainer->setLayout(new QVBoxLayout(appsContainer));
    scrollArea->setWidget(appsContainer);
    scrollArea->setWidgetResizable(true);
    
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
    setParent(shell());
    
    connect ( searchBar, SIGNAL(textChanged(QString)), this, SLOT(setSearch(QString)));
    connect ( KSycoca::self(), SIGNAL(databaseChanged()), this, SLOT(slotRepopulate()));
    connect ( KSycoca::self(), SIGNAL(databaseChanged(const QStringList&)), this, SLOT(slotRepopulate()));
    connect(KWindowSystem::self(), &KWindowSystem::activeWindowChanged, [this](WId id) {
        if(isVisible() && id != shell()->desktop()->winId())
            hide();
    });
};

// Misc
QString &normalize(QString cmd)
{
    return cmd
        .replace("%f", "", Qt::CaseInsensitive)
        .replace("%u", "", Qt::CaseInsensitive)
        .replace("%d", "", Qt::CaseInsensitive)
        .replace("%n", "", Qt::CaseInsensitive)
        .replace("%k", "")
        .replace("%v", "");
};

// Events
void Q::Dash::showEvent(QShowEvent *)
{
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
    KWindowSystem::setState(winId(), NET::SkipTaskbar);
    
    Display *display = QX11Info::display();
    Atom atom = XInternAtom(display, "_KDE_SLIDE", false);

    QVarLengthArray<long, 1024> data(4);

    data[0] = shell()->getStrutLeft();
    data[1] = mySlidePosition;
    data[2] = 200;
    data[3] = 200;
    
     XChangeProperty(display, winId(), atom, atom, 32, PropModeReplace,
             reinterpret_cast<unsigned char *>(data.data()), data.size());
};

// Configurations
void Q::Dash::load( KConfigGroup *grp )
{
    iconSize = grp->readEntry("IconSize", 48);
    searchBar->setVisible(grp->readEntry("ShowSearch", true));
    myPosition = (DashPosition)grp->readEntry("Position", 0);
    myWidth = grp->readEntry("Width", 0);
    myHeight = grp->readEntry("Height", 0);
    mySlidePosition = grp->readEntry("SlidePositition", 0);
    
    slotRepopulate();
};

void Q::Dash::save( KConfigGroup *grp )
{
};

// Populate apps
void Q::Dash::slotRepopulate()
{
    QLayoutItem* item;
    while ( ( item = appsLayout()->takeAt( 0 ) ) != NULL )
    {
        delete item->widget();
        delete item;
    }
    if(search.isEmpty())
        repopulate(KServiceGroup::root());
    else
        repopulate(KServiceGroup::root(), 0, search);
    appsLayout()->addStretch();
};

bool Q::Dash::repopulate( KServiceGroup::Ptr group, QHBoxLayout *layout, const QString &filter )
{
    if (!group || !group->isValid())
        return 0;
    
    bool ret = false;
    KServiceGroup::List list = group->entries(true /* sorted */, true /* excludeNoDisplay */,
            true /* allowSeparators */, false /* sortByGenericName */);

    for (KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); it++) {
        const KSycocaEntry::Ptr p = (*it);
        if (p->isType(KST_KService))
        {
            KService::Ptr a(static_cast<KService*>(p.data()));
            if( a->isApplication() && layout )
            {
                if(filter != 0 && !a->name().contains(filter, Qt::CaseInsensitive) && !a->comment().contains(filter, Qt::CaseInsensitive))
                {
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
                ret = true;
            }
            else
                qDebug() << "Dunno here" << p->entryPath();
        }
        else if (p->isType(KST_KServiceGroup))
        {
            KServiceGroup::Ptr g(static_cast<KServiceGroup*>(p.data()));
            g->setShowEmptyMenu(false);
            if( g->entries(true,true).count() != 0 )
            {
                QLabel *item = new QLabel(g->caption());
                appsLayout()->addWidget(item);
                QWidget *widget = new QWidget();
                QHBoxLayout *layout = new QHBoxLayout();
                widget->setLayout(layout);
                appsLayout()->addWidget(widget);
                if (repopulate(g, layout, search))
                    ret = true;
                else {
                    delete item;
                    delete widget;
                }
            }
        }
        else
            qDebug() << "Dunno" << p->entryPath();
    }
    if (layout)
        layout->addStretch(1);
    return ret;
};

// Search
void Q::Dash::setSearch(QString s)
{
    search = s;
    slotRepopulate();
};

// ----------
Q::DashButton::DashButton(const QString &name, Shell *parent) :
QPushButton(static_cast<QWidget *>(parent)),
Q::Model(name, parent),
mySize(QSize(48,48))
{
    setIconSize(mySize);
    setMinimumSize(mySize);
};

void Q::DashButton::load(KConfigGroup *grp)
{
    setIcon(QIcon::fromTheme(grp->readEntry("Icon", "kde")));
    int size = grp->readEntry("Size", 48);
    mySize = QSize(size, size);
    setIconSize(mySize);
    setMinimumSize(mySize);
};

void Q::DashButton::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    shell()->dash()->setVisible(!shell()->dash()->isVisible());
};
