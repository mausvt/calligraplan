/* This file is part of the KDE project
  Copyright (C) 2004-2007 Dag Andersen <danders@get2net.dk>
  Copyright (C) 2011 Dag Andersen <danders@get2net.dk>
  Copyright (C) 2016 Dag Andersen <danders@get2net.dk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#ifndef KPTCOMMAND_H
#define KPTCOMMAND_H

#include "plankernel_export.h"

#include <kundo2command.h>

#include <QPointer>
#include <QHash>

#include "kptappointment.h"
#include "kptnode.h"
#include "kptduration.h"
#include "kpttask.h"
#include "kptwbsdefinition.h"

class QString;
/**
 * @file
 * This file includes undo/redo commands for kernel data structures
 */

/// The main namespace
namespace KPlato
{

class Locale;
class Account;
class Accounts;
class Project;
class Task;
class Calendar;
class CalendarDay;
class Relation;
class ResourceGroupRequest;
class ResourceRequest;
class ResourceGroup;
class Resource;
class Schedule;
class StandardWorktime;

class PLANKERNEL_EXPORT NamedCommand : public KUndo2Command
{
public:
    explicit NamedCommand( const KUndo2MagicString& name )
        : KUndo2Command( name )
    {}
    virtual void redo() { execute(); }
    virtual void undo() { unexecute(); }

    virtual void execute() = 0;
    virtual void unexecute() = 0;

protected:
    /// Set all scheduled in the m_schedules map to their original scheduled state
    void setSchScheduled();
    /// Set all schedules in the m_schedules map to scheduled state @p state
    void setSchScheduled( bool state );
    /// Add a schedule to the m_schedules map along with its current scheduled state
    void addSchScheduled( Schedule *sch );

    QHash<Schedule*, bool> m_schedules;

};

class PLANKERNEL_EXPORT MacroCommand : public KUndo2Command
{
public:
    explicit MacroCommand( const KUndo2MagicString& name = KUndo2MagicString() )
        : KUndo2Command( name )
    {}
    ~MacroCommand();

    void addCommand( KUndo2Command *cmd );

    virtual void redo() { execute(); }
    virtual void undo() { unexecute(); }

    virtual void execute();
    virtual void unexecute();

    bool isEmpty() const { return cmds.isEmpty(); }

protected:
    QList<KUndo2Command*> cmds;
};


class PLANKERNEL_EXPORT CalendarAddCmd : public NamedCommand
{
public:
    CalendarAddCmd( Project *project, Calendar *cal, int pos, Calendar *parent, const KUndo2MagicString& name = KUndo2MagicString() );
    ~CalendarAddCmd();
    void execute();
    void unexecute();

private:
    Project *m_project;
    Calendar *m_cal;
    int m_pos;
    Calendar *m_parent;
    bool m_mine;
};

class PLANKERNEL_EXPORT CalendarRemoveCmd : public NamedCommand
{
public:
    CalendarRemoveCmd( Project *project, Calendar *cal, const KUndo2MagicString& name = KUndo2MagicString() );
    ~CalendarRemoveCmd();
    void execute();
    void unexecute();

private:
    Project *m_project;
    Calendar *m_parent;
    Calendar *m_cal;
    int m_index;
    bool m_mine;
    MacroCommand *m_cmd;
};

class PLANKERNEL_EXPORT CalendarMoveCmd : public NamedCommand
{
public:
    CalendarMoveCmd( Project *project, Calendar *cal, int position, Calendar *parent, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project *m_project;
    Calendar *m_cal;
    int m_newpos;
    int m_oldpos;
    Calendar *m_newparent;
    Calendar *m_oldparent;
};

class PLANKERNEL_EXPORT CalendarModifyNameCmd : public NamedCommand
{
public:
    CalendarModifyNameCmd( Calendar *cal, const QString& newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Calendar *m_cal;
    QString m_newvalue;
    QString m_oldvalue;
};

class PLANKERNEL_EXPORT CalendarModifyParentCmd : public NamedCommand
{
public:
    CalendarModifyParentCmd( Project *project, Calendar *cal, Calendar *newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    ~CalendarModifyParentCmd();
    void execute();
    void unexecute();

private:
    Project *m_project;
    Calendar *m_cal;
    Calendar *m_newvalue;
    Calendar *m_oldvalue;
    MacroCommand *m_cmd;

    int m_oldindex;
    int m_newindex;
};

class PLANKERNEL_EXPORT CalendarModifyTimeZoneCmd : public NamedCommand
{
public:
    CalendarModifyTimeZoneCmd( Calendar *cal, const QTimeZone &value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~CalendarModifyTimeZoneCmd();
    void execute();
    void unexecute();

private:
    Calendar *m_cal;
    QTimeZone m_newvalue;
    QTimeZone m_oldvalue;
    MacroCommand *m_cmd;
};

#ifdef HAVE_KHOLIDAYS
class PLANKERNEL_EXPORT CalendarModifyHolidayRegionCmd : public NamedCommand
{
public:
    CalendarModifyHolidayRegionCmd( Calendar *cal, const QString &value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~CalendarModifyHolidayRegionCmd();
    void execute();
    void unexecute();

private:
    Calendar *m_cal;
    QString m_newvalue;
    QString m_oldvalue;
};
#endif

class PLANKERNEL_EXPORT CalendarAddDayCmd : public NamedCommand
{
public:
    CalendarAddDayCmd( Calendar *cal, CalendarDay *newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    ~CalendarAddDayCmd();
    void execute();
    void unexecute();

protected:
    Calendar *m_cal;
    CalendarDay *m_newvalue;
    bool m_mine;
};

class PLANKERNEL_EXPORT CalendarRemoveDayCmd : public NamedCommand
{
public:
    CalendarRemoveDayCmd( Calendar *cal, CalendarDay *day, const KUndo2MagicString& name = KUndo2MagicString() );
    CalendarRemoveDayCmd( Calendar *cal, const QDate &day, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

protected:
    Calendar *m_cal;
    CalendarDay *m_value;
    bool m_mine;

private:
    void init();
};

class PLANKERNEL_EXPORT CalendarModifyDayCmd : public NamedCommand
{
public:
    CalendarModifyDayCmd( Calendar *cal, CalendarDay *value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~CalendarModifyDayCmd();
    void execute();
    void unexecute();

private:
    Calendar *m_cal;
    CalendarDay *m_newvalue;
    CalendarDay *m_oldvalue;
    bool m_mine;
};

class PLANKERNEL_EXPORT CalendarModifyStateCmd : public NamedCommand
{
public:
    CalendarModifyStateCmd( Calendar *calendar, CalendarDay *day, CalendarDay::State value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~CalendarModifyStateCmd();
    void execute();
    void unexecute();

private:
    Calendar *m_calendar;
    CalendarDay *m_day;
    CalendarDay::State m_newvalue;
    CalendarDay::State m_oldvalue;
    MacroCommand *m_cmd;
};

class PLANKERNEL_EXPORT CalendarModifyTimeIntervalCmd : public NamedCommand
{
public:
    CalendarModifyTimeIntervalCmd( Calendar *calendar, TimeInterval &newvalue, TimeInterval *value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Calendar *m_calendar;
    TimeInterval *m_value;
    TimeInterval m_newvalue;
    TimeInterval m_oldvalue;
};

class PLANKERNEL_EXPORT CalendarAddTimeIntervalCmd : public NamedCommand
{
public:
    CalendarAddTimeIntervalCmd( Calendar *calendar, CalendarDay *day, TimeInterval *value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~CalendarAddTimeIntervalCmd();
    void execute();
    void unexecute();

protected:
    Calendar *m_calendar;
    CalendarDay *m_day;
    TimeInterval *m_value;
    bool m_mine;
};

class PLANKERNEL_EXPORT CalendarRemoveTimeIntervalCmd : public CalendarAddTimeIntervalCmd
{
public:
    CalendarRemoveTimeIntervalCmd( Calendar *calendar, CalendarDay *day, TimeInterval *value, const KUndo2MagicString& name = KUndo2MagicString() );

