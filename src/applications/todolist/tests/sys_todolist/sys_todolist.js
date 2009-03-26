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

include("calendar.js");

//TESTED_COMPONENT=PIM: Tasks (18705)

testcase = {
    initTestCase: function()
    {
        waitForQtopiaStart();
    },

    init: function()
    {
        /* start the app */
        startApplication( "Tasks" );
        df = dateFormat();
        setDateFormat("Y-M-D");
    },

    cleanup: function()
    {
        setDateFormat(df);
        startApplication( "Tasks" );

        select("View Category...", optionsMenu()) ;
        waitForTitle( "Select Category" );
        select("All") ;
        waitFor() { return ( getList(softMenu()).contains("Back") ); }
        select("Back", softMenu());

        if ( getList(optionsMenu()).contains("Show completed tasks") ) {
            select( "Show completed tasks", optionsMenu() );
        }

        lt = getText();
        while( lt != "" ){
            thisTask = lt.split("\n")[0];
            select(thisTask);
            waitForTitle("Task Details");
            expectMessageBox( "Tasks", "<qt>Are you sure you want to delete: " + lt.split("\n")[0] + "?</qt>", "Yes") {
                select( "Delete task", optionsMenu() );
            }
            lt = getText();
        }

        gotoHome();
    },

/*
    \req QTOPIA-1702
    \groups Acceptance
*/
    creating_a_task: function()
    {
        create_task({
            description: "qtopia task",
            priority:    "2 - High",
            status:      "In Progress",
            percent:     20,
            due:         new QDate(2007, 03, 01),
            start:       new QDate(2007, 02, 28),
            categories:  ["Personal"]
        });
    },

/*
    \req QTOPIA-1702
*/
    adding_an_empty_task: function()
    {
        // Verify there are no tasks
        l = getText();
        verify( l == "", l );

        // Select 'New task' from context menu
        select( "New task", optionsMenu() );

        // Verify that the 'New Task' dialog is created
        compare( currentTitle(), "New Task" );

        // Change the 'Priority' to '4 - Low'
        select("4 - Low", "Priority") ;

        // Select 'Back' to attempt to create the task
        select("Back", softMenu());

        // Verify that the task is not created by viewing all tasks in the task list
        // (only applies if there are no other tasks)
        verify( getText() == "" );
    },

/*
    \req QTOPIA-1704
*/
    editing_a_task: function() {
        priority = "1 - Very High";
        oldDesc = "edit task";
        newDesc = "Altered task";
        status = "In Progress";
        dueDate =   new QDate(2007, 03, 29);
        startDate = new QDate(2007, 01, 02);
        percent = 75;
        categories = [ "Business", "Personal" ];
        noteText = "this is a note, wooo!";

        create_task({ description: oldDesc });

        // Select an existing task from the tasks list
        select(oldDesc) ;

        // Verify that the task's details are shown
        compare( currentTitle(), "Task Details" );
        verify( -1 != getText().indexOf(oldDesc) );

        // Edit the task
        select( "Edit task", optionsMenu() );

        // Verify that the 'Edit Task' dialog is displayed
        waitForTitle( "Edit Task" );
        compare( getSelectedText("Desc."), oldDesc );

        // Modify the 'Priority' of the task
        edit_current_task({ "priority": priority });
        select("Back", softMenu());

        // Verify that the task's details show the changed priority value.
        verify( -1 != getText().indexOf(oldDesc) );
        verify( -1 != getText().indexOf("Priority: " + priority.charAt(0)) );

        // Edit the current task again
        select( "Edit task", optionsMenu() );
        waitForTitle( "Edit Task" );

        edit_current_task({
            description: newDesc,
            priority:    priority,
            status:      status,
            percent:     percent,
            due:         dueDate,
            start:       startDate,
            categories:  categories,
            note:        noteText
        });
        select("Back", softMenu());

        verify_current_task({
            description: newDesc,
            priority:    priority,
            status:      status,
            percent:     percent,
            due:         dueDate,
            start:       startDate,
            categories:  categories,
            note:        noteText
        });
        // Verify that the changes made to the task are also updated in the task list
        select("Back", softMenu());

        verify( -1 != getText().toLowerCase().indexOf(newDesc.toLowerCase()) );
        verify( -1 == getText().toLowerCase().indexOf(oldDesc.toLowerCase()) );
    },

/*
    \req QTOPIA-1703
*/
    deleting_a_task: function()
    {
        desc = "prepare to be deleted";
        create_task({ description: desc });

        // Select a task from the task list to view its details
        select(desc);

        // Select the 'Delete' option from the context menu
        // Verify that a confirmation dialog is displayed
        // Select 'No'
        expectMessageBox( "Tasks", "<qt>Are you sure you want to delete: " + desc + "?</qt>", "No") {
            select( "Delete task", optionsMenu() );
        }
        // Verify that the task is not deleted.

        select("Back", softMenu());
        select(desc);
        // Select 'Delete' again from the context menu.
        // This time select 'Yes' from the confirmation dialog.
        expectMessageBox( "Tasks", "<qt>Are you sure you want to delete: " + desc + "?</qt>", "Yes") {
            select( "Delete task", optionsMenu() );
        }

        // Verify that the task list is displayed and that selected the task is deleted
        verify(!getText().contains(desc));
    },

/*
    \req QTOPIA-1704
*/
    completing_a_task: function()
    {
        task1 = "task1";
        task2 = "task2";
        dueDate = new QDate(2003, 1, 1);
        completeDate = new QDate(2004, 1, 2);

        // Preconditions: Two uncompleted tasks exist
        create_task({ description: task1 });
        create_task({ description: task2 });

        select(task1) ;
        verify(!getText().contains("Status: Completed"));
        select("Back", softMenu());

        select(task2) ;
        verify(!getText().contains("Status: Completed"));
        select("Back", softMenu());

        // Select an uncompleted task from the task list
        select(task1) ;
        select( "Edit task", optionsMenu() );
        waitForTitle( "Edit Task" );

        // Change the 'Status' of the task to 'Completed'
        edit_current_task({
            description: task1,
            status:      "Completed"
        });

        // Verify that the task's 'Status' is displayed as 'Completed'.
        select("Back", softMenu());
        verify_current_task({
            description: task1,
            status:      "Completed"
        });

        select("Back", softMenu());

        // Verify that the task is now displayed with the 'Completed' icon
        verifyImage( "first_completed_task_list", "", "Verify that the '" + task1 + "' task is displayed alongside an icon indicating completion." );

        // Verify that "Mark task complete" is not in context menu
        verify( !getList(optionsMenu()).contains("Mark task complete") );

        // Mark task as incomplete
        select("Mark task incomplete", optionsMenu()) ;
        verifyImage( "both_incomplete2", "", "Verify that the '" + task1 + "' task is not displayed alongside an icon indicating completion." );

        // Mark task as complete
        select("Mark task complete", optionsMenu()) ;
        verifyImage( "first_completed_task_list2", "", "Verify that the '" + task1 + "' task is displayed alongside an icon indicating completion." );

        // Select the task's description to view the task's details.
        select(task1);
        verify_current_task({
            description: task1,
            status:      "Completed"
        });
        select( "Back", softMenu() );

        // Select an uncompleted task from the task list to view the tasks details
        select(task2) ;

        // Select 'Edit' from the context menu
        select( "Edit task", optionsMenu() );

        // Set as completed via percent and complete date
        edit_current_task({
            description: task2,
            percent:     100,
            due:         dueDate,
            complete:    completeDate
        });
        select("Back", softMenu());

        verify_current_task({
            description: task2,
            percent:     100,
            due:         dueDate,
            complete:    completeDate
        });
        select("Back", softMenu());

        verifyImage( "second_completed_task_list", "", "Verify that the '" + task2 + "' task is displayed alongside an icon indicating completion." );
    },

/*
    \req QTOPIA-1701
*/
    view_task_list: function()
    {
        create_task({ description: "b_incomplete_task", status: "In Progress" });
        create_task({ description: "a_incomplete_task", status: "In Progress" });
        create_task({ description: "p2_incomplete_task", status: "In Progress", priority: "2 - High" });
        create_task({ description: "p1_incomplete_task", status: "In Progress", priority: "1 - Very High" });
        create_task({ description: "b_complete_task", status: "Completed" });
        create_task({ description: "a_complete_task", status: "Completed" });
        create_task({ description: "p2_complete_task", status: "Completed", priority: "2 - High" });
        create_task({ description: "p1_complete_task", status: "Completed", priority: "1 - Very High" });

        if ( getList(optionsMenu()).contains("Show completed tasks") ) {
            select( "Show completed tasks", optionsMenu() );
        }
        verify( getList() == "p1_incomplete_task,p2_incomplete_task,a_incomplete_task,b_incomplete_task,p1_complete_task,p2_complete_task,a_complete_task,b_complete_task" );

        select( "Hide completed tasks", optionsMenu() );
        verify( getList() == "p1_incomplete_task,p2_incomplete_task,a_incomplete_task,b_incomplete_task" );
    },

/*
    \req QTOPIA-1706
*/
    task_priorities: function()
    {
        create_task({ description: "very_low_task", priority: "5 - Very Low" });
        create_task({ description: "very_high_task", priority: "1 - Very High" });
        create_task({ description: "high_task", priority: "2 - High" });
        create_task({ description: "low_task", priority: "4 - Low" });
        create_task({ description: "normal_task", priority: "3 - Normal" });

        verify( getList() == "very_high_task,high_task,normal_task,low_task,very_low_task" );
    },

/*
    \req QTOPIA-1708
*/
    filtering_task_list: function()
    {
        create_task({ description: "personal_incomplete", status: "In Progress", categories:  ["Personal"] });
        create_task({ description: "personal_complete", status: "Completed", categories:  ["Personal"] });
        create_task({ description: "business_incomplete", status: "In Progress", categories:  ["Business"] });
        create_task({ description: "business_complete", status: "Completed", categories:  ["Business"] });

        // Make sure completed tasks are shown
        if ( getList(optionsMenu()).contains("Show completed tasks") ) {
            select( "Show completed tasks", optionsMenu() );
        }

        // Confirm that tasks are listed in correct order
        verify( getList() == "business_incomplete,personal_incomplete,business_complete,personal_complete" );

        // Filter out the completed tasks
        select( "Hide completed tasks", optionsMenu() );
        verify( getList() == "business_incomplete,personal_incomplete" );

        // Show only Business category
        select( "View Category...", optionsMenu() );
        waitForTitle( "Select Category" );
        select( "Business" );
        select( "Back", softMenu() );
        verify( getList() == "business_incomplete" );
        select( "Show completed tasks", optionsMenu() );
        verify( getList() == "business_incomplete,business_complete" );

        // Show only Personal category
        select( "View Category...", optionsMenu() );
        waitForTitle( "Select Category" );
        select( "Personal" );
        select( "Back", softMenu() );
        verify( getList() == "personal_incomplete,personal_complete" );

        // Show All tasks again
        select( "View Category...", optionsMenu() );
        waitForTitle( "Select Category" );
        select( "All" );
        select( "Back", softMenu() );
        verify( getList() == "business_incomplete,personal_incomplete,business_complete,personal_complete" );
    },

/*
    \req QTOPIA-1712
*/
    link_tasks: function()
    {
        create_task({ description: "task1" });
        create_task({ description: "task2" });

        // Add link to task2 in the Notes section of task1
        select( "task1" );
        waitForTitle( "Task Details" );
        select( "Edit task", optionsMenu() );
        waitForTitle( "Edit Task" );
        select( "Notes", tabBar() );
        select( "Insert Link", optionsMenu() );
        waitForTitle( "Select Source" );

        select( "Tasks" );
        waitForTitle( "Select Tasks" );
        select( "task2" );
        waitForTitle( "Edit Task" );

        // Confirm we're looking at Task Details of task1...
        select( "Back", softMenu() );
        waitForTitle( "Task Details" );
        verify( getText().indexOf("task1") == 0 );

        // which includes the link to task2...
        verify( getText().indexOf("task2") != -1 );

        // Select the link to task2
        keyClick( Qt.Key_Up );
        waitFor() { return getSelectedText().indexOf("task2") == 0 };
        select( "Select", softMenu() );

        // Confirm that the Task Details for task2 are now shown
        waitFor() { return getText().indexOf("task2") == 0 };

        // TODO: Should also include a link to a task from another application, eg, Contacts
    },

/*
    \req QTOPIA-1712
*/
    calendar_integration_data:
    {
        test1: [ "calendar task 1", new QDate(2007, 03, 01) ]
    },

    calendar_integration: function( taskName, taskDate )
    {
        // Create a new task
        create_task({ description: taskName, due: taskDate });

        // Launch Calendar
        gotoHome();
        startApplication( "Calendar" );
        waitForTitle( "Calendar" );

        // Go to the task date
        Calendar.enterDateKeyClicks( taskDate );
        waitFor() { return ( getList(softMenu()).contains("View") ); }
        select( "View", softMenu() );
        waitForTitle( "Task Details" );

        // Verify that the task detail are shown
        verify( getText().indexOf(taskName) == 0 );
        select( "Back", softMenu() );
        waitForTitle( "Calendar" );
    },

/*
    \req QTOPIA-1707
*/
    assigning_a_task_to_a_category_data:
    {
        manuallycreatinganewcategory:[ "Testtask", "Testcategory" ]
    },

    assigning_a_task_to_a_category: function(taskname, categoryname)
    {
        select( "View Category...", optionsMenu() );
        waitForTitle("Select Category");
        select( "New Category", optionsMenu() );
        waitForTitle( "New Category" );
        enter( categoryname );
        keyClick( Qt.Key_Back );
        waitForTitle("Select Category");
        verify( getList().contains(categoryname) );
        select( "Back", softMenu() );
        create_task({
            description: taskname,
            priority:    "2 - High",
            status:      "In Progress",
            percent:     20,
            due:         new QDate(2007, 03, 01),
            start:       new QDate(2007, 02, 28),
            categories:  [categoryname]
        });
    },

/*
    \req
    \groups Performance
*/
    perf_startup: function()
    {
        Performance.testStartupTime("Applications/Tasks");
    }
}

