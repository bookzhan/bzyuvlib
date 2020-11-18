之前基于RenderScript写了一个YUV转RGBA的工程，地址：https://www.bzblog.online/wordpress/index.php/2020/01/19/yuvrenderscript/ 喜欢的可以去看看，比Android原生的ScriptIntrinsicYuvToRGB要强大很多，但是近期在使用的过程中发现RenderScript比Google的libyuv速度要慢很多，于是我又基于libyuv写了一个YUV转换的工程，同时支持Camera1，Camera2输出的YUV转换,以及对YUV镜像，旋转，从YUV输出灰度图，灰度图转成RGBA, 单通道平移，具体支持的功能如下：

1. yuv420pToRGBA/yuv420pToBGRA
2. preHandleYUV
3. yv12ToRGBA/yv12ToBGRA
4. nv21ToRGBA/nv21ToBGRA
5. cropNV21/cropYUV420
6. zoomYUV420
7. bitmapToYUV420
8. yuvToGrey
9. greyToRGBA
10. translationSingleChannel
11. 支持RGBA的旋转，镜像



#### 如何使用：

##### 1.先添加maven { url 'https://dl.bintray.com/bookzhan/bzlib' }，如下所示

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

##### 2.然后implementation 'com.luoye.bzlib:bzyuv:1.1.16'



如果帮到了你，请给一个star