    void execute();
    void unexecute();
};

class PLANKERNEL_EXPORT CalendarModifyWeekdayCmd : public NamedCommand
{
public:
    CalendarModifyWeekdayCmd( Calendar *cal, int weekday, CalendarDay *value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~CalendarModifyWeekdayCmd();
    void execute();
    void unexecute();

private:
    int m_weekday;
    Calendar *m_cal;
    CalendarDay *m_value;
    CalendarDay m_orig;
};

class PLANKERNEL_EXPORT CalendarModifyDateCmd : public NamedCommand
{
public:
    CalendarModifyDateCmd( Calendar *cal, CalendarDay *day, const QDate &value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Calendar *m_cal;
    CalendarDay *m_day;
    QDate m_newvalue, m_oldvalue;
};

class PLANKERNEL_EXPORT ProjectModifyDefaultCalendarCmd : public NamedCommand
{
public:
    ProjectModifyDefaultCalendarCmd( Project *project, Calendar *cal, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project *m_project;
    Calendar *m_newvalue, *m_oldvalue;
};


class PLANKERNEL_EXPORT NodeDeleteCmd : public NamedCommand
{
public:
    explicit NodeDeleteCmd( Node *node, const KUndo2MagicString& name = KUndo2MagicString() );
    ~NodeDeleteCmd();
    void execute();
    void unexecute();

private:
    Node *m_node;
    Node *m_parent;
    Project *m_project;
    int m_index;
    bool m_mine;
    QList<Appointment*> m_appointments;
    MacroCommand *m_cmd;
    MacroCommand *m_relCmd;
};

class PLANKERNEL_EXPORT TaskAddCmd : public NamedCommand
{
public:
    TaskAddCmd( Project *project, Node *node, Node *after, const KUndo2MagicString& name = KUndo2MagicString() );
    ~TaskAddCmd();
    void execute();
    void unexecute();

private:
    Project *m_project;
    Node *m_node;
    Node *m_after;
    bool m_added;
};

class PLANKERNEL_EXPORT SubtaskAddCmd : public NamedCommand
{
public:
    SubtaskAddCmd( Project *project, Node *node, Node *parent, const KUndo2MagicString& name = KUndo2MagicString() );
    ~SubtaskAddCmd();
    void execute();
    void unexecute();

private:
    Project *m_project;
    Node *m_node;
    Node *m_parent;
    bool m_added;
    MacroCommand *m_cmd;
};


class PLANKERNEL_EXPORT NodeModifyNameCmd : public NamedCommand
{
public:
    NodeModifyNameCmd( Node &node, const QString& nodename, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    QString newName;
    QString oldName;
};

class PLANKERNEL_EXPORT NodeModifyLeaderCmd : public NamedCommand
{
public:
    NodeModifyLeaderCmd( Node &node, const QString& leader, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    QString newLeader;
    QString oldLeader;
};

class PLANKERNEL_EXPORT NodeModifyDescriptionCmd : public NamedCommand
{
public:
    NodeModifyDescriptionCmd( Node &node, const QString& description, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    QString newDescription;
    QString oldDescription;
};

class PLANKERNEL_EXPORT NodeModifyConstraintCmd : public NamedCommand
{
public:
    NodeModifyConstraintCmd( Node &node, Node::ConstraintType c, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    Node::ConstraintType newConstraint;
    Node::ConstraintType oldConstraint;

};

class PLANKERNEL_EXPORT NodeModifyConstraintStartTimeCmd : public NamedCommand
{
public:
    NodeModifyConstraintStartTimeCmd( Node &node, const QDateTime& dt, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    QDateTime newTime;
    DateTime oldTime;
    QTimeZone m_timeZone;
};
class PLANKERNEL_EXPORT NodeModifyConstraintEndTimeCmd : public NamedCommand
{
public:
    NodeModifyConstraintEndTimeCmd( Node &node, const QDateTime& dt, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    QDateTime newTime;
    DateTime oldTime;
    QTimeZone m_timeZone;
};
class PLANKERNEL_EXPORT NodeModifyStartTimeCmd : public NamedCommand
{
public:
    NodeModifyStartTimeCmd( Node &node, const QDateTime& dt, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    QDateTime newTime;
    DateTime oldTime;
    QTimeZone m_timeZone;
};
class PLANKERNEL_EXPORT NodeModifyEndTimeCmd : public NamedCommand
{
public:
    NodeModifyEndTimeCmd( Node &node, const QDateTime& dt, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    QDateTime newTime;
    DateTime oldTime;
    QTimeZone m_timeZone;
};
class PLANKERNEL_EXPORT NodeModifyIdCmd : public NamedCommand
{
public:
    NodeModifyIdCmd( Node &node, const QString& id, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    QString newId;
    QString oldId;
};

class PLANKERNEL_EXPORT NodeIndentCmd : public NamedCommand
{
public:
    explicit NodeIndentCmd( Node &node, const KUndo2MagicString& name = KUndo2MagicString() );
    ~NodeIndentCmd();
    void execute();
    void unexecute();

private:
    Node &m_node;
    Node *m_oldparent, *m_newparent;
    int m_oldindex, m_newindex;
    MacroCommand *m_cmd;
};

class PLANKERNEL_EXPORT NodeUnindentCmd : public NamedCommand
{
public:
    explicit NodeUnindentCmd( Node &node, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    Node *m_oldparent, *m_newparent;
    int m_oldindex, m_newindex;
};

class PLANKERNEL_EXPORT NodeMoveUpCmd : public NamedCommand
{
public:
    explicit NodeMoveUpCmd( Node &node, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    Project *m_project;
    bool m_moved;
};

class PLANKERNEL_EXPORT NodeMoveDownCmd : public NamedCommand
{
public:
    explicit NodeMoveDownCmd( Node &node, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    Project *m_project;
    bool m_moved;
};

class PLANKERNEL_EXPORT NodeMoveCmd : public NamedCommand
{
public:
    NodeMoveCmd( Project *project, Node *node, Node *newParent, int newPos, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project *m_project;
    Node *m_node;
    Node *m_newparent;
    Node *m_oldparent;
    int m_newpos;
    int m_oldpos;
    bool m_moved;
    MacroCommand m_cmd;
};

class PLANKERNEL_EXPORT AddRelationCmd : public NamedCommand
{
public:
    AddRelationCmd( Project &project, Relation *rel, const KUndo2MagicString& name = KUndo2MagicString() );
    ~AddRelationCmd();
    void execute();
    void unexecute();

private:
    Relation *m_rel;
    Project &m_project;
    bool m_taken;

};

class PLANKERNEL_EXPORT DeleteRelationCmd : public NamedCommand
{
public:
    DeleteRelationCmd( Project &project, Relation *rel, const KUndo2MagicString& name = KUndo2MagicString() );
    ~DeleteRelationCmd();
    void execute();
    void unexecute();

private:
    Relation *m_rel;
    Project &m_project;
    bool m_taken;

};

class PLANKERNEL_EXPORT ModifyRelationTypeCmd : public NamedCommand
{
public:
    ModifyRelationTypeCmd( Relation *rel, Relation::Type type, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project *m_project;
    Relation *m_rel;
    Relation::Type m_newtype;
    Relation::Type m_oldtype;

};

class PLANKERNEL_EXPORT ModifyRelationLagCmd : public NamedCommand
{
public:
    ModifyRelationLagCmd( Relation *rel, Duration lag, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project *m_project;
    Relation *m_rel;
    Duration m_newlag;
    Duration m_oldlag;

};

class PLANKERNEL_EXPORT AddResourceRequestCmd : public NamedCommand
{
public:
    AddResourceRequestCmd( ResourceGroupRequest *group, ResourceRequest *request, const KUndo2MagicString& name = KUndo2MagicString() );
    ~AddResourceRequestCmd();
    void execute();
    void unexecute();

private:
    ResourceGroupRequest *m_group;
    ResourceRequest *m_request;
    bool m_mine;

};

class PLANKERNEL_EXPORT RemoveResourceRequestCmd : public NamedCommand
{
public:
    RemoveResourceRequestCmd( ResourceGroupRequest *group, ResourceRequest *request, const KUndo2MagicString& name = KUndo2MagicString() );
    ~RemoveResourceRequestCmd();
    void execute();
    void unexecute();

private:
    ResourceGroupRequest *m_group;
    ResourceRequest *m_request;
    bool m_mine;

};

class PLANKERNEL_EXPORT ModifyResourceRequestUnitsCmd : public NamedCommand
{
public:
    ModifyResourceRequestUnitsCmd( ResourceRequest *request, int oldvalue, int newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ResourceRequest *m_request;
    int m_oldvalue, m_newvalue;

};

class PLANKERNEL_EXPORT ModifyResourceRequestRequiredCmd : public NamedCommand
{
public:
    ModifyResourceRequestRequiredCmd( ResourceRequest *request, const QList<Resource*> &value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ResourceRequest *m_request;
    QList<Resource*> m_oldvalue, m_newvalue;

};

class PLANKERNEL_EXPORT ModifyResourceGroupRequestUnitsCmd : public NamedCommand
{
public:
    ModifyResourceGroupRequestUnitsCmd( ResourceGroupRequest *request, int oldvalue, int newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ResourceGroupRequest *m_request;
    int m_oldvalue, m_newvalue;

};

class PLANKERNEL_EXPORT ModifyEstimateCmd : public NamedCommand
{
public:
    ModifyEstimateCmd( Node &node, double oldvalue, double newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    ~ModifyEstimateCmd();
    void execute();
    void unexecute();

private:
    Estimate *m_estimate;
    double m_oldvalue, m_newvalue;
    int m_optimistic, m_pessimistic;
    MacroCommand *m_cmd;

};

class PLANKERNEL_EXPORT EstimateModifyOptimisticRatioCmd : public NamedCommand
{
public:
    EstimateModifyOptimisticRatioCmd( Node &node, int oldvalue, int newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Estimate *m_estimate;
    int m_oldvalue, m_newvalue;

};

class PLANKERNEL_EXPORT EstimateModifyPessimisticRatioCmd : public NamedCommand
{
public:
    EstimateModifyPessimisticRatioCmd( Node &node, int oldvalue, int newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Estimate *m_estimate;
    int m_oldvalue, m_newvalue;

};

class PLANKERNEL_EXPORT ModifyEstimateTypeCmd : public NamedCommand
{
public:
    ModifyEstimateTypeCmd( Node &node, int oldvalue, int newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Estimate *m_estimate;
    int m_oldvalue, m_newvalue;

};

class PLANKERNEL_EXPORT ModifyEstimateUnitCmd : public NamedCommand
{
public:
    ModifyEstimateUnitCmd( Node &node, Duration::Unit oldvalue, Duration::Unit newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Estimate *m_estimate;
    Duration::Unit m_oldvalue, m_newvalue;
};

class PLANKERNEL_EXPORT EstimateModifyRiskCmd : public NamedCommand
{
public:
    EstimateModifyRiskCmd( Node &node, int oldvalue, int newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Estimate *m_estimate;
    int m_oldvalue, m_newvalue;

};

class PLANKERNEL_EXPORT ModifyEstimateCalendarCmd : public NamedCommand
{
public:
    ModifyEstimateCalendarCmd( Node &node, Calendar *oldvalue, Calendar *newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Estimate *m_estimate;
    Calendar *m_oldvalue, *m_newvalue;

};


class PLANKERNEL_EXPORT AddResourceGroupRequestCmd : public NamedCommand
{
public:
    AddResourceGroupRequestCmd( Task &task, ResourceGroupRequest *request, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Task &m_task;
    ResourceGroupRequest *m_request;
    bool m_mine;
};

class PLANKERNEL_EXPORT RemoveResourceGroupRequestCmd : public NamedCommand
{
public:
    explicit RemoveResourceGroupRequestCmd( ResourceGroupRequest *request, const KUndo2MagicString& name = KUndo2MagicString() );
    RemoveResourceGroupRequestCmd( Task &task, ResourceGroupRequest *request, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Task &m_task;
    ResourceGroupRequest *m_request;
    bool m_mine;
};

class PLANKERNEL_EXPORT AddResourceCmd : public NamedCommand
{
public:
    AddResourceCmd( ResourceGroup *group, Resource *resource, const KUndo2MagicString& name = KUndo2MagicString() );
    ~AddResourceCmd();
    void execute();
    void unexecute();

protected:

