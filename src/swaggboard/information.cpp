//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

#include "information.hpp"
#include "ui_information.h"
#include <QDesktopServices>

Information::Information(QWidget *parent) : QDialog(parent), ui(new Ui::Information)
{
    this->ui->setupUi(this);
}

Information::~Information()
{
    delete ui;
}
