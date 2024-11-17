add_rules("mode.debug", "mode.release", "assets", "shaders")
add_requires("libsdl", "bgfx2", "imgui", { configs = { sdl2_no_renderer = true } })

rule("assets")
	set_extensions(".ktx", ".ogg")

	local assets = "assets/"

	after_build(function (target)
		local build_dir = target:targetdir()
		os.mkdir(build_dir)
		os.cp(assets, build_dir)
	end)
rule_end()

rule("shaders")
    set_extensions(".sc", ".vert", ".frag", ".comp")
    on_buildcmd_file(function (target, batchcmds, shaderfile, opt)
        import("lib.detect.find_program")
        import("core.base.option")

        batchcmds:show_progress(opt.progress, "${color.build.object}compiling.shaderc %s", shaderfile)

        -- get bgfx shaderc
        local shaderc = find_program("shadercRelease") or find_program("shadercDebug")
        assert(shaderc, "bgfx shaderc not found! please check your bgfx installation.")

        -- determine arguments for shaderc from fileconfig
        local fileconfig = target:fileconfig(shaderfile)

        local output_filename
        if fileconfig and fileconfig.output_name then
            output_filename = fileconfig.output_name
        else
            local filename = path.filename(shaderfile)
            output_filename = filename:match("^(.*)%.sc$") or filename
            if fileconfig and fileconfig.array_name then
                output_filename = output_filename .. ".h"
            else
                output_filename = output_filename .. ".bin"
            end
        end

        local output_dir
        if fileconfig and fileconfig.output_dir then
            output_dir = fileconfig.output_dir
        else
            output_dir = "shaders"
        end

        local vardef_filename
        if fileconfig and fileconfig.vardef then
            vardef_filename = fileconfig.vardef
        else
            vardef_filename = path.join(path.directory(shaderfile), "varying.def.sc")
        end

        -- determine platform-specific shaderc arguments
        local bgfx_platforms = {
            windows = "windows",
            macosx = "osx",
            linux = "linux"
        }
        local bgfx_types = {
            "vertex",
            "fragment",
            "compute"
        }
        local bgfx_default_profiles = {
            windows = {
                vertex = {dx11 = "s_5_0", glsl = "120"},
                fragment = {dx11 = "s_5_0", glsl = "120"},
                compute = {dx11 = "s_5_0", glsl = "430"},
            },
            macosx = {
                vertex = {metal = "metal", glsl = "120"},
                fragment = {metal = "metal", glsl = "120"},
                compute = {metal = "metal", glsl = "430"}
            },
            linux = {
                vertex = {glsl = "120", spirv = "spirv"},
                fragment = {glsl = "120", spirv = "spirv"},
                compute = {glsl = "430", spirv = "spirv"}
            }
        }

        local shader_type
        if fileconfig and fileconfig.type then
            if table.contains(bgfx_types, fileconfig.type) then
                shader_type = fileconfig.type
            else
                raise("unsupported shader type " .. fileconfig.type)
            end
        elseif shaderfile:match("^vs_.*%.sc$") or shaderfile:match("%.vert$") then
            shader_type = "vertex"
        elseif shaderfile:match("^fs_.*%.sc$") or shaderfile:match("%.frag$") then
            shader_type = "fragment"
        elseif shaderfile:match("^cs_.*%.sc$") or shaderfile:match("%.comp$") then
            shader_type = "compute"
        else
            raise("cannot determine shader type from file name " .. path.filename(shaderfile))
        end

        -- build command args
        local args = {
            "-f", shaderfile,
            "--type", shader_type,
            "--varyingdef", vardef_filename,
            "--platform", bgfx_platforms[target:plat()],
        }

        if fileconfig and fileconfig.array_name then
            table.insert(args, "--bin2c")
            if fileconfig.array_name ~= true then
                table.insert(args, fileconfig.array_name)
            end
        end

        -- print(target:pkg("bgfx"):installdir())
        for _, includedir in ipairs(target:get("includedirs")) do
            table.insert(args, "-i")
            table.insert(args, includedir)
        end

        local mtime = 0
        local shader_profiles
        if fileconfig and fileconfig.profiles then
            shader_profiles = fileconfig.profiles
        else
            shader_profiles = bgfx_default_profiles[target:plat()][shader_type]
        end
        for folder, profile in pairs(shader_profiles) do
            -- set output dir
            local outputdir = path.join(target:targetdir(), output_dir, folder)
            batchcmds:mkdir(outputdir)
            local binary = path.join(outputdir, output_filename)

            -- compiling
            local real_args = {}
            table.join2(real_args, args)
            table.insert(real_args, "-o")
            table.insert(real_args, binary)
            table.insert(real_args, "--profile")
            table.insert(real_args, profile)
            if option.get("verbose") then
                batchcmds:show(shaderc .. " " ..  os.args(real_args))
            end
            batchcmds:vrunv(shaderc, real_args)

            if (mtime == 0) then mtime = os.mtime(binary) end
        end

        -- add deps
        batchcmds:add_depfiles(shaderfile)
        batchcmds:set_depmtime(mtime)
    end)
