//------------------------------------------------------------------------------
//
//      Abstract coupling model
//      (c) maisvendoo 04/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Abstract coupling model
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 04/09/2018
 */

#include    "coupling.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Coupling::Coupling()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Coupling::~Coupling()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Coupling::reset()
{
    calls_count = 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Coupling::loadConfiguration(QString cfg_path)
{
    loadConfig(cfg_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Coupling::loadConfig(QString cfg_path)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Coupling *loadCoupling(QString lib_path)
{
    Coupling *coupling = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetCoupling getCoupling = (GetCoupling) lib.resolve("getVehicle");

        if (getCoupling)
        {
            coupling = getCoupling();
        }
    }

    return coupling;
}
