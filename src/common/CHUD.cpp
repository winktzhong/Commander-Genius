/*
 * CHUD.cpp
 *
 *  Created on: 01.07.2010
 *      Author: gerstrong
 */

#include "CHUD.h"
#include "engine/CCamera.h"
#include "common/CBehaviorEngine.h"
#include "sdl/CVideoDriver.h"
#include "sdl/extensions.h"
#include "graphics/CGfxEngine.h"
#include "StringUtils.h"

const int EFFECT_TIME = 10;
const int EFFECT_SPEED = 10;

CHUD::CHUD(unsigned long &score, signed char &lives,
           unsigned int &charges, const int id,
           int *camlead) :
m_score(score),
m_lives(lives),
m_charges(charges),
m_oldScore(score),
m_oldCharges(charges),
mpCamlead(camlead),
timer(0)
{
    setup(id);
}

void CHUD::createHUDBlit()
{    
#if SDL_VERSION_ATLEAST(2, 0, 0)
    mpHUDBlit.reset( CG_CreateRGBSurface( m_Rect ), &SDL_FreeSurface );
    mpHUDBlit.reset(g_pVideoDriver->convertThroughBlitSfc(mpHUDBlit.get()), &SDL_FreeSurface);

    SDL_SetSurfaceAlphaMod( mpHUDBlit.get(), 220);
#else    
    auto *blit = g_pVideoDriver->getBlitSurface();
    SDL_PixelFormat *format = blit->format;

    SDL_Surface *sfc = SDL_CreateRGBSurface( SDL_SWSURFACE,
                m_Rect.w, m_Rect.h, RES_BPP,
                format->Rmask,
                format->Gmask,
                format->Bmask,
                format->Amask );

    mpHUDBlit.reset( sfc, &SDL_FreeSurface );
#endif
}

void CHUD::setup(const int id)
{
    mId = id;

    m_Rect.x = 8;	m_Rect.y = 4;        

    if(g_pBehaviorEngine->mPlayers > 3)
    {
        m_Rect.x = 0;	m_Rect.y = 0;
    }

    size_t Episode = g_pBehaviorEngine->getEpisode();

    if( Episode >= 1 && Episode <= 3 )
    {
        m_Rect.w = 84;	m_Rect.h = 30;
        m_Rect.x += (m_Rect.w-4)*id;
        createHUDBlit();
        CreateVorticonBackground();
    }
    else // Galaxy HUD
    {
        m_Rect.w = 80;	m_Rect.h = 30;
        mHUDBox = *g_pGfxEngine->getSprite(mId,"HUDBACKGROUND");

        #if SDL_VERSION_ATLEAST(2, 0, 0)
        #else
            mHUDBox.optimizeSurface();
        #endif
        m_Rect.h = mHUDBox.getHeight();
        m_Rect.w = mHUDBox.getWidth()-7;
        m_Rect.x += (m_Rect.w-2)*id;

        #if SDL_VERSION_ATLEAST(2, 0, 0)
            SDL_SetSurfaceBlendMode(mHUDBox.getSDLSurface(), SDL_BLENDMODE_NONE);
        #else
            SDL_SetAlpha(mHUDBox.getSDLSurface(), 0, 0);
        #endif

        createHUDBlit();
    }
}

/**
 * \brief This function prepares the Background, so in process it can be rendered.
 * This function should only be called once!
 */
