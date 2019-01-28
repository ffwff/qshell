// TODO port me to appindicator/kdestatusnotifier

#include <QAbstractEventDispatcher>
#include <QWidget>
#include <QByteArray>
#include <QDebug>

#include <QX11Info>
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <fixx11h.h>

#include "systray.h"
#include "shell.h"
#include "model.h"

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

bool Q::SystrayEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* ev = static_cast<xcb_generic_event_t *>(message);
        qDebug()<<ev->response_type ;
        if(ev->response_type == XCB_CLIENT_MESSAGE)
        {
            xcb_client_message_event_t* cev = (xcb_client_message_event_t*)ev;
            qDebug() << "XCB"<< cev->window;
        }
    }
    return false;
};

// ---------

Atom net_system_tray_selection,
     net_system_tray_opcode,
     net_message_data_atom;

Q::Systray::Systray(const QString &name, Q::Shell *shell) : QWidget(), Model(name, shell)
{
//     qDebug() << "SYSTRAY";
//         
//     Display *display = QX11Info::display();
//     
//     char trayatom[20] = {0};
//     qsnprintf(trayatom, 20, "_NET_SYSTEM_TRAY_S%d", DefaultScreen(display));
//     
//     net_system_tray_selection = XInternAtom(display, trayatom, False);
//     net_system_tray_opcode = XInternAtom(display, "_NET_SYSTEM_TRAY_OPCODE", False);
//     net_message_data_atom = XInternAtom(display, "_NET_SYSTEM_TRAY_MESSAGE_DATA", False);
//     
//     // Acquire system tray
//     XSetSelectionOwner(display,
//                        net_system_tray_selection,
//                        effectiveWinId(),
//                        CurrentTime);
// 
//     WId root = QX11Info::appRootWindow();
//     
//     if (XGetSelectionOwner(display, net_system_tray_selection) == effectiveWinId()) {
//         XClientMessageEvent xev;
// 
//         xev.type = ClientMessage;
//         xev.window = root;
// 
//         xev.message_type = XInternAtom(display, "MANAGER", False);
//         xev.format = 32;
//         xev.data.l[0] = CurrentTime;
//         xev.data.l[1] = net_system_tray_selection;
//         xev.data.l[2] = effectiveWinId();
//         xev.data.l[3] = 0;       /* Manager specific data */
//         xev.data.l[4] = 0;       /* Manager specific data */
// 
//         XSendEvent(display, root, False, StructureNotifyMask, (XEvent *)&xev);
//     }
//     else
//     {
//         qDebug() <<"SAD";
//     }
//     
//     QAbstractEventDispatcher::instance()->installNativeEventFilter(new SystrayEventFilter());
};
