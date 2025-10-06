/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>

int endsWith(const char *str, const char *suffix)
{
    int lenstr;
    int lensuffix;

    if (!str || !suffix)
        return 0;
    lenstr = (int)strlen(str);
    lensuffix = (int)strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void trimTrailingSpaces(char* path) {
    int i;
    int len = (int)strlen(path);
    for (i=len-1;i>=0;i--) {
        if (path[i]==' ')
            path[i] = 0;
        else
            break;
    }
}

void stringReplace(const char* searchStr, const char* replaceStr, char* str, int len) {
    char* p = strstr(str, searchStr);
    int sLen = (int)strlen(searchStr);
    int rLen = (int)strlen(replaceStr);

    while (p) {
        memmove(p+rLen, p+sLen, strlen(p)-sLen+1);
        strncpy(p, replaceStr, rLen);
        p+=rLen;
        p=strstr(p, searchStr);
    }
}