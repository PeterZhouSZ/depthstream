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

#ifndef GIMAGE_PAINT_H
#define GIMAGE_PAINT_H

#include "image.h"

namespace gimage
{

template<class T> void validRange(Image<T> &image, T from, T to)
{
    for (int d=0; d<image.getDepth(); d++)
    {
      for (long k=0; k<image.getHeight(); k++)
      {
        for (long i=0; i<image.getWidth(); i++)
        {
          T v=image.get(i, k, d);
          
          if ((v < from || v > to) && image.isValidS(v))
          {
            for (int j=0; j<image.getDepth(); j++)
              image.setInvalid(i, k, j);
          }
        }
      }
    }
}

template<class T> void paste(Image<T> &image, const Image<T> &image2, long i0=0, long k0=0, int d0=0, float opacity=1.0f)
{
    if (opacity < 1)
    {
      opacity=std::max(0.0f, opacity);
      
      for (int d=0; d<image2.getDepth(); d++)
        if (d0+d >= 0 && d0+d < image.getDepth())
          for (long k=0; k<image2.getHeight(); k++)
            if (k0+k >= 0 && k0+k < image.getHeight())
              for (long i=0; i<image2.getWidth(); i++)
                if (i0+i >= 0 && i0+i < image.getWidth())
                {
                  float v=(1-opacity)*image.get(i0+i, k0+k, d0+d);
                  v+=opacity*image2.get(i, k, d);
                  image.set(i0+i, k0+k, d0+d, static_cast<typename Image<T>::store_t>(v));
                }
    }
    else
    {
      for (int d=0; d<image2.getDepth(); d++)
        if (d0+d >= 0 && d0+d < image.getDepth())
          for (long k=0; k<image2.getHeight(); k++)
            if (k0+k >= 0 && k0+k < image.getHeight())
              for (long i=0; i<image2.getWidth(); i++)
                if (i0+i >= 0 && i0+i < image.getWidth())
                  image.set(i0+i, k0+k, d0+d, image2.get(i, k, d));
    }
}

template<class T> void drawLine(Image<T> &image, long x1, long y1, long x2,
  long y2, T v, int c=-1)
{
    int cmin=0;
    int cmax=image.getDepth()-1;
    
    if (c >= 0)
      cmin=cmax=c;
    
    long dx=x2-x1;
    long dy=y2-y1;
    
    if (dx != 0 || dy != 0)
    {
      long adx=abs(dx);
      long ady=abs(dy);
      
      if (adx > ady)
      {
        long i, n;
        
        if (dx >= 0)
        {
          i=x1;
          n=x2;
        }
        else
        {
          i=x2;
          n=x1;
          dx=-dx;
          dy=-dy;
          x1=x2;
          y1=y2;
        }
        
        if (i < 0)
          i=0;
        
        if (n > image.getWidth()-1)
          n=image.getWidth()-1;
        
        while (i <= n)
        {
          long k=y1+(i-x1)*dy/dx;
          if (k >= 0 && k < image.getHeight())
          {
            for (c=cmin; c<=cmax; c++)
              image.set(i, k, c, v);
          }
          
          i++;
        }
      }
      else
      {
        long k, n;
        
        if (dy >= 0)
        {
          k=y1;
          n=y2;
        }
        else
        {
          k=y2;
          n=y1;
          dx=-dx;
          dy=-dy;
          x1=x2;
          y1=y2;
        }
        
        if (k < 0)
          k=0;
        
        if (n > image.getHeight()-1)
          n=image.getHeight()-1;
        
        while (k <= n)
        {
          long i=x1+(k-y1)*dx/dy;
          if (i >= 0 && i < image.getWidth())
          {
            for (c=cmin; c<=cmax; c++)
              image.set(i, k, c, v);
          }
          
          k++;
        }
      }
    }
    else
    {
      if (x1 >= 0 && y1 >= 0 && x1 < image.getWidth() && y1 < image.getHeight())
      {
        for (c=cmin; c<=cmax; c++)
          image.set(x1, y1, c, v);
      }
    }
}

template<class T> void drawRect(Image<T> &image, long x, long y, long w,
  long h, T v, int c=-1)
{
    long x0=x-(w>>1);
    long y0=y-(h>>1);
    
    drawLine(image, x0, y0, x0+w-1, y0, v, c);
    drawLine(image, x0+w-1, y0, x0+w-1, y0+h-1, v, c);
    drawLine(image, x0+w-1, y0+h-1, x0, y0+h-1, v, c);
    drawLine(image, x0, y0+h-1, x0, y0, v, c);
}

}

#endif
