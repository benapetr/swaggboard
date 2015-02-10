//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QDrag>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QUrl>
#include <QMimeData>
#include <QFileInfo>
#include <QDragMoveEvent>
#include "items.hpp"
#include "mainwindow.hpp"
#include "musicfinder.hpp"
#include "shortcuthelper.hpp"
#include "ui_items.h"

Items::Items(QWidget *parent) : QFrame(parent), ui(new Ui::Items)
{
    this->ui->setupUi(this);
    QStringList header;
    this->setAcceptDrops(true);
    this->mw = (MainWindow*)parent;
    header << "Shortcut" << "Sound" << "Loop" << "Volume";
    this->ui->tableWidget->setColumnCount(4);
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->tableWidget->setShowGrid(false);
    this->ui->tableWidget->setColumnWidth(0, 80);
    this->ui->tableWidget->setColumnWidth(1, 200);
    this->ui->tableWidget->setColumnWidth(2, 60);
    this->on_horizontalSlider_valueChanged(100);
    this->ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

Items::~Items()
{
    delete this->ui;
}

QSlider *Items::GetVolumeBar()
{
    return this->ui->horizontalSlider;
}

QTableWidget *Items::GetWidget()
{
    return this->ui->tableWidget;
}

void Items::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *xx = event->mimeData();
    QList<QUrl> urls = xx->urls();
    if (urls.count() > 0)
    {
        foreach(QUrl url, urls)
        {
            QString path = url.url();
            if (path.startsWith("file:///"))
                path = path.mid(8);
            if (QFileInfo(path).isDir())
                return;
        }
        event->acceptProposedAction();
    }
}

void Items::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void Items::dragLeaveEvent(QDragLeaveEvent *event)
{
    //event->acceptProposedAction();
}

void Items::dropEvent(QDropEvent *event)
{
    const QMimeData *xx = event->mimeData();
    QList<QUrl> urls = xx->urls();
    if (urls.count() > 0)
    {
        foreach(QUrl url, urls)
        {
            QString path = url.url();
            if (path.startsWith("file:///"))
                path = path.mid(8);
            if (QFileInfo(path).isDir())
                continue;
            int row = this->mw->Make(SOUNDS);
            MusicFinder *finder = this->mw->GetFinder(row);
            if (finder)
                finder->SetFile(path);
        }
        event->acceptProposedAction();
    }
}

void Items::on_tableWidget_cellChanged(int row, int column)
{
    this->mw->changed = true;
    if (column != 0)
        return;
    if (this->mw->SL.count() <= row)
        return;
    this->mw->SL.at(row)->RegisterKeys(this->GetWidget()->item(row, column)->text());
}

void Items::on_horizontalSlider_valueChanged(int value)
{
    this->mw->Volume(value);
    this->GetVolumeBar()->setToolTip("VOLUME IS " + QString::number(value) + "% RIGHT NOW");
}
