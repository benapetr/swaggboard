//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <QDialog>
#include <QList>
#include <QCheckBox>
#include "definitions.hpp"

#ifdef WIN
//#include <mmdeviceapi.h>
#include <windows.h>
//#include <audioclient.h>
#endif

namespace Ui
{
    class Options;
}

struct IMMDevice;

class OutputDevice
{
    public:
        OutputDevice();
        ~OutputDevice();
        QString Name;
};

class Options : public QDialog
{
        Q_OBJECT
    public:
        static void Initialize();
        static QList<OutputDevice> devices;
        static bool nodevices;

        explicit Options(QWidget *parent = 0);
        ~Options();
        QList<QCheckBox*> bl;

    private slots:
        void on_buttonBox_rejected();
        void on_buttonBox_accepted();

    private:
        Ui::Options *ui;
};

#endif // OPTIONS_HPP
