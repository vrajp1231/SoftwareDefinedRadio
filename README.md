# Software Defined Radio (SDR) Using Linux
#### An SDR implementation in hardware using Vivado and controlled through software using embedded Linux. This project has been designed for the Diligent Zybo Z7 FPGA.
<br>

## Setup Instructions

### Step 1: Boot FPGA with Linux
- Ensure the onboard Programming Mode Select jumper is switched to SD.
- Connect the FPGA to a computer over UART using TeraTerm, PuTTY or similar software using the J12 port. (Baud Rate: 115200)
- Boot the FPGA with an SD card loaded with the embedded Linux image.

### Step 2: Configure Network Settings
- Using the UART serial connection previously set up, check the FPGA's IP address using the following:
  ```bash
  ifconfig
  ```
- You can configure the IP address to whatever you like using the following:
  ```bash
  ifconfig eth0 <ip_address> netmask 255.255.255.0
  ```

### Step 3: Load Web Server Files to FPGA
- Using a tool such as MobaXterm, connect to the FPGA.
- Transfer all files to the FPGA (Or simply transfer the Patel_Lab11.tgz file to the FPGA and extract it there).
- Ensure all CGI files are executable by using the following when in the `cgi-bin` folder:
  ```bash
  chmod +x *
  ```

### Step 4: Start Web Server
- Navigate to the location of the `init_network.sh` file and use the following to start the web server:
  ```bash
  sudo bash ./init_network.sh -i <ip_address> -p <port>
  ```
  - Default IP Address: 192.168.0.78
  - Default Port: 8080
- A successful execution of the command will output a confirmation message that the HTTP server is running on the user-defined port.
- NOTE: Initiating `init_network.sh` using an ip address and port that is different than what you set in Step 2 will cause the device to disconnect from MobaXterm.

### Step 5: Using the Web Interface
- Using a web browser, navigate to the IP address and port used when starting the web server
  ```bash
  http://<ip_address>:<port>
  ```
- You should be presented with an interface (shwon below) where you can input desired values to control the SDR, as well as configure UDP streaming.

![alt text](https://github.com/vrajp1231/SoftwareDefinedRadio/blob/main/images/Webportal.png "Web Portal")

- NOTE: Please make sure to run the "Initialize SDR box first before the Tune Radio box"
