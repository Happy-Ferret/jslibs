export USERPROFILE=$PREV_USERPROFILE

[ "$BUILD" == "" ] && BUILD=opt
[ "$BUILD_METHOD" == "" ] && BUILD_METHOD=build

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


echo action: $BUILD_METHOD
echo config: $BUILD
echo -n opts: 
[ "$NOLIBS" != "" ] && echo -n " NOLIBS"
echo


startTime=$((`date +%s`))

date > $LOGFILE
echo "$BUILD_METHOD $BUILD" >> $LOGFILE

if [ "$NOLIBS" == "" ]; then

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
fi

if [ "$1" == "" ]; then

	for slnFile in $(ls $TOP/src/*/*.sln); do
		
		baseName=$(basename $slnFile)
		dirName=$(dirname $slnFile)
		fileName=${slnFile%.*}


		if [[ "$EXCLUDE" == *$baseName* ]]; then
		
			echo "    skip $baseName"
		else
		
			if [ "$NOLIBS" == "" ]; then
				fileToBuild=$baseName
			else
				fileToBuild=$fileName.vcproj
			fi

		
			echo "building $fileToBuild ..."
			
			(echo;echo;echo) >> $LOGFILE
			cd $dirName
			vcbuild.exe //M$NUMBER_OF_PROCESSORS //nohtmllog //nologo //useenv $VCBUILD_OPT $fileToBuild "$BUILD|WIN32" >> $LOGFILE 2>&1
			[ $? != 0 ] && echo ... failed.
		fi
	done
else

	slnFile=$TOP/src/$1/$1.sln
	echo building $slnFile ...	
	(echo;echo;echo) >> $LOGFILE
	cd $(dirname $slnFile)
	vcbuild.exe //M$NUMBER_OF_PROCESSORS //nohtmllog //nologo //useenv $VCBUILD_OPT $(basename $slnFile) "$BUILD|WIN32" >> $LOGFILE 2>&1
	[ $? != 0 ] && echo ... failed.

fi

endTime=$((`date +%s`))

echo Build done in $(($endTime-$startTime))s.
