//-----------------------------------------------------------------------------
//
//      Библиотека для работы со звуком
//      (c) РГУПС, ВЖД 24/03/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------


#include "asound.h"
#include <QFile>
#include <QTimer>

// ****************************************************************************
// *                         Класс AListener                                  *
// ****************************************************************************
//-----------------------------------------------------------------------------
// КОНСТРУКТОР
//-----------------------------------------------------------------------------
AListener::AListener()
{
    // Открываем устройство
    device_ = alcOpenDevice(NULL);
    // Создаём контекст
    context_ = alcCreateContext(device_, NULL);
    // Устанавливаем текущий контекст
    alcMakeContextCurrent(context_);

    // Инициализируем положение слушателя
    memcpy(listenerPosition_,    DEF_LSN_POS, 3 * sizeof(float));
    // Инициализируем вектор "скорости передвижения" слушателя
    memcpy(listenerVelocity_,    DEF_LSN_VEL, 3 * sizeof(float));
    // Инициализируем векторы направления слушателя
    memcpy(listenerOrientation_, DEF_LSN_ORI, 6 * sizeof(float));

    // Устанавливаем положение слушателя
    alListenerfv(AL_POSITION,    listenerPosition_);
    // Устанавливаем скорость слушателя
    alListenerfv(AL_VELOCITY,    listenerVelocity_);
    // Устанавливаем направление слушателя
    alListenerfv(AL_ORIENTATION, listenerOrientation_);
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AListener& AListener::getInstance()
{
    // Создаем статичный экземпляр класса
    static AListener instance;
    // Возвращаем его при каждом вызове метода
    return instance;
}



// ****************************************************************************
// *                            Класс ASound                                  *
// ****************************************************************************
//-----------------------------------------------------------------------------
// КОНСТРУКТОР
//-----------------------------------------------------------------------------
ASound::ASound(QString soundname, QObject *parent): QObject(parent),
    soundName_(soundname),      // Сохраняем название звука
    canDo_(false),              // Сбрасываем флаг
    canPlay_(false),            // Сбрасываем флаг
    format_(0),                 // Обнуляем формат
    source_(0),                 // Обнуляем источник
    buffer_(0),                 // Обнуляем буффер
    wavData_(NULL),             // Обнуляем массив дорожки
    sourceVolume_(DEF_SRC_VOLUME),  // Громкость по умолч.
    sourcePitch_(DEF_SRC_PITCH),    // Скорость воспроизведения по умолч.
    sourceLoop_(false)         // Зацикливание по-умолч.
{
    // Инициализируем позицию источника
    memcpy(sourcePosition_, DEF_SRC_POS, 3 * sizeof(float));
    // Инициализируем вектор "скорости передвижения" источника
    memcpy(sourceVelocity_, DEF_SRC_VEL, 3 * sizeof(float));

    // Создаём контейнер аудиофайла
    file_ = new QFile();
    // Загружаем звук
    loadSound_(soundname);
}



//-----------------------------------------------------------------------------
// ДЕСТРУКТОР
//-----------------------------------------------------------------------------
ASound::~ASound()
{
    if (wavData_)
    {
        // Контейнер секции data файла wav
        delete[] wavData_;
    }
    // Удаляем источник
    alDeleteSources(1, &source_);
    // Удаляем буфер
    alDeleteBuffers(1, &buffer_);
}



//-----------------------------------------------------------------------------
// Полная подготовка файла
//-----------------------------------------------------------------------------
void ASound::loadSound_(QString soundname)
{
    // Сбрасываем флаги
    canDo_ = false;
    canPlay_ = false;

    // Сохраняем название звука
    soundName_ = soundname;

    // Загружаем файл
    loadFile_(soundname);

    // Читаем информационный раздел 44байта
    readWaveInfo_();

    // Определяем формат аудио (mono8/16 - stereo8/16) OpenAL
    defineFormat_();

    // Генерируем буфер и источник
    generateStuff_();

    // Настраиваем источник
    configureSource_();

    // Можно играть звук
    if (canDo_)
    {
        canPlay_ = true;
    }
}



//-----------------------------------------------------------------------------
// Загрузка файла (в т.ч. из ресурсов)
//-----------------------------------------------------------------------------
void ASound::loadFile_(QString soundname)
{
    // Загружаем файл в контейнер
    file_->setFileName(soundname);

    // Проверяем, существует ли файл
    if (!file_->exists())
    {
        lastError_ = "NO_SUCH_FILE: ";
        lastError_.append(soundname);
        canDo_ = false;
        return;
    }

    // Пытаемся открыть файл
    if (file_->open(QIODevice::ReadOnly))
    {
        canDo_ = true;
    }
    else
    {
        canDo_ = false;
        lastError_ = "CANT_OPEN_FILE_FOR_READING: ";
        lastError_.append(soundname);
        return;
    }
}



//-----------------------------------------------------------------------------
// Чтение информации о файле .wav
//-----------------------------------------------------------------------------
void ASound::readWaveInfo_()
{
    if (canDo_)
    {
        // Если ранее был загружен другой файл
        if (wavData_)
        {
            delete[] wavData_;
        }

        // Читаем все 44 байта информации в массив
        QByteArray arr = file_->read(sizeof(wave_info_t));

        // Переносим все значения из массива в струтуру
        memcpy((unsigned char*) &wave_info_,
               (unsigned char*) arr.data(),
               sizeof(wave_info_t));

        // ////////////////////////////////////////////// //
        //      Далее идет проверка данных. Проверка      //
        //  представляет из себя поиск позиции подстроки  //
        //      в исходной строке (если она там есть).    //
        // ////////////////////////////////////////////// //
        checkValue(wave_info_.chunkId, "RIFF", "NOT_RIFF_FILE");
        checkValue(wave_info_.format, "WAVE", "NOT_WAVE_FILE");
        checkValue(wave_info_.subchunk1Id, "fmt", "NO_fmt");
        checkValue(wave_info_.subchunk2Id, "data", "NO_data");

        if (canDo_)
        {
            // Читаем из файла сами медиа данные зная их размер
            arr = file_->read(wave_info_.subchunk2Size);
            // Создаем массив для данных
            wavData_ = new unsigned char[wave_info_.subchunk2Size];
            // Переносим данные в массив
            memcpy((unsigned char*) wavData_,
                   (unsigned char*) arr.data(),
                   wave_info_.subchunk2Size);
        }

        // Закрываем файл
        file_->close();
    }
}



//-----------------------------------------------------------------------------
// Определение формата аудио (mono8/16 - stereo8/16)
//-----------------------------------------------------------------------------
void ASound::defineFormat_()
{
    if (canDo_)
    {
        if (wave_info_.bitsPerSample == 8)      // Если бит в сэмпле 8
        {
            if (wave_info_.numChannels == 1)    // Если 1 канал
            {
                format_ = AL_FORMAT_MONO8;
            }
            else                                // Если 2 канала
            {
                format_ = AL_FORMAT_STEREO8;
            }
        }
        else if (wave_info_.bitsPerSample == 16)// Если бит в сэмпле 16
        {
            if (wave_info_.numChannels == 1)    // Если 1 канал
            {
                format_ = AL_FORMAT_MONO16;
            }
            else                                // Если 2 канала
            {
                format_ = AL_FORMAT_STEREO16;
            }
        }
        else                                    // Если все плохо
        {
            lastError_ = "UNKNOWN_AUDIO_FORMAT";
            canDo_ = false;
        }
    }
}



//-----------------------------------------------------------------------------
// Генерация буфера и источника
//-----------------------------------------------------------------------------
void ASound::generateStuff_()
{
    if (canDo_)
    {
        // Генерируем буфер
        alGenBuffers(1, &buffer_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_GENERATE_BUFFER";
            return;
        }

        // Генерируем источник
        alGenSources(1, &source_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_GENERATE_SOURCE";
            return;
        }

        // Настраиваем буфер
        alBufferData(buffer_,
                     format_,
                     wavData_,
                     wave_info_.subchunk2Size,
                     wave_info_.sampleRate);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_MAKE_BUFFER_DATA";
            return;
        }
    }
}



