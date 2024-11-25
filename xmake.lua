add_repositories("local-repo repo")

add_rules("mode.debug", "mode.release", "assets", "@bgfx-custom/shaders")
add_requires("libsdl", "bgfx-custom", "imgui", { configs = { sdl2_no_renderer = true } })

rule("assets")
	set_extensions(".ktx", ".ogg")

	local assets = "assets/"

	after_build(function (target)
		local build_dir = target:targetdir()
		os.mkdir(build_dir)
		os.cp(assets, build_dir)
	end)
rule_end()

target("test_bgfx")
    set_kind("binary")
    set_languages("cxx20")
    add_files("src/**.cpp")
    add_files("src/shaders/*.vert", { output_dir = "shaders", profiles = { spirv = "spirv" } })
    add_files("src/shaders/*.frag", { output_dir = "shaders", profiles = { spirv = "spirv" } })
    add_files("assets/*")
	add_packages("libsdl", "bgfx-custom", "imgui")

	if is_plat("windows") then
		add_rules("win.sdk.application")
	end

	if is_mode("debug") then
		add_defines("DEBUG")
		if is_plat("windows") then
			add_ldflags("/subsystem:console", { force = true })
		end
	end
target_end()