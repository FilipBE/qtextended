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

#ifndef QSPARSELIST_P_H
#define QSPARSELIST_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QList>
#include <QString>
#include <QtDebug>

template< typename T >
struct QSparseListFrame
{
    int offset;

    QSparseListFrame< T > **location;
    QSparseListFrame< T > *next;
    QSparseListFrame< T > **priorityLocation;
    QSparseListFrame< T > *higherPriority;

    QList< T > values;
};

template< typename T, int frameSize, int frameCount >
class QSparseList
{
public:
    QSparseList();

    virtual ~QSparseList();
    T value( int index ) const;

    virtual int valueCount() const = 0;

protected:
    virtual QList< T > values( int index, int count ) = 0;

    void insertRange( int index, int count );
    void removeRange( int index, int count );
    void refreshRange( int index, int count );
    void clearCache();

private:
    T uncachedValue( int index, QSparseListFrame< T > **insertLocation );
    QSparseListFrame< T > *createFrame( QSparseListFrame< T > **insertLocation, int index, int offset );
    void insertFrame( QSparseListFrame< T > **insertLocation, QSparseListFrame< T > *frame );
    void populateFrame( QSparseListFrame< T > *frame, int index, int count );
    void appendToFrame( QSparseListFrame< T > *frame, int index, int count );
    void retireFrame( QSparseListFrame< T > *frame );
    void prioritizeFrame( QSparseListFrame< T > *frame );

    QSparseListFrame< T > *m_firstFrame;
    QSparseListFrame< T > *m_firstFreeFrame;
    QSparseListFrame< T > *m_lowestPriorityFrame;

    QSparseListFrame< T > m_frames[ frameSize ];
};


template< typename T, int frameSize, int frameCount >
QSparseList< T, frameSize, frameCount >::QSparseList()
{
    clearCache();
}

template< typename T, int frameSize, int frameCount >
QSparseList< T, frameSize, frameCount >::~QSparseList()
{
}

template< typename T, int frameSize, int frameCount >
T QSparseList< T, frameSize, frameCount >::value( int index ) const
{
    QSparseListFrame< T > *const *frameLocation = &m_firstFrame;

    int relativeIndex = index;

    while( *frameLocation )
    {
        relativeIndex -= (*frameLocation)->offset;

        if( relativeIndex < 0 )
        {
            break;
        }
        else if( relativeIndex < (*frameLocation)->values.count() )
        {
            if( (*frameLocation)->higherPriority )
                const_cast< QSparseList< T, frameSize, frameCount > * >( this )->prioritizeFrame( const_cast< QSparseListFrame< T > * >( *frameLocation ) );

            return (*frameLocation)->values.at( relativeIndex );
        }
        else
        {
            relativeIndex -= (*frameLocation)->values.count();
            frameLocation = &(*frameLocation)->next;
        }
    }

    return const_cast< QSparseList< T, frameSize, frameCount > * >( this )->uncachedValue( index, const_cast< QSparseListFrame< T > ** >( frameLocation ) );
}

template< typename T, int frameSize, int frameCount >
void QSparseList< T, frameSize, frameCount >::insertRange( int index, int count )
{
    QSparseListFrame< T > *frame = m_firstFrame;

    int relativeIndex = index;

    while( frame )
    {
        relativeIndex -= frame->offset;

        if( relativeIndex <= 0 )    // Insert before frame.
        {
            frame->offset += count;

            break;
        }
        else if( relativeIndex < frame->values.count() )   // Insert into frame.
        {
            int oldCount = frame->values.count();

            frame->values = frame->values.mid( 0, relativeIndex );

            if( frame->next )
                frame->next->offset += count + oldCount - frame->values.count();

            break;
        }
        else    // After frame.
        {
            relativeIndex -= frame->values.count();

        frame = frame->next;
        }
    }

}

template< typename T, int frameSize, int frameCount >
void QSparseList< T, frameSize, frameCount >::removeRange( int index, int count )
{
    QSparseListFrame< T > *frame = m_firstFrame;

    int relativeIndex = index;
    int relativeCount = count;

    while( frame )
    {
        relativeIndex -= frame->offset;

        if( relativeIndex >= frame->values.count() )   // Completely after frame.
        {
            relativeIndex -= frame->values.count();

            frame = frame->next;
        }
        else if( relativeIndex + relativeCount < 0 )    // Completely Before frame.
        {
            frame->offset -= relativeCount;

            break;
        }
        else if( relativeIndex <= 0 && relativeIndex + relativeCount < frame->values.count() - 1 ) // Lower part of frame.
        {
            frame->values = frame->values.mid( relativeIndex + relativeCount );

            frame->offset += relativeIndex;

            break;
        }
        else if( relativeIndex > 0 && relativeIndex + relativeCount < frame->values.count() - 1 ) // Entirely within frame.
        {
            frame->values = frame->values.mid( 0, relativeIndex ) + frame->values.mid( relativeIndex + relativeCount );

            break;
        }
        else if( relativeIndex <= 0 && relativeIndex + relativeCount >= frame->values.count() - 1 ) // Consumes frame.
        {
            if( frame->next )
                frame->next->offset += frame->offset + relativeIndex;

            relativeCount -= frame->values.count() - relativeIndex;
            relativeIndex += frame->offset;

            QSparseListFrame< T > *nextFrame = frame->next;

            retireFrame( frame );

            if( relativeCount == 0 )
                break;

            frame = nextFrame;
        }

        else if( relativeIndex > 0 && relativeIndex + relativeCount >= frame->values.count() - 1 ) // Upper part of frame.
        {
            int oldCount = frame->values.count();

            frame->values = frame->values.mid( 0, relativeIndex );

            relativeCount -= oldCount - relativeIndex;
            relativeIndex = 0;

            if( relativeCount == 0 )
                break;

            frame = frame->next;
        }
        else
        {
            Q_ASSERT( false );
        }
    }
}

