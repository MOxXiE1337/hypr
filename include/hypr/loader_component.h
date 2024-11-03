#pragma once

#include "logman.h"

namespace hypr
{
	class Loader;

	class LoaderComponent
	{
	private:
		Loader* loader_;
		LogManager            logman_;
	public:
		LoaderComponent() = delete;
		LoaderComponent(Loader* loader, const std::string& name) : loader_(loader), logman_(name) { }

		Loader& GetLoader()
		{
			return *loader_;
		}

		LogManager& GetLogManager()
		{
			return logman_;
		}
	};
}
