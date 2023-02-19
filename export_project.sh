#!/bin/bash



main()
{
    cleanup
    randomize_version
    export_project
    bundle
    copy
    echo "Export complete, and copied to remote."
}

cleanup()
{
    rm -rf public //HYDRA/Projects/testMultiplayer.7z
    mkdir public
}

export_project()
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




randomize_version()
{
    cat << EOF > ./testMultiplayer/versionLabel.gd
extends Label

func _ready():
	text = "$(openssl rand -hex 16)"
EOF
}





cd $(dirname $0)
main

