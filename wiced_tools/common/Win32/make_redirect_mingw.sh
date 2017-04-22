SDK_ROOT=../../..
OUTPUT=$SDK_ROOT/make.exe

# Compile make.exe
gcc -o $OUTPUT make_redirect.c

# Copy to subproject directories
cp $OUTPUT $SDK_ROOT/BCM20729-B0

# Clean up
rm -f $OUTPUT
