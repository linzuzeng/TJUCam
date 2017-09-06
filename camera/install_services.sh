sudo cp *.service /etc/systemd/system/
sudo cp ./mosquitto.conf /etc/mosquitto/mosquitto.conf 
sudo systemctl daemon-reload 
sudo systemctl disable *.service
sudo systemctl enable autowifi.service
sudo systemctl enable factorymode.service
sudo systemctl enable avahi-daemon
rm ~/.ssh/id_*
chmod 600 id_*
sudo cp id_* /root/.ssh/
