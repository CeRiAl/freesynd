/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2011  Joey Parrish  <joey.parrish@gmail.com>         *
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#ifndef VERSION_H
#define VERSION_H

class format_version
{
    public:
        format_version(unsigned char vMaj, unsigned char vMin)
            : major_(vMaj), minor_(vMin)
        {
        }

        inline int major() const { return major_; }
        inline int minor() const { return minor_; }

        // example: v1.1 has combined value of 0x0101
        inline int combined() const { return ((major_ << 8) | minor_); }

        inline bool operator==(int value) const { return combined() == value; }
        inline bool operator!=(int value) const { return combined() != value; }

    private:
        int major_;
        int minor_;
};

#endif