void CHUD::CreateVorticonBackground()
{
    // Create a surface for the Background
    mpBackground.reset( SDL_ConvertSurface( mpHUDBlit.get(), mpHUDBlit->format, 0), &SDL_FreeSurface );

    #if SDL_VERSION_ATLEAST(2, 0, 0)
    #else
        SDL_SetAlpha(mpBackground.get(), 0, 0);
    #endif

	SDL_Rect headsrcrect, headdstrect;
	headsrcrect.x = 0;
	headsrcrect.y = 0;
	headdstrect.w = headsrcrect.w = 16;
	headdstrect.h = headsrcrect.h = 16;
    headdstrect.x = 0;
    headdstrect.y = 11;

    mKeenHeadSprite = g_pGfxEngine->getSprite(mId,PMAPDOWNFRAME);

    SDL_Surface *keenHeadSfc = mKeenHeadSprite.getSDLSurface();

#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_SetSurfaceBlendMode( keenHeadSfc, SDL_BLENDMODE_NONE);
#else
    SDL_SetAlpha(keenHeadSfc, 0, 0);
#endif

    SDL_BlitSurface( keenHeadSfc, &headsrcrect, mpBackground.get(), &headdstrect );


	int sprite=0;
	const int Episode = g_pBehaviorEngine->getEpisode();
	if(Episode == 1) sprite = OBJ_RAY_DEFSPRITE_EP1;
	else if(Episode == 2) sprite = OBJ_RAY_DEFSPRITE_EP2;
	else if(Episode == 3) sprite = OBJ_RAY_DEFSPRITE_EP3;

	// Draw the shot
    mKeenGunSprite = g_pGfxEngine->getSprite(mId,sprite);
    headdstrect.w = headsrcrect.w = mKeenGunSprite.getWidth();
    headdstrect.h = headsrcrect.h = mKeenGunSprite.getHeight();
    headdstrect.x = 45-(headsrcrect.w/2);
    headdstrect.y = 19-(headsrcrect.h/2);

    SDL_Surface *keenGunSfc = mKeenGunSprite.getSDLSurface();

#if SDL_VERSION_ATLEAST(2, 0, 0)
    SDL_SetSurfaceBlendMode(keenGunSfc, SDL_BLENDMODE_NONE);
#else
    SDL_SetAlpha(keenGunSfc, 0, 0);
#endif


    SDL_BlitSurface( keenGunSfc, &headsrcrect, mpBackground.get(), &headdstrect );

	// Draw the rounded borders
    DrawCircle(0, 0, 76);
    DrawCircle(17-4, 15-2, 22);
    DrawCircle(58-4, 15-2, 22);
}

// Draw a circle on the surface
void CHUD::DrawCircle(int x, int y, int width)
{
	SDL_Rect text, outline;

	Uint8 r,g,b;
	CFont &Font = g_pGfxEngine->getFont(1);
	Font.getBGColour(&r, &g, &b);

	outline.x = x+4;
	outline.y = y;
	outline.w = width-8;
	outline.h = 12;
	SDL_FillRect(mpBackground.get(), &outline, SDL_MapRGBA(mpBackground->format, 0,0,0,255)); // Black
	outline.x = x+2;
	outline.y = y+1;
	outline.w = width-4;
	outline.h = 10;
	SDL_FillRect(mpBackground.get(), &outline, SDL_MapRGBA(mpBackground->format, 0,0,0,255)); // Black
	outline.x = x+1;
	outline.y = y+2;
	outline.w = width-2;
	outline.h = 8;
	SDL_FillRect(mpBackground.get(), &outline, SDL_MapRGBA(mpBackground->format, 0,0,0,255)); // Black
	outline.x = x;
	outline.y = y+4;
	outline.w = width;
	outline.h = 4;
	SDL_FillRect(mpBackground.get(), &outline, SDL_MapRGBA(mpBackground->format, 0,0,0,255)); // Black
	text.x = x+4;
	text.y = y+1;
	text.w = width-8;
	text.h = 10;
	SDL_FillRect(mpBackground.get(), &text, SDL_MapRGBA(mpBackground->format, r,g,b,255)); // Background colour
	text.x = x+2;
	text.y = y+2;
	text.w = width-4;
	text.h = 8;
	SDL_FillRect(mpBackground.get(), &text, SDL_MapRGBA(mpBackground->format, r,g,b,255)); // Background colour
	text.x = x+1;
	text.y = y+4;
	text.w = width-2;
	text.h = 4;
	SDL_FillRect(mpBackground.get(), &text, SDL_MapRGBA(mpBackground->format, r,g,b,255)); // Background colour
}

