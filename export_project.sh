#!/bin/bash



main()
{
    cleanup
    build
    bundle
    copy
}

cleanup()
{
    rm -rf public //HYDRA/Projects/testMultiplayer.7z
    mkdir public
}

build()
{
    godot --export-debug "Win" ../public/testMultiplayer.exe --path ./testMultiplayer/ --headless
}

bundle()
{
    pushd public
    7z.exe a testMultiplayer.7z testMultiplayer.exe testMultiplayer.pck ../../../bin/steam_api64.dll ../../../bin/steam_appid.txt
    popd
}

copy()
{
    cp ./public/testMultiplayer.7z //HYDRA/Projects
    rm public/testMultiplayer.exe public/testMultiplayer.pck
}

godot()
{
    ../../bin/godot.windows.editor.dev.x86_64.exe $@
}










cd $(dirname $0)
main

