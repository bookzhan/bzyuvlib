之前基于RenderScript写了一个YUV转RGBA的工程，地址：https://www.bzblog.online/wordpress/index.php/2020/01/19/yuvrenderscript/ 喜欢的可以去看看，比Android原生的ScriptIntrinsicYuvToRGB要强大很多，但是近期在使用的过程中发现RenderScript比Google的libyuv速度要慢很多，于是我又基于libyuv写了一个YUV转换的工程，同时支持Camera1，Camera2输出的YUV转换,以及对YUV镜像，旋转，支持的功能如下：

1. yuv420pToRGBA/yuv420pToBGRA
2. preHandleYUV420p
3. yv12ToRGBA/yv12ToBGRA
4. nv21ToRGBA/nv21ToBGRA
5. cropNV21/cropYUV420



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

##### 2.然后implementation 'com.luoye.bzlib:bzyuv:1.1.11'



如果帮到了你，请点亮你的小星星