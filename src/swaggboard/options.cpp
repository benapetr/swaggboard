//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

#include "options.hpp"
#include "ui_options.h"
#include <QSettings>
#include <QAudioOutput>

#if QT_VERSION >= 0x050000
QList<QAudioDeviceInfo> Options::devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
#endif
int Options::PreferredDevice = -1;

Options::Options(QWidget *parent) : QDialog(parent), ui(new Ui::Options)
{
    this->ui->setupUi(this);
    foreach (QAudioDeviceInfo i, devices)
        this->ui->comboBox->addItem(i.deviceName());
    QSettings s;
    this->ui->comboBox->setCurrentIndex(s.value("device", 0).toInt());
}

Options::~Options()
{
    delete this->ui;
}

void Options::on_buttonBox_rejected()
{
    this->close();
}

void Options::on_buttonBox_accepted()
{
    QSettings s;
    s.setValue("device", this->ui->comboBox->currentIndex());
    this->PreferredDevice = this->ui->comboBox->currentIndex();
    this->close();
}
