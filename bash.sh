#use this command to make this bash file executable : 
#  chmod +x bash.sh
# then every time you want to make the project, type "./bash.sh" in your root directory
mkdir build
cd build
cmake -DBUILD_TTTCORE=PREBUILT ..
make
