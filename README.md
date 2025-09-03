# rps
Microcontroller Rock, Paper, Scissors Game

## Overview

This project uses an ESP32 based microcontroller with an integrated screen, push button, WiFi networking, and a gyroscope. The project allows two users to play the rock, paper, scissors game.

## Background

I have a collection of microcontrollers and sensors. I am working my way through the collection of equipment I have and looking at fun projects I can put together, and also expand my knowledge base. 

## What did I use?

Here’s a picture of the components I used for this project:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/5ba04133-7bd8-41b8-aded-257570c71e6f" />

The AtomS3 microcontroller has an integrated screen that also acts as a push button. This is not a touch screen. The microcontroller has a 200mAh battery attached to the base. Here’s a closer look at the battery:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/46eb71c9-b3e2-4960-a038-e9de18404cc8" />

The battery has an on/off switch and a battery level indicator that lights an LED next to 25%, 50%, 75% and 100% capacity. M5Stack also have a 190mAh stick battery:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/e96f29be-3b96-486f-aad7-676e165bdbf4" />

This is what the stick battery looks like when attached to the AtomS3:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/2b91773c-cd5f-4b5e-8418-cb8334478e36" />

This style of battery is much longer than the battery base. However, it still allows a USB-C for charging and uploading new Sketches, as well as access to the Grove Interface to attach a sensor. I decided to use the battery base for this project as I liked to form factor better, and the battery base has slightly more capacity.

## What does it look like?

The project starts by running a short demo:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/e5ba7eb7-7421-49f7-8f3c-24769ed601cb" />

To select the rock, paper or scissors object, the AtomS3 must be tilted to the left or right. The screen button is pressed to select an icon.

The next phase initializes the integrated WiFi:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/039a4e8c-b29b-4d71-b1c6-2c25fc126318" />

The red circle indicates that there is no WiFi connection yet. Once the WiFi interface has been initialized and a client and server selected, the orange circle will be displayed:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/dae8adad-b894-41a8-8c34-20530d76e87b" />

Once the other player has been found, one of the following icons will be displayed:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/14f78683-f9d3-49a5-96d5-979c3d45fabc" />

This is the rock icon. Here’s paper:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/028e86a2-b37e-48b8-ad90-d0ca61247e57" />

And scissors:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/65e0c684-9cfd-4610-a745-e5e96b3d8bd2" />

There is a player 1 (orange) and player 2 (green) icon at the bottom of the screen along with scores for the number of games and rounds:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/779334a6-ee9d-4c42-9d2e-9cb7ab5c819e" />

Player 1 is always the device that player is holding. Player 2 is always the remote player. The following icon is display when a user wins a game:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/a3142cf4-0542-428a-8c15-a23190ef7aa6" />

This is a traditional rock, paper, scissors game:

|Object 1  |Object 2  | Winner               |
|----------|----------|----------------------|
| Rock     | Scissors | Rock beats scissors  |
| Paper    | Rock     | Paper beats rock     |
| Scissors | Paper    | Scissors beats paper |

This is what happens when a game is lost:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/00e9a6e8-0c90-409d-bfda-6535a3d69f5f" />

The game, of course, can be a draw:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/eaff0b5d-ded2-4dfa-bfae-6f97f3670cc4" />

The number of games per round, and the number of rounds is configurable in the Sketch. When a player wins all the rounds, the following icon is displayed:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/b6460d6d-f953-4400-85ca-e53a99293093" />

This is supposed to be a crown. Second place is as follows:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/a33e7360-6aa5-4b84-8194-a19c8a610356" />

The game and round scores will then reset to 0, so the next series of rounds can be played.

The Sketch randomizes the order of the rock, paper, scissors objects as the start of each game. This is only 6 different combinations, but prevents opposing players counting the number of tilts it takes to view an object. If a fixed order was used, opposing players could have an advantage selecting a winning object.

## Technical Overview

The AtomS3 has a small form factor - 0.94 x 0.94 x 0.51 inches (24mm x 24mm x 13mm). The integrated screen has 128 x 128 pixels and is not a touch screen. However, the screen doubles as a button. The AtomS3 has a number of other integrated devices like WiFi, Bluetooth, an accelerometer and a gyroscope.

I knew I wanted to create a two player game, but part of the challenge with this project was the amount of screen real estate and the lack of the touch screen. For other projects, I have placed button icons on a touch screen to drive the user interface. I chose the rock, paper, scissors game as this would be easier to represent on the screen real estate provided, along with player scores.

