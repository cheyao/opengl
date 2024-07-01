#!/bin/bash
DLLS=`wine ~/.wine/x64/dumpbin.exe /dependents $1 | grep '.*.dll'  | sed '/msvcrt.dll/d' | sed '/KERNEL32.dll/d' | sed 's/ //g' | sed 's/\r$//'`

for fn in $DLLS ; do
	cp /usr/x86_64-w64-mingw32/bin/${fn} .
done
cp /usr/x86_64-w64-mingw32/bin/libwinpthread-1.dll .
cp /usr/x86_64-w64-mingw32/bin/libssp-0.dll .
