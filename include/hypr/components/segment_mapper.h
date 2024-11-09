#pragma once
#include "../hypr.h"
#include "../loader_component.h"

#include <hyprfile/segments_file.h>

namespace hypr
{
	enum class SegmentMapperMode
	{
		kUnknown,
		kStatic,
		kDynamic
	};

	class SegmentMapper : public LoaderComponent
	{
	public:
		struct Segment
		{
			uint32_t ordinal;
			uintptr_t address;
			size_t vsize;
			size_t rsize;
			std::shared_ptr<uint8_t[]> data;
		};

	private:
		SegmentMapperMode mode_;
		uintptr_t base_address_;
		std::vector<Segment> segments_;
		

	public:
		SegmentMapper() = delete;
		SegmentMapper(Loader* loader);

		uintptr_t GetBaseAddress() { return base_address_; }
		void SetBaseAddress(uintptr_t address) { base_address_ = address; }

		const std::vector<Segment>& GetSegments() { return segments_; }
		
		// manually load a segment, useful when u only need one segment ( in code like bin.h )
		bool LoadNativeSegment(const hyprfile::SegmentsFile::Segment& segment);

		// .hseg file
		bool LoadSegmentsFileFromMemory(const void* data, size_t size);
		bool LoadSegmentsFileFromFile(const std::string& path);
		bool LoadSegmentsFileFromResource(uint32_t id, const std::string& type);
	};
}

//namespace hypr
//{
//	constexpr size_t kSegmentMapperMaxTranslateCacheNumber = 10;
//
//	enum class SegmentMapperMode
//	{
//		kUnknown,
//		kStatic,
//		kDynamic
//	};
//
//	struct Segment
//	{
//		uint16_t ordinal;
//		segaddr_t segment_address;
//		uintptr_t mapped_address;
//		size_t    size;
//		const uint8_t* data;
//	};
//
//	class SegmentMapper : public LoaderComponent
//	{
//	private:
//		bool      loaded_;
//		segaddr_t imagebase_;
//		std::unordered_map<uint16_t, Segment> segments_;
//
//		std::list<uint16_t> translate_cache_; // max size == 2
//	public:
//		SegmentMapper() = delete;
//		SegmentMapper(Loader* loader);
//
//		segaddr_t GetImagebase() { return imagebase_; }
//		void SegImagebase(segaddr_t imagebase);
//
//		bool LoadNativeSegment(const Segment& segment);
//		bool LoadNativeSegments(const std::vector<Segment>& segments);
//
//		// .hseg file
//		bool LoadSegmentsFileFromMemory(const void* data, size_t size);
//		bool LoadSegmentsFileFromFile(const std::string& path);
//		bool LoadSegmentsFileFromResource(const std::string& name, const std::string& type);
//
//		uintptr_t TranslateAddress(segaddr_t address); // translate segment address to mapped address
//		uintptr_t TranslateOffset(ptrdiff_t offset); // equals TranslateAddress(GetImagebase() + offset)
//		void      PrintTranslateCache();
//	private:
//
//		friend Loader;
//
//		bool MapSegment(uint16_t ordinal);
//		bool MapSegments();
//	};
//}
