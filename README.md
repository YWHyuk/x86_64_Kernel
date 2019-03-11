# x86_64_Kernel
For study of the process of kernel
<How To Run kernel>
  * 개발 환경
  -> install the qemu(Don't use the window version)
  
  * How to build
  -> $ make all       
  
  * How to run the image file
  -> $ qemu-system-x86_64 -fda 디스크.이미지 -hda 외부장치.이미지 ( "디스크.이미지"에는 만들어진 디스크 이름을, 
                                                                  "외부장치.이미지"에는 마운트할 외부 저장 장치 이름)
 
  