I decided to use WiFi for the devices to communicate, and I could use the AtomS3 screen button to select an image. After running some tests, the Z axis reading from the gyroscope was used to determine if the device had been tilted past a certain point, which would cycle through the rock, paper, scissors images.

The project uses an Arduino Sketch to initialize the microcontroller display, WiFi and gyroscope. The Sketch creates all the images, as sprites, controls game play and updates scores.

Sprites are used for the majority of graphics in this project as they prevent a lot of flicker when the screen is updated.

The following diagram describes the initial WiFi configuration:

<img width="50%" height="50%" alt="image" src="https://github.com/user-attachments/assets/50c9befc-e9aa-43b5-bb1f-fe09a6c47f20" />

Both devices create WiFi access points with a common prefix, like RPS, and a unique identifier. The unique identifier is taken from the MAC address of the device. 

Each device will then scan available networks and filter out access points that start with the RPS prefix. As soon as each device finds 2 access points, the lexically lower access point name (RPS-ABC is lexically lower than RPS-DEF) is selected as the main server access point. The other device will delete its access point and connect to the server access point.

Once this process has completed, the devices can communicate with each other. The only data passed between the devices is the object selected by each player.

## What do I need?

You will need:

1. The project uses 2 AtomS3 microcontrollers from M5Stack.
       
2. I used a battery base, but the AtomS3 can be powered from a USB-C cable.
       
3. The application only uses the standard M5Stack and AtomS3 libraries. 
       
4. A PC with Windows, Linux, or a Mac to install the Arduino IDE which can be downloaded here https://www.arduino.cc/en/software/. 
       
5. A USB A to USB C cable to connect the PC or Mac to the AtomS3.
       
