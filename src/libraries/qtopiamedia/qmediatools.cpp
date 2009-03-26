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

#include "qmediatools.h"

#include <QtGui>

/*!
    \class QMediaContentContext
    \inpublicgroup QtMediaModule


    \brief The QMediaContentContext class allows multiple related player
    objects to be grouped together.

    The QMediaContentContext is useful when multiple player objects are
    required to always work with the same media content object. The class
    allows related player objects to be group together and updated when the
    shared media content object changes.

    \ingroup multimedia

    \keyword {player object}

    \section1 Player Objects

    By convention player objects are defined by a setMediaContent() slot in the
    class declaration. The following declaration outlines a simple player object
    class.

    \code
        class PlayerObject : public QObject
        {
            Q_OBJECT
        public:
            PlayerObject( QObject* parent = 0 );

        public slots:
            void setMediaContent( QMediaContent* content );
        };
    \endcode

    After creating a context add related player objects to the context. When
    setMediaContent() is called the context will also call setMediaContent()
    for all player objects added to the context.

    \code
        PlayerObject *a = new PlayerObject( this );
        PlayerObject *b = new PlayerObject( this );

        QMediaContentContext *context = new QMediaContentContext( this );
        context->addObject( a );
        context->addObject( b );
    \endcode

    This way all player objects added to the context will always be working
    with the same media content object.

    QMediaContentContext is a \l {player object}.

    \sa QMediaContent
*/

/*!
    \fn QMediaContentContext::QMediaContentContext( QObject* parent )

    Constructs an empty media content context.

    The \a parent argument is passed to the QObject constructor.
*/
QMediaContentContext::QMediaContentContext( QObject* parent )
    : QObject( parent ), m_content( 0 )
{ }

/*!
    \fn QMediaContentContext::~QMediaContentContext()

    Destroys the context.
*/
QMediaContentContext::~QMediaContentContext()
{ }

/*!
    \fn void QMediaContentContext::addObject( QObject* object )

    Adds \a object to the context. The object should be a \l {player object}.
*/

void QMediaContentContext::addObject(QObject* object)
{
    if (connect(this, SIGNAL(contentChanged(QMediaContent*)),
                object, SLOT(setMediaContent(QMediaContent*)))) {

        if (m_content != 0)
            QMetaObject::invokeMethod(object, "setMediaContent", Q_ARG(QMediaContent*, m_content));
    }
}

/*!
    \fn void QMediaContentContext::removeObject( QObject* object )

    Removes \a object from the context.
*/

void QMediaContentContext::removeObject( QObject* object )
{
    disconnect(object, SLOT(setMediaContent(QMediaContent*)));
}

/*!
    \fn QMediaContent* QMediaContentContext::content() const

    Returns the current media content object for the context.
*/
QMediaContent* QMediaContentContext::content() const
{
    return m_content;
}

/*!
    \fn void QMediaContentContext::contentChanged( QMediaContent* content )

    The media content object has changed to \a content within the context.

    \sa setMediaContent()
*/

/*!
    \fn void QMediaContentContext::setMediaContent( QMediaContent* content )

    Sets \a content as the media content object for the context as well as all
    player objects added to the context.

    \sa contentChanged()
*/

void QMediaContentContext::setMediaContent(QMediaContent* content)
{
    if (m_content != content) {
        m_content = content;
        emit contentChanged(m_content);
    }
}

class QMediaControlNotifierPrivate
{
public:
    bool isvalid;
};

