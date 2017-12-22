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

#include "fileimagewindow.h"
#include "imageadapter.h"

#include <gutil/exception.h>
#include <gimage/io.h>
#include <gimage/view.h>

#include <fstream>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::ostringstream;

using gutil::Exception;
using gutil::Properties;
using gutil::IOException;

using gimage::ImageU8;
using gimage::ImageU16;
using gimage::ImageFloat;
using gimage::getImageIO;
using gimage::getNewImageName;
using gimage::loadViewProperties;

namespace bgui
{

namespace
{

string createHelpText(bool incl_view)
{
    ostringstream out;
    
    out << "\n";
    out << "Files:\n";
    out << "<cursor left>  Load previous image\n";
    out << "<cursor right> Load next image\n";
#ifdef INCLUDE_X11
    out << "'u'            Reload current image and toggle watching the current image for changes\n";
#else
    out << "'u'            Reload current image\n";
#endif
    out << "'k'            Switch between keep, keep_all and not keeping the settings when (re)loading\n";
    out << "'c'            Capture current content of the window and store it next to the original image\n";
    out << "'d'            Rename current image by adding the suffix .bak and remove it from the list\n";
    
    if (incl_view)
    {
      out << "\n";
      out << "3D Visualization:\n";
      out << "'p'            Open part of disparity image (with parameter file) in plyv\n";
    }
    
    return out.str();
}

}

void FileImageWindow::load(unsigned int &pos, bool down, int w, int h, bool size_max)
{
    ImageAdapterBase *adapt=0;
    
      // load image and store in adatper
    
    while (adapt == 0 && pos < list.size())
    {
      ImageU8    *imageu8=0;
      ImageU16   *imageu16=0;
      ImageFloat *imagefloat=0;
      
      try
      {
        imageu8=new ImageU8();
        getImageIO().load(*imageu8, list[pos].c_str());
        adapt=new ImageAdapter<unsigned char>(imageu8, vmin, vmax, true);
        imageu8=0;
      }
      catch (const Exception &ex)
      {
        delete imageu8;
        imageu8=0;
        
        try
        {
          imageu16=new ImageU16();
          getImageIO().load(*imageu16, list[pos].c_str());
          adapt=new ImageAdapter<unsigned short>(imageu16, vmin, vmax, true);
          imageu16=0;
        }
        catch (const Exception &ex)
        {
          delete imageu16;
          imageu16=0;
          
          try
          {
            imagefloat=new ImageFloat();
            getImageIO().load(*imagefloat, list[pos].c_str());
            adapt=new ImageAdapter<float>(imagefloat, vmin, vmax, true);
            imagefloat=0;
          }
          catch (const Exception &ex)
          {
            delete imagefloat;
            imagefloat=0;
            
            cerr << "Cannot load image: " << list[pos] << endl;
            
            if (down)
            {
              list.erase(list.begin()+pos);
              
              if (pos > 0 && pos >= list.size())
                pos--;
            }
            else
            {
              list.erase(list.begin()+pos);
              
              if (pos > 0)
                pos--;
            }
          }
        }
      }
    }
    
      // watch file
    
    if (watch_file)
    {
      removeFileWatch(wid);
      
      if (pos < list.size())
        wid=addFileWatch(list[pos].c_str());
      else
        watch_file=false;
    }
    
      // set image adapter and title
    
    if (adapt != 0)
    {
      Properties prop;
      getImageIO().loadProperties(prop, list[pos].c_str());
      
      int rotation, flip;
      prop.getValue("rotation", rotation, "0");
      prop.getValue("flip", flip, "0");
      
      adapt->setRotationFlip(rotation/90, flip);
      
      if (imin < imax && kp == keep_all)
      {
        adapt->setMinIntensity(imin);
        adapt->setMaxIntensity(imax);
      }
      
      if (w > 0 && h > 0)
      {
        adapt->setMapping(map);
        adapt->setChannel(channel);
        setAdapter(adapt, true, keep_none, w, h, size_max);
      }
      else
      {
        if (kp == keep_none)
        {
          adapt->setMapping(map);
          adapt->setChannel(channel);
        }
        
        setAdapter(adapt, true, kp);
      }
    }
    else
      setAdapter(0);
    
    updateTitle();
}

void FileImageWindow::updateTitle()
{
    ImageAdapterBase *adapt=getAdapter();
    
      // set image adapter and title
    
    if (adapt != 0)
    {
      ostringstream os;
      
      string name=list[current];
      
      os << name << " - " << adapt->getOriginalWidth() << "x" <<
        adapt->getOriginalHeight() << "x" << adapt->getOriginalDepth() <<
        " " << adapt->getOriginalType();
      
      if (watch_file || kp != keep_none)
      {
        os << " -";
        
        if (watch_file)
          os << " " << "watch";
        
        if (kp == keep_most)
          os << " " << "keep";
        
        if (kp == keep_all)
          os << " " << "keep_all";
      }
      
      setTitle(os.str().c_str());
    }
    else
    {
      setAdapter(0);
      setTitle("Image");
      setInfoText("No image!");
    }
}

void FileImageWindow::saveContent(const char *basename)
{
    ImageU8 image;
    
    getContent(image);
    
      // find file name that is not used
    
    string base=basename;
    
    size_t i=base.rfind('.');
    if (i != base.npos)
      base=base.substr(0, i);
    
    string name=getNewImageName(base);
    
    if (name.size() > 0)
    {
      getImageIO().save(image, name.c_str());
      setInfoLine(("Saved as "+name).c_str());
    }
    else
      setInfoLine("Sorry, cannot determine file name for storing image!");
}

FileImageWindow::FileImageWindow(const vector<string> &files, int firstfile,
  bool watch, int x, int y, int w, int h, bool size_max, double init_min,
  double init_max, double valid_min, double valid_max, keep k, mapping m,
  int c, const char *viewcmd)
{
    if (files.empty())
      throw IOException("The list of files must not be empty");
    
    addHelpText(createHelpText(viewcmd != 0));
    
    if (viewcmd != 0)
      vc=viewcmd;
    
    list=files;
    current=firstfile;
    
    imin=init_min;
    imax=init_max;
    vmin=valid_min;
    vmax=valid_max;
    kp=k;
    map=m;
    channel=c;
    watch_file=watch;
    wid=-1;
    
    if (w <= 0 || h <= 0)
    {
      getDisplaySize(w, h);
      size_max=true;
    }
    
    load(current, true, w, h, size_max);
    
    setVisible(true);
    
    if (x >= 0 && y >= 0)
      setPosition(x, y);
}

FileImageWindow::~FileImageWindow()
{ }

void FileImageWindow::onKey(char c, SpecialKey key, int x, int y)
{
    switch (key)
    {
      case k_left: /* load previous image */
          if (current > 0)
          {
            current--;
            load(current, false);
          }
          
          setInfoLine("");
          break;
      
      case k_right: /* load next image */
          current++;
          if (current < list.size())
            load(current, true);
          else
            current--;
          
          setInfoLine("");
          break;
      
      default:
          break;
    }
    
    switch (c)
    {
      case 'u': /* update image and toggle watch flag */
#ifdef INCLUDE_X11
          watch_file=!watch_file;
          
          if (watch_file)
          {
            load(current, true);
          }
          else
          {
            removeFileWatch(wid);
            updateTitle();
          }
#else
          load(current, true);
#endif
          break;
      
      case 'k':
          switch (kp)
          {
            case keep_none:
                kp=keep_most;
                break;
            
            case keep_most:
                kp=keep_all;
                break;
            
            case keep_all:
                kp=keep_none;
                break;
          }
          
          updateTitle();
          break;
      
      case 'c':
          if (current < list.size())
            saveContent(list[current].c_str());
          break;
      
      case 'd':
          if (current < list.size())
          {
            string name=list[current];
            
            list.erase(list.begin()+current);
            
            if (current > 0 && current >= list.size())
              current--;
            
            load(current, true);
            
            if (rename(name.c_str(), (name+".bak").c_str()) != 0)
              setInfoLine("Cannot remove image file!");
            else
              setInfoLine("");
            
            if (list.size() == 0)
              sendClose();
          }
          break;
      
      case 'p':
          if (vc.size() > 0)
          {
            if (getAdapter()->getOriginalDepth() == 1)
            {
              try
              {
                Properties prop;
                loadViewProperties(prop, list[current].c_str(),
                  getenv("CVKIT_SPATH"), false);
                
                long xp, yp, wp, hp;
                visibleImagePart(xp, yp, wp, hp);
                
                ostringstream out;
                
#ifdef WIN32
                out << "start \"plyv\" ";
#endif
                out << "\"" << vc << "\" \"";
                out << list[current];
                out << ",x=" << xp;
                out << ",y=" << yp;
                out << ",w=" << wp;
                out << ",h=" << hp;
                out << "\"";
#ifndef WIN32
                out << " &";
#endif
                cout << out.str() << endl;
                
                if (system(out.str().c_str()) == -1)
                  cout << "Failed!" << endl;
              }
              catch (const IOException &ex)
              {
                setInfoLine("Missing parameter files!");
              }
            }
            else
              setInfoLine("Not a disparity image!");
          }
          break;
    }
    
    ImageWindow::onKey(c, key, x, y);
}

void FileImageWindow::onFileChanged(int watchid)
{
    if (watchid == wid)
      load(current, true);
    else
      ImageWindow::onFileChanged(watchid);
}

}
