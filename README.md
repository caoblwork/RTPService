RTPService
==========

RTP h264和AAC等的传输程序


这是一个库程序，包含h264的rtp，aac的rtp，以及android上的rtp传输程序

接收端暂时在windows上使用，还没有包含更多的界面，解码使用ffmpeg，因时间有限，我会慢慢完善。


root 下面三个文件夹

 1 decode1 是mfc rtp接收程序
 
 2 H264T 是android 程序，发送rtp
 
 3 RootRtpService 是一个静态库，windows上rtp发送和组播程序，以及directshow
 
   的采集程序。
   
   
   后面会加：AAC rtp 发送程序， mfc 完整的接收播放代码，人脸检测代码，并且开始规范代码。
             android程序发送不全面，不针对所有android摄像头，此发送根据硬件自行修改，
             后面会修改适应所有厂家摄像头。
