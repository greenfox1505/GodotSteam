#!/bin/bash



main()
{
    cleanup
    pullSDK
    #setup
}

setup()
{
    mkdir -p ../../bin
    cp sdk/redistributable_bin/win64/steam_api64.dll ../../bin/
}

cleanup()
{
    #cleanup
    rm -rf out.zip sdk
}

pullSDK()
{
    curl -o out.zip https://partner.steamgames.com/downloads/steamworks_sdk.zip
    unzip out.zip
}


cd $(dirname $0)

main