    ResourceGroup *m_group;
    Resource *m_resource;
    int m_index;
    bool m_mine;
};

class PLANKERNEL_EXPORT RemoveResourceCmd : public AddResourceCmd
{
public:
    RemoveResourceCmd( ResourceGroup *group, Resource *resource, const KUndo2MagicString& name = KUndo2MagicString() );
    ~RemoveResourceCmd();
    void execute();
    void unexecute();

private:
    QList<ResourceRequest*> m_requests;
    QList<Appointment*> m_appointments;
    MacroCommand m_cmd;
};

class PLANKERNEL_EXPORT MoveResourceCmd : public NamedCommand
{
public:
    MoveResourceCmd( ResourceGroup *group, Resource *resource, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project &m_project;
    Resource *m_resource;
    ResourceGroup *m_oldvalue, *m_newvalue;
    MacroCommand cmd;
};

class PLANKERNEL_EXPORT ModifyResourceNameCmd : public NamedCommand
{
public:
    ModifyResourceNameCmd( Resource *resource, const QString& value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:

    Resource *m_resource;
    QString m_newvalue;
    QString m_oldvalue;
};
class PLANKERNEL_EXPORT ModifyResourceInitialsCmd : public NamedCommand
{
public:
    ModifyResourceInitialsCmd( Resource *resource, const QString& value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    QString m_newvalue;
    QString m_oldvalue;
};
class PLANKERNEL_EXPORT ModifyResourceEmailCmd : public NamedCommand
{
public:
    ModifyResourceEmailCmd( Resource *resource, const QString& value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    QString m_newvalue;
    QString m_oldvalue;
};
class PLANKERNEL_EXPORT ModifyResourceAutoAllocateCmd : public NamedCommand
{
public:
    ModifyResourceAutoAllocateCmd( Resource *resource, bool value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    bool m_newvalue;
    bool m_oldvalue;
};
class PLANKERNEL_EXPORT ModifyResourceTypeCmd : public NamedCommand
{
public:
    ModifyResourceTypeCmd( Resource *resource, int value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    int m_newvalue;
    int m_oldvalue;
};

class PLANKERNEL_EXPORT ModifyResourceUnitsCmd : public NamedCommand
{
public:
    ModifyResourceUnitsCmd( Resource *resource, int value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    int m_newvalue;
    int m_oldvalue;
};

class PLANKERNEL_EXPORT ModifyResourceAvailableFromCmd : public NamedCommand
{
public:
    ModifyResourceAvailableFromCmd( Resource *resource, const QDateTime& value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    QDateTime m_newvalue;
    DateTime m_oldvalue;
    QTimeZone m_timeZone;
};
class PLANKERNEL_EXPORT ModifyResourceAvailableUntilCmd : public NamedCommand
{
public:
    ModifyResourceAvailableUntilCmd( Resource *resource, const QDateTime& value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    QDateTime m_newvalue;
    DateTime m_oldvalue;
    QTimeZone m_timeZone;
};

class PLANKERNEL_EXPORT ModifyResourceNormalRateCmd : public NamedCommand
{
public:
    ModifyResourceNormalRateCmd( Resource *resource, double value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    double m_newvalue;
    double m_oldvalue;
};
class PLANKERNEL_EXPORT ModifyResourceOvertimeRateCmd : public NamedCommand
{
public:
    ModifyResourceOvertimeRateCmd( Resource *resource, double value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    double m_newvalue;
    double m_oldvalue;
};
class PLANKERNEL_EXPORT ModifyResourceCalendarCmd : public NamedCommand
{
public:
    ModifyResourceCalendarCmd( Resource *resource, Calendar *value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    Calendar *m_newvalue;
    Calendar *m_oldvalue;
};
class PLANKERNEL_EXPORT ModifyRequiredResourcesCmd : public NamedCommand
{
public:
    ModifyRequiredResourcesCmd( Resource *resource, const QStringList &value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    QStringList m_newvalue;
    QStringList m_oldvalue;
};
class PLANKERNEL_EXPORT ModifyResourceAccountCmd : public NamedCommand
{
public:
    ModifyResourceAccountCmd( Resource *resource, Account *account, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    Account *m_newvalue;
    Account *m_oldvalue;
};
class PLANKERNEL_EXPORT AddResourceTeamCmd : public NamedCommand
{
public:
    AddResourceTeamCmd( Resource *team, const QString &member, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_team;
    QString m_member;
};
class PLANKERNEL_EXPORT RemoveResourceTeamCmd : public NamedCommand
{
public:
    RemoveResourceTeamCmd( Resource *team, const QString &member, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_team;
    QString m_member;
};

class PLANKERNEL_EXPORT RemoveResourceGroupCmd : public NamedCommand
{
public:
    RemoveResourceGroupCmd( Project *project, ResourceGroup *group, const KUndo2MagicString& name = KUndo2MagicString() );
    ~RemoveResourceGroupCmd();
    void execute();
    void unexecute();

protected:

