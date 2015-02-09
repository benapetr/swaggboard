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
#include "definitions.hpp"
#include <QAudioDeviceInfo>

namespace Ui
{
    class Options;
}

class Options : public QDialog
{
        Q_OBJECT
    public:
#if QT_VERSION >= 0x050000
        static QList<QAudioDeviceInfo> devices;
#endif
        static int PreferredDevice;
        explicit Options(QWidget *parent = 0);
        ~Options();

    private slots:
        void on_buttonBox_rejected();
        void on_buttonBox_accepted();

    private:
        Ui::Options *ui;
};

#endif // OPTIONS_HPP
