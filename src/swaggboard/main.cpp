//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QApplication>
#include <QSettings>
#include "mainwindow.hpp"
#include "definitions.hpp"
#include "shortcuthelper.hpp"
#include "options.hpp"

#ifdef _MSC_VER
#    pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

#ifdef WIN
#include <windows.h>
#ifdef PlaySound
    #undef PlaySound
#endif

int winmain(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QApplication::processEvents();

    Options::Initialize();
    QSettings s;
    Options::PreferredDevice = s.value("device", -1).toInt();

    MSG msg;
    while(GetMessage(&msg,NULL,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_HOTKEY)
        {
            foreach (ShortcutHelper *s, w.SL)
            {
                if (s->id == msg.wParam)
                {
                    w.PlaySound(s->file);
                }
            }
            //if (msg.wParam == 1) qDebug() << "Hot Key activated : ALT + B";
            //if (msg.wParam == 2) qDebug() << "Hot Key activated : ALT + D";
        }
    }
    return msg.wParam;
}
#endif

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("petrbena");
    QApplication::setApplicationName("swaggboard");
#ifdef WIN
    return winmain(argc, argv);
#else
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
#endif
}
