**HOW TO INSTALL NUORDS PROXY**  [(?)](README.md)  
  
**macOS | Installation:**  
  
1. Mount NuoRDS Proxy DMG file.
   
   ``` 
   hdiutil attach PATH_TO_NUORDS_PROXY_DMG
   ``` 

2. Open Terminal and navigate into a mounted volume:  
  
   ```
   cd "/Volumes/NuoRDS Proxy"  
   ```
  
3. Run the installer script:  
  
   ```
   sudo sh nrdproxy_install  
   ```
  
4. Create "nrdproxyd.cfg" using "nrdproxyd.cfg.simple" or "nrdproxyd.cfg.sample". 
  
   Learn more about [NuoRDS Proxy configuration.](CONFIG.md) 
  
5. Test your configuration with verbose output:  
  
   ```
   /usr/local/bin/nrdproxyd -lc -lv -cf nrdproxyd.cfg  
   ```
  
5. Copy "nrdproxyd.cfg" to "/etc/nuords/" directory:  
  
   ```
   sudo cp ./nrdproxyd.cfg /etc/nuords/nrdproxyd.cfg  
   ```
  
6. Start NuoRDS Proxy daemon:  
  
   ```
   sudo nrdproxy_service start -w
   ```  
  
7. Check log messages in /var/log/nrdproxyd.log
  
**macOS | Uninstallation:**
  
1. Mount NuoRDS Proxy DMG file.
   
   ``` 
   hdiutil attach PATH_TO_NUORDS_PROXY_DMG
   ``` 

2. Open Terminal and navigate into a mounted volume:  
  
   ```
   cd "/Volumes/NuoRDS Proxy"  
   ```
  
3. Run the uninstaller script:  
  
   ```
   sudo sh nrdproxy_uninstall  
   ```
   
**macOS | Troubleshooting:**  
  
1. Stop NuoRDS Proxy daemon:  
  
   ```
   sudo nrdproxy_service stop -w
   ```  
  
2. Launch proxy with verbose output:  
  
   ```
   /usr/local/bin/nrdproxyd -lc -lv  
   ```
  
3. See additional options:  
  
   ```
   /usr/local/bin/nrdproxyd -h  
   ```
  
4. To change daemon startup rules, edit plist file:  
  
   ```
   /Library/LaunchDaemons/com.nuords.nrdproxyd.plist  
   ```
  
5. After solving the problem, start proxy daemon:  
  
   ```
   sudo nrdproxy_service start -w
   ```  
  
**Other OS | Installation:**  
  
*IMPORTANT*: Unfortunately we do not have installer scripts for other operating systems.  
  
1. Unpack NuoRDS Proxy ZIP package into a directory.  

2. Set executable permission to "nrdproxyd":   
  
   ```
   chmod +x ./nrdproxyd  
   ```
  
3. Create "nrdproxyd.cfg" using "nrdproxyd.cfg.simple" or "nrdproxyd.cfg.sample".  
   
   Learn more about [NuoRDS Proxy configuration.](CONFIG.md)
   
4. Test your configuration with verbose output:  
  
   ```
   ./nrdproxyd -lc -lv -cf nrdproxyd.cfg  
   ```
   
5. Create the daemon/service config to launch "nrdproxyd" properly at system startup.  
  
See "nrdproxyd -h" for all options you can use to meet your system requirements.  
  
------------------------------  
[< NuoRDS Proxy](README.md)    
  