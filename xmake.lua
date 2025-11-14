-- set minimum xmake version
set_xmakever("2.8.2")

-- includes
includes("lib/commonlibf4")

-- set project
set_project("EscapeFreezeNG")
set_license("GPL-3.0")

-- project version
local version = "1.1.0"
local major, minor, patch = version:match("^(%d+)%.(%d+)%.(%d+)$")
set_version(version)

-- set defaults
set_languages("c++23")
set_warnings("allextra")

-- set policies
set_policy("package.requires_lock", true)

-- add rules
add_rules("mode.release", "mode.releasedbg", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")

-- add options
set_config("commonlib_ini", true)
set_config("commonlib_xbyak", true)

-- targets
target("EscapeFreezeNG")
    -- add dependencies to target
    add_deps("commonlibf4")

    -- add commonlibsse plugin
    add_rules("commonlibf4.plugin", {
        name = "EscapeFreezeNG",
        author = "TheGamerX20",
        description = "Escape Condition Locks in FO4 NG & AE!"
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")

    -- pass major version
    add_defines(
        "PLUGIN_VERSION_MAJOR=" .. major,
        "PLUGIN_VERSION_MINOR=" .. minor,
        "PLUGIN_VERSION_PATCH=" .. patch
    )
