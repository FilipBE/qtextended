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

#include "photoediteffect.h"

/*!
    \class PhotoEditEffect
    \inpublicgroup QtEssentialsModule
    \brief The PhotoEditEffect class provides an interface for plug-ins that implement image effects for the Pictures application.

    \internal

    \section1 Effect Descriptions

    Each plug-in includes configuration file that describes the filters it provides.  The
    description includes the name of the effect, the resource path of an image that previews the
    filter, and a list input parameters.

    The description for an effect with the id \c Example and no parameters may look like the
    following:

    \code
    [Example]
    Name[] = Example Effect
    Preview = exampleeffect/preview
    \endcode

    Parameters are listed in a \c Parameters child group of the effect description.  Each parameter
    has a name, type, and default value, where the type may be either \c int or QColor.  Integer
    types also have minimum, maximum, and step values that describe the range of selectable values.

    The description for the \c Example effect with configurable \c Color and \c Intensity parameters
    may look like the following:

    \code
    [Example]
    Name[] = Example Effect
    Preview = exampleeffect/preview
    Parameters/Color/Name[] = Color
    Parameters/Color/Type = QColor
    Parameters/Color/Default = #FF0000
    Parameters/Intensity/Name[] = Intensity
    Parameters/Intensity/Type = int
    Parameters/Intensity/Default = 50
    Parameters/Intensity/Minimum = 0
    Parameters/Intensity/Maximum = 100
    Parameters/Intensity/Step = 5
    \endcode

    The configuration file for plug-ins are installed to \c etc/photoedit and have the same name
    as the plug-in but with a \c .conf extension.  So for example the configuration file for the
    \c exampleeffect plug-in would be named \c {exampleeffect.conf}.

    The effect descriptions are translatable and should include a \c [Translation] group as per
    \l {Non-code Translatables}.

    \section1 Installation

    Plug-ins implementing the PhotoEditEffect interface are installed to \c plugin/photoediteffect
    either by locating the project in a sub-directory of a photoediteffect directory, or adding the
    line \c {plugin_type=photoediteffect} to the projects \c .pro file.
*/

/*!
    \fn PhotoEditEffect::applyEffect(const QString &effect, const QMap<QString, QVariant> &settings, QImage *image)

    Applies an \a effect to an \a image.

    Any parameters required by the effect are passed in \a settings.
*/

/*!
    Destroys a photo edit effect.
*/
PhotoEditEffect::~PhotoEditEffect()
{
}
