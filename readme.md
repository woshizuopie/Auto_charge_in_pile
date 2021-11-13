# 自动回充项目开发记录
---
## 简述：
+ 自动回充分为**机器人端**和**充电桩端**，机器人主动靠近充电桩，完成握手识别等指令之后开始充电，机器人端充电主控板需要与充电桩接触之后才能上电，且上电后1s内需要向机器人核心板上报指令，通知机器人刹车。
+ 机器人端与充电桩端通过充电金属条进行单总线通讯，由于正式开启充电的时候电流较大，因此，在正式开始通电时不允许通讯！

---
## 配套板卡信息
### 机器人端：
+ 主控STM32L431KBU6TR
+ 主时钟
	选择MSI生成，**主频48MHz**
+ 配置USART2
	波特率115200，用于输出调试信息
+ 配置TIM6基本定时器
    为单总线通讯提供us定时，10us产生一次中断（10us延时不精准）

### 充电桩端：
+ 主控STM32F03C8T6
+ 主时钟
	使用HSI内部高速时钟，**主频36MHz**
+ 配置TIM4基本定时器
  为单总线通讯提供us定时，10us产生一次中断（**10us延时不精准，平均每10us延时会多1us**）
+ 等等

---
## 单总线通讯
+ 主机发送复位信号，发起握手，等待从机应答
![在这里插入图片描述](https://img-blog.csdnimg.cn/2020112510463234.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzMzMyMzE0,size_16,color_FFFFFF,t_70#pic_center)
+ 主机写一个bit时序
![在这里插入图片描述](https://img-blog.csdnimg.cn/20201119111614826.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzMzMyMzE0,size_16,color_FFFFFF,t_70#pic_center)
+ 主机读一个bit时序
![在这里插入图片描述](https://img-blog.csdnimg.cn/20201119111614804.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzMzMyMzE0,size_16,color_FFFFFF,t_70#pic_center)
+ **每次通讯长度为三个字节**
+  主机**读写格式**
第一个字节：方向 0x00写 0x01读
第二个字节：寄存器地址
第三个字节：数据字节

+ 从机每次读取数据等待时间最大10ms，因此**每次数据操作间隔要求10ms以上**
+ **有效数据不能使用0xFF**

从机寄存器

|寄存器名|地址| 备注|
|:-------:|:--:|:--------:|
|Who_Am_I |0x00| |
|Device_ID|0x01|默认为0x40（九尾）|
|CAN_Send_Status|0x02|0x00：关闭can发送数据；  0x01：握手成功，开始can通讯发送准备充电消息|
|Charge_Ready|0x03|0x00：从机未就绪； 0x01：从机就绪|
|Charge_Status|0x04|0x00：从机等待，未充电； 0x01：从机关闭通讯，1s后开始充电|
|Device_ID_Check|0x05|设备id校验位|

---
### 补充变更：
增加握手信号滤波处理，解决干扰信号导致机器人端产生错误应答，造成烧坏板卡通讯电路电阻的问题。
### 实现原理：
充电桩空闲时会反复发送握手信号。机器人端自动回充板接触上充电桩后会检测握手信号，检测到两个连续的握手信号之后在第三个握手信号处进行应答处理。
![在这里插入图片描述](https://img-blog.csdnimg.cn/f95727c400fc432295fdb1b7c9b1166c.png)
**第一次握手信号，不应答:**
+ 1.机器人接触上充电桩后上电，等待总线300ms内拉低；若超时则出错复位，否则继续。
+ 2.等待总线1ms内拉高；若出错则复位，否则继续。

**第二次握手信号，不应答：**

+ 3.等待总线40-70ms内拉低；若出错则复位，否则继续。
+ 	4.等待总线1ms内拉高；若出错则复位，否则继续。
+ 5.判断ADC电压是否小于13V；若出错则复位，否则继续。

**第三次握手信号，应答：**

+ 6.等待总线拉低，超时100ms；若出错则复位，否则继续。
+ 7.等待总线700us内拉高，若出错则复位，否则继续。
+ 8.200us后拉低总线100us应答总线。



## 通讯总体流程
### 主机（充电桩端）
0、主机发送复位信号握手，检测从机
1、主机读取设备ID
2、主机通知从机启动CAN通讯，增大电流提供can通讯功率
3、等待3s后关闭增大电流使能（不关闭增大电流功能无法进行通讯）
4、读从机准备就绪寄存器
5、写从机准备充电寄存器，通知从机1s后开始充电
6、打开MOS管开始充电
7、5s后开始低电流检测（从机启动起来需要5s。小花生机器为避免打火问题，将5s改为200ms）

### 从机（机器人端）
+ 如果准备充电寄存器位未置位，则不断应答主机通讯
+ 如果准备充电寄存器位置位，则拉高总线（充电时拉低总线会烧毁器件）、打开充电mos管、使能充电芯片、打开重新芯片、置位can通讯标志位
+ 如果can发送寄存器置位，则配置can（此时电流足够能够配置成功），重启ACS711电流采集模块，置位can通讯标志位，发起can通讯通知机器人刹车，设置can通讯1s倒计时
+ can通讯发送任务：如果can通讯标志位置位则发起can通讯，200ms发送一次
+ can通讯接收任务：检测can通讯接收标志位，如果接收标志位置位，则读取接收数据
	1、接收到“上位机通知下位机收到开始充电通知”则关闭can通讯
	2、接收到“上位机发送停止充电消息”则关闭can通讯，失能充电芯片，关闭充电mos管，从机充电准备位取消置位，从机准备就绪标志位取消置位

### 主机读取从机一个字节：
+ 主机连续对从机8个bit位的读操作即可完成对一个字节的读。

### 主机写从机一个字节：
+ 主机连续对从机8个bit位的写操作即可完成对一个字节的写。

### 主机写从机寄存器：
+ 主机先发送一个字节表示方向，我们称之为方向字节；方向字节为0x01表示本次为主机对从机的一次读寄存器操作，方向字节为0x00表示本次为主机对从机的一次写寄存器操作；主机发送完方向字节之后发送需要操作的对应的寄存器地址；之后根据方向字节，如果是主机读寄存器，则从机返回一个字节对应寄存器的值，如果是主机写寄存器，则主机继续发送一个字节需要写入寄存器的值

|第一个字节(方向字节)	| 第二个字节（操作的寄存器地址） |	第三个字节（具体数据） |
| -- | -- | -- |
|0x00：主机写从机寄存器  0x01：主机读从机寄存器	| ADDR：需要操作的寄存器地址 | 	Data：如果为写操作则Data为需要写入的数据字节；如果为读操作则Data为读取出的数据字节


----
## CAN通讯
+ CAN通讯协议
![在这里插入图片描述](https://img-blog.csdnimg.cn/20201119120459747.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQzMzMyMzE0,size_16,color_FFFFFF,t_70#pic_center)

+ CAN通讯过程：
详见[自动回充下位机流程](https://pudutech.yuque.com/docs/share/7c906d67-0fff-4cfd-a66c-e3a3ca9bd71a?#)

---
## 操作注意事项：
+ 每次充电之后需要等待30s才能重新怼上去充电（新版无此要求）
+ 注意使用CUBEMX重新配置工程时，can初始化配置的函数会被覆盖



## 优化方案

1、can时间要求缩短

+ 修改充电桩端
  + 调整读取设备ID错误时的延时
  + 调整通知从机启动can通讯时打开增大电流使能的延时

+ 修改 握手环节读REG_CHARGE_READY寄存器失败等待从机就绪倒计时 超时时间100ms

## 升级记录
### V1.0.2 老版本
  1. 优化握手时间
  2. 完善各种保护
  3. 优化灯光指示
### V1.0.3 老版本
  1. 优化握手成功之后，超时检测关断时间由10s优化至3s
  2. 增加充电电流过流保护限制（大于12A充电1s进入保护）
  3. 修改灯光指示
  4. 充电中过流保护（大于12A）故障不解除，红灯常亮，等待端口输出电压大于3V解除
  5. 充电中短路保护（大于25A，电流采样芯片过流输出保护），故障不解除，红灯常亮，等待端口输出电压大于3V解除
  6. 充电前电极片短路，红灯常亮保护，充电前判断端口电压连续低于3V时长100ms，进入保护状态，充电中直接进入
### PJ0006_PW3_AutoCharge_in_pile_GJ_V1.1.1
  + 优化代码结构，提升握手速度
  + 修复提测过程中出现的若干bug
  + 修复“主机插上充电器充电后，再插上从机端充电器充电，会出现主机与从机两边同时充电的现象，无法完全从主机端切到由从机端充电”，加电源适配器接入时等待充电桩断开处理
  + 充电桩握手成功之后必须马上打开充电桩前端mos管，（目的是互锁关闭充电器前端mos管），否则当充电器没有拔出时，充电桩放开充电，电流会给充电器内电容充电导致使用充电桩充电之后马上检测到有充电器接入
  + 调整过温保护温度恢复点
### PJ0006_PW3_AutoCharge_in_pile_GJ_V1.1.2
  + 增加从机ID预留
### PJ0006_PW3_AutoCharge_in_pile_GJ_V1.1.3
  + 修复握手过程中输出短路保护误触发bug
### PJ0006_PW3_AutoCharge_in_pile_GJ_V1.1.4
  + 增加保护之后复位从机操作，解决出现故障且故障解除之后不能立即充电问题
### PJ0006_PW3_AutoCharge_in_pile_GJ_V1.1.5
  + 修复握手失败之后大概率误报短路故障bug
 ### PJ0006_PW3_AutoCharge_in_pile_GJ_V1.1.6
  + 支持充电桩灯板，修改了R灯和G灯的IO引脚
 ### PJ0006_PW3_AutoCharge_in_pile_GJ_V1.2.0
  + 调整充电结束关断电流，修复过流bug
 ### PJ0006_PW3_AutoCharge_in_pile_GJ_V1.3.1
  + 关闭29V输出之后增加延时，保证电压跌落之后再打开通讯mos，防止烧坏mos或通讯电路
 ### PJ0006_PW3_AutoCharge_in_pile_GJ_V1.4.0
  + 握手通讯流程，针对ID为0x33的增加ID校验流程
  + 调整握手成功之后开启低电流检测时间和滤波等级