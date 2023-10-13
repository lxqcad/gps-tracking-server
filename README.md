# gps-tracking-server
This is a background server running on TCP port that continuously receives gps signals transmitted from smart card tracking devices.

Use Makefile to compile the C sources into binary. The "gps-cron.php" file must be loaded as a cron job running in the background 
to update the database of devices that are offline. The "gps-data-interface.php" file is called by the gps tracking server binary
to update the database of gps signals received from tracking devices.

