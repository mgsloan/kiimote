# Install dependencies
sudo apt-get install git-core libbluetooth-dev liblo-dev freeglut3-dev libsdl1.2-dev

# Install wiiuse
git clone https://github.com/godbyk/wiiuse.git

cd wiiuse
mkdir build
cd build
cmake ../
make -j4
sudo make install

#compile wiiconnect:
cd drumkit
make wiiconnect

# If you start getting 
# CMake Error at cmake/cmake-2.9.0-modules/RequireOutOfSourceBuild.cmake:41 (message):
#  You must set a binary directory that is different from your source
#  directory...

# Then remove any CMakeCache.txt files, wipe build, recreate the build
# directory as described above.

Assuming there were no errors you should then be able to run ./wiiconnect.
As soon as the program launches it begins looking for available Wiimotes, so make sure the Wiimote is in discoverable mode (by pressing either the red sync button inside the battery compartment or the 1 and 2 buttons at the same time).  After a delay of a few seconds you should see that the Wiimote is connected and feel the Wiimote vibrate.  
You can now activate the accelerometers and begin sending data by presssing the + button.  
For a short time, Wekinator will recognize the input.  After ~10 seconds Chuck will stop reading and you will have to relaunch Chuck and wiiconnect to get it working again (don't worry, I am currently working to address this issue).