function edit_current_task(o)
{
    if (o.description != undefined) {
        // Enter the task description in the 'Desc.' field
        enter( o.description, "Desc." ) ;
    }
    if (o.priority != undefined) {
        // Change the 'Priority'
        select( o.priority, "Priority" ) ;
    }
    if (o.due != undefined) {
        // Enable the 'Due' check-box and verify that the date widget selector becomes enabled
        setChecked( true, "qpe/Tasks:Due:" );
        enter(o.due, "qpe/Tasks:Due:/Due:");
    }

    select( "Progress", tabBar() );

    if (o.status != undefined) {
        // Set the 'Status'
        select( o.status, "Status" );
    }
    if (o.percent != undefined) {
        // Verify that the '%' field becomes enabled
        compare(getText("Progress"), "0%");
        enter( new Number(o.percent).toString(), "Progress" );
    }
    if (o.start != undefined) {
        setChecked( true, "qpe/Tasks:Started:" );
        enter(o.start, signature("qpe/Tasks:Started:", 1));
    }
    if (o.complete != undefined) {
        // Bug - this ends up overwriting start date... (198458)

        // Enable the 'Completed' check-box and verify that the date widget selector becomes enabled
        setChecked( true, "qpe/Tasks:Completed:" );
        enter(o.complete, signature("qpe/Tasks:Completed:", 1));
    }

    select( "Notes", tabBar() );

    if (o.categories != undefined) {
        select( "Category" );
        waitForTitle( "Select Category" );

        select( "Uncheck all", optionsMenu() );

        for (var i=0; i<o.categories.length; ++i) {
            select(o.categories[i] );
        }
        select( "Back", softMenu() );
    }

    if (o.note != undefined) {
        enter(o.note, "Notes");
    }
}