//-----------------------------------------------------------------------------
// Настройка источника
//-----------------------------------------------------------------------------
void ASound::configureSource_()
{
    if (canDo_)
    {
        // Передаём источнику буфер
        alSourcei(source_, AL_BUFFER, buffer_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_ADD_BUFFER_TO_SOURCE";
            return;
        }

        // Устанавливаем громкость
        alSourcef(source_, AL_GAIN, 0.01f * sourceVolume_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_APPLY_VOLUME";
            return;
        }

        // Устанавливаем скорость воспроизведения
        alSourcef(source_, AL_PITCH, sourcePitch_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_APPLY_PITCH";
            return;
        }

        // Устанавливаем зацикливание
        alSourcei(source_, AL_LOOPING, (char)sourceLoop_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_APPLY_LOOPING";
            return;
        }

        // Устанавливаем положение
        alSourcefv(source_, AL_POSITION, sourcePosition_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_APPLY_POSITION";
            return;
        }

        // Устанавливаем вектор "скорости передвижения"
        alSourcefv(source_, AL_VELOCITY, sourceVelocity_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_APPLY_VELOCITY";
            return;
        }
    }
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int ASound::getDuration()
{
    if (canDo_)
    {
        double subchunk2Size = wave_info_.subchunk2Size;
        double byteRate = wave_info_.byteRate;
        int duration = subchunk2Size/byteRate*100.0;
        return 10*duration;
    }
    return 0;
}



//-----------------------------------------------------------------------------
// (слот) Установить громкость
//-----------------------------------------------------------------------------
void ASound::setVolume(int volume)
{
    if (canPlay_)
    {
        sourceVolume_ = volume;

        if (sourceVolume_ > MAX_SRC_VOLUME)
            sourceVolume_ = MAX_SRC_VOLUME;

        if (sourceVolume_ < MIN_SRC_VOLUME)
            sourceVolume_ = MIN_SRC_VOLUME;

        alSourcef(source_, AL_GAIN, 0.01f * sourceVolume_);
    }
}



//-----------------------------------------------------------------------------
// Вернуть громкость
//-----------------------------------------------------------------------------
int ASound::getVolume()
{
    return sourceVolume_;
}



//-----------------------------------------------------------------------------
// (слот) Установить скорость воспроизведения
//-----------------------------------------------------------------------------
void ASound::setPitch(float pitch)
{
    if (canPlay_)
    {
        sourcePitch_ = pitch;
        alSourcef(source_, AL_PITCH, sourcePitch_);
    }
}



//-----------------------------------------------------------------------------
// Вернуть скорость воспроизведения
//-----------------------------------------------------------------------------
float ASound::getPitch()
{
    return sourcePitch_;
}



//-----------------------------------------------------------------------------
// (слот) Установить зацикливание
//-----------------------------------------------------------------------------
void ASound::setLoop(bool loop)
{
    if (canPlay_)
    {
        sourceLoop_ = loop;
        alSourcei(source_, AL_LOOPING, (char)sourceLoop_);
    }
}



//-----------------------------------------------------------------------------
// Вернуть зацикливание
//-----------------------------------------------------------------------------
bool ASound::getLoop()
{
    return sourceLoop_;
}



//-----------------------------------------------------------------------------
// (слот) Установить положение
//-----------------------------------------------------------------------------
void ASound::setPosition(float x, float y, float z)
{
    if (canPlay_)
    {
        sourcePosition_[0] = x;
        sourcePosition_[1] = y;
        sourcePosition_[2] = z;
        alSourcefv(source_, AL_POSITION, sourcePosition_);
    }
}



//-----------------------------------------------------------------------------
// Вернуть положение
//-----------------------------------------------------------------------------
void ASound::getPosition(float &x, float &y, float &z)
{
    x = sourcePosition_[0];
    y = sourcePosition_[1];
    z = sourcePosition_[2];
}



//-----------------------------------------------------------------------------
// (слот) Установить вектор "скорости передвижения"
//-----------------------------------------------------------------------------
void ASound::setVelocity(float x, float y, float z)
{
    if (canPlay_)
    {
        sourceVelocity_[0] = x;
        sourceVelocity_[1] = y;
        sourceVelocity_[2] = z;
        alSourcefv(source_, AL_VELOCITY, sourceVelocity_);
    }
}



//-----------------------------------------------------------------------------
// Вернуть вектор "скорости перемещения"
//-----------------------------------------------------------------------------
void ASound::getVelocity(float &x, float &y, float &z)
{
    x = sourceVelocity_[0];
    y = sourceVelocity_[1];
    z = sourceVelocity_[2];
}



//-----------------------------------------------------------------------------
// (слот) Играть звук
//-----------------------------------------------------------------------------
void ASound::play()
{
    if (canPlay_)
    {
        alSourcePlay(source_);
    }
}



//-----------------------------------------------------------------------------
// (слот) Приостановить звук
//-----------------------------------------------------------------------------
void ASound::pause()
{
    if (canPlay_)
    {
        alSourcePause(source_);
    }
}



//-----------------------------------------------------------------------------
// (слот) Остановить звук
//-----------------------------------------------------------------------------
void ASound::stop()
{
    if (canPlay_)
    {
        alSourceStop(source_);
    }
}



//-----------------------------------------------------------------------------
// Вернуть последюю ошибку
//-----------------------------------------------------------------------------
QString ASound::getLastError()
{
    QString foo = lastError_;
    lastError_.clear();
    return foo;
}



//-----------------------------------------------------------------------------
// Играет ли звук
//-----------------------------------------------------------------------------
bool ASound::isPlaying()
{
    ALint state;
    alGetSourcei(source_, AL_SOURCE_STATE, &state);
    return(state == AL_PLAYING);
}



//-----------------------------------------------------------------------------
// Приостановлен ли звук
//-----------------------------------------------------------------------------
bool ASound::isPaused()
{
    ALint state;
    alGetSourcei(source_, AL_SOURCE_STATE, &state);
    return(state == AL_PAUSED);
}



//-----------------------------------------------------------------------------
// Остановлен ли звук
//-----------------------------------------------------------------------------
bool ASound::isStopped()
{
    ALint state;
    alGetSourcei(source_, AL_SOURCE_STATE, &state);
    return(state == AL_STOPPED);
}



//-----------------------------------------------------------------------------
// Метод проверки необходимых параметров
//-----------------------------------------------------------------------------
void ASound::checkValue(std::string baseStr, const char targStr[], QString err)
{
    if (canDo_)
    {
        // // /////////////////////////////////////////////// //
        // // Важно, чтобы подстрока начиналась с 0 элемента! //
        // //    иначе проверку нельзя считать достоверной    //
        // // /////////////////////////////////////////////// //
        if (baseStr.find(targStr) != 0)
        {
            lastError_ = err;
            canDo_ = false;
        }
    }
}




//-----------------------------------------------------------------------------
//
//      Класс управления очередью запуска звуков
//      (c) РГУПС, ВЖД 17/08/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Класс управления очередью запуска звуков
 *  \copyright РУГПС, ВЖД
 *  \author Ковшиков С. В.
 *  \date 17/08/2017
 */


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ASoundController::ASoundController(QObject *parent)
    : QObject(parent)
    , prepared_(false)
    , currentSound_(-1)
    , timerMain_(Q_NULLPTR)
{
    timerMain_ = new QTimer(this);
    timerMain_->setSingleShot(true);
    connect(timerMain_, SIGNAL(timeout()),
            this, SLOT(onTimerMain()));
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ASoundController::~ASoundController()
{

}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ASound* ASoundController::appendSound(ASound *soundPtr)
{
    if (soundPtr)
    {
        listSounds_.append(soundPtr);
    }

    return soundPtr;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
ASound* ASoundController::appendSound(QString soundPath)
{
    listSounds_.append(new ASound(soundPath, this));
    QString zu = listSounds_.last()->getLastError();
    return listSounds_.last();
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ASoundController::prepare()
{
    if (listSounds_.empty())
        return;

    currentSound_ = 0;
    if (!listSounds_.first()->getLoop())
    {
        timerMain_->setInterval(listSounds_.first()->getDuration());
        int zu = listSounds_.first()->getDuration();
        zu = zu;
    }
    prepared_ = true;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ASoundController::begin()
{
    if (prepared_)
    {
        if (!listSounds_[currentSound_]->isPlaying())
        {
            timerMain_->start();
            listSounds_.first()->play();
        }
    }
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ASoundController::end()
{
    if (prepared_ && listSounds_[currentSound_]->isPlaying())
    {
        ++currentSound_;
        listSounds_[currentSound_]->play();
        listSounds_[currentSound_ - 1]->stop();
        currentSound_ = 0;
    }
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ASoundController::stop()
{
    timerMain_->stop();
    foreach (ASound* sound , listSounds_)
    {
        sound->stop();
    }
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ASoundController::onTimerMain()
{
    ++currentSound_;
    listSounds_[currentSound_]->play();

    if (currentSound_ > 0)
        listSounds_[currentSound_ - 1]->stop();
}
