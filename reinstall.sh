sudo apt-get install libx11-dev libxrandr-dev libasound2-dev git cmake make

cd `dirname $0`
if [ ! -d Urho3D ]
then
	cd ..
	if [ ! -d Urho3D ]
	then
  	git clone https://github.com/Urho3D/Urho3D
	fi
	cd `dirname $0`
	ln -s ../Urho3D
fi

cd Urho3D
git pull
./cmake_generic.sh . -URHO3D_ANGELSCRIPT=0 -URHO3D_NAVIGATION=0 -URHO3D_URHO2D=0 -URHO3D_SAMPLES=0 -URHO3D_TOOLS=0
make
cd `dirname $0`

git pull
qmake Quatter.pro
make
sudo make uninstall
sudo make install
sudo update-icon-caches ~/.local/share/icons/