6. The git utility to access the GIT repository (git clone https://github.com/davygotgit/rps.git) or visit https://github.com/davygotgit/rps and download a ZIP file.
       
Information for the AtomS3 is here https://shop.m5stack.com/products/atoms3-dev-kit-w-0-85-inch-screen?srsltid=AfmBOopt5nD39U37aXiBoG0NVdpNemAwwlcTpk_d5b11qEsgkTIgplJW.

Additional documentation for the AtomS3 is here https://docs.m5stack.com/en/core/AtomS3. This link includes device schematics and links to code samples.

The battery base is here https://shop.m5stack.com/products/atomic-battery-base-200mah?srsltid=AfmBOoqrfJBjzaEaqW-dwLNe70HqHTS3-_pTmtk3QpxSdL-vrqNxkb89. 

The main M5Stack site is here https://m5stack.com/.

There is also an AtomS3R here https://shop.m5stack.com/products/atoms3r-dev-kit. The AtomS3R is a newer version the AtomS3 with more memory, better WiFi and an integrated magnetic sensor.

The alternate battery stick is here https://shop.m5stack.com/products/atom-tailbat?srsltid=AfmBOorXnXMIEabnpW8x2X9wLjKV-ahEB5827rfHIEUr6clkBZEVn_xu.

I used the example here https://github.com/m5stack/M5AtomS3/blob/main/examples/Basics/imu/imu.ino to learn how to get data from the AtomS3 gyroscope. I also used the examples from https://github.com/m5stack/M5-RoverC/blob/master/examples/RoverC_JoyC_Remote/Master/Master.ino and https://github.com/m5stack/M5-RoverC/blob/master/examples/RoverC_JoyC_Remote/Remote/Remote.ino to learn how to configure a unique access point and scan for other access points.

## How do I install and configure the tools?

Here are some instructions for downloading and installing GIT https://github.com/git-guides/install-git.

Here are some instructions on how to download and install the Arduino IDE https://docs.arduino.cc/software/ide-v2/tutorials/getting-started/ide-v2-downloading-and-installing/.

M5Stack have an excellent quick start guide here https://docs.m5stack.com/en/arduino/m5atomecho/program. I don’t recall having to install any driver on my version of Ubuntu 24.04.2 LTS. It’s possible this is already included in the kernel. On Linux you must add your account to the dialout group by running the following bash command:

	sudo usermod -a -G dialout <your_account>

For example, if your user account is fantasticfred:

	sudo usermod -a -G dialout  fantasticfred

You must log out your current session and log back in again for this change to become active.

For Linux systems, I would first see if your Arduino IDE can see the AtomS3 device before attempting to install any drivers.

You know you are connected to the AtomS3 if you see something similar to the following status (bottom right) in the Arduino IDE:

<img width="50%" height="100%" alt="image" src="https://github.com/user-attachments/assets/bed4f1a4-d29e-4de3-9180-32d9a878eb4d" />

## How do I build and install the application?

You need to download the code from the GIT repository. This can be done by visiting https://github.com/davygotgit/rps and downloading, and then extracting, a ZIP file or by running the following terminal command from bash, a Windows Command Prompt or any suitable GIT access tool:

	git clone https://github.com/davygotgit/rps.git

There are a couple of options to build the application for the first time. Option 1 is:

1. Start the Arduino IDE.
2. Create a new project using the File -> New Sketch menu option.
3. Save the project using the name rps by using the File -> Save menu option.
4. Open the src/rps.ino file, from repository, using another editor, and copy/paste the contents over the skeleton project.
       
Option 2 is:
       
1. Start the Arduino IDE.
2. Create a new project using the File -> New Sketch menu option.
3. Save the project using the name rps by using the File -> Save menu option.
4. Use the Sketch -> Show Sketch Folder menu option to get the location of the project (Sketch location). This will be similar to Home/Arduino/rps on Linux.
5. Close the IDE.
6. Copy the rps.ino file from the src subdirectory of the repository to the Sketch location.
7. Start the Arduino IDE and load the rps project.
       
Once you have the initial project saved, you can just load it from File -> Open Recent menu option.

With the Sketch loaded, connect the AtomS3 using the USB A to USB C cable,. Ensure the M5AtomS3 board is selected and the USB port shows a connected status. Press the Upload button on the toolbar. The Sketch will be compiled and transferred to the AtomS3. The application will start after the transfer completes.

## What if I don’t want the demo?

I am not entirely convinced the demo really shows how to use the device. But, it’s there just in case it might be useful. The rps.ino Sketch has a #define called RUN_DEMO. Setting the RUN_DEMO define to a value of 0 will disable the demo code and the Sketch will go straight into network configuration. You must recompile and reload the Sketch to the AtomS3 any time you change the RUN_DEMO setting.

## Do you have any tips for using M5Stack microcontrollers, sensors, or units?

The majority of M5Stack controllers have library on github. The AtomS3 library is here https://github.com/m5stack/M5AtomS3. The libraries usually have basic and advance samples to give you an idea of how to initialize and then work with the integrated devices like the screen, speaker, microphone, or IMU (accelerometer). The examples can also be found using File -> Examples from the Arduino IDE.

Over the past couple of years, M5Stack have been moving to a unified set of libraries and header files. You can find additional examples here https://github.com/m5stack/M5Unified.

All the examples will give you the confidence that a particular microcontroller and component e.g. IMU are working before you add more sensors and your own code. This will significantly cut down on your debug time as problems may arise from interaction between multiple sensors or your own code.

## Were there any challenges creating this project?

I ran into 4 challenges in this project.

The initial WiFi configuration has to be set to AP and Station mode to allow an access point to be created and other networks to be scanned. Once the main server has been selected, the device that acts as the client has to switch back to Station mode so it can connect to the server access point.

The WiFi scanning code uses an STL set to track the scanned SSIDs. The STL set is ordered and will not allow duplicates. This means the first SSID in the set can be used by both devices. However, the scan was not always picking up the access point the local device created, so this is now added manually to the set.

The Z axis reading on the gyroscope could trigger an update to a different object, as the user was pressing the screen button to select the actual object they wanted. I had to add a timer so that the gyroscope is only read every 1.5 seconds. This seems to give a balance between responsive image updates and the time it takes to press the screen button to select an object.

After adding new sprites for the first and second place icons, the Sketch started to behave erratically. Sometimes it would display player sprites, and the object sprites, and sometimes it would not. The only thing I added was 2 new sprites. It occurred to me that the sprites might be taking up a lot of memory. The main gaming area is 128 x 98 x 24-bit pixels which is a lot of memory. I overcame the problem by dropping the color depth to 8-bit and 1-bit for a number of sprites, which dramatically reduced the amount of memory the sprites used. I also converted 3 sprites, the network disconnected (red circle), waiting (orange circle) and connected (green circle), to code that just draws the shape. This reduced memory usage and the screen flicker is not too bad for these objects.
