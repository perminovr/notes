#ifndef CHECK_DELEGATE_H
#define CHECK_DELEGATE_H


namespace perri {

/*! ----------------------------------------------------------------------------------------
 * @brief: Interface for _CheckDelegate & CheckDelegate
 * -----------------------------------------------------------------------------------------
 * */
class ICheckDelegate {
public:
	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	virtual ~ICheckDelegate() {}

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	virtual bool Check() const = 0;
};



/*! ----------------------------------------------------------------------------------------
 * @brief: Temporary class needed to hide content of the validation object
 * -----------------------------------------------------------------------------------------
 * */
template <class OBJECT>
class _CheckDelegate : public ICheckDelegate {
	typedef bool (OBJECT::*pCheckFunc)(void);

public:
	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	_CheckDelegate(OBJECT *obj, pCheckFunc obj_Check)
	{
		this->obj = obj;
		this->obj_Check = obj_Check;
	}

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	virtual bool Check() const
	{
		return (obj != 0)? (obj->*obj_Check)() : 0;
	}

private:
	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	OBJECT *obj;

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	pCheckFunc obj_Check;
};



/*! ----------------------------------------------------------------------------------------
 * @brief: Provides the ability to verify an event for validity without knowing
 * the content of the validation object
 * -----------------------------------------------------------------------------------------
 * */
class CheckDelegate {
public:
	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	CheckDelegate(ICheckDelegate *iChDeleg = 0)
	{
		this->iChDeleg = iChDeleg;
	}

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	bool Check() const
	{
		return (this->iChDeleg != 0)? this->iChDeleg->Check() : 0;
	}

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	CheckDelegate& operator=(ICheckDelegate* iChDeleg)
	{
		this->iChDeleg = iChDeleg;
		return *this;
	}

private:
	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	ICheckDelegate *iChDeleg;
};



/*! ----------------------------------------------------------------------------------------
 * @brief: Provides to define check method are independent of object content
 * -----------------------------------------------------------------------------------------
 * */
template <class OBJECT>
ICheckDelegate* NewCheckDelegate(OBJECT* obj, bool (OBJECT::*pCheckFunc)(void))
{
	return new _CheckDelegate<OBJECT> (obj, pCheckFunc);
}

}

#endif
