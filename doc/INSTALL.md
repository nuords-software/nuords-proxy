**HOW TO INSTALL NUORDS PROXY**  [(?)](../README.md)


**macOS Installation**

1. Unpack NuoRDS Proxy package into a directory.

2. Open Terminal app and run setup script: 

   sudo sh nrdproxy_mac_install

3. Create "nrdproxyd.cfg" using "nrdproxyd.cfg.simple" or "nrdproxyd.cfg.template".

4. Test your configuration with verbose output:

   /usr/local/bin/nrdproxyd -lc -lv -cf nrdproxyd.cfg 

5. Copy "nrdproxyd.cfg" to "/etc/nuords/" directory:

   sudo cp ./nrdproxyd.cfg /etc/nuords/nrdproxyd.cfg

6. Start NuoRDS Proxy daemon: 

   sudo nrdproxyctl start -w

7. Check log messages in /var/log/nrdproxyd.log


**macOS Troubleshooting**

1. Stop NuoRDS Proxy daemon daemon:

   sudo nrdproxyctl stop -w 

2. Launch proxy with verbose output: 
  
   /usr/local/bin/nrdproxyd -lc -lv
  
3. See additional options: 

   /usr/local/bin/nrdproxyd -h

4. To change daemon startup rules, edit plist file: 

   /Library/LaunchDaemons/com.nuords.nrdproxyd.plist
  
5. After solving the problem, start proxy daemon:

   sudo nrdproxyctl start -w 


**Other OS Installation**

*IMPORTANT*: Unfortunately we do not have installer scripts for other operating systems.

1. Unpack NuoRDS Proxy package into a directory.

4. Create "nrdproxyd.cfg" using "nrdproxyd.cfg.simple" or "nrdproxyd.cfg.template".

5. Test your configuration with verbose output:

   ./nrdproxyd -lc -lv -cf nrdproxyd.cfg 
   
6. Create the daemon/service config to launch "nrdproxyd" properly at system startup. 

See "nrdproxyd -h" for all options you can use to meet your system requirements.

------------------------------
[< NuoRDS Proxy](../README.md) 
