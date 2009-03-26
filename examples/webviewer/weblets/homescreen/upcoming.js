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

    var upcoming = {

    update: function() {
        var realtable = document.getElementById('upcoming_table');
        var table = realtable.tBodies[0];

        // Clear the table
        while (table.rows.length > 0) {
            table.deleteRow(0);
        }

        // See if we have any rows
        if ( this.appointments.rowCount() > 0) {
            for (var i=0; i < this.appointments.rowCount(); i++) {
                var appt = this.appointments.appointment(i);
                var row = table.insertRow(i);
                row.addEventListener("click", new Function("com.trolltech.services.Calendar.showOccurrence('" + appt.uid + "', new Date('" + appt.startInCurrentTZ + "'))"), true);
                row.innerHTML = "<td class='upcomingtime'>" + (appt.isAllDay ? com.trolltech.formatDate(appt.startInCurrentTZ) : com.trolltech.formatDateTime(appt.startInCurrentTZ))
                    + "</td><td>&nbsp;-&nbsp;<td>" + appt.description + "</td>";
            }
        } else {
            // Add a "No upcoming appointments" row.
            var cell = table.insertRow(0).insertCell(0);
            cell.innerHTML = '<i>No upcoming appointments</i>';
        }
    },

    init: function() {
        this.lastcount = 0;
        this.appointments = com.trolltech.appointments(new Date(), 3);
        this.appointments.modelReset.connect(this, this.update);
        this.update();
    }
}
try {
upcoming.init();
} catch(e){dolog(e);}