    ResourceGroup *m_group;
    Project *m_project;
    int m_index;
    bool m_mine;
    MacroCommand *m_cmd;
};

class PLANKERNEL_EXPORT AddResourceGroupCmd : public RemoveResourceGroupCmd
{
public:
    AddResourceGroupCmd( Project *project, ResourceGroup *group, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();
};

class PLANKERNEL_EXPORT ModifyResourceGroupNameCmd : public NamedCommand
{
public:
    ModifyResourceGroupNameCmd( ResourceGroup *group, const QString& value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ResourceGroup *m_group;
    QString m_newvalue;
    QString m_oldvalue;
};

class PLANKERNEL_EXPORT ModifyResourceGroupTypeCmd : public NamedCommand
{
    public:
        ModifyResourceGroupTypeCmd( ResourceGroup *group, int value, const KUndo2MagicString& name = KUndo2MagicString() );
        void execute();
        void unexecute();

    private:
        ResourceGroup *m_group;
        int m_newvalue;
        int m_oldvalue;
};

class PLANKERNEL_EXPORT ModifyCompletionEntrymodeCmd : public NamedCommand
{
public:
    ModifyCompletionEntrymodeCmd( Completion &completion, Completion::Entrymode value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Completion &m_completion;
    Completion::Entrymode oldvalue;
    Completion::Entrymode newvalue;
};

class PLANKERNEL_EXPORT ModifyCompletionStartedCmd : public NamedCommand
{
public:
    ModifyCompletionStartedCmd( Completion &completion, bool value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Completion &m_completion;
    bool oldvalue;
    bool newvalue;
};

class PLANKERNEL_EXPORT ModifyCompletionFinishedCmd : public NamedCommand
{
public:
    ModifyCompletionFinishedCmd( Completion &completion, bool value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Completion &m_completion;
    bool oldvalue;
    bool newvalue;
};

class PLANKERNEL_EXPORT ModifyCompletionStartTimeCmd : public NamedCommand
{
public:
    ModifyCompletionStartTimeCmd( Completion &completion, const QDateTime &value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Completion &m_completion;
    DateTime oldvalue;
    QDateTime newvalue;
    QTimeZone m_timeZone;
};

class PLANKERNEL_EXPORT ModifyCompletionFinishTimeCmd : public NamedCommand
{
public:
    ModifyCompletionFinishTimeCmd( Completion &completion, const QDateTime &value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Completion &m_completion;
    DateTime oldvalue;
    QDateTime newvalue;
    QTimeZone m_timeZone;
};

class PLANKERNEL_EXPORT AddCompletionEntryCmd : public NamedCommand
{
public:
    AddCompletionEntryCmd( Completion &completion, const QDate &date, Completion::Entry *value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~AddCompletionEntryCmd();
    void execute();
    void unexecute();

private:
    Completion &m_completion;
    QDate m_date;
    Completion::Entry *newvalue;
    bool m_newmine;
};

class PLANKERNEL_EXPORT RemoveCompletionEntryCmd : public NamedCommand
{
public:
    RemoveCompletionEntryCmd( Completion &completion, const QDate& date, const KUndo2MagicString& name = KUndo2MagicString() );
    ~RemoveCompletionEntryCmd();
    void execute();
    void unexecute();

private:
    Completion &m_completion;
    QDate m_date;
    Completion::Entry *value;
    bool m_mine;
};

class PLANKERNEL_EXPORT ModifyCompletionEntryCmd : public NamedCommand
{
public:
    ModifyCompletionEntryCmd( Completion &completion, const QDate &date, Completion::Entry *value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~ModifyCompletionEntryCmd();
    void execute();
    void unexecute();

private:
    MacroCommand *cmd;
};

class PLANKERNEL_EXPORT ModifyCompletionPercentFinishedCmd : public NamedCommand
{
public:
    ModifyCompletionPercentFinishedCmd( Completion &completion, const QDate &date, int value, const KUndo2MagicString& name = KUndo2MagicString() );

