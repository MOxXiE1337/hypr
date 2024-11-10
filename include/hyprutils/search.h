#pragma once
#include "hyprutils.h"

namespace hyprutils
{
	template <typename _MapType, typename _Key, typename _Type>
	class MapSearch
	{
	private:
		_MapType map_;
	public:
		void AddElement(_Key key, std::shared_ptr<_Type> obj)
		{
			map_.insert({ key, obj });
		}

		void RemoveElement(_Key key)
		{
			auto it = map_.find(key);
			if (it == map_.end())
				return;
			map_.erase(it);
		}

		auto Begin()
		{
			return map_.begin();
		}

		auto End()
		{
			return map_.end();
		}

		std::shared_ptr<_Type> Find(_Key key)
		{
			auto it = map_.find(key);
			if (it != map_.end())
				return it->second;
			return {};
		}
	};

	template <typename _Key, typename _Type>
	using BSearch = MapSearch<std::map<_Key, std::shared_ptr<_Type>>, _Key, _Type>; // binary search
	template <typename _Key, typename _Type>
	using HSearch = MapSearch<std::unordered_map<_Key, std::shared_ptr<_Type>>, _Key, _Type>; // hash search
}