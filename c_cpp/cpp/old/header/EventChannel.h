#ifndef CHANNEL_EVENTS_H_
#define CHANNEL_EVENTS_H_

#include <list>
#include <map>
#include <algorithm>


namespace perri {		
	
template <typename TDATA>
class ISubscriber {
public:
	virtual void Notify(TDATA data) = 0;
	virtual ~ISubscriber() {}
};



template <typename TDATA>
class EventChannel {		
public: 
	void Publish(unsigned long long int id, TDATA data) const {
		try {
			const std::list < ISubscriber <TDATA> * > &t = this->topics.at(id);
			for (auto &s : t) {
				s->Notify(data);
			}
		} catch (std::out_of_range &ex) {}
	}
	void Subscribe(unsigned long long int id, ISubscriber <TDATA> *subscriber) {
		std::list < ISubscriber <TDATA> * > &t = this->topics[id];
		if (std::find(t.begin(), t.end(), subscriber) == t.end())
			t.push_back(subscriber);
	}
	void Unsubscribe(unsigned long long int id, ISubscriber <TDATA> *subscriber) {
		try {
			std::list < ISubscriber <TDATA> * > &t = this->topics.at(id);
			if (std::find(t.begin(), t.end(), subscriber) != t.end())
				t.remove(subscriber);
		} catch (std::out_of_range &ex) {}
	}
	
private:	
	std::map <unsigned long long int, std::list < ISubscriber <TDATA> * > > topics;
};



/* Do not forget to define in your cpp:
template <typename TDATA>
unsigned long long int perri::IPublisher<TDATA>::g_id;
 * */
template <typename TDATA>
class IPublisher {
public:		
	IPublisher(EventChannel <TDATA> *channel) { 
		this->id = this->g_id++;
		this->channel = channel;
	}
	void Publish(TDATA data) const {
		this->channel->Publish(this->id, data);
	}
	unsigned long long int GetId() const { 
		return this->id; 
	}
	virtual ~IPublisher() {}

private:
	EventChannel <TDATA> *channel;
	unsigned long long int id;
	static unsigned long long int g_id;
};	

}

#endif /* CHANNEL_EVENTS_H_ */
