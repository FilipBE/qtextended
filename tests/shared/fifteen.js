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

Fifteen = {
    /* finds the middle of an area defined by the row and column in a uniform grid
    maths can also be done by finding the ratio and multiplying with value */
    select_position: function(sel_col, sel_row, tot_col, tot_row)
    {
        var geo = getGeometry();
        var select_x = geo.x() + ((geo.width()/(tot_col * 2)) * ((sel_col * 2) - 1));
        var select_y = geo.y() + ((geo.height()/(tot_row * 2)) * ((sel_row * 2) - 1));
        // print( "select_x: " + select_x + " select_y: " + select_y );
        mouseClick(select_x, select_y);
    }
};

