
# 对所有样本依次计算时频图 并保存
import matplotlib.pyplot as plt
import numpy as np
import pywt    #小波变化
from scipy.io import loadmat #主要用于处理不同格式的科学数据文件
import os #os模块是一个内置模块，提供了与操作系统交互的功能
import shutil#提供了许多高级的文件操作功能，如文件和目录的复制、删除、归档、压缩和解压等
import warnings#用于在程序运行时发出警告信息
warnings.filterwarnings("ignore")#忽略所有类型的警告信息

shutil.rmtree('image/')#rmtree 是 shutil 模块中的一个函数，用于递归地删除指定的目录及其所有子目录和文件
os.mkdir('image/')#mkdir 是 os 模块中的一个函数，用于创建一个新的目录
os.mkdir('image/train')
os.mkdir('image/test')
# In[]
def Spectrum(data,label,path):
    label=label.reshape(-1,)#reshape(-1,)通常用于将多维数组转换为一维数组
    for i in range(data.shape[0]):#假设 data 是一个二维数组，比如有 n个信号，每个信号有 m个采样点，那么 data.shape[0] 就是 n，循环会遍历每个信号
        sampling_rate=64000# 采样频率64000
        wavename = 'cmor3-3'#cmor是复Morlet小波，其中3－3表示Fb－Fc，Fb是带宽参数，Fc是小波中心频率。
        totalscal = 256#小波变换中使用的尺度数。尺度数决定了频谱的分辨率，尺度越多，分辨率越高
        fc = pywt.central_frequency(wavename)# 小波的中心频率，中心频率是小波函数的主要频率成分，对于后续的尺度计算很重要
        cparam = 2 * fc * totalscal#用于生成小波变换的尺度序列
        scales = cparam / np.arange(totalscal, 1, -1)#np.arange(totalscal,1, -1)生成从 totalscal 到1 的整数序列，步长为-1，即递减序列。然后用 cparam除以这个序列，得到尺度序列。尺度序列在小波变换中用于确定不同频率的分析。
        [cwtmatr, frequencies] = pywt.cwt(data[i], scales, wavename, 1.0 / sampling_rate)
        #这行代码调用 pywt.cwt 函数进行连续小波变换。
        # data[i] 是当前循环中的信号数据，scales 是之前生成的尺度序列，wavename 是小波函数的名称，1.0 / sampling_rate 是采样间隔时间。
        # 函数返回两个结果：cwtmatr 是小波变换系数矩阵，frequencies 是对应的频率数组。
        t=np.arange(len(data[i]))/sampling_rate#从0到信号长度的整数序列，除以采样率 sampling_rate，得到时间序列
        t,frequencies = np.meshgrid(t,frequencies)#使用 np.meshgrid 函数生成二维网格。t 是时间轴，frequencies 是频率轴。np.meshgrid会将这两个一维数组扩展成二维网格，以便后续绘制时频图
        plt.pcolormesh(t, frequencies, abs(cwtmatr),shading='auto',cmap='jet')
        #用 plt.pcolormesh 绘制时频图。
        #t 是时间网格，frequencies 是频率网格，abs(cwtmatr) 是小波变换系数的绝对值矩阵，表示信号的能量分布。
        # shading='auto' 表示自动选择阴影绘制方式，cmap='jet' 设置颜色映射为Jet色图
        plt.axis('off')#关闭了坐标轴的显示，通常用于生成纯图像而不需要显示坐标
        plt.savefig(path+'/'+str(i)+'_'+str(label[i])+'.jpg', bbox_inches='tight',pad_inches = 0)
        #当前图形保存为 JPEG 文件。文件路径是 path 目录下，文件名是 i_label[i].jpg，其中 i 是当前循环的索引，label[i] 是当前信号的标签。
        #bbox_inches='tight' 表示调整图像大小以适应内容，pad_inches=0 表示不添加额外的边距。
        plt.close()
data=loadmat('result/data_process.mat')

Spectrum(data['train_x'],data['train_y'],'image/train')
Spectrum(data['test_x'],data['test_y'],'image/test')
