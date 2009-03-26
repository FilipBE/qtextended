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

#include <QStringList>
#include "qcameracontrol.h"

class ControlData {
public:
    QCameraControl::CanonicalName name;
    QCameraControl::GUIElement gui;
    QString custom;
    quint32 id;
    QString description;
    int min;
    int max;
    int step;
    QStringList value_strings;
    int default_value_index;

};


/*!
    \class QCameraControl
    \inpublicgroup QtMediaModule
    \brief The QCameraControl class represents a user controlled feature on the camera device
    e.g Saturation, Brightness
*/

/*!
    The parameter given by \a canonicalName holds a common indentifier to a canonical setting e.g Brightness. If a control has no canonical name e.g  the control is a custom device filter for Sharpness
    then \a customName provides a description string to it.
    Parameter \a id must be a unique identifier that is control specific.
    The parameter \a description is a user readable string in a GUI context.
    The control takes values between [ \a min and \a max in stride length \a step
    The value \a defaultValueIndex holds the initial default index.
    If the control represents a a multiple choice option then \a valueStrings holds the user readable strings
    that are parameterized by the values in the range [\a min, \a max].
    The GUI element that represents this control e.g Slider,Combobox is given by \a guiElement
*/

QCameraControl::QCameraControl( QCameraControl::CanonicalName canonicalName, quint32 id,
                                QString& description, int min,
                                int max, int step, int defaultValueIndex,
                                QStringList& valueStrings,
                                QCameraControl::GUIElement guiElement, QString customName)
:m_d(new ControlData)
{
    m_d->gui = guiElement;
    m_d->id = id;
    m_d->description = description;
    m_d->min = min;
    m_d->max = max;
    m_d->step = step;
    m_d->value_strings = valueStrings;
    m_d->default_value_index = defaultValueIndex;
    m_d->name = canonicalName;
    m_d->custom = customName;
}



/*!
    Destructor
*/
QCameraControl::~QCameraControl()
{
    delete m_d;
    m_d = 0;

}


/*!
    \enum QCameraControl::GUIElement
    Indicates the graphical representation and potential useability of the control

    \value Slider  A linear range of values e.g QSlider
    \value Menu    A multiple choice selection e.g QComboBox
    \value CheckBox A boolean state e.g QCheckBox
    \value PushButton An action e.g  QPushButton

*/

/*!
    \enum QCameraControl::CanonicalName
    Indicates the function of the control

    \value    Brightness    Brightness
    \value    Contrast  Contrast
    \value    Saturation    Saturation
    \value    Hue           Hue
    \value    AutoWhiteBalance   Auto White balance mode
    \value    RedBalance    Red balance
    \value    BlueBalance   Blue balance
    \value    Gamma         Gamma
    \value    AutoGain      Do auto gain
    \value    Exposure      Exposure control
    \value    Gain          Gain control
    \value    FlashControl  Flash control
    \value    Custom    Custom defined control
*/

/*!
  Returns the controls id
*/
quint32 QCameraControl::id() const { return m_d->id; }

/*!
    Returns a description of what the controls does
*/
QString QCameraControl::description() const { return m_d->description; }

/*!
    Returns a canonical identifier for the control
*/

QCameraControl::CanonicalName QCameraControl::name() const { return m_d->name; }

/*!
    Returns name for Custom control
*/
QString QCameraControl::custom() const { return m_d->custom; }

/*!
    Returns the step size between values the control value can take
*/

qint32 QCameraControl::step() const { return m_d->step; }

/*!
    Returns the minumum allowable range  for the control
*/
qint32 QCameraControl::min() const  { return m_d->min; }

/*!
    Returns the maximum allowable range for the control
*/
qint32 QCameraControl::max() const { return m_d->max; }

/*!
    Returns an indexed list of descriptors for Menu type controls
*/
QStringList QCameraControl::valueStrings() const { return m_d->value_strings; }

/*!
    Returns the default value , set by the driver for this control
*/
int QCameraControl::defaultValue() const { return m_d->default_value_index; }

/*!
    Returns the type of GUI or Widget the control can be visually represented as
*/
QCameraControl::GUIElement QCameraControl::gui() const { return m_d->gui; }

