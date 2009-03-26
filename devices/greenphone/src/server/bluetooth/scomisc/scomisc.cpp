/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifdef QTOPIA_BLUETOOTH

#include <qtopiaipcadaptor.h>
#include <qtopialog.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include <QObject>
#include <QSocketNotifier>
#include <QBluetoothAudioGateway>

/*
    The purpose of this class is to make sure that when the other side has closed the SCO connection,
    the gateway is notified.  This can happen when e.g. the headset transfer to Audio Gateway button
    is pressed.  The class assumes that we have a PCM sco connection, e.g. the only select notification
    we will receive is when the socket is closed.

    The reason we operate on this socket is because the HF/HS AudioGateway service implementation hands
    us a bare SCO socket file descriptor.
*/

class ScoListener : public QObject
{
    Q_OBJECT
public:
    ScoListener(int fd, QObject *parent = 0);
    ~ScoListener();

    void setFd(int fd);

private slots:
    void socketClosed(int fd);

private:
    QList<QBluetoothAudioGateway *> m_audioGateways;
    QSocketNotifier *m_notifier;
};

ScoListener::ScoListener(int fd, QObject *parent)
    : QObject(parent)
{
    QBluetoothAudioGateway *hf = new QBluetoothAudioGateway("BluetoothHandsfree");
    m_audioGateways.append(hf);

    QBluetoothAudioGateway *hs = new QBluetoothAudioGateway("BluetoothHeadset");
    m_audioGateways.append(hs);

    m_notifier = new QSocketNotifier(fd, QSocketNotifier::Read);
    connect(m_notifier, SIGNAL(activated(int)), this, SLOT(socketClosed(int)));
}

ScoListener::~ScoListener()
{
    for (int i = 0; i < m_audioGateways.size(); i++) {
        delete m_audioGateways.at(i);
    }

    delete m_notifier;
}

/*
    In this function we detect that the socket has been closed.  The Audio Gateways do not
    manage this socket, so we need to notify them to release audio and update their state
*/
void ScoListener::socketClosed(int fd)
{
    qLog(Bluetooth) << "ScoListener::socketClosed was called with fd:" << fd;

    char buf[64];
    ssize_t r = read(fd, buf, 64);
    
    qLog(Bluetooth) << "read returned:" << r;
    if (r <= 0) {
        m_notifier->setEnabled(false);
        qLog(Bluetooth) << "Sco Socket was closed!";
        if (r == -1) {
            qLog(Bluetooth) << strerror(errno);
        }
        for (int i = 0; i < m_audioGateways.size(); i++) {
            if (m_audioGateways[i]->audioEnabled()) {
                m_audioGateways[i]->releaseAudio();
                return;
            }
        }
    }
}

struct btsco_handle
{
    ScoListener *listener;
};

void bt_sco_close(void *handle)
{
    btsco_handle *h = reinterpret_cast<btsco_handle *>(handle);
    delete h->listener;
    delete h;
}

/*
    This function initializes a new bt_sco handle.
*/
bool bt_sco_open(void **handle, const char *)
{
    btsco_handle *h = new btsco_handle;
    h->listener = NULL;

    *handle = reinterpret_cast<void *>(h);
    return true;
}

/*
    This function is called whenever a file descriptor for the bt_sco
    handle is set or unset.  If the fd is -1, then it is unset (e.g.
    audio should not be routed to the bluetooth headset device,
    otherwise it is set and the audio should be routed.  We perform
    the necessary Greenphone hardware ioctls to set the audio
    routing to the correct configuration.
*/
bool bt_sco_set_fd(void *handle, int fd)
{
    btsco_handle *h = reinterpret_cast<btsco_handle *>(handle);

    if (fd == -1) {
        delete h->listener;
        h->listener = NULL;
    }
    else {
        h->listener = new ScoListener(fd);
        qLog(Bluetooth) << "Created a new ScoListener";
    }

    return true;
}

/*
    Find a btsco device.  Return an empty string if no audio routing
    capabilities exist and a non-empty byte array otherwise.  This
    will be passed to bt_sco_open in order to actually open the device.
*/
QByteArray find_btsco_device(const QByteArray &idPref = QByteArray())
{
    Q_UNUSED(idPref);
    return QByteArray("default");
}

#include "scomisc.moc"

#endif
