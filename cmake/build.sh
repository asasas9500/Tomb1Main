#!/usr/bin/env bash

cd

rm -f -r zlib
rm -f -r FFmpeg
rm -f -r SDL
rm -f -r /share

git clone --depth 1 https://github.com/madler/zlib.git
pushd zlib
export LD="\"$(cygpath -w "$(dirname "$(command -v cl)")/link")\""
export LOC=-MT
nmake.exe -e -f 'win32\Makefile.msc' \
    && make -t -f win32/Makefile.gcc \
        SHARED_MODE=1 \
        BINARY_PATH=/share/bin \
        INCLUDE_PATH=/share/include \
        LIBRARY_PATH=/share/lib \
        STATICLIB=zlib.lib \
        SHAREDLIB=zlib1.dll \
        IMPLIB=zdll.lib \
        install \
    && make -f win32/Makefile.gcc \
        SHARED_MODE=1 \
        BINARY_PATH=/share/bin \
        INCLUDE_PATH=/share/include \
        LIBRARY_PATH=/share/lib \
        STATICLIB=zlib.lib \
        SHAREDLIB=zlib1.dll \
        IMPLIB=zdll.lib \
        install
unset LD
unset LOC
popd

git clone --depth 1 --branch n4.4.1 https://git.ffmpeg.org/ffmpeg.git
pushd FFmpeg
sed -i.old '/#\s*include\s*<unistd\.h>/d' /share/include/zconf.h
export INCLUDE="$INCLUDE;$(cygpath -w /share/include)"
export LIB="$LIB;$(cygpath -w /share/lib)"
./configure \
        --toolchain=msvc \
        --arch=x86 \
        --prefix=/share \
        --enable-gpl \
        --enable-decoder=pcx \
        --enable-decoder=png \
        --enable-decoder=gif \
        --enable-decoder=mjpeg \
        --enable-decoder=mpeg4 \
        --enable-decoder=mdec \
        --enable-decoder=h264 \
        --enable-decoder=h264_qsv \
        --enable-decoder=libopenh264 \
        --enable-decoder=png \
        --enable-demuxer=mov \
        --enable-demuxer=avi \
        --enable-demuxer=h264 \
        --enable-demuxer=str \
        --enable-demuxer=image2 \
        --enable-zlib \
        --enable-static \
        --enable-small \
        --disable-debug \
        --disable-ffplay \
        --disable-ffprobe \
        --disable-doc \
        --disable-network \
        --disable-htmlpages \
        --disable-manpages \
        --disable-podpages \
        --disable-txtpages \
        --disable-asm \
        --extra-ldflags=zlib.lib \
    && make \
    && make install
mv /share/include/zconf.h.old /share/include/zconf.h
unset INCLUDE
unset LIB
popd

git clone --depth 1 https://github.com/libsdl-org/SDL.git
pushd SDL
export _CL_='-DHAVE_LIBC -MT'
msbuild.exe -nologo -p:Configuration=Release -p:Platform=Win32 -p:ConfigurationType=StaticLibrary -p:OutDir="$(cygpath -w /share/lib/)" 'VisualC\SDL\SDL.vcxproj' \
    && msbuild.exe -nologo -p:Configuration=Release -p:Platform=Win32 -p:ConfigurationType=StaticLibrary -p:OutDir="$(cygpath -w /share/lib/)" 'VisualC\SDLmain\SDLmain.vcxproj' \
    && cp -a include /share/include/SDL2
unset _CL_
popd