    void execute();
    void unexecute();

private:
    Completion &m_completion;
    QDate m_date;
    int m_newvalue, m_oldvalue;
    MacroCommand cmd;
};

class PLANKERNEL_EXPORT ModifyCompletionRemainingEffortCmd : public NamedCommand
{
public:
    ModifyCompletionRemainingEffortCmd( Completion &completion, const QDate &date, const Duration &value, const KUndo2MagicString &name = KUndo2MagicString() );

    void execute();
    void unexecute();

private:
    Completion &m_completion;
    QDate m_date;
    Duration m_newvalue, m_oldvalue;
    MacroCommand cmd;
};

class PLANKERNEL_EXPORT ModifyCompletionActualEffortCmd : public NamedCommand
{
public:
    ModifyCompletionActualEffortCmd( Completion &completion, const QDate &date, const Duration &value, const KUndo2MagicString &name = KUndo2MagicString() );

    void execute();
    void unexecute();

private:
    Completion &m_completion;
    QDate m_date;
    Duration m_newvalue, m_oldvalue;
    MacroCommand cmd;
};

/**
 * Add used effort for @p resource.
 * Note that the used effort definition in @p value must contain entries for *all* dates.
 * If used effort is already defined it will be replaced.
 */
class PLANKERNEL_EXPORT AddCompletionUsedEffortCmd : public NamedCommand
{
public:
    AddCompletionUsedEffortCmd( Completion &completion, const Resource *resource, Completion::UsedEffort *value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~AddCompletionUsedEffortCmd();
    void execute();
    void unexecute();

private:
    Completion &m_completion;
    const Resource *m_resource;
    Completion::UsedEffort *oldvalue;
    Completion::UsedEffort *newvalue;
    bool m_newmine, m_oldmine;
};

class PLANKERNEL_EXPORT AddCompletionActualEffortCmd : public NamedCommand
{
public:
    AddCompletionActualEffortCmd( Completion::UsedEffort &ue, const QDate &date, const Completion::UsedEffort::ActualEffort &value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~AddCompletionActualEffortCmd();
    void execute();
    void unexecute();

private:
    Completion::UsedEffort &m_usedEffort;
    QDate m_date;
    Completion::UsedEffort::ActualEffort oldvalue;
    Completion::UsedEffort::ActualEffort newvalue;
};


class PLANKERNEL_EXPORT AddAccountCmd : public NamedCommand
{
public:
    AddAccountCmd( Project &project, Account *account, Account *parent = 0, int index = -1, const KUndo2MagicString& name = KUndo2MagicString() );
    AddAccountCmd( Project &project, Account *account, const QString& parent, int index = -1, const KUndo2MagicString& name = KUndo2MagicString() );
    ~AddAccountCmd();
    void execute();
    void unexecute();

protected:
    bool m_mine;

private:
    Project &m_project;
    Account *m_account;
    Account *m_parent;
    int m_index;
    QString m_parentName;
};

class PLANKERNEL_EXPORT RemoveAccountCmd : public NamedCommand
{
public:
    RemoveAccountCmd( Project &project, Account *account, const KUndo2MagicString& name = KUndo2MagicString() );
    ~RemoveAccountCmd();
    void execute();
    void unexecute();

private:
    Project &m_project;
    Account *m_account;
    Account *m_parent;
    int m_index;
    bool m_isDefault;
    bool m_mine;
    MacroCommand m_cmd;
};

class PLANKERNEL_EXPORT RenameAccountCmd : public NamedCommand
{
public:
    RenameAccountCmd( Account *account, const QString& value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Account *m_account;
    QString m_oldvalue;
    QString m_newvalue;
};

class PLANKERNEL_EXPORT ModifyAccountDescriptionCmd : public NamedCommand
{
public:
    ModifyAccountDescriptionCmd( Account *account, const QString& value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Account *m_account;
    QString m_oldvalue;
    QString m_newvalue;
};

class PLANKERNEL_EXPORT NodeModifyStartupCostCmd : public NamedCommand
{
public:
    NodeModifyStartupCostCmd( Node &node, double value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    double m_oldvalue;
    double m_newvalue;
};

class PLANKERNEL_EXPORT NodeModifyShutdownCostCmd : public NamedCommand
{
public:
    NodeModifyShutdownCostCmd( Node &node, double value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    double m_oldvalue;
    double m_newvalue;
};

class PLANKERNEL_EXPORT NodeModifyRunningAccountCmd : public NamedCommand
{
public:
    NodeModifyRunningAccountCmd( Node &node, Account *oldvalue, Account *newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    Account *m_oldvalue;
    Account *m_newvalue;
};

class PLANKERNEL_EXPORT NodeModifyStartupAccountCmd : public NamedCommand
{
public:
    NodeModifyStartupAccountCmd( Node &node, Account *oldvalue, Account *newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    Account *m_oldvalue;
    Account *m_newvalue;
};

class PLANKERNEL_EXPORT NodeModifyShutdownAccountCmd : public NamedCommand
{
public:
    NodeModifyShutdownAccountCmd( Node &node, Account *oldvalue, Account *newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Node &m_node;
    Account *m_oldvalue;
    Account *m_newvalue;
};

class PLANKERNEL_EXPORT ModifyDefaultAccountCmd : public NamedCommand
{
public:
    ModifyDefaultAccountCmd( Accounts &acc, Account *oldvalue, Account *newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Accounts &m_accounts;
    Account *m_oldvalue;
    Account *m_newvalue;
};

class PLANKERNEL_EXPORT ResourceModifyAccountCmd : public NamedCommand
{
public:
    ResourceModifyAccountCmd( Resource &resource, Account *oldvalue, Account *newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource &m_resource;
    Account *m_oldvalue;
    Account *m_newvalue;
};

class PLANKERNEL_EXPORT ProjectModifyConstraintCmd : public NamedCommand
{
public:
    ProjectModifyConstraintCmd( Project &node, Node::ConstraintType c, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project &m_node;
    Node::ConstraintType newConstraint;
    Node::ConstraintType oldConstraint;

};

class PLANKERNEL_EXPORT ProjectModifyStartTimeCmd : public NamedCommand
{
public:
    ProjectModifyStartTimeCmd( Project &node, const QDateTime& dt, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project &m_node;
    QDateTime newTime;
    DateTime oldTime;
    QTimeZone m_timeZone;
};

class PLANKERNEL_EXPORT ProjectModifyEndTimeCmd : public NamedCommand
{
public:
    ProjectModifyEndTimeCmd( Project &project, const QDateTime& dt, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project &m_node;
    QDateTime newTime;
    DateTime oldTime;
    QTimeZone m_timeZone;
};


class PLANKERNEL_EXPORT AddScheduleManagerCmd : public NamedCommand
{
public:
    AddScheduleManagerCmd( Project &project, ScheduleManager *sm, int index = -1, const KUndo2MagicString& name = KUndo2MagicString() );
    AddScheduleManagerCmd( ScheduleManager *parent, ScheduleManager *sm, int index = -1, const KUndo2MagicString& name = KUndo2MagicString() );
    ~AddScheduleManagerCmd();
    void execute();
    void unexecute();

protected:
    Project &m_node;
    ScheduleManager *m_parent;
    ScheduleManager *m_sm;
    int m_index;
    MainSchedule *m_exp;
    bool m_mine;
};

class PLANKERNEL_EXPORT DeleteScheduleManagerCmd : public AddScheduleManagerCmd
{
public:
    DeleteScheduleManagerCmd( Project &project, ScheduleManager *sm, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    MacroCommand cmd;
};

class PLANKERNEL_EXPORT MoveScheduleManagerCmd : public NamedCommand
{
public:
    MoveScheduleManagerCmd( ScheduleManager *sm, ScheduleManager *newparent, int newindex, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ScheduleManager *m_sm;
    ScheduleManager *m_oldparent;
    int m_oldindex;
    ScheduleManager *m_newparent;
    int m_newindex;
    MacroCommand m_cmd;
};

class PLANKERNEL_EXPORT ModifyScheduleManagerNameCmd : public NamedCommand
{
public:
    ModifyScheduleManagerNameCmd( ScheduleManager &sm, const QString& value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ScheduleManager &m_sm;
    QString oldvalue, newvalue;
};

class PLANKERNEL_EXPORT ModifyScheduleManagerAllowOverbookingCmd : public NamedCommand
{
public:
    ModifyScheduleManagerAllowOverbookingCmd( ScheduleManager &sm, bool value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ScheduleManager &m_sm;
    bool oldvalue, newvalue;
};

class PLANKERNEL_EXPORT ModifyScheduleManagerDistributionCmd : public NamedCommand
{
public:
    ModifyScheduleManagerDistributionCmd( ScheduleManager &sm, bool value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ScheduleManager &m_sm;
    bool oldvalue, newvalue;
};

class PLANKERNEL_EXPORT CalculateScheduleCmd : public NamedCommand
{
public:
    CalculateScheduleCmd( Project &project, ScheduleManager *sm, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project &m_node;
    QPointer<ScheduleManager> m_sm;
    bool m_first;
    MainSchedule *m_oldexpected;
    MainSchedule *m_newexpected;
};

class PLANKERNEL_EXPORT BaselineScheduleCmd : public NamedCommand
{
public:
    explicit BaselineScheduleCmd( ScheduleManager &sm, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ScheduleManager &m_sm;
};

class PLANKERNEL_EXPORT ResetBaselineScheduleCmd : public NamedCommand
{
public:
    explicit ResetBaselineScheduleCmd( ScheduleManager &sm, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ScheduleManager &m_sm;
};

class PLANKERNEL_EXPORT ModifyScheduleManagerSchedulingDirectionCmd : public NamedCommand
{
public:
    ModifyScheduleManagerSchedulingDirectionCmd( ScheduleManager &sm, bool value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ScheduleManager &m_sm;
    bool oldvalue, newvalue;
};

class PLANKERNEL_EXPORT ModifyScheduleManagerSchedulerCmd : public NamedCommand
{
public:
    ModifyScheduleManagerSchedulerCmd( ScheduleManager &sm, int value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ScheduleManager &m_sm;
    int oldvalue, newvalue;
};

class PLANKERNEL_EXPORT ModifyScheduleManagerSchedulingGranularityCmd : public NamedCommand
{
public:
    ModifyScheduleManagerSchedulingGranularityCmd( ScheduleManager &sm, int value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    ScheduleManager &m_sm;
    int oldvalue, newvalue;
};

class PLANKERNEL_EXPORT ModifyStandardWorktimeYearCmd : public NamedCommand
{
public:
    ModifyStandardWorktimeYearCmd( StandardWorktime *wt, double oldvalue, double newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();
private:
    StandardWorktime *swt;
    double m_oldvalue;
    double m_newvalue;
};

class PLANKERNEL_EXPORT ModifyStandardWorktimeMonthCmd : public NamedCommand
{
public:
    ModifyStandardWorktimeMonthCmd( StandardWorktime *wt, double oldvalue, double newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();
private:
    StandardWorktime *swt;
    double m_oldvalue;
    double m_newvalue;
};

class PLANKERNEL_EXPORT ModifyStandardWorktimeWeekCmd : public NamedCommand
{
public:
    ModifyStandardWorktimeWeekCmd( StandardWorktime *wt, double oldvalue, double newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();
private:
    StandardWorktime *swt;
    double m_oldvalue;
    double m_newvalue;
};

class PLANKERNEL_EXPORT ModifyStandardWorktimeDayCmd : public NamedCommand
{
public:
    ModifyStandardWorktimeDayCmd( StandardWorktime *wt, double oldvalue, double newvalue, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();
private:
    StandardWorktime *swt;
    double m_oldvalue;
    double m_newvalue;
};

class PLANKERNEL_EXPORT DocumentAddCmd : public NamedCommand
{
public:
    DocumentAddCmd( Documents& docs, Document *value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~DocumentAddCmd();
    void execute();
    void unexecute();
private:
    Documents& m_docs;
    Document *m_value;
    bool m_mine;
};

class PLANKERNEL_EXPORT DocumentRemoveCmd : public NamedCommand
{
public:
    DocumentRemoveCmd( Documents& docs, Document *value, const KUndo2MagicString& name = KUndo2MagicString() );
    ~DocumentRemoveCmd();
    void execute();
    void unexecute();
private:
    Documents& m_docs;
    Document *m_value;
    bool m_mine;
};

class PLANKERNEL_EXPORT DocumentModifyUrlCmd : public NamedCommand
{
public:
    DocumentModifyUrlCmd( Document *doc, const QUrl &url, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();
private:
    Document *m_doc;
    QUrl m_value;
    QUrl m_oldvalue;
};

class PLANKERNEL_EXPORT DocumentModifyNameCmd : public NamedCommand
{
public:
    DocumentModifyNameCmd( Document *doc, const QString &value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();
private:
    Document *m_doc;
    QString m_value;
    QString m_oldvalue;
};

class PLANKERNEL_EXPORT DocumentModifyTypeCmd : public NamedCommand
{
public:
    DocumentModifyTypeCmd( Document *doc, Document::Type value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();
private:
    Document *m_doc;
    Document::Type m_value;
    Document::Type m_oldvalue;
};

class PLANKERNEL_EXPORT DocumentModifyStatusCmd : public NamedCommand
{
public:
    DocumentModifyStatusCmd( Document *doc, const QString &value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();
private:
    Document *m_doc;
    QString  m_value;
    QString  m_oldvalue;
};

class PLANKERNEL_EXPORT DocumentModifySendAsCmd : public NamedCommand
{
    public:
        DocumentModifySendAsCmd( Document *doc, const Document::SendAs value, const KUndo2MagicString& name = KUndo2MagicString() );
        void execute();
        void unexecute();
    private:
        Document *m_doc;
        Document::SendAs m_value;
        Document::SendAs m_oldvalue;
};

class PLANKERNEL_EXPORT WBSDefinitionModifyCmd : public NamedCommand
{
public:
    WBSDefinitionModifyCmd( Project &project, const WBSDefinition value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();
private:
    Project &m_project;
    WBSDefinition m_newvalue, m_oldvalue;
};

class PLANKERNEL_EXPORT InsertProjectCmd : public MacroCommand
{
public:
    InsertProjectCmd( Project &project, Node *parent, Node *after, const KUndo2MagicString& name = KUndo2MagicString() );

