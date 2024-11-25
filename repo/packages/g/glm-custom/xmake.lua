package("glm-custom")
    set_homepage("https://glm.g-truc.net/")
    set_description("OpenGL Mathematics (GLM)")
    set_license("MIT")

    add_urls("https://github.com/Paynzin/glm.git")
    add_versions("latest", "1644632ca53fa5788f6916756861f6f86ccbe84c")

    add_configs("header_only", {description = "Use header only version.", default = false, type = "boolean"})
    add_configs("cxx_standard", {description = "Select c++ standard to build.", default = "14", type = "string", values = {"98", "11", "14", "17", "20"}})
    add_configs("modules", {description = "Build with C++20 modules support.", default = false, type = "boolean"})
    add_configs("simd", {description = "Select simd optimizations to use", default = "none", type = "string", values = {"none", "SSE2", "SSE3", "SSSE3", "SSE4_1", "SSE4_2", "AVX", "AVX2"}})

    on_load(function (package)
        if package:config("modules") then
            package:config_set("header_only", false)
            package:config_set("cxx_standard", "20")
        elseif package:config("header_only") then
            package:set("kind", "library", {headeronly = true})
        else
            package:add("deps", "cmake")
        end
    end)

    on_install(function (package)
        if not package:config("modules") then
            if package:config("header_only") then
                os.cp("glm", package:installdir("include"))
            else
                io.replace("CMakeLists.txt", "NOT GLM_DISABLE_AUTO_DETECTION", "FALSE")
                local configs = {"-DGLM_BUILD_TESTS=OFF"}
                table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
                table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
                table.insert(configs, "-DCMAKE_CXX_STANDARD=" .. package:config("cxx_standard"))
                if package:config("simd") ~= "none" then
            		table.insert(configs, "-DGLM_ENABLE_SIMD_" .. package:config("simd") .. "=ON")
                end
                import("package.tools.cmake").install(package, configs)
            end
        else
            io.writefile("xmake.lua", [[ 
                target("glm")
                    set_kind("$(kind)")
                    set_languages("c++20")
                    add_headerfiles("./(glm/**.hpp)")
                    add_headerfiles("./(glm/**.h)")
                    add_headerfiles("./(glm/**.inl)")
                    add_includedirs(".")
                    add_files("glm/**.cpp")
                    add_files("glm/**.cppm", {public = true})
            ]])
            import("package.tools.xmake").install(package)
        end
    end)

    on_test(function (package)
        local cxx_standard = "c++" .. package:config("cxx_standard")
        assert(package:check_cxxsnippets({test = [[
            #include <glm/glm.hpp>
            #include <glm/ext/matrix_clip_space.hpp>
            void test() {
                glm::mat4 proj = glm::perspective(glm::radians(45.f), 1.33f, 0.1f, 10.f);
            }
        ]]}, {configs = {languages = cxx_standard}}))
    end)
package_end()