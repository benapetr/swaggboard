//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef ITEMS_HPP
#define ITEMS_HPP

#include <QFrame>

namespace Ui
{
    class Items;
}

class QTableWidget;
class QSlider;
class MainWindow;

class Items : public QFrame
{
        Q_OBJECT

    public:
        explicit Items(QWidget *parent = 0);
        ~Items();
        QSlider *GetVolumeBar();
        QTableWidget *GetWidget();

    protected:
         void dragEnterEvent(QDragEnterEvent *event);
         void dragMoveEvent(QDragMoveEvent *event);
         void dragLeaveEvent(QDragLeaveEvent *event);
         void dropEvent(QDropEvent *event);
    private slots:
        void on_tableWidget_cellChanged(int row, int column);
        void on_horizontalSlider_sliderMoved(int position);
    private:
        MainWindow *mw;
        Ui::Items *ui;
};

#endif // ITEMS_HPP
