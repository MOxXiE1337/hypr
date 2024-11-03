#pragma once
#include <hypr/loader.h>
#include <hyprutils/singleton.h>

class hack : public hypr::Loader, public hyprutils::Singleton<hack>
{
private:
public:
	hack() : Loader("myhack") {}

	bool PrevMap()
	{
		return true;
	}

	bool PrevInvoke()
	{
		return true;

	}

	bool Invoke()
	{
		return true;

	}
};
