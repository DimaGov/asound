#include    "brake-crane.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeCrane::BrakeCrane(QObject *parent) : BrakeDevice (parent)
  , Ver(1.0)  
  , Vbp(1.0)
  , Qer(0.0)
  , Qbp(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeCrane::~BrakeCrane()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeCrane::getBrakePipeInitPressure() const
{
    return getY(BP_PRESSURE);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double BrakeCrane::getEqReservoirPressure() const
{
    return getY(ER_PRESSURE);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane::setBrakePipeFlow(double Qbp)
{
    this->Qbp = Qbp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane::setEqResrvoirFlow(double Qer)
{
    this->Qer = Qer;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane::setChargePressure(double p0)
{
    this->p0 = p0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane::setFeedLinePressure(double pFL)
{
    this->pFL = pFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane::setBrakePipePressure(double pTM1)
{
    this->pTM1 = pTM1;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void BrakeCrane::ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(t)

    dYdt[BP_PRESSURE] = Qbp / Vbp;
    dYdt[ER_PRESSURE] = Qer / Ver;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
BrakeCrane *loadBrakeCrane(QString lib_path)
{
    BrakeCrane *crane = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetBrakeCrane getBrakeCrane = reinterpret_cast<GetBrakeCrane>(lib.resolve("getBrakeCrane"));

        if (getBrakeCrane)
        {
            crane = getBrakeCrane();
        }
    }

    return crane;
}
