## 

当时在B站看到华为发的这个项目相关视频便入手了，由于看到issue里的部分问题，所以一直没有着手加装电池。直到上周看到固件0.3更新了，于是把键盘拆了，看了下里面的肋板之间的布局，用游标卡尺量了下，定了锂电的尺寸，又采购了一波便开始动手折腾了。

# **准备**

## **材料**

- 润和Neptune开发板

* CH9350 HID转UART板

+ 杜邦线若干

* TypeC公头带板、母头带板(没用上)

- 2000mah锂电（型号604060，尺寸肋板间槽限制）

## **指导文档**

具体可看[官方文档](https://gitee.com/openharmony-sig/vendor_oh_fun/blob/master/hihope_neptune-oh_hid/02_%E6%93%8D%E4%BD%9C%E6%8C%87%E5%AF%BC%E6%96%87%E6%A1%A3/02_%E6%93%8D%E4%BD%9C%E6%8C%87%E5%AF%BC%E6%96%87%E6%A1%A3_%E9%80%82%E9%85%8D%E8%BD%AF%E4%BB%B6V0.3%E7%89%88%E6%9C%AC.md)，不再赘述。


操作过程没难点，就是烙铁技术不行，整了好久。


## **锂电加装**

###     先上改造完的内部图,见图1，电工胶布挡住了具体走线。因为怕压到线材，所以我把肋板削了几个口子走线。

<div align="center"><img src="https://s3.bmp.ovh/imgs/2021/08/827ef534359d75e9.jpg" width = 500 height = 300 />

<h1 style="text-align:center">图1</h1>

</div>

### 立柱削掉一些，正好能卡住充放电板。


### 大体上就是锂电连接充放电板，充放电板供电（通过TypeC公头）给Neptune开发板，而Neptune开发板的四个引脚就直接如官方未加装锂电的示意图连接。具体看图2，图3。


<div align="center"><img src="https://gitee.com/openharmony-sig/vendor_oh_fun/raw/master/hihope_neptune-oh_hid/02_%E6%93%8D%E4%BD%9C%E6%8C%87%E5%AF%BC%E6%96%87%E6%A1%A3/meta/wps40.jpg" width = 500 height = 300 />
</div>


<center>图2</center>


<div align="center"><img src="https://gitee.com/openharmony-sig/vendor_oh_fun/raw/master/hihope_neptune-oh_hid/02_%E6%93%8D%E4%BD%9C%E6%8C%87%E5%AF%BC%E6%96%87%E6%A1%A3/meta/wps6.jpg" width = 500 height = 300 />
</div>


<center>图3</center>


## **使用感受**
### 先上总体感受：日用基本无太大毛病。偶发按一键延迟后出现多个字母现象，按下一个键后正常。
### 手机、平板使用alt+3、4两个配置位，电脑使用alt+2配置位。切换正常。alt+1 不可用，且alt+1无法进行连接，具体原因未知（我不懂，希望官方能指导下如果能在哪里看日志之类的就好了）
### 由于锂电充放电板在负载电流持续小于50mA时会关闭输出，所以第二天需要插一下充电口唤醒（有外接按键焊盘的地方，我没弄）。
### 还有就是键盘玩游戏（原神）会忽然没反应，只有鼠标又跳不出游戏，只能关机，具体原因不清楚（我不懂*2），改用手机玩了。以及偶尔能在控制面板蓝牙设备处看到键盘在已配对-已连接之间反复变动，此时键盘不可用，按键时CH9350可见蓝光闪烁一次，平时蓝光常亮。再者，FN键不可用，键盘大小写等指示灯熄灭，功能正常，原因未知（我不懂*3）。