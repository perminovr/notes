#ifndef OOP_DELEGATOR_H_
#define OOP_DELEGATOR_H_


namespace perri {
	
class IDeleg {		
public:
	// ***************************************************************************************
	// TODO add your methods here
	virtual bool Method(void) const = 0;
	// ***************************************************************************************
	virtual ~IDeleg() {}
};	



class Delegator {
private:	
	template <typename OBJECT> class _Deleg;
	
public:
	// ***************************************************************************************
	// TODO add your methods here
	
	bool Method(void) const
	{
		return (this->iDeleg != 0)? this->iDeleg->Method() : 0;
	}	
	
	// ***************************************************************************************
	// TODO implement your creators here
	
	template <typename OBJECT>
	static IDeleg* NewDeleg(OBJECT* obj, bool (OBJECT::*pMethod)(void))
	{
		return new _Deleg <OBJECT> (obj, pMethod);
	}
	
	// ***************************************************************************************
	
	Delegator(IDeleg *iDeleg = nullptr)
	{
		this->iDeleg = iDeleg;
	}
	
	virtual ~Delegator()
	{
		delete iDeleg;
	}
	
	Delegator& operator=(IDeleg* iDeleg)
	{
		this->iDeleg = iDeleg;
		return *this;
	}
		
private:
	IDeleg *iDeleg;
	
	// ***************************************************************************************
	// TODO add your methods here
	template <typename OBJECT>
	class _Deleg : public IDeleg {
		typedef bool (OBJECT::*pMethod)(void);
		
	public:			
		virtual bool Method(void) const
		{
			return (obj != 0)? (obj->*obj_Method)() : 0;
		}
		
		_Deleg(OBJECT *obj, pMethod obj_Method)
		{
			this->obj = obj;
			this->obj_Method = obj_Method;
		}

	private:
		OBJECT *obj;			
		pMethod obj_Method;
	};
	// ***************************************************************************************
};	

}

#endif /* OOP_DELEGATOR_H_ */