template< typename T, int frameSize, int frameCount >
void QSparseList< T, frameSize, frameCount >::refreshRange( int index, int count )
{
    QSparseListFrame< T > *frame = m_firstFrame;

    int relativeIndex = index;
    int relativeCount = count;

    while( frame )
    {
        relativeIndex -= frame->offset;

        if( relativeIndex >= frame->values.count() )   // Completely after frame.
        {
            relativeIndex -= frame->values.count();

            frame = frame->next;
        }
        else if( relativeIndex + relativeCount < 0 )    // Completely Before frame.
        {
            break;
        }
        else if( relativeIndex <= 0 && relativeIndex + relativeCount < frame->values.count() - 1 ) // Lower part of frame.
        {
            frame->values = values( index, relativeIndex + relativeCount ) + frame->values.mid( relativeIndex + relativeCount );

            break;
        }
        else if( relativeIndex > 0 && relativeIndex + relativeCount < frame->values.count() - 1 ) // Entirely within frame.
        {
            frame->values = frame->values.mid( 0, relativeIndex ) + values( index, relativeCount ) + frame->values.mid( relativeIndex + relativeCount );

            break;
        }
        else if( relativeIndex <= 0 && relativeIndex + relativeCount >= frame->values.count() - 1 ) // Consumes frame.
        {
            index -= relativeIndex;

            frame->values = values( index, frame->values.count() );

            index += frame->values.count();

            relativeCount -= frame->values.count() - relativeIndex;
            relativeIndex = 0;

            if( relativeCount == 0 )
                break;

            frame = frame->next;
        }

        else if( relativeIndex > 0 && relativeIndex + relativeCount >= frame->values.count() - 1 ) // Upper part of frame.
        {
            frame->values = frame->values.mid( 0, relativeIndex ) + values( index, frame->values.count() - relativeIndex );

            index += frame->values.count() - relativeIndex;

            relativeCount -= frame->values.count() - relativeIndex;
            relativeIndex = 0;

            if( relativeCount == 0 )
                break;

            frame = frame->next;
        }
        else
        {
            Q_ASSERT( false );
        }
    }
}

template< typename T, int frameSize, int frameCount >
void QSparseList< T, frameSize, frameCount >::clearCache()
{
    m_firstFrame = 0;
    m_firstFreeFrame = &(m_frames[ 0 ]);
    m_lowestPriorityFrame = 0;

    m_frames[ 0 ].location = &m_firstFreeFrame;

    for( int i = 0; i < frameCount; i++ )
    {
        m_frames[ i ].offset = 0;
        m_frames[ i ].priorityLocation = 0;
        m_frames[ i ].higherPriority = 0;

        if( i != 0 )
        {
            m_frames[ i ].location = &(m_frames[ i - 1 ].next);
            m_frames[ i - 1 ].next = &(m_frames[ i ]);
        }

        m_frames[ i ].values.clear();
    }

    m_frames[ frameCount - 1 ].next = 0;
}

template< typename T, int frameSize, int frameCount >
T QSparseList< T, frameSize, frameCount >::uncachedValue( int index, QSparseListFrame< T > **insertLocation )
{
    int offset = index;
    QSparseListFrame< T > *nextFrame = m_firstFrame;
    QSparseListFrame< T > *previousFrame = 0;

    while( nextFrame != *insertLocation )
    {
        offset -= nextFrame->offset + nextFrame->values.count();

        previousFrame = nextFrame;
        nextFrame = nextFrame->next;
    }

    if( previousFrame && previousFrame->values.count() + offset < frameSize )    // Previous frame has room to append the desired content.
    {
        int appendIndex = index - offset;
        int frameIndex = appendIndex - previousFrame->values.count();

        int count = qMin( valueCount() - appendIndex, frameSize - previousFrame->values.count() );

        if( nextFrame )
            count = qMin( count, nextFrame->offset );

        appendToFrame( previousFrame, appendIndex, count );

        return previousFrame->values.at( index - frameIndex );
    }
    else if( offset < frameSize ) // A frame directly following the previous will include the desired index.
    {
        int frameIndex = index - offset;

        QSparseListFrame< T > *frame = createFrame( insertLocation, frameIndex, 0 );

        return frame->values.at( index - frameIndex );
    }
    else
    {
        int frameIndex = index - qMin( index % frameSize, offset );
        int frameOffset = offset - index + frameIndex;

        QSparseListFrame< T > *frame = createFrame( insertLocation, frameIndex, frameOffset );

        return frame->values.at( index - frameIndex );
    }
}

