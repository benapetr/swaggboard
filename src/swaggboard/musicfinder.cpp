//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "musicfinder.hpp"
#include "shortcuthelper.hpp"
#include "ui_musicfinder.h"
#include <QFileDialog>

MusicFinder::MusicFinder(QWidget *parent, ShortcutHelper *hx) : QFrame(parent), ui(new Ui::MusicFinder)
{
    this->helper = hx;
    this->ui->setupUi(this);
}

MusicFinder::~MusicFinder()
{
    delete this->ui;
}

void MusicFinder::SetFile(QString path)
{
    this->ui->lineEdit->setText(path);
    this->file = path;
    if (path == "stop")
    {
        this->helper->file = path;
        this->ui->pushSelect->setEnabled(false);
    }
}

QString MusicFinder::RetrievePath()
{
    return this->file;
}

void MusicFinder::Stop()
{
    this->SetFile("stop");
}

void MusicFinder::on_pushSelect_clicked()
{
    QFileDialog fd(this);
    QStringList fltr;
#ifdef WIN
    fltr << "Music files (*.mp3 *.wav *.aiff *.wma *.midi)"
#else
    fltr << "Music files (*.mp3 *.wav)"
#endif
         << "Any files (*)";
    fd.setNameFilters(fltr);
    fd.exec();
    QStringList f = fd.selectedFiles();
    if (f.isEmpty())
        return;

    file = f.at(0);
    this->helper->file = file;
    this->ui->lineEdit->setText(file);
}
