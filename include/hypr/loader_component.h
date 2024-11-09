#pragma once
#include <hyprutils/logmanager.h>

namespace hypr
{
	class Loader;

	class LoaderComponent
	{
	private:
		Loader* loader_;
		hyprutils::LogManager            logman_;
	public:
		LoaderComponent() = delete;
		LoaderComponent(Loader* loader, const std::string& name) : loader_(loader), logman_(name) { }

		Loader& GetLoader()
		{
			return *loader_;
		}

		hyprutils::LogManager& GetLogManager()
		{
			return logman_;
		}
	};
}
