**HOW TO BUILD NUORDS PROXY** [(?)](../README.md)

**macOS:**

1. Xcode and dev. tools are required.

2. Navigate into the "src/mac" directory.  

   cd src/mac

3. Make the product with a shipping package

   make ship

**Linux:**

1. GCC 4+ and dev. tools are required.
   
2. Navigate into the "src/linux" directory.  

   cd src/linux

3. Make the product with a shipping package

   make ship

**Unix:**

1. GCC 4+ and dev. tools are required.

2. Navigate into the "src/unix" directory.  

   cd src/unix

3. Make the product with a shipping package

   make ship

**Windows:**

1. VisualStudio 11+ is required.

2. Open "src/win/nrdproxy.sln" in MS VisualStudio.

3. Select a build configuration and a platform.

4. Build the solution.

**TESTING:**

1. Navigate into a shipping directory.

2. Check the daemon executable functionality:

   chmod +x ./nrdproxyd    
   
   ./nrdproxyd -h

3. Create nrdproxyd.cfg using nrdproxyd.cfg.simple or nrdproxyd.cfg.template.

4. Adjust system firewall if necessary (default ports: 3389(TCP)).

5. Launch proxy in verbose console mode:

   ./nrdproxyd -lc -lv -cf nrdproxyd.cfg
   
------------------------------
[< NuoRDS Proxy](../README.md) 

