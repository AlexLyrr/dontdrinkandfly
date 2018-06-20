# dontdrinkandfly

Welcome to our code! :-)

## installation
For non ble operation there are no installation requirements.

For ble the following steps need to be followed:

- For ubuntu 16.04 the standard bluez package needs to be manually upgraded using the following commands:

  - sudo apt-get install libglib2.0-dev libdbus-1-dev libudev-dev libical-dev libreadline6 libreadline6-dev
  
  - wget http://www.kernel.org/pub/linux/bluetooth/bluez-5.50.tar.xz
  
  - tar xf bluez-5.50.tar.xz
  
  - cd bluez-5.30

  - ./configure

  - make
  
  - sudo make install
  
  - sudo apt-get install checkinstall
 
- For ubuntu 18.04 the standard bluez package is fine, make sure that it is installed by running:

  sudo apt-get install bluez
  
- Now make sure the correct version is installed by running 

  bluetoothctl --version
  
- The version should be >=5.42

- Now install gattlib by downloading and running the following .deb file:

  https://github.com/labapart/gattlib/releases/download/dev/gattlib_dbus_0.2-dev_x86_64.deb
  
- Install bluetooth dev libraries:

  sudo apt-get install libbluetooth-dev libreadline-dev 

## running
Running is very simple:
- Connect the board
- Optionally connect the joystick
- Run one of the following commands:
  - make computer
  
    This runs the board code + gui, no joystick/ble
  - make joystick
  
    This runs the board code + gui + joystick, no ble
  - make computer-ble
  
    This runs the board code + gui + ble, no joystick
  - make joystick-ble
  
    This runs the board code + gui + joystick + ble
