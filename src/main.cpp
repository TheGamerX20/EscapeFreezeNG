#include <pch.h>

// Patches
#include <EscapeFreeze.h>

namespace Main
{
    // Config Options
    static REX::INI::Bool iEscapeFreezePatch{ "Patches"sv, "EnableEscapeFreezePatch"sv, true };

    F4SE_PLUGIN_PRELOAD(const F4SE::LoadInterface* a_f4se)
    {
        // Init
        F4SE::Init(a_f4se);
        REX::INFO("Escape Freeze NG Initializing...");

        // Get the Trampoline and Allocate
		auto& trampoline = REL::GetTrampoline();
		trampoline.create(128);

        // Load the Config
        const auto config = REX::INI::SettingStore::GetSingleton();
        config->Init("Data/F4SE/Plugins/EscapeFreezeNG.ini", "Data/F4SE/Plugins/EscapeFreezeNGCustom.ini");
        config->Load();

        // Install Escape Freeze
        if (iEscapeFreezePatch.GetValue() == true)
        {
            if (EscapeFreeze::Install())
            {
                REX::INFO("Escape Freeze NG Initialized!");
            }
        }

        // Finished
        return true;
    }
}
