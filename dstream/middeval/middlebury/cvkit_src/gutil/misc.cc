/*
 * This file is part of the Computer Vision Toolkit (cvkit).
 *
 * Author: Heiko Hirschmueller
 *
 * Copyright (c) 2014, Institute of Robotics and Mechatronics, German Aerospace Center
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "misc.h"

#include <sstream>
#include <iostream>

#ifdef __GNUC__
#include <dirent.h>
#endif

using std::string;
using std::vector;
using std::set;
using std::stringstream;

bool gutil::isMSBFirst()
{
    int p=1;
    char *c=reinterpret_cast<char *>(&p);
    
    if (c[0] == 1)
      return false;
    
    return true;
}

void gutil::trim(string &s)
{
    size_t pos;
    
    pos=0;
    while (pos < s.size() && isspace(s[pos]))
      pos++;
    
    if (pos > 0)
      s=s.substr(pos);
    
    pos=s.size();
    if (pos > 0)
      pos--;
    
    while (pos >= 0 && isspace(s[pos]))
      pos--;
    
    if (pos < s.size()-1)
      s=s.substr(0, pos+1);
}

void gutil::split(vector<string> &list, const string &s, char delim, bool skip_empty)
{
    stringstream in(s);
    string elem;
    
    while (getline(in, elem, delim))
    {
      if (!skip_empty || elem.size() > 0)
        list.push_back(elem);
    }
}

void gutil::getFileList(set<string> &list, const string &prefix,
  const string &suffix)
{
#ifdef __GNUC__
    string dir="", pref=prefix;
    size_t pos=pref.find_last_of("/\\");
    
    if (pos < pref.size())
    {
      dir=pref.substr(0, pos+1);
      pref=pref.substr(pos+1);
    }
    
    DIR *p;
    if (dir.size() > 0)
      p=opendir(dir.c_str());
    else
      p=opendir(".");
    
    if (p == 0)
      throw IOException("Cannot open directory: "+dir);
    
    list.clear();
    
    struct dirent *entry=readdir(p);
    while (entry != 0)
    {
      string name=entry->d_name;
      
      if (name.find(pref) ==  0 && (suffix.size() == 0 ||
        name.rfind(suffix) == name.size()-suffix.size()))
        list.insert(dir+name);
      
      entry=readdir(p);
    }
    
    closedir(p);
#else
    assert(false);
#endif
}

