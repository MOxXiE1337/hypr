#include <hypr/loader.h>

namespace hypr
{
	Loader::Loader(const std::string& name) : 
		logman_(name),
		segment_mapper_(this)
	{

	}

	void Loader::Load()
	{
		// actually if there's anything wrong, the process just exit and the functions under won't return XD
		// but u can also use return false to terminate the loading process gently

		if (!PrevMap())
			return;

		// map segments
		if (!GetSegmentMapper().MapSegments())
			return;

		if (!PrevInvoke())
			return;

		// rebuild iat and relocs

		// apply hooks, patches


		if (!Invoke())
			return;

		if (!AfterInvoke())
			return;
	}


}