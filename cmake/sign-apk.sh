#!/bin/sh
RED='\033[0;31m'
NC='\033[0m'

if [ -z "$1" ]; then
	echo -e "${RED}Usage: $0 your-app.apk$NC"
	exit 1
fi

if ! type "zipalign" > /dev/null; then
	echo -e "${RED}Please install zipalign$NC"
	exit 1
fi

if ! type "apksigner" > /dev/null; then
	echo -e "${RED}Please install apksignerm$NC"
	exit 1
fi

DIR=`mktemp -d`
DN=`dirname "$1"`
BN=`basename "$1"`
OUT="$DN/repacked-$BN"
OUT_ALIGNED="$DN/aligned-$BN"
OUT_SIGNED="$DN/signed-$BN"
KEYFILE=~/Developer/cheyao-android-key.keystore

# Debug mode
set -x

# Repack without the META-INF in case it was already signed
# and flag resources.arsc as no-compress:
unzip -q "$1" -d "$DIR"
pushd .
cd $DIR

rm -rf "$DIR/META-INF"
zip -n "resources.arsc" -r ../repacked.$$.apk *

popd

mv "$DIR/../repacked.$$.apk" "$OUT"

# Align
rm -f "$OUT_ALIGNED"
zipalign -p -v 4 "$OUT" "$OUT_ALIGNED"

# Verify
zipalign -vc 4 "$OUT_ALIGNED"

# Sign
apksigner sign -verbose -ks "$KEYFILE" --out "$OUT_SIGNED" "$OUT_ALIGNED"

# Cleanup
rm -rf "$DIR"
rm -rf "$OUT_ALIGNED"
rm -rf "$OUT"

echo -e "\033[0;32m== Done: $OUT_SIGNED$NC"

