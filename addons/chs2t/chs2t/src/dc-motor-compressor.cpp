#include    "dc-motor-compressor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorCompressor::DCMotorCompressor(QString config_path, QObject *parent) : Device(parent)
//  , p0(1.5)
//  , p0(2.1)
//  , Mmax(455.8)
//  , s_kr(0.154)
//  , Un(380.0)
//  , Vnk(0.05)

  , p(0.0)
  , Q(0.0)
  , U_power(0.0)
  , omega0(157.08)

  , Mxx(50.0)
  , J(0.5)

  , R(56.9)
  , U(0.0)
  , cPhi(14.2)
  , I(0.0)
  , Ma(0.0)

{
    std::fill(K.begin(), K.end(), 0);

    load_config(config_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DCMotorCompressor::~DCMotorCompressor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::setExternalPressure(double press)
{
    p = press;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double DCMotorCompressor::getAirFlow() const
{
    return Q;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::preStep(state_vector_t &Y, double t)
{
    Q_UNUSED(t)

    Q = K[4] * pf(Y[1] - p);

    emit soundSetPitch("Motor_Compressor", static_cast<float>(Y[0] / omega0));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::ode_system(const state_vector_t &Y,
                                 state_vector_t &dYdt,
                                 double t)
{
    Q_UNUSED(t)

    double Mr = Physics::fricForce(Mxx, Y[0]);

    I = (U - cPhi * Y[0]) / R;

    Ma = cPhi * I;

    dYdt[0] = (Ma - Mr) / J;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DCMotorCompressor::load_config(QString cfg_path)
{
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Device";

        int order = 1;
        if (cfg.getInt(secName, "Order", order))
        {
            y.resize(static_cast<size_t>(order));
            std::fill(y.begin(), y.end(), 0);
        }

        for (size_t i = 1; i < K.size(); ++i)
        {
            QString coeff = QString("K%1").arg(i);
            cfg.getDouble(secName, coeff, K[i]);
        }
    }
}