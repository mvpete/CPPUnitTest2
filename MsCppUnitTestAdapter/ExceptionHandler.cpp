#include "ExceptionHandler.h"
#include <algorithm>
#include <vector>

std::vector<ExceptionHandler*> handlers_;
HANDLE h_ = 0;

LONG WINAPI OnExceptionThunk(PEXCEPTION_POINTERS ex)
{
	for (auto h : handlers_)
	{
		h->OnException(ex);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}


void RegisterHandler(ExceptionHandler *h)
{
	if (!h)
		return;

	if (!h_)
	{
		h_ = AddVectoredExceptionHandler(1, &OnExceptionThunk);
	}
	handlers_.emplace_back(h);
}

void UnregisterHandler(ExceptionHandler *h)
{
	auto i = std::find_if(handlers_.begin(), handlers_.end(), [rhs=h](ExceptionHandler *h)
	{
		return rhs == h;
	});
	if (i != handlers_.end())
		handlers_.erase(i);
	if (handlers_.empty())
	{
		RemoveVectoredExceptionHandler(h_);
		h_ = 0;
	}
}