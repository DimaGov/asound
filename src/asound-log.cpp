//------------------------------------------------------------------------------
//
//      ASound log messages handler
//      (c) DimaGVRH, 09/01/2020
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief ASound log messages handler
 * \copyright DimaGVRH
 * \author DimaGVRH
 * \date 09/01/2020
 */

#include    "asound-log.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LogFileHandler::LogFileHandler(const std::string &file)
{
    QString fname = "..\\logs\\" + QString::fromStdString(file);

    file_ = new QFile();

    file_->setFileName(fname);

    canDo_ = file_->exists();

    log_.open(fname.toStdString());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LogFileHandler::~LogFileHandler()
{
    log_.close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LogFileHandler::notify(const std::string msg)
{
    if (canDo_)
        log_ << msg << std::endl;
}
