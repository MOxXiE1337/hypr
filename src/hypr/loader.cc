#include <hypr/loader.h>

namespace hypr
{
	Loader::Loader(const std::string& name) : 
		logman_(name),
		runtime_dump_(this),
		segment_mapper_(this)
	{

	}

	void Loader::Load()
	{
		if (!PrevMap())
			return;

		// map segments
		//if (!GetSegmentMapper().MapSegments())
		//	return;

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