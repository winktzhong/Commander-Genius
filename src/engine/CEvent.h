/*
 * CEvent.h
 *
 *  Created on: 03.08.2010
 *      Author: gerstrong
 *
 *  Intendet to represent an event.
 *  Doesn't do any handling.
 *  TODO: If more events come and this file gets long we might have to split it up.
 */

#ifndef CEVENT_H_
#define CEVENT_H_

#include <cstdint>
#include <string>
#include <list>
#include "common/direction.h"
#include "CVec.h"
#include "graphics/CBitmap.h"
#include <vector>
#include <memory>

class CSpriteObject;


// TODO: These Event Interfaces must go to another file. Those are Inlevel Galaxy Events

struct CEvent { virtual ~CEvent() {} };

struct EventEnterLevel : CEvent {
	uint16_t data;
	EventEnterLevel(const uint16_t d) : data(d) {}
};


struct InvokeFunctorEvent : CEvent
{
	virtual void operator()() = 0;
};

struct EventEndGamePlay : CEvent
{};

struct EventDieKeenPlayer  : CEvent
{
    const int playerID;
    const bool gameOver;
    const int levelObj;
    const std::string levelName;

    EventDieKeenPlayer(const int id, const bool go, const int lo, const std::string &ln) :
        playerID(id), gameOver(go),
        levelObj(lo), levelName(ln){}
};

struct EventPlayerTeleportFromLevel : CEvent
{};

struct EventExitLevel : CEvent {
	const uint16_t levelObject;
	const bool sucess;
	const bool teleport;
    const int who;
    EventExitLevel(const uint16_t l, const bool s, const bool t, const bool lWho) :
        levelObject(l), sucess(s), teleport(t), who(lWho) {}
};

struct EventExitLevelWithFoot : CEvent {
	const uint16_t levelObject;
    const int who;
    EventExitLevelWithFoot(const uint16_t l, const int lWho) : levelObject(l), who(lWho) {}
};


struct EventRestartLevel : CEvent {};


struct EventPlayerEndLevel : CEvent {
	const uint16_t levelObject;
	const bool sucess;
    const int who;
	EventPlayerEndLevel(const EventExitLevel &ev) :
                    levelObject(ev.levelObject), sucess(ev.sucess),
                    who(ev.who) {}
};

struct EventPlayerRideFoot : CEvent {
	const uint16_t levelObject;
	EventPlayerRideFoot(const EventExitLevelWithFoot ev) :
					levelObject(ev.levelObject) {}
};


struct EventPlayTrack : CEvent {
	const uint32_t track;
	EventPlayTrack(const uint16_t t) : track(t) {}
};


/**
 *	Event designated for a Bitmap Message like those when Keen talks
 *	to the council Members or Princess Lindsey
 */
struct EventSendDialog : CEvent
{
	const std::string Msg;
	EventSendDialog(const std::string& lMsg) : Msg(lMsg) {}
};

struct EventSendBitmapDialogMsg : EventSendDialog
{
	const CBitmap &BitmapRef;
	const direction_t Direction;
	EventSendBitmapDialogMsg(const CBitmap &lBitmapRef,
				 const std::string& lMsg,
				 const direction_t& lDirection = LEFT) :
				  EventSendDialog(lMsg),
				  BitmapRef(lBitmapRef),
				  Direction(lDirection)
				  {}
};


/**
 * This event collects a vector of EventSendBitmapDialogMsg event, so all
 * the message boxes are loading at once and display one by one holding the paused game
 */

struct EventSendBitmapDialogMessages : CEvent
{
	EventSendBitmapDialogMessages( std::vector< std::shared_ptr<EventSendBitmapDialogMsg> > lMsgs ) :
		msgs(lMsgs) {}

	std::vector< std::shared_ptr<EventSendBitmapDialogMsg> > msgs;
};




/**
 *  \description small structure which holds a matching selection text to an event.
 *  			 It is used
 */
struct TextEventMatchOption
{
	std::string text;
	std::shared_ptr<CEvent> event;
};

/**
 *	\description This event triggers a MessageBox where you can select multiple items
 *
 *	\param		Message This Text will be shown when the Box is triggered
 *	\param 		OptionStrings The Text to the option which can be selected
 *							  Depending on the size of the
 */
struct EventSendSelectionDialogMsg : CEvent {

	const std::string Message;
	std::list<TextEventMatchOption> Options;

	EventSendSelectionDialogMsg(const std::string& lMsg) :
								Message(lMsg){}

	void addOption(const std::string& ltext, CEvent *levent)
	{
		TextEventMatchOption NewOption;
		NewOption.text = ltext;
		NewOption.event.reset( levent );
		Options.push_back(NewOption);
	}
};

/**
 *	\description This event will spawn a new object
 *
 *	\param		pObject Pointer to the allocated memory of the Object. Caution: This allocation
 */
struct EventSpawnObject : CEvent {

	const CSpriteObject *pObject;

	EventSpawnObject( const CSpriteObject* pObject ) :
					  pObject( pObject ) {}
};

/**
 *	\description This event will erase all the enemies shots, etc, except the player
 */
struct EventEraseAllEnemies : CEvent {};



struct EventSpawnFoot : CEvent {

	const int x,y;
	const int foeID;

	EventSpawnFoot( const int lx, const int ly, const int lfoe ) :
					  x(lx), y(ly), foeID(lfoe) {}
};




#endif /* CEVENT_H_ */
