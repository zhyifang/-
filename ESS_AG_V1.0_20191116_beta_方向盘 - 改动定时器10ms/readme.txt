2018年10月18日14:41:29
1.可以通过按键，指示灯校准角度传感器，270度可以识别。通过485发送角度数据。

2019年3月11日14:03:26
1.校准方向
2.中位点设置
3.配对，无线通信
4.指示灯
5.

0514
forward :0 
backward: 1
设置电压 ok
设置中位点 ok
传感器校准 o'k 
配对 ok~	

0515
不需要回零 need reset去掉

0516
取消不需要回零


设置指令发送一次，转到NullType,按一次发一次，收到确认不直接退出，用户确认后才退出转油门信号
需要回0 才可以判断方向计算角度

0521
油门线性计算

0603
新增角度传感器调试

0604
可以读取角度，AGC值128
调整功耗

0625
磁铁距离调整到状态值为0X30 (MD|ML),AGC = 128,pwm输出打开可以避免方向盘抖动引起的角度变化

0703
电机故障细分显示：
LED = ERROR-7  快闪

0x1000  comm_fail 01   0x0d-7 = 6
0x01      uv_warm   09   0x10-7 = 9
0x02      ov             02   0x08-7 = 1
0x04      uv	03   0x0a-7 = 3
0x08      oc	04   0x09 - 7 = 2
0x40      ot_warm  05   0x0e - 7 = 7
0x10      stall          06  0x0b - 7 = 4
0x20      ot             07  0x0f - 7 = 8
0x80      hall          08  0x0c - 7 = 5

0731
校准点范围左右偏差10度，蜂鸣器告警，开机显示

0809
去掉电压设置，调整设置项顺序，存储驱动位置

0819
开机发送位置
关机发送关机
断信蜂鸣器不提示故障
断信保持驱动位置
LED1: 电机校准
LED2: 中位点设置
LED3: 故障清除
LED4: 左右舷设置
LED5: 无线配对
LED6: 方向盘校准


0823
关机保存电机位置

0827
关机切换电池供电

0829
配对成功蜂鸣器提醒

0905
方向盘校准改不判断固定角度

0907
电池电压检测,3.5V低压，3.7v撤销告警，低压告警无法控制

0926
测试问题修复
1.方向盘角度调整
2.异常数据过滤，防止角度跳变导致控制异常
3.方向盘在校准时第一个灯卡住不退出：
4.大负载下，极限位置，最左边最，右边5LED报错
5.上电边界位置角度读取为0，转动后角度正常:原来读取非正常位置数据返回改为最近位置角度
6.蜂鸣器告警进入设置界面不持续响
7.方向盘达到最左边，重新配对，驱动卡死报堵转，方向盘上电位置数据有问题？

1024
方向盘校准顺序调转
手动校准丝杆发送2
配对后重发开机指令传送电机位置
