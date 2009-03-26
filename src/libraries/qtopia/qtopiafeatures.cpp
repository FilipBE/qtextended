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

#include "qtopiafeatures.h"
#include <QObject>
#include <QString>
#include <QValueSpaceItem>
#include <QValueSpaceObject>

// declare QtopiaFeaturesPrivate
class QtopiaFeaturesPrivate : public QObject
{
Q_OBJECT
public:
    QtopiaFeaturesPrivate();
    void setFeature(const QString &);
    void removeFeature(const QString &);
    bool hasFeature(const QString &);

    QValueSpaceObject *m_setFeatures;
    QValueSpaceItem *m_hasFeatures;

    QList<QString> m_features;

private slots:
    void refreshFeatures();
};
Q_GLOBAL_STATIC(QtopiaFeaturesPrivate, qtopiaFeatures);

// define QtopiaFeaturesPrivate
QtopiaFeaturesPrivate::QtopiaFeaturesPrivate()
: m_setFeatures(0), m_hasFeatures(0)
{
}

void QtopiaFeaturesPrivate::setFeature(const QString &feature)
{
    if(!m_setFeatures)
        m_setFeatures = new QValueSpaceObject("/System/Features", this);
    m_setFeatures->setAttribute(feature, true);
    m_features.append(feature);
}

void QtopiaFeaturesPrivate::removeFeature(const QString &feature)
{
    if(!m_setFeatures)
        m_setFeatures = new QValueSpaceObject("/System/Features", this);
    m_setFeatures->removeAttribute(feature);
    m_features.removeAll(feature);
}

bool QtopiaFeaturesPrivate::hasFeature(const QString &feature)
{
    if(!m_hasFeatures) {
        m_hasFeatures = new QValueSpaceItem("/System/Features", this);
        QObject::connect(m_hasFeatures, SIGNAL(contentsChanged()),
                         this, SLOT(refreshFeatures()));
        refreshFeatures();
    }

    return m_features.contains(feature);
}

void QtopiaFeaturesPrivate::refreshFeatures()
{
    Q_ASSERT(m_hasFeatures);
    m_features = m_hasFeatures->subPaths();
}

/*!
  \class QtopiaFeatures
    \inpublicgroup QtBaseModule

  \ingroup QtopiaServer
  \brief The QtopiaFeatures class allows applications to set and query the
         available Qt Extended features.

  Qt Extended features are elements of the system that are conditionally available
  depending on the platform or device configuration. Features can be added and
  removed during device execution.

  Features can be queried through the QtopiaFeatures API, or by directly
  examining the Qt Extended value space item \c {/System/Features/<Feature Name>}.

  \ingroup environment
 */

// define QtopiaFeatures

/*!
  Set \a feature as enabled.
 */
void QtopiaFeatures::setFeature(const QString &feature)
{
    if(feature.isEmpty()) return;
    QtopiaFeaturesPrivate *fp = qtopiaFeatures();
    if(fp) fp->setFeature(feature);
}

/*!
  Removes the \a feature.
*/
void QtopiaFeatures::removeFeature(const QString &feature)
{
    if(feature.isEmpty()) return;
    QtopiaFeaturesPrivate *fp = qtopiaFeatures();
    if(fp) fp->removeFeature(feature);
}

/*!
  Returns true if \a feature is enabled, false otherwise.
 */
bool QtopiaFeatures::hasFeature(const QString &feature)
{
    if(feature.isEmpty()) return false;
    QtopiaFeaturesPrivate *fp = qtopiaFeatures();
    if(fp) return fp->hasFeature(feature);
    else return false;
}

#include "qtopiafeatures.moc"
