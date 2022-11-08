# **Delay Tolerant Network (DTN)**
#### Autonomous Observation Base using Delay Tolerant Networks for Unconnected Zones


**Team Members:** Christian Jimenez (jimenezc2019@fau.edu), Ricardo Morales Solorzano (rmoralessolo2016@fau.edu), Dayton Taveras (dtaveras2019@fau.edu), Cesar Montes (cmontes2019@fau.edu), Jamari Morris (jamarimorris2020@fau.edu)

## Project Summary 
The main goal of this project is to monitor wildlife precisely and without human alteration. Our plan is to implement a secure device that will be constantly recording/getting information from multiple sensors (temperature, rain drop, UV sensor, humidity, among others). This information gathered will be sent from the transmitter node (wildlife node) to the receiver node (main node) through some communication layers such as Wifi/Ethernet and radio communication (LoRa Devices). When the information is received by the main node, this will be the one in charge of processing and displaying  the information in a more organized and  simple way in a web interface.

## IMPORTANT LINKS

- To learn more about ION as a DTN implementation please refeer to this [link](https://www.nasa.gov/directorates/heo/scan/engineering/technology/disruption_tolerant_networking_software_options_ion)
  - The link providess access to instructional videos and powerpoints released by ***NASA*** on how ***ION*** works and how to create scenarios in the DTN Dev kit
- *Download* NASA [dev-kit](https://www.mitre.org/download-nasas-dtn-development-kit) (optional)
  - this will have a lot of extra material that will help to undertand ION Scenarios
- *NASA ION course materials* go to the following [link](https://sourceforge.net/p/ion-dtn/wiki/NASA_ION_Course/)


## Hardware Used
  - Raspberry pi Zero W
  - LoRa Device (to be specified)

## Sofware versions
  - **OS** - Parrot 5.18.0

  - **pyion** - 4.1.0

  - **ION** - 4.1.0

## Getting ION (Interplanetary Overlay Network) & pyion on linux

To download ION source original file please go  [Here](https://sourceforge.net/projects/ion-dtn/files/)


### Steps for ION
1. Download ION software
2. move ***ion-open-source-4.1.0.tar.gz*** to a desired folder
3. ```tar -xf ion-open-source-4.1.0.tar.gz```
4. ```cd ion-open-source-4.1.0```
5. ```autoheader```
6. ```aclocal```
7. ```autoconf```
8. ```automake```
9. ```./configure CFLAGS='-O0 -ggdb3' CPPFLAGS='-O0 -ggdb3' CXXFLAGS='-O0 -ggdb3'```
10. ```make```
11. ```sudo make install```
12. ```sudo ldconfig```


*Note:* For a personalized installation or problems installing please  reefer to the *README.txt* in the "ion-open-source-4.1.0" folder


### Steps for pyion instalation
for extra information please reefer to [pyion page](https://pyion.readthedocs.io/en/latest/)
1. ```sudo apt-get install autotools-dev automake python3-dev```
2. Create ION_HOME enviroment export ION_HOME=/"your-ion-folder-path"
3. Download the pyion version corresponding to the ION version
     - ```git clone --branch v4.1.0 https://github.com/msancheznet/pyion.git```

2. ```sudo -E python3 setup.py install```

### RLWRAP instalation
rlwrap is a 'readline wrapper', a small utility that uses the GNU Readline library to allow the editing of keyboard input for any command.
**how to install:**
``` sudo apt install rlwrap```

## Before usage :
  - **Change Host files information**
You will need to change the information of the host files or use the temprate file so with your network information to 
guarrantee that the nodes has the correct connection.


