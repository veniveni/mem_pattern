all:
	ndk-build
	adb push libs/arm64-v8a/test /data/local/tmp/test
	adb shell chmod +x /data/local/tmp/test
# 	adb shell /data/local/tmp/test -k