String.prototype.verifyContains = function()
{
    obj = arguments[0];
    prefix = "";
    postfix = "";
    if (obj == undefined) return;
    if (arguments[1] != undefined) {
        obj = arguments[1];
        prefix = arguments[0];
    }
    if (arguments[2] != undefined) {
        postfix = arguments[2];
    }

    // If this contains ANY str in strs, verification succeeds.
    var strs = [];
    if (obj instanceof QDate) {
        var now = new QDate();
        strs.push(obj.toString("ddd, M/d/yy"));
        strs.push(obj.toString("ddd, yyyy-MM-dd"));
        strs.push(obj.toString("dddd, MMMM d, yyyy"));
        strs.push(obj.toString("ddd, d MMM"));
        if (obj == now) {
            strs.push("today");
            strs.push("Today");
        }
    } else {
        strs.push(obj.toString());
    }

    var match = false;
    var fullstr = "";
    var sep = "";
    for (var i = 0; i < strs.length && !match; ++i) {
        fullstr = fullstr + sep + "'" + prefix + strs[i] + postfix + "'";
        sep = ",";
        match = (this.indexOf(prefix + strs[i] + postfix) != -1);
    }

    verify(match, "Text '" + this + "' didn't contain one or more of: " + fullstr);
}

function verify_current_task(o)
{
    waitForTitle( "Task Details" );

    // Verify that the task is now displayed with the description, priority and completed status
    text = getText();

    if (o.description != undefined) text.toLowerCase().verifyContains( o.description.toLowerCase() );
    if (o.priority != undefined)    text.verifyContains( "Priority: " + o.priority.charAt(0) );
    if (o.status != undefined)      text.toLowerCase().verifyContains( "status: " + o.status.toLowerCase() );
    if (o.percent != undefined && o.percent >= 0 && o.percent <= 99)
                                    text.verifyContains( "Completed: ", o.percent, " percent");
    if (o.due != undefined)         text.verifyContains( "Due: ", o.due );
    if (o.start != undefined)       text.verifyContains( "Started: ", o.start );
    if (o.complete != undefined)    text.verifyContains( "Completed: ", o.complete );
    if (o.note != undefined)        text.verifyContains( "Notes: \n", o.note );

    if (undefined != o.categories && 0 != o.categories.length) {
        // Verify category
        select( "Back", softMenu() );
        waitForTitle( "Tasks" );

        select( "Options", softMenu() );
        verify( getList(optionsMenu()).contains("View Category..."));
        select("View Category...", optionsMenu());
        waitForTitle( "Select Category" );
        select("Unfiled") ; // this deselects all others

        for (var i=0; i<o.categories.length; ++i) {
            select(o.categories[i] );
        }
        select("Back", softMenu());

        // If we can reopen the task from the task list with category view on, all is good
        select(o.description);

        // Now go back and select "All" categories to prevent later problems
        select("Back", softMenu());

        verify( getList(optionsMenu()).contains("View Category..."));
        select("View Category...", optionsMenu());
        waitForTitle( "Select Category" );
        select("All") ; // this deselects all others
        select("Back", softMenu());
        select(o.description);
    }
}


function create_task(o)
{
    // Select 'New' from the context menu
    select( "New task", optionsMenu() );

    // Verify that a 'New Task' dialog is displayed
    waitForTitle( "New Task" );

    // Enter the task details
    edit_current_task(o);
    select("Back", softMenu());

    select("View Category...", optionsMenu());
    waitForTitle( "Select Category" );
    select("All");
    select("Back", softMenu());

    select(o.description);

    // Verify that the task is now displayed with the description, priority and completed status
    verify_current_task(o);
    select("Back", softMenu());
    waitForTitle("Tasks");
}