/*!
    \class QMediaControlNotifier
    \inpublicgroup QtMediaModule


    \brief The QMediaControlNotifier class watches a media content object for
    the availability of a given media control.

    \ingroup multimedia

    When a control becomes available for the media content object the class
    emits a valid() signal. Similarly the class emits an invalid() signal when
    the control is not available.

    The valid() signal indicates that the control for the media content
    object can be constructed and used immediately. The invalid() signal
    indicates that the control cannot be constructed and previously constructed
    controls of the same type for the media content object should no longer be
    used.

    The following example defines a simple volume settings player object
    class that uses the QMediaControlNotifier and QMediaControl classes.

    \code
        class VolumeSettings : public QObject
        {
            Q_OBJECT
        public:
            VolumeSettings( QObject* parent = 0 )
                : QObject( parent ), control( 0 )
            {
                notifier = new QMediaControlNotifier( QMediaControl::name(), this );
                connect( notifier, SIGNAL(valid()), this, SLOT(activate()) );
                connect( notifier, SIGNAL(invalid()), this, SLOT(deactivate()) );
            }

            void setVolume( int volume )
            {
                if ( control )
                    control->setVolume( volume );
            }
    \endcode

    By convention all media controls have a static member function that returns
    a QString containing a name that identifies the control.

    \code
        public slots:
            void setMediaContent( QMediaContent* content )
            {
                notifier->setMediaContent( content );
            }
    \endcode

    If a class contains other related player objects, rather than calling
    setMediaContent() directly on each, the notifier and related player
    objects could be grouped together using the QMediaContentContext class.

    \code
        private slots:
            void activate()
            {
                control = new QMediaControl( notifier->content() );
            }

            void deactivate()
            {
                delete control;
                control = 0;
            }

        private:
            QMediaControlNotifier *notifier;
            QMediaControl *control;
        };
    \endcode

    When a media control becomes invalid the results of interacting with that
    control are undefined. For player objects it is common practice to delete
    and set to null invalid control member variables.

    QMediaControlNotifier is a \l {player object}.

    \sa QMediaContent
*/

/*!
    \fn QMediaControlNotifier::QMediaControlNotifier( const QString& control, QObject* parent )

    Constructs a control notifier to watch for the availability of \a control.
    By convention all controls have a static member function that returns a
    QString containing a name that identifies the control.

    \code
        QMediaControlNotifier *notifier = new QMediaControlNotifier( QMediaVideoControl::name(), this );
    \endcode

    The \a parent argument is passed to the QObject constructor.
*/
QMediaControlNotifier::QMediaControlNotifier( const QString& control, QObject* parent )
    : QObject( parent ), m_control( control ), m_content( 0 )
{
    m_d = new QMediaControlNotifierPrivate;
    m_d->isvalid = false;
}

/*!
    \fn QMediaControlNotifier::~QMediaControlNotifier()

    Destroys the notifier.
*/
QMediaControlNotifier::~QMediaControlNotifier()
{
    delete m_d;
}

/*!
    \fn QMediaContent* QMediaControlNotifier::content() const

    Returns the media content which the notifier is currently watching.
*/
QMediaContent* QMediaControlNotifier::content() const
{
    return m_content;
}

/*!
    \fn void QMediaControlNotifier::valid()

    The control that the notifier is watching for is available and controls
    constructed of that type for the media content object will be valid.
*/

/*!
    \fn void QMediaControlNotifier::invalid()

    The control that the notifier is watching for is no longer avaiable and
    controls of that type previously constructed for the media content object
    are invalid.
*/

/*!
    \fn QMediaControlNotifier::setMediaContent( QMediaContent* content )

    Sets \a content as the media content to watch.

    If the control that the notifier is watching for is currently vaild for
    the previous media content object, an invalid() signal will be emitted.
*/
void QMediaControlNotifier::setMediaContent( QMediaContent* content )
{
    if( m_content ) {
        m_d->isvalid = false;
        emit invalid();

        m_content->disconnect( this );
    }

    m_content = content;

    if (m_content) {
        connect( m_content, SIGNAL(controlAvailable(QString)),
            this, SLOT(evaluate()) );
        connect( m_content, SIGNAL(controlUnavailable(QString)),
            this, SLOT(evaluate()) );

        evaluate();
    }
}

void QMediaControlNotifier::evaluate()
{
    bool available = m_content->controls().contains( m_control );

    if( m_d->isvalid ) {
        if( !available ) {
            m_d->isvalid = false;
            emit invalid();
        }
    } else {
        if( available ) {
            m_d->isvalid = true;
            emit valid();
        }
    }
}