    void execute();
    void unexecute();

protected:
    void addAccounts( Account *account, Account *parent, QList<Account*> &unused, QMap<QString, Account*> &all );
    void addCalendars( Calendar *calendar, Calendar *parent, QList<Calendar*> &unused, QMap<QString, Calendar*> &all );
    void addChildNodes( Node *node );

private:
    Project *m_project;
    Node *m_parent;
    Node *m_after;

};

class PLANKERNEL_EXPORT WorkPackageAddCmd : public NamedCommand
{
public:
    WorkPackageAddCmd( Project *project, Node *node, WorkPackage *wp, const KUndo2MagicString& name = KUndo2MagicString() );
    ~WorkPackageAddCmd();
    void execute();
    void unexecute();
private:
    Project *m_project;
    Node *m_node;
    WorkPackage *m_wp;
    bool m_mine;

};

class PLANKERNEL_EXPORT ModifyProjectLocaleCmd : public MacroCommand
{
public:
    ModifyProjectLocaleCmd( Project &project, const KUndo2MagicString &name );
    void execute();
    void unexecute();
private:
    Project &m_project;
};

class PLANKERNEL_EXPORT ModifyCurrencySymolCmd : public NamedCommand
{
public:
    ModifyCurrencySymolCmd( Locale *locale, const QString &value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Locale *m_locale;
    QString m_newvalue;
    QString m_oldvalue;
};

class  PLANKERNEL_EXPORT ModifyCurrencyFractionalDigitsCmd : public NamedCommand
{
public:
    ModifyCurrencyFractionalDigitsCmd( Locale *locale, int value, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Locale *m_locale;
    int m_newvalue;
    int m_oldvalue;
};

class  PLANKERNEL_EXPORT AddExternalAppointmentCmd : public NamedCommand
{
public:
    AddExternalAppointmentCmd( Resource *resource, const QString &pid, const QString &pname, const QDateTime &start, const QDateTime &end, double load, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    QString m_pid;
    QString m_pname;
    QDateTime m_start;
    QDateTime m_end;
    double m_load;
};

class  PLANKERNEL_EXPORT ClearExternalAppointmentCmd : public NamedCommand
{
public:
    ClearExternalAppointmentCmd( Resource *resource, const QString &pid, const KUndo2MagicString& name = KUndo2MagicString() );
    ~ClearExternalAppointmentCmd();
    void execute();
    void unexecute();

private:
    Resource *m_resource;
    QString m_pid;
    Appointment *m_appointments;
};

class  PLANKERNEL_EXPORT ClearAllExternalAppointmentsCmd : public NamedCommand
{
public:
    explicit ClearAllExternalAppointmentsCmd( Project *project, const KUndo2MagicString& name = KUndo2MagicString() );
    void execute();
    void unexecute();

private:
    Project *m_project;
    MacroCommand m_cmd;
};

class  PLANKERNEL_EXPORT SharedResourcesFileCmd : public NamedCommand
{
public:
    explicit SharedResourcesFileCmd(Project *project, const QString &newValue, const KUndo2MagicString& name = KUndo2MagicString());
    void execute();
    void unexecute();

private:
    Project *m_project;
    QString m_oldValue;
    QString m_newValue;
};

class  PLANKERNEL_EXPORT UseSharedResourcesCmd : public NamedCommand
{
public:
    explicit UseSharedResourcesCmd(Project *project, bool newValue, const KUndo2MagicString& name = KUndo2MagicString());
    void execute();
    void unexecute();

private:
    Project *m_project;
    bool m_oldValue;
    bool m_newValue;
};

class  PLANKERNEL_EXPORT SharedProjectsUrlCmd : public NamedCommand
{
public:
    explicit SharedProjectsUrlCmd(Project *project, const QUrl &newValue, const KUndo2MagicString& name = KUndo2MagicString());
    void execute();
    void unexecute();

private:
    Project *m_project;
    QUrl m_oldValue;
    QUrl m_newValue;
};

class  PLANKERNEL_EXPORT LoadProjectsAtStartupCmd : public NamedCommand
{
public:
    explicit LoadProjectsAtStartupCmd(Project *project, bool newValue, const KUndo2MagicString& name = KUndo2MagicString());
    void execute();
    void unexecute();

private:
    Project *m_project;
    bool m_oldValue;
    bool m_newValue;
};

}  //KPlato namespace

#endif //COMMAND_H
