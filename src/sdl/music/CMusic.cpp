/*
 * CMusic.cpp
 *
 *  Created on: 02.05.2009
 *      Author: gerstrong
 */

#include "sdl/sound/CSound.h"
#include "sdl/sound/Sampling.h"
#include "StringUtils.h"
#include "CMusic.h"
#include "CLogFile.h"
#include "FindFile.h"
#include "fileio/ResourceMgmt.h"
#include "fileio/compression/CHuffman.h"
#include "sdl/music/COGGPlayer.h"
#include "sdl/music/CIMFPlayer.h"
#include <fstream>
#include <string.h>


bool CMusic::loadTrack(const CExeFile& ExeFile, const int track)
{
	std::unique_ptr<CIMFPlayer> imfPlayer( new CIMFPlayer(g_pSound->getAudioSpec()) );
	imfPlayer->loadMusicTrack(ExeFile, track);

	if(!imfPlayer->open())
	{
	    return false;
	}

	mpPlayer = move(imfPlayer);

	return true;
}


bool CMusic::load(const CExeFile& ExeFile, const int level)
{

#if defined(OGG) || defined(TREMOR)
    std::unique_ptr<COGGPlayer> oggPlayer( new COGGPlayer(g_pSound->getAudioSpec()) );
    if(oggPlayer->loadMusicForLevel(ExeFile, level))
    {
        mpPlayer = move(oggPlayer);
        return true;
    }
#endif

    std::unique_ptr<CIMFPlayer> imfPlayer( new CIMFPlayer(g_pSound->getAudioSpec()) );
    imfPlayer->loadMusicForLevel(ExeFile, level);

    if(!imfPlayer->open())
    {
        return false;
    }

    mpPlayer = move(imfPlayer);

    return true;
}

bool CMusic::load(const std::string &musicfile)
{
	mpPlayer.reset();

	if(musicfile == "")
		return false;

	const SDL_AudioSpec &audioSpec = g_pSound->getAudioSpec();

	if(audioSpec.format != 0)
	{
		std::string extension = GetFileExtension(musicfile);

		stringlwr(extension);

		if( extension == "imf" )
		{
		    std::unique_ptr<CIMFPlayer> imfPlayer( new CIMFPlayer(audioSpec) );
		    if(!imfPlayer->loadMusicFromFile(musicfile))
		      return false;
		    mpPlayer = move(imfPlayer);
		}
		else if( extension == "ogg" )
		{
#if defined(OGG) || defined(TREMOR)
		    std::unique_ptr<COGGPlayer> oggPlayer( new COGGPlayer(musicfile, audioSpec) );
		    mpPlayer = move( oggPlayer );
#else
		    g_pLogFile->ftextOut("Music Manager: Neither OGG bor TREMOR-Support are enabled! Please use another build<br>");
		    return false;
#endif
		}

		if(!mpPlayer->open())
		{
		    mpPlayer.reset();
		    g_pLogFile->textOut(PURPLE,"Music Manager: File could not be opened: \"%s\". File is damaged or something is wrong with your soundcard!<br>", musicfile.c_str());
		    return false;
		}
		return true;

	}
	else
	{
		g_pLogFile->textOut(PURPLE,"Music Manager: I would like to open the music for you. But your Soundcard seems to be disabled!!<br>");
	}

	return false;
}

void CMusic::reload()
{
	if(!mpPlayer)
	{
		return;
	}

	mpPlayer->reload();
}

void CMusic::play()
{
	if(!mpPlayer)
	{
		return;
	}

	mpPlayer->play(true);
}

void CMusic::pause()
{
	if(!mpPlayer)
		return;

	mpPlayer->play(false);
}

void CMusic::stop()
{
	if(!mpPlayer)
		return;

	// wait until the last chunk has been played, and only shutdown then.
	while(m_busy);

	mpPlayer->close();
	mpPlayer.reset();
}

// length only refers to the part(buffer) that has to be played
void CMusic::readWaveform(Uint8* buffer, size_t length)
{
	m_busy = false;

	if( !mpPlayer )
		return;

	m_busy = true;

	mpPlayer->readBuffer(buffer, length);

	m_busy = false;
}

bool CMusic::LoadfromSonglist(const std::string &gamepath, const int &level)
{
    bool fileloaded = false;
    std::ifstream Tablefile;

    std::string musicpath = getResourceFilename("songlist.lst", gamepath, false, false);

    if(musicpath != "")
    	fileloaded = OpenGameFileR(Tablefile, musicpath);

    if(fileloaded)
    {
    	std::string str_buf;
    	std::string music_filename;
    	char c_buf[256];
    	int detected_level=-1;
    	size_t next_pos = 0;

    	while(!Tablefile.eof())
    	{
        	Tablefile.getline(c_buf, 256);

        	str_buf = c_buf;
        	next_pos = str_buf.find(' ')+1;
        	str_buf = str_buf.substr(next_pos);
        	next_pos = str_buf.find(' ');

        	// Get level number
        	detected_level = atoi(str_buf.substr(0, next_pos).c_str());

        	str_buf = str_buf.substr(next_pos);
        	next_pos = str_buf.find('"')+1;
        	str_buf = str_buf.substr(next_pos);
        	next_pos = str_buf.find('"');

        	// Get the music filename to be read
        	music_filename = str_buf.substr(0, next_pos);

    		if( detected_level == level )	// found the level! Load the song!
    		{
    			// Get the song filename and load it!
    			std::string filename = getResourceFilename( music_filename, gamepath, false, true);
    			if( load(filename) )
    				play();
    			Tablefile.close();
    			return true;
    		}
    	}
    	Tablefile.close();
    }
	return false;
}

bool CMusic::LoadfromMusicTable(const std::string &gamepath, const std::string &levelfilename)
{
    bool fileloaded = false;
    std::ifstream Tablefile;

    std::string musicpath = getResourceFilename(JoinPaths("music", "table.cfg"), gamepath, false, true);

    if(musicpath != "")
    	fileloaded = OpenGameFileR(Tablefile, musicpath);

    if(fileloaded)
    {
    	std::string str_buf;
    	char c_buf[256];

    	while(!Tablefile.eof())
    	{
        	Tablefile.get(c_buf, 256, ' ');
    		while(c_buf[0] == '\n') memmove (c_buf, c_buf+1, 254);
        	str_buf = c_buf;
    		if( str_buf == levelfilename )	// found the level! Load the song!
    		{
    			// Get the song filename and load it!
    			Tablefile.get(c_buf, 256);
    			str_buf = c_buf;
    			TrimSpaces(str_buf);
    			std::string filename = getResourceFilename(JoinPaths("music", str_buf), gamepath, false, true);
    			if( load(filename) )
    				play();
    			Tablefile.close();
    			return true;
    		}
    		Tablefile.get(c_buf, 256);
    		while(!Tablefile.get() == '\n'); // Skip the '\n' delimiters, so next name will be read.
    	}
    	Tablefile.close();
    }
	return false;
}

CMusic::~CMusic()
{
	stop();
}

