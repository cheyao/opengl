#!/bin/bash
DLLS=`wine ~/.wine/x86/dumpbin.exe /dependents $1 | grep '.*.dll'  | sed '/msvcrt.dll/d' | sed '/KERNEL32.dll/d' | sed 's/ //g' | sed 's/\r$//'`

for fn in $DLLS ; do
	cp /usr/i686-w64-mingw32/bin/${fn} .
done
cp /usr/i686-w64-mingw32/bin/libwinpthread-1.dll .
cp /usr/i686-w64-mingw32/bin/libssp-0.dll .
