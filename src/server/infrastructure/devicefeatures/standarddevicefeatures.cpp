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

#include "standarddevicefeatures.h"
#include "qtopiaserverapplication.h"
#include "qtopiainputevents.h"

#include <qvaluespace.h>

#include <QDebug>
#include <QtopiaFeatures>
#include <QPowerStatus>

static bool clamFlipKeyMonitor = true;
static bool inputFeatures = true;



class StandardDeviceFeaturesImpl : public QObject, public QtopiaKeyboardFilter
{
    Q_OBJECT
public:
    StandardDeviceFeaturesImpl(QObject *parent=0);

    void disableClamshellMonitor();

    // key filter.
    bool filter(int unicode, int keycode, int modifiers, bool press,
                bool autoRepeat);

private:
    QValueSpaceItem *backlightVsi;
    QValueSpaceObject *clamshellVso;
    bool clamOpen;
};

static StandardDeviceFeaturesImpl *sdfi = 0;

StandardDeviceFeaturesImpl::StandardDeviceFeaturesImpl(QObject *parent)
: QObject(parent), clamshellVso(0), clamOpen(true)
{
    sdfi = this;

    if (clamFlipKeyMonitor) {
        QtopiaInputEvents::addKeyboardFilter(this);
        clamshellVso = new QValueSpaceObject("/Hardware/Devices");
        clamshellVso->setAttribute("ClamshellOpen", clamOpen);
    }

    if(inputFeatures) {
        QSettings btnCfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
        btnCfg.beginGroup("Device");
        QStringList ilist = btnCfg.value("Input").toString().split(',', QString::SkipEmptyParts);
        if (!ilist.isEmpty()) {
            for(int ii = 0; ii < ilist.count(); ++ii)
                QtopiaFeatures::setFeature(ilist.at(ii));
        } else {
            if (Qtopia::mousePreferred()) {
                //# ifdef QPE_NEED_CALIBRATION
                QtopiaFeatures::setFeature("Calibrate");
                //# endif
                QtopiaFeatures::setFeature("Touchscreen");
            } else {
                QtopiaFeatures::setFeature("Keypad");
            }
        }
    }
}


void StandardDeviceFeaturesImpl::disableClamshellMonitor()
{
    delete clamshellVso;
    clamshellVso = 0;
}

bool StandardDeviceFeaturesImpl::filter(int unicode, int keycode,
                                    int modifiers, bool press, bool autoRepeat)
{
    Q_UNUSED(unicode);
    Q_UNUSED(modifiers);
    Q_UNUSED(autoRepeat);

    if (clamFlipKeyMonitor && keycode == Qt::Key_Flip) {
        clamOpen = !press;
        clamshellVso->setAttribute("ClamshellOpen", clamOpen);
    }

    return false;
}

QTOPIA_TASK(StandardDeviceFeatures, StandardDeviceFeaturesImpl);

/*!
  \namespace StandardDeviceFeatures
  \ingroup QtopiaServer
    \inpublicgroup QtBaseModule
  \brief The StandardDeviceFeatures namespace contains methods to disable
  the standard device feature handling.

  The core Qt Extended sub-systems rely on various device specific status
  information being maintained in the value space.
  To ease customizability, all the simple hardware device monitoring
  in Qt Extended are collectively managed in one place - the
  \c {StandardDeviceFeatures} server task.

  To customize hardware device monitoring, an integrator needs only to implement their
  own version and disable the default behaviour through one of the methods
  available in the StandardDeviceFeatures namespace.

  The documentation for each of the following methods lists both what
  device it monitors, as well as the expected result when the device state
  changes.  The intent is to make the task of replicating each very simple.
 */

/*!
  The clamshell monitor is responsible monitoring when a clamshell style
  phone (flip phone) is opened or closed and updating the value space.
  The value that must be maintained is:

  \list
  \i /Hardware/Device/ClamshellOpen - true if the clamshell is in an open state.
  \endlist

  The default clamshell monitoring uses Qt::Key_Flip press events
  to indicate a closed state and Qt::Key_Flip release events to indicate
  an open state.

  Invoking this method will disable the default clamshell monitoring.
 */
void StandardDeviceFeatures::disableClamshellMonitor()
{
    clamFlipKeyMonitor = false;
    if (sdfi) sdfi->disableClamshellMonitor();
}

/*!
  The QtopiaFeatures class allows applications to query certain properties about
  the Qt Extended configuration.  Based on the configured input modes, the following
  features are set automatically:

  \table
  \header \o Feature \o Description
  \row \o \c {Touchscreen} \o The primary input for the device is via a
  touchscreen.  That is, \c {Qtopia::mousePreferred() == true}.
  \row \o \c {Calibrate} \o The touch screen device requires calibration.  This
  is set if \c {Touchscreen} is set.
  \row \o \c {KeyPad} \o The primary input for the device is via a keypad.  That
  is, \c {Qtopia::mousePreferred() == false}.
  \endtable.

  The device integrator can override these default features by setting the
  \c {Device/Input} value in the defaultbuttons configuration file.  The
  value should be a comma separated list of the features to set.  If \i {any}
  features are specified in this manner, the automatic features above are not
  set.

  Invoking this method will disable the setting of features automatically.
 */
void StandardDeviceFeatures::disableInputFeatures()
{
    Q_ASSERT(!sdfi && !"StandardDeviceFeatures::disableInputFeatures() must be called before the instantiation of the StandardDeviceFeatures task.");
    inputFeatures = false;
}

#include "standarddevicefeatures.moc"
