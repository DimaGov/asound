#include    "es1-motor.h"
#include    "physics.h"

ES1Motor::ES1Motor() : Vehicle ()
  , traction_level(0.0)
  , inc_loc(false)
  , dec_loc(false)
{

}

ES1Motor::~ES1Motor()
{

}

void ES1Motor::keyProcess()
{
    double traction_step = 0.1;

    if (keys[97] && !inc_loc)
    {
        traction_level +=  traction_step;
        inc_loc = true;
    }
    else
    {
        inc_loc = false;
    }

    if (keys[100] && !dec_loc)
    {
        traction_level -=  traction_step;
        dec_loc = true;
    }
    else
    {
        dec_loc = false;
    }
}

void ES1Motor::step(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    traction_level = Physics::cut(traction_level, -1.0, 1.0);

    for (size_t i = 0; i < Q_a.size(); i++)
    {
        double torque = traction_level * traction_char(velocity) * wheel_diameter / num_axis / 2.0;
        Q_a[i] = torque;
    }

    DebugMsg = QString("Время: %1 Скор.: %2 Тяга: %3")
            .arg(t, 10, 'f', 1)
            .arg(velocity, 6, 'f', 1)
            .arg(traction_level, 4, 'f', 1);
}

double ES1Motor::traction_char(double v)
{
    double max_traction = 127.5e3;

    double vn = 81.0 / Physics::kmh;

    if (abs(v) <= vn)
        return max_traction;
    else
        return max_traction * vn / v;
}

GET_VEHICLE(ES1Motor)
