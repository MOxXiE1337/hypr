#include <hypr/components/runtime_dump.h>

#include <hyprfile/runtime_dump_file.h>
#include <hyprutils/file.h>
#include <hyprutils/resource.h>

namespace hypr
{
    RuntimeDump::RuntimeDump(Loader* loader) : LoaderComponent(loader, "RuntimeDump")
    {
    }

    bool RuntimeDump::LoadRuntimeDumpFileFromMemory(const void* data, size_t size)
    {
        hyprutils::LogManager& logman = GetLogManager();

        hyprfile::RuntimeDumpFile runtime_dump_file{};
        if (!runtime_dump_file.Load(data, size))
        {
            logman.Error("failed to parse runtime dump file");
            return false;
        }

        auto load_module = [&](const hyprfile::RuntimeDumpFile::ModuleRecord& module)
            {
                std::shared_ptr<ModuleRecord> mod = std::make_shared<ModuleRecord>();
                mod->name = module.name;
                mod->imagebase = module.imagebase;
                mod->imagesize = module.imagesize;

                std::vector<hyprfile::RuntimeDumpFile::ProcRecord> procs;
                runtime_dump_file.GetProcRecords(module, procs);
                
                for (auto& hproc : procs)
                {
                    ProcRecord proc{
                        hproc.ordinal,
                        reinterpret_cast<uintptr_t>(GetProcAddress(LoadLibraryA(module.name), hproc.name)),
                        hproc.name
                    };

                    mod->procs.push_back(proc);
                    //gprocs_[hproc.address] = proc;
                }
                modules_.push_back(mod);
                
            };

        std::vector<hyprfile::RuntimeDumpFile::ModuleRecord> modules;
        runtime_dump_file.GetModuleRecords(modules);

        for (auto& module : modules)
        {
            load_module(module);
        }
        
        logman.Log("loaded {} modules", static_cast<int>(modules.size()));

        return true;
    }
   
    bool RuntimeDump::LoadRuntimeDumpFileFromFile(const std::string& path)
    {
        hyprutils::LogManager& logman = GetLogManager();

        size_t size = 0;
        std::shared_ptr<uint8_t[]> buffer = hyprutils::ReadFileToMemory(path, &size);

        if (!buffer || size == 0)
        {
            logman.Error("failed to read runtime dump file \"{}\"", path);
            return false;
        }

        return LoadRuntimeDumpFileFromMemory(buffer.get(), size);
    }

    bool RuntimeDump::LoadRuntimeDumpFileFromResource(uint32_t id, const std::string& type)
    {
        hyprutils::LogManager& logman = GetLogManager();

        size_t size = 0;
        std::shared_ptr<uint8_t[]> buffer = hyprutils::ReadResourceToMemory(id, type, &size);

        if (!buffer || size == 0)
        {
            logman.Error("failed to read runtime dump file from resource {}", id);
            return false;
        }

        return LoadRuntimeDumpFileFromMemory(buffer.get(), size);
    }
}

