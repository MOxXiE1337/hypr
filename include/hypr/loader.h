#pragma once
#include "components/runtime_dump.h"
#include "components/segment_mapper.h"


namespace hypr
{
	class Loader
	{
	private:
		hyprutils::LogManager    logman_;
		RuntimeDump   runtime_dump_;
		SegmentMapper segment_mapper_;
	public:
		Loader() = delete;
		Loader(const std::string& name);

		hyprutils::LogManager& GetLogManager() { return logman_; }
		RuntimeDump& GetRuntimeDump() { return runtime_dump_; }
		SegmentMapper& GetSegmentMapper() { return segment_mapper_; }

	public:
		virtual bool PrevMap() = 0;
		virtual bool PrevInvoke() = 0;
		virtual bool Invoke() = 0;
		virtual bool AfterInvoke() { return true; }

		void Load();
	};
}
