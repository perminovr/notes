#include "Event.h"

using namespace perri;



uint32_t Event::eventGUID = 0;



Event::Event()
	: eventID(Event::eventGUID)
{
	Event::eventGUID++;
}



uint32_t Event::GetID() const
{
	return this->eventID;
}



bool Event::operator==(const Event &event) const
{
	return this->eventID == event.GetID();
}



int EventHandler::Register(Event &event)
{
	if ( std::find(this->events.begin(), this->events.end(), &event) != this->events.end() )
		return -1;
	this->events.push_back(&event);
	return 0;
}



int EventHandler::Unregister(Event &event)
{
	if ( std::find(this->events.begin(), this->events.end(), &event) == this->events.end() )
		return -1;
	this->events.remove(&event);
	return 0;
}



int EventHandler::UnregisterAll()
{
	this->events.clear();
	return 0;
}




int EventHandler::Raise(Event &event)
{
	if ( std::find(this->events.begin(), this->events.end(), &event) == this->events.end() )
		return -1;
	this->raised_events.push_back(&event);
	this->cv.notify_all();
	return 0;
}



EventHandler::Result EventHandler::Wait(uint32_t timeout)
{
	std::mutex mtx;
	std::unique_lock <std::mutex> lck(mtx);
	auto pred = [this](){
		return this->AtLeastOne(this->events);
	};

	if (timeout) {
		return (this->cv.wait_for(lck, std::chrono::milliseconds(timeout), pred))?
				Result::EventUp : Result::Timeout;
	} else {
		this->cv.wait(lck, pred);
		return Result::EventUp;
	}
}



int EventHandler::ResetEvents()
{
	if (this->raised_events.size() == 0)
		return -1;
	this->raised_events.clear();
	return 0;
}



bool EventHandler::AtLeastOne(const std::list <Event> &events) const
{
	for (auto &ie : this->raised_events)
		for (auto &ee : events)
			if (*ie == ee)
				return true;
	return false;
}



bool EventHandler::AtLeastOne(const std::list <Event*> &events) const
{
	for (auto &ie : this->raised_events)
		for (auto &ee : events)
			if (*ie == *ee)
				return true;
	return false;
}



bool EventHandler::isRaised(const Event &event) const
{
	for (auto &ie : this->raised_events)
		if (*ie == event)
			return true;
	return false;
}



const std::list <Event*>& EventHandler::GetOccurredEvents() const
{
	return this->raised_events;
}


