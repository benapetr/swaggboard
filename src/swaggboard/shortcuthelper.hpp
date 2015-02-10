//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#ifndef SHORTCUTHELPER_HPP
#define SHORTCUTHELPER_HPP

#include "definitions.hpp"
#include <QStringList>

class ShortcutHelper
{
    public:
        static unsigned int LastID;

        ShortcutHelper();
        ~ShortcutHelper();
        void Register();
        void Unregister();
        bool ValidateShortcut(QString key);
        void RegisterKeys(QString key);
        QString GetKeys() {    return this->keys;    }
        unsigned int id;
        int offset;
        QString file;
    private:
        void Parse();
        QStringList part;
        bool is_parsed;
        bool is_registered;
        bool is_valid;
        QString internal_key;
        QString keys;
        bool loop;
};

#endif // SHORTCUTHELPER_HPP
