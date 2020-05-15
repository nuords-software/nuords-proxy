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
  
4. Sign the produced DMG file with your Develeoper ID Certificate:  
  
   ```  
   codesign --all-architectures --verbose --force --options runtime --identifier APPLICATION_ID --sign "CERTIFICATE_ID" PATH_TO_DMG_FILE  
   ``` 
    
5. If you want to notarize DMG file, then submit it to Apple:  
  
   [How to submit application for notarization](https://developer.apple.com/documentation/xcode/notarizing_macos_software_before_distribution)  
  
6. Once the DMG is approved, staple the ticket:
     
   ```  
   xcrun stapler staple PATH_TO_DMG_FILE  
   ```  
  
------------------------------  
[< NuoRDS Proxy](README.md) 