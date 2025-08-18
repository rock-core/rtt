/***************************************************************************
  tag: FMTC  Tue Mar 11 21:49:25 CET 2008  TaskCore.cpp

                        TaskCore.cpp -  description
                           -------------------
    begin                : Tue March 11 2008
    copyright            : (C) 2008 FMTC
    email                : peter.soetens@fmtc.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/



#include "TaskCore.hpp"
#include "../ExecutionEngine.hpp"
#include "ActivityInterface.hpp"
#include "Logger.hpp"
#include "internal/CatchConfig.hpp"

namespace RTT {
    using namespace detail;

    using namespace std;

    TaskCore::TaskCore(TaskState initial_state /*= Stopped*/ )
        :  ee( new ExecutionEngine(this) )
           ,mTaskState(initial_state)
           ,mInitialState(initial_state)
           ,mTargetState(initial_state)
           ,mTriggerOnStart(true)
           ,mCycleCounter(0)
           ,mIOCounter(0)
           ,mTimeOutCounter(0)
           ,mTriggerCounter(0)
    {
    }

    TaskCore::~TaskCore()
    {
        if ( ee->getParent() == this ) {
            delete ee;
        }
        // Note: calling cleanup() here has no use or even dangerous, as
        // cleanupHook() is a virtual function and the user code is already
        // destroyed. The user's subclass is responsible to make this state
        // transition in its destructor if required.
    }

    TaskCore::TaskState TaskCore::getTaskState() const {
        return mTaskState;
    }

    TaskCore::TaskState TaskCore::getTargetState() const {
        return mTargetState;
    }

    bool TaskCore::update()
    {
        return this->engine()->getActivity() && this->engine()->getActivity()->execute();
    }

    bool TaskCore::trigger()
    {
        return this->engine()->getActivity() && this->engine()->getActivity()->timeout();
    }

    void TaskCore::setTaskState(TaskState state) {
        mTaskState = state;
    }

    void TaskCore::setTargetState(TaskState state) {
        mTargetState = state;
    }

    bool TaskCore::configure() {
        if ( mTaskState != Stopped && mTaskState != PreOperational) {
            return false;
        }

        TRY(
            return performConfigureTransition();
        ) CATCH(std::exception const& e,
            log(Error) << "in configure(): switching to exception state because of unhandled exception" << endlog();
            log(Error) << "  " << e.what() << endlog();
            exception();
        ) CATCH_ALL(
            log(Error) << "in configure(): switching to exception state because of unhandled exception" << endlog();
            exception();
        )
        return false;
    }

    bool TaskCore::performConfigureTransition() {
        setTargetState(Stopped);
        if (!configureHook()) {
            if (mTargetState == Stopped) {
                setTargetState(PreOperational);
                setTaskState(PreOperational);
            }
            return false;
        }

        if (mTargetState == Stopped) {
            setTaskState(Stopped);
            return true;
        }

        log(Error) << "in configure(): state has been changed inside the configureHook" << endlog();
        log(Error) << "  but configureHook returned true. Bailing out." << endlog();
        exception();
        return false;
    }

    bool TaskCore::cleanup() {
        if ( mTaskState != Stopped ) {
            return false;
        }

        TRY(
            return performCleanupTransition();
        ) CATCH(std::exception const& e,
           log(Error) << "in cleanup(): switching to fatal state because of unhandled exception" << endlog();
           log(Error) << "  " << e.what() << endlog();
           fatal();
        ) CATCH_ALL (
           log(Error) << "in cleanup(): switching to fatal state because of unhandled exception" << endlog();
           fatal();
        )
        return false; // no cleanup when running or not configured.
    }

    bool TaskCore::performCleanupTransition() {
        setTargetState(PreOperational);
        cleanupHook();
        if (mTargetState == PreOperational) {
            setTaskState(PreOperational);
            return true;
        }

        if (mTargetState == Exception) {
            performExceptionTransition(PreOperational);
        }
        return false;
    }

    void TaskCore::fatal() {
        setTaskState(FatalError);
        setTargetState(FatalError);
        this->engine()->getActivity() && engine()->getActivity()->stop();
    }

    void TaskCore::error() {
        // detects error() from within start():
        if (mTargetState < Running)
            return;
        setTaskState(RunTimeError);
        setTargetState(RunTimeError);
    }

    void TaskCore::exception() {
        if (mTargetState == TaskCore::Exception) {
            // Already transitioned or transitioning to exception, do nothing
            return;
        }

        if (mTargetState != mTaskState) {
            // Already in transition. Set the target state but we assume the transition
            // method will actually call the hooks
            if (inStopTransition() || inCleanupTransition()) {
                // stop() and cleanup() will take care of doing the exception transition
                // after the stopHook call
                setTargetState(Exception);
                return;
            }
        }

        TaskState state = mTaskState;
        setTargetState(Exception);
        TRY (
            performExceptionTransition(state);
        ) CATCH(std::exception const& e,
            log(RTT::Error)
                << "stopHook(), cleanupHook() and/or exceptionHook() raised "
                << e.what() << ", going into Fatal" << endlog();
            fatal();
        ) CATCH_ALL (
            log(Error)
                << "stopHook(), cleanupHook() and/or exceptionHook() raised "
                << "an exception, going into Fatal" << endlog();
            fatal();
        )
    }

    void TaskCore::performExceptionTransition(TaskState state) {
        if (state >= Running) {
            stopHook();
        }
        if (state >= Stopped && mInitialState == PreOperational) {
            cleanupHook();
        }
        exceptionHook();
        setTaskState(Exception);
    }

    bool TaskCore::recover() {
        if ( mTaskState == Exception ) {
            setTaskState(mInitialState);
            setTargetState(mInitialState);
            return true;
        }
        if (mTaskState == RunTimeError && mTargetState >= Running) {
            setTaskState(Running);
            setTargetState(Running);
            return true;
        }
        return false;
    }

    bool TaskCore::start() {
        if ( mTaskState == Stopped ) {
            TRY (
                setTargetState(Running);
                bool successful = startHook();
                if (successful) {
                    if (mTaskState != Running && (mTargetState == mTaskState)) {
                        log(Error) << "in start(): state has been changed inside the startHook" << endlog();
                        log(Error) << "  but startHook returned true. Bailing out." << endlog();
                        exception();
                        return false;
                    }
                    else {
                        setTaskState(Running);
                        if ( mTriggerOnStart )
                            trigger(); // triggers updateHook() in case of non periodic!
                        return true;
                    }
                }
                setTargetState(Stopped);
            ) CATCH(std::exception const& e,
                log(Error) << "in start(): switching to exception state because of unhandled exception" << endlog();
                log(Error) << "  " << e.what() << endlog();
                exception();
            ) CATCH_ALL (
                log(Error) << "in start(): switching to exception state because of unhandled exception" << endlog();
                exception();
            )
        }
        return false;
    }

    bool TaskCore::stop() {
        if ( mTaskState < Running ) {
            // Not running, invalid transition
            return false;
        }

        TRY(
            if (performStopTransition()) {
                return true;
            }
        ) CATCH(std::exception const& e,
            log(Error) << "in stop(): switching to fatal state because of unhandled exception" << endlog();
            log(Error) << "  " << e.what() << endlog();
            fatal();
        ) CATCH_ALL (
            log(Error) << "in stop(): switching to fatal state because of unhandled exception" << endlog();
            fatal();
        )

        if (mTargetState != Exception) {
            return false;
        }

        TRY(
            performExceptionTransition(Stopped);
            return false;
        ) CATCH(std::exception const& e,
            log(Error) << "in stop(): switching to fatal state because of unhandled exception during exception transition" << endlog();
            log(Error) << "  " << e.what() << endlog();
            fatal();
        ) CATCH_ALL (
            log(Error) << "in stop(): switching to fatal state because of unhandled exception during exception transition" << endlog();
            fatal();
        )

        return false;
    }

    bool TaskCore::performStopTransition() {
        TaskState origState = mTaskState;

        setTargetState(Stopped);
        if ( !engine()->stopTask(this) ) {
            setTargetState(origState);
            return false;
        }

        // If updateHook was running, it might have changed the
        // state itself (e.g. stopped or exception). Make sure that
        // stopping is still relevant
        if ( mTaskState < Running ) {
            return true;
        }

        stopHook();
        if (mTargetState == Stopped) {
            setTaskState(Stopped);
            return true;
        }
        return false;
    }

    bool TaskCore::activate() {
        this->engine()->getActivity() && this->engine()->getActivity()->start();
        return isActive();
    }

    void TaskCore::cleanupHook() {
    }

    bool TaskCore::isRunning() const {
        return mTaskState >= Running;
    }

    bool TaskCore::isConfigured() const {
        return mTaskState >= Stopped;
    }

    bool TaskCore::inFatalError() const {
        return mTaskState == FatalError;
    }

    bool TaskCore::inConfigureTransition() const {
        return mTaskState == PreOperational && mTargetState == Stopped;
    }

    bool TaskCore::inStartTransition() const {
        return mTaskState == Stopped && mTargetState == Running;
    }

    bool TaskCore::inStopTransition() const {
        return (mTaskState == Running || mTaskState == RunTimeError) &&
               mTargetState == Stopped;
    }

    bool TaskCore::inCleanupTransition() const {
        return mTaskState == Stopped && mTargetState == PreOperational;
    }

    bool TaskCore::inException() const {
        return mTaskState == Exception;
    }

    bool TaskCore::inRunTimeError() const {
        return mTaskState == RunTimeError;
    }

    bool TaskCore::isActive() const
    {
        return this->engine()->getActivity() && this->engine()->getActivity()->isActive();
    }

    Seconds TaskCore::getPeriod() const
    {
        return this->engine()->getActivity() ? this->engine()->getActivity()->getPeriod() : -1.0;
    }

    bool TaskCore::setPeriod(Seconds s)
    {
        return this->engine()->getActivity() && this->engine()->getActivity()->setPeriod(s);
    }

    unsigned TaskCore::getCpuAffinity() const
    {
        return this->engine()->getActivity() ? this->engine()->getActivity()->getCpuAffinity() : ~0;
    }

    bool TaskCore::setCpuAffinity(unsigned cpu)
    {
        return this->engine()->getActivity() && this->engine()->getActivity()->setCpuAffinity(cpu);
    }

    bool TaskCore::configureHook() {
        return true;
    }

    bool TaskCore::startHook()
    {
        return true;
    }

    void TaskCore::errorHook() {
    }

    void TaskCore::updateHook()
    {
    }

    bool TaskCore::breakUpdateHook()
    {
        return true;
    }

    void TaskCore::exceptionHook() {
    }

    void TaskCore::stopHook()
    {
    }
}

