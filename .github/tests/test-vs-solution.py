#!/usr/bin/env python3
"""Check platform selection, relocation and real MSBuild routing of the IDE solution."""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

sys.stdout.reconfigure(encoding="utf-8")
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--generator", default="Visual Studio 18 2026")
parser.add_argument("--build", action="store_true", help="build with MSBuild on Windows")
args = parser.parse_args()
repo = Path(__file__).resolve().parents[2]
platforms = ("x64", "Win32", "ARM64", "ARM64EC")
namespace = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}


def run(command):
    print(subprocess.list2cmdline([str(x) for x in command]), flush=True)
    subprocess.run(command, check=True)


with tempfile.TemporaryDirectory(prefix="vs solution test ", dir=repo) as temporary:
    stage = Path(temporary)
    for static in (True, False):
        source = stage / ("static tree" if static else "dll tree")
        (source / "Source").mkdir(parents=True)
        (source / "cmake").mkdir()
        for name in ("VisualStudioSolution.cmake", "VisualStudioProject.vcxproj.in", "BuildVisualStudio.cmake"):
            shutil.copyfile(repo / "cmake" / name, source / "cmake" / name)
        (source / "Source" / "probe.h").write_text("int probe();\n", encoding="utf-8")
        (source / "Source" / "probe.cpp").write_text('''#include "probe.h"
int probe() { return PROBE_FEATURE; }
''', encoding="utf-8")
        (source / "main.cpp").write_text('''#include <cstdio>
#include "probe.h"
int main() { std::printf("feature=%d bits=%d\\n", probe(), int(sizeof(void*) * 8)); }
''', encoding="utf-8")
        (source / "CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.21)
project(FreeImageFixture LANGUAGES C CXX)
option(FREEIMAGE_STATIC "Static library" ON)
option(FREEIMAGE_VS_ALL_PLATFORMS "Generate IDE solution" OFF)
option(BUILD_PROBE_FEATURE "Test forwarded feature" OFF)
if(FREEIMAGE_STATIC)
    add_library(FreeImage STATIC Source/probe.cpp Source/probe.h)
else()
    add_library(FreeImage SHARED Source/probe.cpp Source/probe.h)
    set_target_properties(FreeImage PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()
include_directories("${PROJECT_SOURCE_DIR}/Source")
target_compile_definitions(FreeImage PRIVATE PROBE_FEATURE=$<BOOL:${BUILD_PROBE_FEATURE}>)
add_executable(DriverProbe main.cpp)
target_link_libraries(DriverProbe PRIVATE FreeImage)
if(FREEIMAGE_VS_ALL_PLATFORMS)
    include(cmake/VisualStudioSolution.cmake)
    freeimage_generate_visual_studio_solution("${PROJECT_BINARY_DIR}/visualstudio" "${VS_TEST_GENERATOR}")
endif()
''', encoding="utf-8")
        build = source / "build"
        host_generator = args.generator if os.name == "nt" else "Ninja"
        command = ["cmake", "-S", str(source), "-B", str(build), "-G", host_generator,
                   f"-DFREEIMAGE_STATIC={'ON' if static else 'OFF'}",
                   "-DFREEIMAGE_VS_ALL_PLATFORMS=ON", "-DBUILD_PROBE_FEATURE=ON",
                   "-DCMAKE_CONFIGURATION_TYPES:STRING=Debug;Release",
                   f"-DVS_TEST_GENERATOR={args.generator}"]
        if os.name == "nt":
            command += ["-A", "x64"]
        run(command)
        front = build / "visualstudio"
        solution = (front / "FreeImage.sln").read_text(encoding="utf-8")
        for selected in (*platforms, "All Platforms"):
            for configuration in ("Debug", "Release"):
                assert f"{configuration}|{selected} = {configuration}|{selected}" in solution
        for platform in platforms:
            path = front / f"FreeImage-{platform}.vcxproj"
            project = ET.parse(path).getroot()
            guid = project.find(".//m:ProjectGuid", namespace).text
            for selected in (*platforms, "All Platforms"):
                for configuration in ("Debug", "Release"):
                    active = f"{guid}.{configuration}|{selected}.ActiveCfg = {configuration}|{platform}"
                    enabled = f"{guid}.{configuration}|{selected}.Build.0 = {configuration}|{platform}"
                    assert active in solution
                    assert (enabled in solution) == (selected in (platform, "All Platforms"))
            output = project.find(".//m:NMakeOutput", namespace).text
            assert output.endswith("FreeImage.lib" if static else "FreeImage.dll")
            text = path.read_text(encoding="utf-8")
            assert str(source) not in text and source.as_posix() not in text
            assert not re.search(r"(?<![A-Za-z])[A-Za-z]:[\\/]", text)
            assert "-DFREEIMAGE_VS_PLATFORM:STRING=" + platform in text
        if args.build:
            assert os.name == "nt", "MSBuild validation requires Windows"
            run(["MSBuild.exe", str(front / "FreeImage.sln"), "/p:Configuration=Release", "/p:Platform=x64", "/m:1"])
        moved = stage / ("moved & shared static" if static else "moved & shared dll")
        source.rename(moved)
        front = moved / "build" / "visualstudio"
        # All source items and the module reference must resolve after moving.
        for path in front.glob("*.vcxproj"):
            project = ET.parse(path).getroot()
            relative = project.find(".//m:FreeImageSourceDir", namespace).text
            relative = relative.replace("$(MSBuildThisFileDirectory)", "").replace("\\", "/")
            actual_source = (front / relative).resolve()
            assert actual_source == moved.resolve()
            for element in project.findall(".//*[@Include]"):
                value = element.attrib["Include"]
                if value.startswith("$(FreeImageSourceDir)"):
                    item = value.replace("$(FreeImageSourceDir)", "").replace("\\", "/")
                    assert (actual_source / item).is_file(), item
        if args.build:
            run(["MSBuild.exe", str(front / "FreeImage.sln"), "/p:Configuration=Release", "/p:Platform=All Platforms", "/m:1"])
            for platform in platforms:
                directory = front / "artifacts" / platform / "Release"
                assert (directory / ("FreeImage.lib" if static else "FreeImage.dll")).is_file()
                assert (directory / "DriverProbe.exe").is_file()
            # The moved x64 build must have used a new cache, leaving the old
            # location-specific cache alone rather than editing or deleting it.
            assert len(list((front / "cache" / "x64").glob("*/CMakeCache.txt"))) == 2
            for platform, bits in (("x64", 64), ("Win32", 32)):
                executable = front / "artifacts" / platform / "Release" / "DriverProbe.exe"
                output = subprocess.check_output([str(executable)])
                assert f"feature=1 bits={bits}".encode() in output
            # Rebuild and clean use the selected architecture/configuration.
            project = front / "FreeImage-Win32.vcxproj"
            run(["MSBuild.exe", str(project), "/t:Rebuild", "/p:Configuration=Debug", "/p:Platform=Win32"])
            assert (front / "artifacts" / "Win32" / "Debug" / "DriverProbe.exe").is_file()
            run(["MSBuild.exe", str(project), "/t:Clean", "/p:Configuration=Debug", "/p:Platform=Win32"])
            assert not (front / "artifacts" / "Win32" / "Debug" / "DriverProbe.exe").exists()
            assert (front / "artifacts" / "x64" / "Release" / "DriverProbe.exe").is_file()
        print(f"{'Static' if static else 'DLL'}: platform mappings and relocated paths passed", flush=True)
