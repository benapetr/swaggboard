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

static CHAR *                      //   return error message
getLastErrorText(                  //   converts "Last Error" code into text
CHAR *pBuf,                        //   message buffer
ULONG bufSize)                     //   buffer size
{
     DWORD retSize;
     LPTSTR pTemp=NULL;

     if (bufSize < 16)
     {
          if (bufSize > 0)
          {
               pBuf[0]='\0';
          }
          return(pBuf);
     }
     retSize=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|
                           FORMAT_MESSAGE_FROM_SYSTEM|
                           FORMAT_MESSAGE_ARGUMENT_ARRAY,
                           NULL,
                           GetLastError(),
                           LANG_NEUTRAL,
                           (LPTSTR)&pTemp,
                           0,
                           NULL );
     if (!retSize || pTemp == NULL)
     {
          pBuf[0]='\0';
     }
     else
     {
          sprintf(pBuf,"%0.*s (0x%x)",(int)bufSize-16,(char*)pTemp,(unsigned int)GetLastError());
          LocalFree((HLOCAL)pTemp);
     }
     return (pBuf);
}

// https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx
static UINT virtual_key(QString vk)
{
    if (vk.size() == 1)
    {
        char in = vk.at(0).toLatin1();
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
            case 'd':
                return 0x44;
            case 'e':
                return 0x45;
            case 'f':
                return 0x46;
            case 'g':
                return 0x47;
            case 'h':
                return 0x48;
            case 'i':
                return 0x49;
            case 'j':
                return 0x4A;
            case 'k':
                return 0x4B;
            case 'l':
                return 0x4C;
            case 'm':
                return 0x4D;
            case 'n':
                return 0x4E;
            case 'o':
                return 0x4F;
            case 'p':
                return 0x50;
            case 'q':
                return 0x51;
            case 'r':
                return 0x52;
            case 's':
                return 0x53;
            case 't':
                return 0x54;
            case 'u':
                return 0x55;
            case 'v':
                return 0x56;
            case 'w':
                return 0x57;
            case 'x':
                return 0x58;
            case 'y':
                return 0x59;
            case 'z':
                return 0x5A;

        }
    } else if (vk == "f1")
    {
        return 0x70;
    } else if (vk == "f2")
    {
        return 0x71;
    } else if (vk == "f3")
    {
        return 0x72;
    } else if (vk == "f4")
    {
        return 0x73;
    } else if (vk == "f5")
    {
        return 0x74;
    } else if (vk == "f6")
    {
        return 0x75;
    } else if (vk == "f7")
    {
        return 0x76;
    } else if (vk == "f8")
    {
        return 0x77;
    } else if (vk == "f9")
    {
        return 0x78;
    } else if (vk == "f10")
    {
        return 0x79;
    } else if (vk == "f11")
    {
        return 0x7A;
    } else if (vk == "f12")
    {
        return 0x7B;
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
        Warn("Shortcut " + this->keys + " is not a valid shortcut and will not be used, example of good shortcut:\nctrl + x");
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
    if (!RegisterHotKey(NULL, this->id, modifiers, virtual_key(this->internal_key)))
    {
        char mb[400];
        QString error = QString(getLastErrorText(mb, 400));
        Warn("Unable to register shortcut, system error: " + error);
    }
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
            if (this->part[x] != "ctrl" && this->part[x] != "win" && this->part[x] != "alt" && this->part[x] != "shift")
                this->is_valid = false;
        } else if (this->part[x].size() == 1)
        {
            this->internal_key = this->part[x];
        }
        x++;
    }
}
