//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "shortcuthelper.hpp"
#include <QStringList>
#include <QMessageBox>
#ifdef WIN
#include <windows.h>
#endif

unsigned int ShortcutHelper::LastID = 0;

#ifdef WIN

static void Warn(QString reason)
{
    QMessageBox box;
    box.setWindowTitle("Warning");
    box.setText(reason);
    box.exec();
}

static UINT virtual_key(char in)
{
    switch (in)
    {
        case '0':
            return 0x30;
        case '1':
            return 0x31;
        case '2':
            return 0x32;
        case '3':
            return 0x33;
        case '4':
            return 0x34;
        case '5':
            return 0x35;
        case '6':
            return 0x36;
        case '7':
            return 0x37;
        case '8':
            return 0x38;
        case '9':
            return 0x39;
        case 'a':
            return 0x41;
        case 'b':
            return 0x42;
        case 'c':
            return 0x43;
    }
    return 0;
}

#endif

ShortcutHelper::ShortcutHelper()
{
    this->id = LastID++;
    this->is_parsed = false;
    this->is_valid = false;
    this->is_registered = false;
    this->internal_key = '\0';
    this->keys = "None";
}

ShortcutHelper::~ShortcutHelper()
{
    this->Unregister();
}

void ShortcutHelper::Register()
{
    this->Parse();
    if (!this->is_valid)
    {
        Warn("Shortcut " + this->keys + " is not a valid shortcut and will not be used");
        return;
    }
#ifdef WIN
    UINT modifiers = 0;
    if (this->part.contains("shift"))
        modifiers = modifiers | MOD_SHIFT;
    if (this->part.contains("alt"))
        modifiers = modifiers | MOD_ALT;
    if (this->part.contains("win"))
        modifiers = modifiers | MOD_WIN;
    if (this->part.contains("ctrl"))
        modifiers = modifiers | MOD_CONTROL;

    UINT key = virtual_key(this->internal_key);
    if (key == 0)
    {
        Warn(QString("Invalid virtual key: ") + this->internal_key + " shortcut for " + this->keys + " will not be registered");
        return;
    }
    RegisterHotKey(NULL, this->id, modifiers, virtual_key(this->internal_key));
    this->is_registered = true;
#endif
}

void ShortcutHelper::Unregister()
{
    if (!this->is_registered)
        return;
#ifdef WIN
    UnregisterHotKey(NULL, this->id);
#endif
}

bool ShortcutHelper::ValidateShortcut(QString key)
{
    key = key.trimmed();
    if (key.isEmpty())
        return true;
    QStringList items = key.split("+");
    return true;
}

void ShortcutHelper::RegisterKeys(QString key)
{
    this->keys = key;
    this->is_parsed = false;
    this->Unregister();
    this->Register();
}

void ShortcutHelper::Parse()
{
    this->is_parsed = true;
    this->part.clear();
    this->part = this->keys.split("+");
    int x = 0;
    this->is_valid = true;
    while (x < this->part.count())
    {
        this->part[x] = this->part[x].trimmed().toLower();
        if (this->part[x].isEmpty())
        {
            this->part.removeAt(x);
            continue;
        }
        if (this->part[x].size() > 1)
        {
            if (this->part[x] != "ctrl" && this->part[x] != "alt" && this->part[x] != "shift")
                this->is_valid = false;
        } else if (this->part[x].size() == 1)
        {
            this->internal_key = this->part[x].at(0).toLatin1();
        }
        x++;
    }
}
