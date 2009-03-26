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

#ifndef QSCREENINFORMATION_H
#define QSCREENINFORMATION_H

#include <QObject>
#include <QColor>
#include <qtopiaglobal.h>

class QScreenInformationPrivate;
class QScreenInformationProviderPrivate;
class QVariant;

class QTOPIA_EXPORT QScreenInformation : public QObject
{
    Q_OBJECT
public:
    enum Type
    {
        Normal,
        Television,
        Overlay
    };

    explicit QScreenInformation(int screenNumber=-1, QObject *parent=0);
    explicit QScreenInformation(QScreenInformation::Type type, QObject *parent=0);
    ~QScreenInformation();

    int screenNumber() const;

    bool isVisible() const;

    QScreenInformation::Type type() const;

    int clonedScreen() const;
    void setClonedScreen(int value);

    int layer() const;
    void setLayer(int value);

    QList<int> supportedLayers() const;

    QColor transparencyColor() const;

signals:
    void changed();

private:
    QScreenInformationPrivate *d;

    void init(int screenNumber);
};

class QTOPIA_EXPORT QScreenInformationProvider : public QObject
{
    Q_OBJECT
public:
    QScreenInformationProvider(int screenNumber=-1, QObject *parent=0);
    ~QScreenInformationProvider();

    void setVisible(bool value);
    void setType(QScreenInformation::Type value);
    void setClonedScreen(int value);
    void setLayer(int value);
    void setSupportedLayers(const QList<int>& value);
    void setTransparencyColor(const QColor& value);

protected:
    virtual void changeClonedScreen(int value);
    virtual void revertClonedScreen();
    virtual void changeLayer(int value);
    virtual void revertLayer();

private slots:
    void checkForCloneRevert();
    void checkForLayerRevert();
    void setClonedMessage(int value);
    void setLayerMessage(int value);

private:
    QScreenInformationProviderPrivate *d;
};

#endif
