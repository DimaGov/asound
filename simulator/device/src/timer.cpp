#include    "timer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Timer::Timer(double timeout, bool first_process, QObject *parent)
    : QObject(parent)
    , tau(0.0)
    , timeout(timeout)
    , first_process(first_process)
    , fprocess_prev(first_process)
    , is_started(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Timer::~Timer()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Timer::step(double t, double dt)
{
    Q_UNUSED(t)

    if (is_started)
    {
        if ( first_process || (tau >= timeout) )
        {
            emit process();
            tau = 0;            
            first_process = false;
        }

        tau += dt;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Timer::start()
{
    is_started = true;
    reset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Timer::stop()
{
    is_started = false;
    reset();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Timer::reset()
{
    first_process = fprocess_prev;
    tau = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Timer::isStarted() const
{
    return is_started;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Timer::setTimeout(double timeout)
{
    this->timeout = timeout;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Timer::firstProcess(bool first_process)
{
    this->first_process = this->fprocess_prev = first_process;
}
