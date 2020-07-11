I previously wrote a YUV to RGBA project based on RenderScript, address: https://www.bzblog.online/wordpress/index.php/2020/01/19/yuvrenderscript/ If you like it, you can go here to find it, than Android native ScriptIntrinsicYuvToRGB is much more powerful, but recently found that RenderScript is much slower than Google ’s libyuv, so I wrote a YUV conversion project based on libyuv, and also supported the YUV conversion of Camera1 and Camera2 output, and the YUV mirror , Rotation, supported functions are as follows:

1. yuv420pToRGBA/yuv420pToBGRA
2. preHandleYUV420
3. yv12ToRGBA/yv12ToBGRA
4. nv21ToRGBA/nv21ToBGRA
5. cropNV21/cropYUV420
6. zoomYUV420
7. bitmapToYUV420



#### How to use:

##### 1.First add maven {url 'https://dl.bintray.com/bookzhan/bzlib'} as shown below

```
allprojects {
    repositories {
        jcenter()
        maven {
            url 'https://maven.google.com/'
            name 'Google'
        }
        maven { url 'https://dl.bintray.com/bookzhan/bzlib' }
    }
}
```

##### 2.implementation 'com.luoye.bzlib:bzyuv:1.1.12'



If it helps you, please give me a start



[中文文档（Chinese DOC）](https://github.com/bookzhan/bzyuvlib/blob/master/README_cn.md)

