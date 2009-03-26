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

#ifndef QDEVICEBUTTONMANAGER_H
#define QDEVICEBUTTONMANAGER_H

#include <qobject.h>
#include <qstring.h>
#include <qdevicebutton.h>

class QtopiaServiceRequest;
class QTranslatableSettings;

class QTOPIA_EXPORT QDeviceButtonManager : public QObject {
  Q_OBJECT

 public:
  static QDeviceButtonManager& instance();

  const QList<QDeviceButton*>& buttons() const;

  const QDeviceButton* buttonForKeycode(int keyCode) const;
  const QDeviceButton* buttonForKeycode(int keyCode, const QString& context) const;

  void remapPressedAction(int button, const QtopiaServiceRequest& qcopMessage);
  void remapHeldAction(int button, const QtopiaServiceRequest& qcopMessage);
  void remapReleasedAction(int action, const QtopiaServiceRequest& qcopMessage);

  void factoryResetButtons();

 protected:
  QDeviceButtonManager();
  virtual ~QDeviceButtonManager();

  private slots:
    void received(const QString& message, const QByteArray& data);

 private:
  void loadButtons(bool factory=false);
  void loadButtonSettings(QTranslatableSettings&,bool local,bool factory);

  static QPointer<QDeviceButtonManager> m_Instance;
  QList<QDeviceButton*> m_Buttons;
};

#endif
