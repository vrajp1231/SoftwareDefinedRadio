#!/usr/bin/env bash
#
# init_sdr_net.sh
# Usage: ./init_sdr_net.sh -i <IP_ADDRESS> -p <HTTP_PORT>
#

# Default interface
INTERFACE="eth0"

# Default settings
IP_ADDRESS="192.168.0.232"
SUBNET_MASK="255.255.255.0"
HTTP_PORT="8080"

# Parse user input for settings
while getopts "i:p:" opt; do
    case $opt in
        i) IP_ADDRESS="$OPTARG" ;;
        p) HTTP_PORT="$OPTARG" ;;
        *) echo "Usage: $0 -i <IP_ADDRESS> (default = 192.168.0.232) -p <HTTP_PORT> (default = 8080)" >&2; exit 1 ;;
    esac
done

# Validate user inputs
if [ -z "$IP_ADDRESS" ] || [ -z "$HTTP_PORT" ]; then
    echo "Error: Both IP address and HTTP port must be specified."
    echo "Usage: $0 -i <IP_ADDRESS> (default = 192.168.0.232) -p <HTTP_PORT> (default = 8080)"
    exit 1
fi

ifconfig $INTERFACE up		# Start interface
ifconfig $INTERFACE $IP_ADDRESS netmask $SUBNET_MASK	# Assign IP address and subnet mask

# Display network configuration
echo "Network configuration for $INTERFACE:"
ifconfig $INTERFACE

# Setup HTTP server on specified port
echo "Starting HTTP server on port $HTTP_PORT..."
httpd -p $HTTP_PORT &

# Check if the HTTP server started successfully
if [ $? -ne 0 ]; then
    echo "Error: Failed to start HTTP server."
    exit 1
fi

# Display HTTP server status
echo "HTTP server is running on port $HTTP_PORT."

# Print success message
echo "Network initialization complete."