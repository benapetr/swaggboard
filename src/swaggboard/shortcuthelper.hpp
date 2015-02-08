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
#include <QString>

class ShortcutHelper
{
    public:
        ShortcutHelper();
        bool ValidateShortcut(QString key);
        void RegisterKeys(QString key);
        QString GetKeys()
        {
            return this->keys;
        }
        QString file;
    private:
        QString keys;
        bool loop;
};

#endif // SHORTCUTHELPER_HPP
