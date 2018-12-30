#ifndef __EXCEPTION_HANDLER_H__
#define __EXCEPTION_HANDLER_H__

#include <Windows.h>

class ExceptionHandler
{
public:
	virtual ~ExceptionHandler() = default;
	virtual void OnException(PEXCEPTION_POINTERS ex) = 0;
};

void RegisterHandler(ExceptionHandler *h);
void UnregisterHandler(ExceptionHandler *h);


#endif // __EXCEPTION_HANDLER_H__