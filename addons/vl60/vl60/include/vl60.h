//------------------------------------------------------------------------------
//
//      Магистральный электровоз переменного тока ВЛ60.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Роман Бирюков (РомычРЖДУЗ)
//
//      Дата: 28/03/2019
//
//------------------------------------------------------------------------------
#ifndef     VL60_H
#define     VL60_H

#include    "vehicle-api.h"

#include    "vl60-signals.h"
#include    "pantograph.h"
#include    "main-switch.h"
#include    "oscillator.h"
#include    "trac-transformer.h"
#include    "phase-splitter.h"
#include    "motor-fan.h"
#include    "reservoir.h"
#include    "motor-compressor.h"
#include    "pressure-regulator.h"
#include    "ubt367m.h"

/*!
 * \class
 * \brief Основной класс, описывающий весь электровоз
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VL60 : public Vehicle
{
public:

    /// Конструктор
    VL60();

    /// Деструктор
    ~VL60();

    /// Инициализация тормозных приборов
    void initBrakeDevices(double p0, double pTM, double pFL);

private:

    enum
    {
        NUM_PANTOGRAPHS = 2,
        WIRE_VOLTAGE = 25000
    };

    enum
    {
        NUM_MOTOR_FANS = 6,
        MV1 = 0,
        MV2 = 1,
        MV3 = 2,
        MV4 = 3,
        MV5 = 4,
        MV6 = 5
    };

    enum
    {
        MAIN_RESERVOIR_VOLUME = 1000
    };

    float   Uks;
    float   pant1_pos;
    float   pant2_pos;
    float   gv_pos;
    bool    gv_return;

    float   test_lamp;
    int     sig;

    /// Зарядное давление
    double  charge_press;

    /// Тригер тумблера "Токоприемники"
    Trigger pants_tumbler;
    /// Триггер тумблера "Токоприемник передний"
    Trigger pant1_tumbler;
    /// Триггер тумблера "Токоприемник задний"
    Trigger pant2_tumbler;

    /// Триггер тумблера "ГВ вкл/выкл"
    Trigger gv_tumbler;

    /// Тригер тумблена "ГВ вкл. Возврат защиты"
    Trigger gv_return_tumbler;

    /// Тригер тумблера "Фазорасщепитель"
    Trigger fr_tumbler;

    /// Триггеры тумблеров управления мотор-вентиляторами
    std::array<Trigger, NUM_MOTOR_FANS> mv_tumblers;

    /// Тригер тумблера управления мотор-компрессором
    Trigger mk_tumbler;

    /// Токоприемники
    std::array<Pantograph *, NUM_PANTOGRAPHS>   pantographs;

    /// Главный выключатель (ГВ)
    MainSwitch      *main_switch;

    /// Механизм киловольтметра КС
    Oscillator      *gauge_KV_ks;

    /// Тяговый трансформатор
    TracTransformer *trac_trans;

    /// Асинхронный расщепитель фаз
    PhaseSplitter   *phase_spliter;

    /// Мотор-вентиляторы
    std::array<MotorFan *, NUM_MOTOR_FANS> motor_fans;

    /// Главный резервуар
    Reservoir   *main_reservoir;

    /// Мотор-компрессор
    MotorCompressor *motor_compressor;

    /// Регулятор давления в ГР
    PressureRegulator *press_reg;

    /// Устройство блогировки тормозов усл. №367М
    BrakeLock   *ubt;

    /// Поездной кран машиниста (КрМ)
    BrakeCrane  *brake_crane;

    /// Кран впомогательного тормоза (КВТ)
    LocoCrane   *loco_crane;

    void initialization();

    void step(double t, double dt);

    void stepPantographsControl(double t, double dt);

    void stepMainSwitchControl(double t, double dt);

    void stepTracTransformer(double t, double dt);

    void stepPhaseSplitter(double t, double dt);

    void stepMotorFans(double t, double dt);

    void stepMotorCompressor(double t, double dt);

    void stepBrakeControl(double t, double dt);

    void stepSignalsOutput();

    bool getHoldingCoilState() const;

    void keyProcess();
};

#endif // VL60_H

