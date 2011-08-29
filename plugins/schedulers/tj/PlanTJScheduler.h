/* This file is part of the KDE project
 * Copyright (C) 2009 Dag Andersen <danders@get2net.dk>
 * Copyright (C) 2011 Dag Andersen <danders@get2net.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PLANTJPSCHEDULER_H
#define PLANTJPSCHEDULER_H

#include "kplatotj_export.h"

#include "kptschedulerplugin.h"

#include "kptdatetime.h"

#include <QThread>
#include <QObject>
#include <QMap>
#include <QList>

class KLocale;
class QDateTime;

namespace TJ
{
    class Project;
    class Task;
    class Resource;
    class Interval;
}

namespace KPlato
{
    class Project;
    class ScheduleManager;
    class Schedule;
    class MainSchedule;
    class Resource;
    class ResourceRequest;
    class Task;
    class Node;
}
using namespace KPlato;

class PlanTJScheduler : public KPlato::SchedulerThread
{
    Q_OBJECT

private:

public:
    PlanTJScheduler( Project *project, ScheduleManager *sm, QObject *parent = 0 );
    ~PlanTJScheduler();

    bool check();
    bool solve();
    int result;

    /// Fill project data into TJ structure
    bool kplatoToTJ();
    /// Fetch project data from TJ structure
    void kplatoFromTJ();


signals:
    void sigCalculationStarted( Project*, ScheduleManager* );
    void sigCalculationFinished( Project*, ScheduleManager* );
    const char* taskname();

public slots:

protected:
    void run();

    void adjustSummaryTasks( const QList<Node*> &nodes );

    void addResources();
    TJ::Resource *addResource( KPlato::Resource *resource );
    void addTasks();
    TJ::Task *addTask( KPlato::Task *task );
    void addDependencies();
    void addDependencies( TJ::Task *job, Task *task );
    void addRequests();
    void addRequest( TJ::Task *job, Task *task );

    void taskFromTJ( TJ::Task *job, Task *task );

    static DateTime fromTime_t( time_t );
    AppointmentInterval fromTJInterval( const TJ::Interval &tji );

private:
    KLocale *locale() const;

private:
    MainSchedule *m_schedule;
    bool m_recalculate;
    bool m_usePert;
    bool m_backward;
    TJ::Project *m_tjProject;
    DateTime m_starttime;
    DateTime m_targettime;
    qint64 m_timeunit;
    
    QMap<TJ::Task*, Task*> m_taskmap;
    QMap<TJ::Resource*, Resource*> m_resourcemap;
    
};

#endif // PLANTJPSCHEDULER_H