**HOW TO BUILD NUORDS PROXY** [(?)](README.md)  
  
**macOS:**  
  
1. Xcode and dev. tools are required.  
  
2. Navigate into the "src/mac" directory.  
  
   ```  
   cd src/mac  
   ```  
  
3. Make the product and create a shipment: 
  
   ```  
   make ship  
   ```  
  
4. If you want to pack the shipment:  
  
   ```  
   make pack  
   ```  
  
5. Custom build configuration and options:  
  
   - BLD_DEST - Ship folder and pack name.  
   - BLD_OPTS - Additional compiler options.  
   - BLD_CONF - Build config (debug/release).  
  
   Backward compatibility (macOS 10.12+):  
   
   ```  
   make BLD_DEST="macos-10-12-plus" BLD_OPTS="-arch x86_64 -march=x86-64 -mmacosx-version-min=10.12" ship
   ```  
   
   Debug build:  
   
   ```  
   make BLD_CONF="debug" ship
   ``` 
   
   The same BLD_CONF and BLD_DEST should be specified for shipping and packing.  
  
**Linux:**  
  
1. GCC 4+ and dev. tools are required.  
  
2. Navigate into the "src/linux" directory.  
  
   ```  
   cd src/linux
   ```  
  
3. Make the product and create a shipment: 
  
   ```  
   make ship
   ```  
  
4. If you want to pack the shipment:  
  
   ```  
   make pack
   ```  
  
5. Custom build configuration and options:  
  
   - BLD_DEST - Ship folder and pack name.  
   - BLD_OPTS - Additional compiler options.  
   - BLD_CONF - Build config (debug/release).  
  
   x86-64 Architecture:  
  
   ```  
   make BLD_DEST="x86-64" BLD_OPTS="-march=x86-64" ship  
   ```  
  
   Debug build:  
   ```  
   make BLD_CONF="debug" ship  
   ```  
  
   The same BLD_CONF and BLD_DEST should be specified for shipping and packing.  
  
**Unix:**  
  
1. GCC 4+ and dev. tools are required.  
  
2. Navigate into the "src/unix" directory.  
  
   ```  
   cd src/unix
   ```  
  
3. Make the product and create a shipment: 
  
   ```  
   make ship  
   ```  
  
4. If you want to pack the shipment:  
  
   ```  
   make pack
   ```  
  
5. Custom build configuration and options:  
  
   - BLD_DEST - Shipping folder and package name.  
   - BLD_OPTS - Additional compiler options.  
   - BLD_CONF - Build config (debug/release).  
  
   x86-64 Architecture:  
  
   ```  
   make BLD_DEST="x86-64" BLD_OPTS="-march=x86-64" ship  
   ```  
  
   Debug build:  
  
   ```  
   make BLD_CONF="debug" ship  
   ```  
   
   The same BLD_CONF and BLD_DEST should be specified for shipping and packing.  
  
**Windows:**  
  
1. VisualStudio 11+ is required.  
  
2. Open "src/win/nrdproxy.sln" in MS VisualStudio.  
  
3. Select a build configuration and a platform.  
  
4. Build the solution.  
  
**TESTING:**  
  
1. Navigate into a shipping directory.  
  
2. Set executable permissions to "nrdproxyd":  
  
   ```  
   chmod +x ./nrdproxyd  
   ```  
  
3. Create "nrdproxyd.cfg" using "nrdproxyd.cfg.simple" or "nrdproxyd.cfg.sample".  
  
4. Adjust system firewall if necessary (default port: 3389(TCP)).  
  
5. Launch proxy in a verbose console mode:  
  
   ```  
   ./nrdproxyd -lc -lv -cf nrdproxyd.cfg  
   ```  
  
------------------------------  
[< NuoRDS Proxy](README.md)  
  