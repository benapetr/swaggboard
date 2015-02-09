//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

#ifndef INFORMATION_HPP
#define INFORMATION_HPP

#include <QDialog>

namespace Ui {
    class Information;
}

class Information : public QDialog
{
        Q_OBJECT

    public:
        explicit Information(QWidget *parent = 0);
        ~Information();

    private:
        Ui::Information *ui;
};

#endif // INFORMATION_HPP
