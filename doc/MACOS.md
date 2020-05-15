APPLE SINGNATURE AND NOTARIZATION  

If you want to deploy NuoRDS Proxy on several macOS hosts, you may need to sign and notarize 'nrdproxyd' binary.  

1. Make the product shipment without packing it: 
  
   ```  
   make ship  
   ```  
  
   See more build options [here](BUILD.md).  
  
2. Sign the 'nrdproxyd' binary with your Develeoper ID Certificate:  
  
   ```  
   codesign --all-architectures --verbose --force --options runtime --identifier APPLICATION_ID --sign "CERTIFICATE_ID" PATH_TO_NRDPROXYD  
   ```  
  
3. Pack the product shipment:
  
   ```  
   make pack  
   ```  
  
4. Submit produced ZIP package to Apple for notarization.
  
   [How to submit application for notarization](https://developer.apple.com/documentation/xcode/notarizing_macos_software_before_distribution)
  
5. Once the package is approved, staple the ticket to unpacked 'nrdproxyd' binary.
     
   ```  
   xcrun stapler staple PATH_TO_NRDPROXYD  
   ```  

6. Re-Pack the product shipment with notarized binary:

   ```  
   make pack  
   ```  
   
------------------------------  
[< NuoRDS Proxy](README.md) 