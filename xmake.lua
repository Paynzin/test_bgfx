add_rules("mode.debug", "mode.release")
add_rules("@bgfx/shaders")
add_requires("libsdl", "bgfx", "imgui", { configs = { sdl2_no_renderer = true } })

target("test_bgfx")
    set_kind("binary")
    set_languages("cxx20")
    add_files("src/**.cpp")
    add_files("src/shaders/*.vert", { output_dir = "shaders", profiles = { spirv = "spirv" } })
    add_files("src/shaders/*.frag", { output_dir = "shaders", profiles = { spirv = "spirv" } })
	add_rules("win.sdk.application")
	if is_mode("debug") then
		add_ldflags("/subsystem:console", { force = true })
	end
	add_packages("libsdl", "bgfx", "imgui")
target_end()