/**
 * \brief This part of the code will render the entire HUD. Galaxy Version
 */
void CHUD::renderGalaxy()
{
  // Compute the score that really will be seen
  int score, lives, charges;
  score = (m_oldScore<999999999) ? m_oldScore : 999999999;
  lives = (m_lives<99) ? m_lives : 99;
  charges = (m_oldCharges<99) ? m_oldCharges : 99;

  // Draw the HUD with all the digits
  SDL_Surface* blitsfc = mpHUDBlit.get();

  const int w = mHUDBox.getWidth();
  const int h = mHUDBox.getHeight();

  mHUDBox.drawSprite( blitsfc, -4, 0, w, h);

  if(lives >= 0)
  {
    g_pGfxEngine->drawDigits(getRightAlignedString(itoa(score),9), 4, 4, blitsfc );
    g_pGfxEngine->drawDigits(getRightAlignedString(itoa(charges),2),60, 20, blitsfc );
    g_pGfxEngine->drawDigits(getRightAlignedString(itoa(lives),2), 20, 20, blitsfc );

    if(g_pBehaviorEngine->mPlayers > 1 && mId == CCamera::getLead())
    {
        SDL_Rect rect;
        rect.x = 7; rect.y = 29;
        rect.h = 1; rect.w = 10;

        SDL_FillRect(blitsfc, &rect, 0xffff0000);
    }
  }


  SDL_BlitSurface( blitsfc, NULL, g_pVideoDriver->getBlitSurface(), &m_Rect );
}
/**
 * \brief This part of the code will render the entire HUD. Vorticon version
 */
void CHUD::renderVorticon()
{
	// Compute the score that really will be seen
	int score, lives, charges;
    score = (m_oldScore<99999999) ? m_oldScore : 99999999;
	lives = (m_lives<99) ? m_lives : 99;
	charges = (m_oldCharges<99) ? m_oldCharges : 99;

	// Draw the background
	SDL_BlitSurface(mpBackground.get(), NULL, mpHUDBlit.get(), NULL );


	CFont &Font = g_pGfxEngine->getFont(1);

	// Draw the lives
    Font.drawFont(mpHUDBlit.get(), getRightAlignedString(itoa(lives),2), 15, 15);

	// Draw the charges
    Font.drawFont(mpHUDBlit.get(), getRightAlignedString(itoa(charges),2), 56, 15);

    Font.drawFont(mpHUDBlit.get(), getRightAlignedString(itoa(score),8),8, 2);

    SDL_BlitSurface( mpHUDBlit.get(), NULL, g_pVideoDriver->getBlitSurface(), &m_Rect );   
}


void CHUD::render()
{
	size_t Episode = g_pBehaviorEngine->getEpisode();

	timer++;

	if(timer >= EFFECT_TIME)
	{
	    timer = 0;

	    if(m_oldCharges < m_charges)
		m_oldCharges++;
	    else if(m_oldCharges > m_charges)
		m_oldCharges--;
	}

	int delta = (m_score-m_oldScore)/EFFECT_SPEED;

	if(m_oldScore < m_score)
	{
	    if(delta == 0)
		delta = 1;
	    m_oldScore += delta;
	    if(m_oldScore > m_score)
		m_oldScore = m_score;
	}
	else if(m_oldScore > m_score)
	{
    	    if(delta == 0)
		delta = -1;

	    m_oldScore += delta;
    	    if(m_oldScore < m_score)
		m_oldScore = m_score;
	}

	if( Episode>=1 && Episode<=3 )
	  renderVorticon();
	else if( Episode>=4 && Episode<=6 )
      renderGalaxy();
}


void CHUD::sync()
{
    m_oldCharges = m_charges;
    m_oldScore = m_score;
}
