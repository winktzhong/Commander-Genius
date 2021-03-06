/*
 * CBitmap.cpp
 *
 *  Created on: 03.09.2009
 *      Author: gerstrong
 */

#include "CBitmap.h"
#include "CPalette.h"
#include "FindFile.h"
#include "sdl/CVideoDriver.h"

#include "CLogFile.h"

CBitmap::CBitmap()
{}


CBitmap::CBitmap(const std::shared_ptr<SDL_Surface> &bmpSfc)
{
	mpBitmapSurface = bmpSfc;
}

CBitmap::CBitmap(const CBitmap &bitmap) :
mName(bitmap.getName())
{
	SDL_Surface *sfc = bitmap.getSDLSurface();

    if( sfc != nullptr )
    {
        // Create new and compatible surface for that bitmap
        SDL_Surface *optSfc = g_pVideoDriver->convertThroughBlitSfc( sfc );
        mpBitmapSurface.reset( optSfc, &SDL_FreeSurface );
    }

}

///
// Creation Routines
///
bool CBitmap::createSurface(Uint32 flags, SDL_Rect rect, SDL_Color *Palette)
{        
    mpBitmapSurface.reset( SDL_CreateRGBSurface(flags, rect.w, rect.h, 8, 0, 0, 0, 0 ), &SDL_FreeSurface );

    auto bmpSfc = mpBitmapSurface.get();

#if SDL_VERSION_ATLEAST(2, 0, 0)   
    SDL_SetPaletteColors(bmpSfc->format->palette, Palette, 0, 255);
    SDL_SetColorKey(bmpSfc, SDL_TRUE, COLORKEY);
#else
    SDL_SetColors(bmpSfc, Palette, 0, 255);
    SDL_SetColorKey(bmpSfc, SDL_SRCCOLORKEY, COLORKEY);
#endif


    if(mpBitmapSurface)
	  return true;
	else
	  return false;
}

bool CBitmap::optimizeSurface()
{
	if(mpBitmapSurface)
	{
		SDL_Surface *temp_surface;
        temp_surface = g_pVideoDriver->convertThroughBlitSfc(mpBitmapSurface.get());
		mpBitmapSurface.reset( temp_surface, &SDL_FreeSurface );
		return true;
	}
	else
		return false;
}

bool CBitmap::loadHQBitmap( const std::string& filename )
{
	if(!IsFileAvailable(filename))
		return false;

	if( mpBitmapSurface )
	{
		SDL_Surface *tempSurface = SDL_LoadBMP(GetFullFileName(filename).c_str());
		if(tempSurface)
		{
            SDL_Surface *displaysurface = SDL_ConvertSurface(tempSurface,
                                                             mpBitmapSurface->format,
                                                             mpBitmapSurface->flags);
			SDL_BlitSurface(displaysurface, NULL, mpBitmapSurface.get(), NULL);
			SDL_FreeSurface(displaysurface);
			SDL_FreeSurface(tempSurface);
			return true;
		}
	}
	return false;
}

void CBitmap::exchangeColor( const Uint32 oldColor, const Uint32 newColor )
{
    SDL_Surface *bmpSfc = mpBitmapSurface.get();

    if(SDL_MUSTLOCK(bmpSfc))
        SDL_LockSurface(bmpSfc);

    Uint8* pixel = (Uint8*) bmpSfc->pixels;

    const int width = bmpSfc->w;
    const int height = bmpSfc->h;

    Uint32 color = 0;
    auto format = bmpSfc->format;

    const int pixelSize = format->BytesPerPixel;

    for( int y=0 ; y<height ; y++ )
    {
        for( int x=0; x<width ; x++ )
        {
            memcpy(&color, pixel, pixelSize );

            if(color == oldColor) // Here we make it transparent
            {
                memcpy(pixel, &newColor, pixelSize );
            }

            pixel += pixelSize;
        }
    }

    if(SDL_MUSTLOCK(bmpSfc))
        SDL_UnlockSurface(bmpSfc);
}

void CBitmap::exchangeColor( const Uint8 oldR, const Uint8 oldG, const Uint8 oldB,
                             const Uint8 newR, const Uint8 newG, const Uint8 newB)
{
    SDL_Surface *bmpSfc = mpBitmapSurface.get();

    const Uint32 oldColor = SDL_MapRGB( bmpSfc->format, oldR, oldG, oldB );
    const Uint32 newColor = SDL_MapRGB( bmpSfc->format, newR, newG, newB );

    exchangeColor( oldColor, newColor );
}

void CBitmap::setColorKey(const Uint8 r, const Uint8 g, const Uint8 b)
{
    auto bmpSfc = mpBitmapSurface.get();
    auto format = bmpSfc->format;
    auto colorkey = SDL_MapRGB( format, r, g, b );

#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_SetColorKey(bmpSfc, SDL_TRUE, colorkey);
#else
    SDL_SetColorKey(bmpSfc, SDL_SRCCOLORKEY, colorkey);
#endif
}

bool CBitmap::scaleTo(const CRect<Uint16> &gameRes)
{
    SDL_Rect newRect = gameRes.SDLRect();

#if SDL_VERSION_ATLEAST(2, 0, 0)

    if(newRect.w == mpBitmapSurface->w && newRect.h == mpBitmapSurface->h)
        return true;

#endif

    std::shared_ptr<SDL_Surface> newSfc;

    // Need to do that, otherwise it won't work.
    optimizeSurface();

    auto bmpSfc = mpBitmapSurface.get();
    auto bmpFormat = bmpSfc->format;

    newSfc.reset( SDL_CreateRGBSurface(bmpSfc->flags,
                                       newRect.w,
                                       newRect.h,
                                       bmpFormat->BitsPerPixel,
                                       0,
                                       0,
                                       0,
                                       0 ),
                    &SDL_FreeSurface );   

    if(!newSfc)
      return false;



#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_BlendMode blendMode;

    SDL_GetSurfaceBlendMode(bmpSfc, &blendMode);
    SDL_SetSurfaceBlendMode(newSfc.get(), blendMode);

#endif

    SDL_BlitScaledWrap( bmpSfc, &bmpSfc->clip_rect, newSfc.get(), &newRect );

    mpBitmapSurface = newSfc;

    return true;
}


///
// Drawing Routines
///

/**
 * \brief The function that blits the sprite to the main blitSurface
 * \param x		 		X-Coordinate, indicating the position on dst
 * \param y				Y-Coordinate, indicating the position on dst
 */
void CBitmap::draw(const int x, const int y)
{
    _draw( x, y, g_pVideoDriver->getBlitSurface() );
}

/**
 * \brief The function that blits the sprite to dst
 * \param x		 		X-Coordinate, indicating the position on dst
 * \param y				Y-Coordinate, indicating the position on dst
 * \param dst           Surface where to blit that Bitmap
 */
void CBitmap::_draw(const int x, const int y, SDL_Surface *dst) const
{
    SDL_Rect dst_rect, src_rect;

    SDL_Surface *bmpPtr = mpBitmapSurface.get();

    dst_rect.x = x;	dst_rect.y = y;
    src_rect.x = 0; src_rect.y = 0;
    src_rect.w = bmpPtr->w;
    src_rect.h = bmpPtr->h;

    if(dst_rect.x < 0)
    {
        src_rect.x = -dst_rect.x;
        dst_rect.x = 0;
        src_rect.w -= src_rect.x;
    }

    if(dst_rect.y < 0)
    {
        src_rect.y = -dst_rect.y;
        dst_rect.y = 0;
        src_rect.h -= src_rect.y;
    }

    SDL_BlitSurface( bmpPtr, &src_rect, dst, &dst_rect );
}


