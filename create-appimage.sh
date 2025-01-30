#!/bin/bash

mkdir AppImage
cd AppImage
if [ ! -f linuxdeploy-x86_64.AppImage ]; then
  wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
  wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
  chmod +x linuxdeploy-x86_64.AppImage
  chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
fi
./linuxdeploy-x86_64.AppImage --appdir AppDir

chmod +x ../appimage-run.sh

cp ../appimage-run.sh AppDir/AppRunº
cp ../cmake-build-release/bin/syncopa AppDir/usr/bin/syncopa
#cp -r ../cmake-build-release/lib/* AppDir/usr/lib/
cp ../.syncopa.desktop AppDir/syncopa.desktop
cp ../src/icons/syncopa.png AppDir/.DirIcon
cp ../src/icons/syncopa.png AppDir/syncopa.png

export NO_STRIP=true

./linuxdeploy-x86_64.AppImage --appdir AppDir --library /usr/lib/libc.so.6 --plugin qt --output appimage