template< typename T, int frameSize, int frameCount >
QSparseListFrame< T > *QSparseList< T, frameSize, frameCount >::createFrame( QSparseListFrame< T > **insertLocation, int index, int offset )
{
    QSparseListFrame< T > *frame = 0;

    if( m_firstFreeFrame ) // There is a free frame.
    {
        frame = m_firstFreeFrame;
        m_firstFreeFrame = m_firstFreeFrame->next;

        if( m_firstFreeFrame )
            m_firstFreeFrame->location = &m_firstFreeFrame;

        frame->offset = offset;

        insertFrame( insertLocation, frame );
    }
    else if( m_lowestPriorityFrame == *insertLocation ) // The lowest priority frame is already at the insert location (is after the desired index).
    {
        frame = m_lowestPriorityFrame;

        if( frame->next )
            frame->next->offset += frame->offset + frame->values.count() - offset;

        frame->offset = offset;
    }
    else if( m_lowestPriorityFrame->next == *insertLocation ) // The lowest priority frame contains the insert location (is before the desired index).
    {
        frame = m_lowestPriorityFrame;

        if( frame->next )
            frame->next->offset -= offset;

        frame->offset += offset + frame->values.count();
    }
    else // The lowest priority frame is not immediately before or after the insert location.
    {
        frame = m_lowestPriorityFrame;
        m_lowestPriorityFrame = m_lowestPriorityFrame->higherPriority;
        m_lowestPriorityFrame->priorityLocation = &m_lowestPriorityFrame;

        *frame->location = frame->next;

        if( frame->next )
        {
            frame->next->location = frame->location;
            frame->next->offset += frame->offset + frame->values.count();
        }

        frame->offset = offset;

        insertFrame( insertLocation, frame );
    }

    int count = qMin( valueCount() - index, frameSize );

    if( frame->next )
        count = qMin( count, frame->next->offset );

    populateFrame( frame, index, count );

    return frame;
}

template< typename T, int frameSize, int frameCount >
void QSparseList< T, frameSize, frameCount >::insertFrame( QSparseListFrame< T > **insertLocation, QSparseListFrame< T > *frame )
{
    frame->next = (*insertLocation);
    *insertLocation = frame;
    frame->location = insertLocation;

    if( frame->next )
    {
        frame->next->offset -= frame->offset;
        frame->next->location = &frame->next;
    }

    if( m_lowestPriorityFrame )
    {
        frame->higherPriority = m_lowestPriorityFrame->higherPriority;

        if( frame->higherPriority )
            frame->higherPriority->priorityLocation = &frame->higherPriority;

        m_lowestPriorityFrame->higherPriority = frame;

        frame->priorityLocation = &m_lowestPriorityFrame->higherPriority;
    }
    else
    {
        m_lowestPriorityFrame = frame;

        frame->priorityLocation = &m_lowestPriorityFrame;
    }
}

template< typename T, int frameSize, int frameCount >
void QSparseList< T, frameSize, frameCount >::populateFrame( QSparseListFrame< T > *frame, int index, int count )
{
    frame->values = values( index, count );

    if( frame->next )
        frame->next->offset -= count;
}

template< typename T, int frameSize, int frameCount >
void QSparseList< T, frameSize, frameCount >::appendToFrame( QSparseListFrame< T > *frame, int index, int count )
{
    frame->values += values( index, count );

    if( frame->next )
        frame->next->offset -= count;
}

template< typename T, int frameSize, int frameCount >
void QSparseList< T, frameSize, frameCount >::retireFrame( QSparseListFrame< T > *frame )
{
    (*frame->location) = frame->next;

    if( frame->next )
        frame->next->location = frame->location;

    (*frame->priorityLocation) = frame->higherPriority;

    if( frame->higherPriority )
        frame->higherPriority->priorityLocation = frame->priorityLocation;

    frame->location = &m_firstFreeFrame;
    frame->next = m_firstFreeFrame;

    m_firstFreeFrame = frame;

    if( frame->next )
        frame->next->location = &(frame->next);

    frame->offset = 0;
    frame->values.clear();
}

template< typename T, int frameSize, int frameCount >
void QSparseList< T, frameSize, frameCount >::prioritizeFrame( QSparseListFrame< T > *frame )
{
    QSparseListFrame< T > *higherPriority = frame->higherPriority;

    *frame->priorityLocation = frame->higherPriority;
    higherPriority->priorityLocation = frame->priorityLocation;

    frame->higherPriority = higherPriority->higherPriority;

    if( frame->higherPriority )
        frame->higherPriority->priorityLocation = &frame->higherPriority;

    higherPriority->higherPriority = frame;
    frame->priorityLocation = &higherPriority->higherPriority;
}

#endif
