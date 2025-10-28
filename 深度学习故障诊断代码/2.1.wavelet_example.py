# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
# 每种故障取一个样本出来画图举例
import numpy as np
import pywt
import os
from scipy.io import loadmat
# In[]
lis=os.listdir('./data/')#列出指定目录下的所有文件和子目录,lis 变量将保存一个包含所有文件和子目录名称的列表
Len=1024#后续处理中需要截取的信号长度
data=[]#初始化一个空列表 data，用于存储后续处理后的数据
for n,i in enumerate(lis):
    path='./data/'+i
    print('第',n,'类的数据是',path,'这个文件')
    file=loadmat(path)#用 loadmat 函数加载当前路径下的 MATLAB 文件,file 变量将保存一个字典，其中包含了 MATLAB 文件中的所有变量
    file_keys = file.keys()#获取 file 字典中的所有键，即 MATLAB 文件中定义的变量名称
    for key in file_keys:
        if isinstance(key, str) and 'N09_M07_F10_' in key:  # 只选择相应的键，确保是字符串
            # 提取第一个找到的文件内容
            files = file[key][0, 0]['Y'][0][6][2].ravel()
            break  # 取到后退出循环
    start=np.random.randint(0,len(files)-Len)#生成一个随机整数作为起始索引，范围是从 0 到 len(files) - Len
    end=start+Len#计算结束索引，确保截取的信号段长度为 Len
    data.append(files[start:end])#将截取的信号段添加到 data 列表中
# In[] 参数设置
for i in range(4):#处理10个不同的信号数据，对应10种不同的故障类型
    sampling_rate=48000# 采样频率
    wavename = 'cmor3-3'#cmor是复Morlet小波，其中3－3表示Fb－Fc，Fb是带宽参数，Fc是小波中心频率。
    totalscal = 256#设置小波变换中的尺度数为256，尺度数决定了频谱的分辨率，256个尺度提供较高的频谱分辨率
    fc = pywt.central_frequency(wavename)# 小波的中心频率，中心频率是小波函数的主要频率成分，用于后续的尺度计算

    cparam = 2 * fc * totalscal#计算参数 cparam，用于生成尺度序列
    scales = cparam / np.arange(totalscal, 1, -1)#将 cparam除以从 totalscal 到1的递减序列，得到尺度序列，用于小波变换
    [cwtmatr, frequencies] = pywt.cwt(data[i], scales, wavename, 1.0 / sampling_rate)
    #当前信号 data[i] 进行连续小波变换，生成小波变换系数矩阵 cwtmatr 和对应的频率数组 frequencies
    t=np.arange(len(data[i]))/sampling_rate#生成时间轴 t，将信号的样本索引转换为时间值，单位为秒
    plt.figure(figsize=(12,4))#创建一个新的图形，设置大小为12英寸宽，4英寸高，为绘制信号和时频图准备图形空间
    plt.subplot(1,2,1)#创建一个1行2列的子图，选择第一个子图，为绘制时间序列图准备子图
    plt.plot(t, data[i])#绘制信号的时间序列图，显示信号的幅度随时间的变化情况
    plt.xlabel('Time/s')#设置横轴标签为“Time/s”，明确横轴的单位为秒
    plt.ylabel('Amplitude')#设置纵轴标签为“Amplitude”，明确纵轴表示信号的幅度
    plt.title('Fault type'+str(i))#设置子图的标题为“Fault type”加上当前循环的索引 i，标识当前处理的故障类型
    # plt.savefig('result/Fault type'+str(i)+' Signal.jpg')
    # plt.close()
    plt.subplot(1,2,2)#选择第二个子图，为绘制时频图准备子图
    t,frequencies = np.meshgrid(t,frequencies)#生成二维网格 t 和 frequencies，为绘制时频图准备时间-频率网格
    plt.pcolormesh(t, frequencies, abs(cwtmatr),shading='auto',cmap='jet')
    #绘制时频图，使用伪彩色图表示小波变换系数的绝对值，显示信号在不同时间和频率上的能量分布
    plt.xlabel('Time/s')#设置横轴标签为“Time/s”，明确横轴的单位为秒
    plt.ylabel('Frequency/Hz')#设置纵轴标签为“Frequency/Hz”，明确纵轴的单位为赫兹
    plt.title('Fault type'+str(i))#设置子图的标题为“Fault type”加上当前循环的索引 i，标识当前处理的故障类型
    plt.savefig('result/Fault type'+str(i)+' Spectrum.jpg')#将当前图形保存为 JPEG 文件，保存时频图以供后续分析或展示
    plt.close()#关闭当前图形，释放内存

    #总结：
#循环处理每个故障类型：使用 for 循环遍历10个信号数据。
#设置参数：定义采样频率、小波函数、尺度数等参数。
#连续小波变换：对每个信号进行小波变换，生成小波变换系数矩阵和频率数组。
#绘制信号图：绘制信号的时间序列图，显示信号幅度随时间的变化。
#绘制时频图：使用伪彩色图绘制时频图，显示信号在不同时间和频率上的能量分布。
#保存图形：将生成的图形保存为 JPEG 文件，方便后续分析或展示。