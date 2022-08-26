#!/usr/bin/env python3
#coding: utf-8

import os
import shutil
import sys

import licant
import licant.install

licant.include("crow", "crow.g.py")

licant.execute("apps/crowker/make.py")
licant.execute("apps/ctrans/make.py")

target = "libcrow.so"
install_include_path = '/usr/local/include/crow'
install_directory_path = '/usr/lib/'
install_library_path = os.path.join(install_directory_path, target)
install_library_link = os.path.join(install_directory_path, 'libcrow.so')

modules = [
    "crow",
    "crow.crowker",
    "crow.udpgate",
    "crow.tcpgate",
    "crow.serial_gstuff",
]
if sys.platform != "win32":
    modules.extend(["crow.realtime_threads"])

licant.cxx_static_and_shared("libraries",
                             mdepends=modules,
                             static_lib="libcrow.a",
                             shared_lib="libcrow.so",
                             cxxstd="c++17",
                             optimize="-O2",
                             cxx_flags='-fPIC -Wall -pedantic -g -Weffc++ -Wextra',
                             cc_flags='-fPIC -Wall -pedantic -g -Wextra',
                             libs=["igris", "nos"],
                             )

licant.fileset("apps", targets=[
    "ctrans",
    "crowker",
], deps=["libcrow.so"])

licant.install.install_library(tgt="install_libraries",
                               libtgt=["libcrow.a", "libcrow.so"],
                               headers="crow",
                               hroot="crow",
                                   uninstall="uninstall")


@licant.routine(deps=["apps"])
def install_apps():
    licant.do(["install_crowker", "makefile"])
    licant.do(["install_ctrans", "makefile"])


@licant.routine(deps=["install_apps", "install_libraries"])
def install():
    pass


licant.cxx_application("runtests",
                       sources=[
                           "tests/*.cpp",
                           "tests/brocker/*.cpp"
                       ],
                       objects=["libcrow.a"],
                       cxxstd="gnu++17",
                       ccstd="c11",
                       cxx_flags="-fPIC -g -Werror=all -Werror=pedantic -Weffc++ -Wno-deprecated-declarations",
                       cc_flags="-fPIC -g -Werror=all -Werror=pedantic -Wno-deprecated-declarations",
                       include_paths=["tests", "."],
                       libs=["igris", "nos", "pthread"],
                       )

licant.fileset("all", targets=["apps", target, "runtests"])
licant.fileset("install", targets=["install_libraries", "install_apps"])

licant.ex("all")
