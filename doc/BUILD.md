HOW TO BUILD NUORDS PROXY SERVER

----------------------------------
Mac OS:

1. Xcode and dev. tools are required.
2. Go to "mac" directory, e.g "cd mac"
3. Perform "make ship" command

----------------------------------
Unix/Linux:

1. GCC 4+ and dev. tools are required.   
2. Go to "linux" directory, e.g "cd linux"
3. Perform "make ship" command

----------------------------------
Windows:

1. VisualStudio 11+ is required.
2. Open win/nrdproxy.sln in MS VisualStudio.
3. Select build configuration and platform.
4. Build Solution.

----------------------------------
TEST:

1. Navigate to shipping directory.

2. Check daemon file functionality:

   chmod +x ./nrdproxyd    
   
   ./nrdproxyd -h

3. Create nrdproxyd.cfg using nrdproxyd.cfg.simple or nrdproxyd.cfg.template.

4. Adjust system firewall if necessary (default ports: 3389(TCP)).

5. Launch proxy in verbose console mode:

   ./nrdproxyd -lc -lv -cf nrdproxyd.cfg
   
----------------------------------
