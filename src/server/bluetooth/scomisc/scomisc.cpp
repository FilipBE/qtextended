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

#ifdef HAVE_ALSA

#include <qtopialog.h>
#include <qbluetoothaddress.h>
#include <private/qbluetoothnamespace_p.h>
#include <qbluetoothnamespace.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#include <alsa/asoundlib.h>

#define SNDRV_BT_SCO_IOCTL_SET_SCO_SOCKET _IOW ('H', 0x10, int)

#ifndef SND_HWDEP_IFACE_EMUX_WAVETABLE
#define SND_HWDEP_IFACE_EMUX_WAVETABLE (SND_HWDEP_IFACE_USX2Y + 1)
#endif

#ifndef SND_HWDEP_IFACE_BLUETOOTH
#define SND_HWDEP_IFACE_BLUETOOTH (SND_HWDEP_IFACE_EMUX_WAVETABLE + 1)
#endif

#ifndef SNDRV_HWDEP_IFACE_BT_SCO
#define SNDRV_HWDEP_IFACE_BT_SCO (SND_HWDEP_IFACE_BLUETOOTH + 1)
#endif

bool bt_sco_set_fd(void *handle, int sco_fd)
{
    snd_hwdep_t *hnd = reinterpret_cast<snd_hwdep_t *>(handle);

    if (snd_hwdep_ioctl(hnd, SNDRV_BT_SCO_IOCTL_SET_SCO_SOCKET, (void *) sco_fd) < 0) {
        qWarning("Unable to set SCO fd!");
        return false;
    }

    return true;
}

QByteArray find_btsco_device(const QByteArray &idPref = QByteArray())
{
    snd_ctl_t *ctl_handle = 0;

    // These next two will be freed automagically,
    // space is allocated on the stack
    snd_ctl_card_info_t *card_info;
    snd_ctl_card_info_alloca(&card_info);

    snd_hwdep_info_t *hwdep_info;
    snd_hwdep_info_alloca(&hwdep_info);

    for (int card = 0; card < 7; card++) {
        char card_id[32];
        // Check subdevices
        int dev = -1;
        int if_type = 0;

        sprintf(card_id, "hw:%i", card);
        if (snd_ctl_open(&ctl_handle, card_id, 0) < 0) {
            goto next;
        }

        // Read control hardware info from card
        if (snd_ctl_card_info(ctl_handle, card_info) < 0) {
            goto next;
        }

        if (snd_ctl_hwdep_next_device(ctl_handle, &dev) < 0) {
            goto next;
        }

        snd_hwdep_info_set_device(hwdep_info, dev);

        if (snd_ctl_hwdep_info(ctl_handle, hwdep_info) < 0) {
            goto next;
        }

        if_type = snd_hwdep_info_get_iface(hwdep_info);
        if (if_type == SNDRV_HWDEP_IFACE_BT_SCO || if_type==12) {
            // Final check to make sure its the one we're looking for
            if (!idPref.isEmpty()) {
                const char *card_str_id = snd_ctl_card_info_get_id(card_info);
                if (idPref != QByteArray(card_str_id)) {
                    goto next;
                }
            }

            snd_ctl_close(ctl_handle);
            char id[32];

            sprintf(id, "hw:%i,%i", card, dev);
            QByteArray ret(id);
            return ret;
        }

next:
        if (ctl_handle) {
            snd_ctl_close(ctl_handle);
            ctl_handle = 0;
        }
    }

    return QByteArray();
}

void bt_sco_close(void *handle)
{
    snd_hwdep_close(reinterpret_cast<snd_hwdep_t *>(handle));
}

bool bt_sco_open(void **handle, const char *audioDev)
{
    return snd_hwdep_open(reinterpret_cast<snd_hwdep_t **>(handle), audioDev, O_RDWR) >= 0;
}

#else
#include <QByteArray>

bool bt_sco_set_fd(void *, int)
{
    return false;
}

QByteArray find_btsco_device(const QByteArray &idPref = QByteArray())
{
    return QByteArray();
}

void bt_sco_close(void *)
{
}

bool bt_sco_open(void **, const char *)
{
    return false;
}
#endif