rule_end()

package("bgfx2")
    set_homepage("https://bkaradzic.github.io/bgfx/")
    set_description("Cross-platform, graphics API agnostic, “Bring Your Own Engine/Framework” style rendering library")
    set_license("BSD-2-Clause")

    add_urls("https://github.com/bkaradzic/bgfx.git")
    add_versions("7816", "5ecddbf4d51e2dda2a56ae8cafef4810e3a45d87")
    add_versions("8203", "484a5f0c25b53584a6b7fce0702a6bb580072d81")
    add_versions("8674", "f42134876038027667ef7e47c9a612dca1051ef2")
    add_versions("8752", "e9c9d711d7cfb58f565c4a76af041087bb8cc7d7")

    add_resources("7816", "bx", "https://github.com/bkaradzic/bx.git", "51f25ba638b9cb35eb2ac078f842a4bed0746d56")
    add_resources("8203", "bx", "https://github.com/bkaradzic/bx.git", "b9501348c596b68e5e655a8308df5c55f61ecd80")
    add_resources("8674", "bx", "https://github.com/bkaradzic/bx.git", "67dfdf34f642a4a807b75eb600f82f4f04027963")
    add_resources("8752", "bx", "https://github.com/bkaradzic/bx.git", "40df90e0c319553274ec952c9c763136ce252292")
    add_resources("7816", "bimg", "https://github.com/bkaradzic/bimg.git", "8355d36befc90c1db82fca8e54f38bfb7eeb3530")
    add_resources("8203", "bimg", "https://github.com/bkaradzic/bimg.git", "663f724186e26caf46494e389ed82409106205fb")
    add_resources("8674", "bimg", "https://github.com/bkaradzic/bimg.git", "964a5b85483cdf59a30dc006e9bd8bbdde6cb2be")
    add_resources("8752", "bimg", "https://github.com/bkaradzic/bimg.git", "0d1c78e77982f18a9174620bfa5762ffcca9d38c")

    if is_plat("windows") then
        add_syslinks("user32", "gdi32", "psapi")
        add_includedirs("include", "include/compat/msvc")
        add_cxxflags("/Zc:__cplusplus")
    elseif is_plat("macosx") then
        add_frameworks("Metal", "QuartzCore", "Cocoa", "IOKit")
    elseif is_plat("iphoneos") then
        add_frameworks("OpenGLES", "CoreGraphics", "Metal", "QuartzCore", "UIKit")
    elseif is_plat("linux") then
        add_deps("libx11")
        add_syslinks("GL", "pthread", "dl")
    end

    add_deps("genie")

    on_load("windows", "macosx", "linux", "iphoneos", function (package)
        local suffix = package:debug() and "Debug" or "Release"
        for _, lib in ipairs({"bgfx", "bimg", "bx"}) do
            package:add("links", lib .. suffix)
        end
        package:add("defines", "BX_CONFIG_DEBUG=" .. (package:debug() and "1" or "0"))
    end)

    on_install("windows|native", "macosx", "linux", "iphoneos", function (package)
        local bxdir = package:resourcefile("bx")
        local bimgdir = package:resourcefile("bimg")
        local genie = is_host("windows") and "genie.exe" or "genie"
        local args = {}
        if package:is_plat("windows", "macosx", "linux") then
            args = {"--with-tools"}
        end
        if package:config("shared") then
            table.insert(args, "--with-shared-lib")
        end
        os.trycp(path.join("include", "bgfx"), package:installdir("include"))
        os.trycp(path.join(bxdir, "include", "*"), package:installdir("include"))
        os.trycp(path.join(bimgdir, "include", "*"), package:installdir("include"))

        local mode = package:debug() and "Debug" or "Release"
        if package:is_plat("windows") then
            import("package.tools.msbuild")
            import("core.tool.toolchain")

            local msvc = toolchain.load("msvc")
            table.insert(args, "vs" .. msvc:config("vs"))

            local envs = msbuild.buildenvs(package)
            envs.BX_DIR = bxdir
            envs.BIMG_DIR = bimgdir
            os.vrunv(genie, args, {envs = envs})

            local configs = {}
            table.insert(configs, "/p:Configuration=" .. mode)
            table.insert(configs, "/p:Platform=" .. (package:is_arch("x64") and "x64" or "Win32"))
            table.insert(configs, "bgfx.sln")
            os.cd(format(".build/projects/vs%s", msvc:config("vs")))
            msbuild.build(package, configs)

            os.trycp("../../win*_vs*/bin/*.lib|*example*", package:installdir("lib"))
            os.trycp("../../win*_vs*/bin/*.dll", package:installdir("lib"))
            os.trycp("../../win*_vs*/bin/*.lib", package:installdir("lib"))
            os.trycp("../../win*_vs*/bin/*.exe", package:installdir("bin"))
        else
            import("package.tools.make")

            local configs
            local target
            if package:is_plat("macosx") then
                target = (package:is_arch("x86_64") and "osx-x64" or "osx-arm64")
                table.insert(args, "--gcc=" .. target)
                configs = {"-C",
                           ".build/projects/gmake-" .. target,
                           "config=" .. mode:lower()}
            elseif package:is_plat("iphoneos") then
                target = "ios-arm64"
                table.insert(args, "--gcc=" .. target)
                configs = {"-C",
                           ".build/projects/gmake-" .. target,
                           "config=" .. mode:lower()}
            elseif package:is_plat("linux") then
                table.insert(args, "--gcc=linux-gcc")
                target = "linux" .. (package:is_arch("x86_64") and "64" or "32") .. "_gcc"
                configs = {"-C",
                           ".build/projects/gmake-linux-gcc",
                           "config=" .. mode:lower() .. (package:is_arch("x86_64") and "64" or "32")}
            end

            table.insert(args, "gmake")
            table.insert(args, "-j" .. os.cpuinfo("ncpu"))
            local envs = make.buildenvs(package)
            envs.BX_DIR = bxdir
            envs.BIMG_DIR = bimgdir
            os.vrunv(genie, args, {envs = envs})
            make.build(package, configs)

            if package:is_plat("macosx", "iphoneos") then
                os.trycp(".build/" .. target .. "/bin/*.a|*example*", package:installdir("lib"))
                os.trycp(".build/" .. target .. "/bin/*.dylib", package:installdir("lib"))
                os.trycp(".build/" .. target .. "/bin/*|*.*", package:installdir("bin"))
            elseif package:is_plat("linux") then
                os.trycp(".build/" .. target .. "/bin/*.a|*example*", package:installdir("lib"))
                os.trycp(".build/" .. target .. "/bin/*.so", package:installdir("lib"))
                os.trycp(".build/" .. target .. "/bin/*|*.*", package:installdir("bin"))
            end
        end
        package:addenv("PATH", "bin")
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            void test() {
                bgfx::init();
            }
        ]]}, {configs = {languages = "c++17"}, includes = "bgfx/bgfx.h"}))
    end)
package_end()

target("test_bgfx")
    set_kind("binary")
    set_languages("cxx20")
    add_files("src/**.cpp")
    add_files("src/shaders/*.vert", { output_dir = "shaders", profiles = { spirv = "spirv" } })
    add_files("src/shaders/*.frag", { output_dir = "shaders", profiles = { spirv = "spirv" } })
    add_files("assets/*")

	if is_plat("windows") then
		add_rules("win.sdk.application")
		add_cxxflags("/Zc:preprocessor")
	end

	if is_mode("debug") then
		add_defines("DEBUG")
		if is_plat("windows") then
			add_ldflags("/subsystem:console", { force = true })
		end
	end

	add_packages("libsdl", "bgfx2", "imgui")
target_end()