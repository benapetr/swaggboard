//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef MUSICFINDER_HPP
#define MUSICFINDER_HPP

#include <QFrame>

namespace Ui
{
    class MusicFinder;
}

class ShortcutHelper;

class MusicFinder : public QFrame
{
        Q_OBJECT
    public:
        explicit MusicFinder(QWidget *parent, ShortcutHelper *hx);
        ~MusicFinder();
        void SetFile(QString path);
        QString RetrievePath();
        void Stop();
        ShortcutHelper *helper;

    private slots:
        void on_pushSelect_clicked();

    private:
        QString file;
        Ui::MusicFinder *ui;
};

#endif // MUSICFINDER_HPP
