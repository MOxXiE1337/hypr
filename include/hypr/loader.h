#pragma once
#include "components/segment_mapper.h"

namespace hypr
{
	class Loader
	{
	private:
		LogManager    logman_;
		SegmentMapper segment_mapper_;
	public:
		Loader() = delete;
		Loader(const std::string& name);

		LogManager& GetLogManager() { return logman_; }
		SegmentMapper& GetSegmentMapper() { return segment_mapper_; }

	public:
		virtual bool PrevMap() = 0;
		virtual bool PrevInvoke() = 0;
		virtual bool Invoke() = 0;
		virtual bool AfterInvoke() { return true; }

		void Load();
	};
}
