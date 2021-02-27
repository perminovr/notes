#ifndef TON_H
#define TON_H


#include <chrono>


namespace perri {

/*! ----------------------------------------------------------------------------------------
 * @brief: Timer on Delay
 * - 4 objects: IN, PT, Q, ET :
 * 		- Q 	- is TRUE, PT milliseconds after IN had a rising edge
 * 		- IN 	- starts timer with rising edge, resets timer with falling edge
 * 		- PT 	- time to pass, before Q is set
 * 		- ET 	- elapsed time
 * -----------------------------------------------------------------------------------------
 */
class TON {
public:
	/*! ------------------------------------------------------------------------------------
	 * @brief: Sets internal vars
	 * -------------------------------------------------------------------------------------
	 * */
	TON();

	/*! ------------------------------------------------------------------------------------
	 * @brief: Sets PT (in ms) and starts timer with IN = true
	 * -------------------------------------------------------------------------------------
	 * */
	void start(long long int _PT, bool _IN = true);

	/*! ------------------------------------------------------------------------------------
	 * @brief: Stops timer with IN = false, Q = false
	 * -------------------------------------------------------------------------------------
	 * */
	void reset();

	/*! ------------------------------------------------------------------------------------
	 * @brief: Returns time since start in ms
	 * -------------------------------------------------------------------------------------
	 * */
	long long int since() const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Sets up and returns state of Q
	 * -------------------------------------------------------------------------------------
	 * */
	bool check();

private:
	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	mutable std::chrono::milliseconds _ET;
	
	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	std::chrono::milliseconds _PT, _stT;

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	bool _Q, _IN;
};

}

#endif
