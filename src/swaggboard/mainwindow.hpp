//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QList>
#include <QString>

#define STOP 0
#define SOUNDS 1

namespace Ui
{
    class MainWindow;
}

class MusicFinder;
class QSlider;
class Items;
class ShortcutHelper;

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        static QSlider *MakeSlider(QObject *t);
        static MainWindow *mw;

        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();
        void Shutdown();
        void Load(QString path);
        void Stop();
        void PlaySound(QString path, int offset);
        void Save();
        void Volume(int volume);
        void SetOffset(int volume);
        QString GetConfig();
        MusicFinder *GetFinder(int i);
        Items *Values;
        QString File;

    private slots:
        void on_actionExit_triggered();
        void on_actionAdd_shortcut_triggered();
        void on_actionRemove_shortcut_triggered();
        void on_actionPlay_triggered();
        void on_actionStop_playing_triggered();
        void on_actionSave_triggered();
        void on_actionLoad_triggered();
        void OnVolume(int position);
        void closeEvent(QCloseEvent *event);
        void on_actionAdd_shortcut_to_stop_triggered();
        void on_actionHow_to_use_triggered();
        void on_actionSave_as_triggered();
        void on_actionPreferences_triggered();
        void on_actionKeys_triggered();

    private:
        int VolumeOffset;
        Ui::MainWindow *ui;
};

#endif // MAINWINDOW_HPP
