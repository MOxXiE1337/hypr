#include <hypr/loader.h>

namespace hypr
{
	Loader::Loader(const std::string& name) : 
		name_(name),
		logman_(name),
		runtime_dump_(this),
		segment_mapper_(this)
	{

	}

	void Loader::Load()
	{

		static auto print_exiting_msg = [this]()
			{
				hyprutils::LogManager& logman = GetLogManager();
				logman.Error("failed to load {}, exiting...", GetName());
			};

		if (!PrevMap())
		{
			print_exiting_msg();
			return;
		}

		// map segments
		if (!GetSegmentMapper().MapSegments())
		{
			print_exiting_msg();
			return;
		}

		if (!PrevInvoke())
		{
			print_exiting_msg();
			return;
		}

		// rebuild iat and relocs

		// apply hooks, patches


		if (!Invoke())
		{
			print_exiting_msg();
			return;
		}

		if (!AfterInvoke())
		{
			print_exiting_msg();
			return;
		}
	}


}