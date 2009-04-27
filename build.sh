[ -z "$BUILD" ] && BUILD=opt
[ -z "$BUILD_METHOD" ] && BUILD_METHOD=rebuild

BUILD=$(echo $BUILD | tr "[:upper:]" "[:lower:]")
BUILD_METHOD=$(echo $BUILD_METHOD | tr "[:upper:]" "[:lower:]")

TOP=$PWD

if [ "$BUILD_METHOD" == "rebuild" ]; then
	VCBUILD_OPT=//rebuild
else
	VCBUILD_OPT=
fi

export DEST_DIR=$TOP/Win32_$BUILD/

LOGFILE=$TOP/build_$BUILD.log

echo action:$BUILD_METHOD config:$BUILD

date > $LOGFILE
echo "$BUILD_METHOD $BUILD" >> $LOGFILE


echo building JavaScript engine ...
if [ "$BUILD_METHOD" == "rebuild" ]; then
	cd $TOP/libs/js && make clean >> $LOGFILE 2>&1
fi
cd $TOP/libs/js && make all copy >> $LOGFILE 2>&1
[[ $? != 0 ]] && echo ... failed. && exit


echo building NSPR ...
if [ "$BUILD_METHOD" == "rebuild" ]; then
	cd $TOP/libs/nspr && make clean >> $LOGFILE 2>&1
fi
cd $TOP/libs/nspr && make all copy >> $LOGFILE 2>&1
[[ $? != 0 ]] && echo ... failed. && exit


for slnFile in $(ls $TOP/src/*/*.sln); do

	echo building $slnFile ...	
	(echo;echo;echo) >> $LOGFILE
	cd $(dirname $slnFile)
	vcbuild.exe //nohtmllog //nologo //useenv $VCBUILD_OPT $(basename $slnFile) "$BUILD|WIN32" >> $LOGFILE 2>&1
	[ $? != 0 ] && echo ... failed.
done

echo Build done.
