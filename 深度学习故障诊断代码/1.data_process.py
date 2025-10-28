
import os
from scipy.io import loadmat,savemat
import numpy as np
from sklearn.model_selection import train_test_split
#代码涉及到什么函数，参数和函数返回值有什么意义
# In[]
lis=os.listdir('./data/')
#列出指定目录下的所有文件和子目录
#参数: './data/' 表示要列出的目录路径，'.' 表示当前目录，'data/' 是子目录
#返回值: 返回一个列表，包含指定目录下的所有文件和子目录的名称，不包括路径信息。例如，如果 data 目录下有 file1.txt 和 subdir 文件夹，返回值可能是 ['file1.txt', 'subdir']。

N=200;Len=1024 #每种故障取100个样本，每个样本长度是1024

data=np.zeros((0,Len))
#创建一个指定形状（shape）的数组，所有元素初始化为零
#参数shape=(0, Len)：一个元组，表示数组的维度和大小,0行Len列。

label=[]
#空列表：[]，表示一个没有任何元素的列表

#遍历列表 lis，同时获取每个元素的 索引 和 值
for n,i in enumerate(lis):
    path='./data/'+i
    print('第',n,'类的数据是',path,'这个文件')
    file=loadmat(path)#加载 MATLAB 文件，返回一个包含文件内容的字典
    file_keys = file.keys()#获取字典中的所有键，用于后续处理
    for key in file_keys:
        if isinstance(key, str) and 'N09_M07_F10_' in key:  # 只选择相应的键，确保是字符串
            # 提取第一个找到的文件内容
            files = file[key][0,0]['Y'][0][6][2].ravel()
            content = file[key][0, 0]['Y'][0][6][2]
            print(f"提取的内容: {content}")
            break  # 取到后退出循环
    data_=[]
    for i in range(N):
        start=np.random.randint(0,len(files)-Len)#用于生成随机整数，范围在 [low, high)之间
        end=start+Len
        data_.append(files[start:end])#将截取的子数组添加到 data_ 列表中。每次循环都会添加一个长度为1024的子数组到 data_ 列表中
        label.append(n)#将一个对象添加到列表的末尾
    data_=np.array(data_)#data_ 最初是一个列表，经过 np.array() 转换后，变成了一个 NumPy 数组
    
    data=np.vstack([data,data_])#将 data 和 data_两个数组按行垂直堆叠，形成一个新的二维数组,它们的列数必须一致，以确保堆叠操作成功
label=np.array(label)

# 9：1划分数据集

train_x,test_x,train_y,test_y=train_test_split(data,label,test_size=.1,random_state=0)
#train_x和train_y是训练集的特征和标签，用于模型的训练
#test_x和test_y是测试集的特征和标签，用于模型的评估
#test_size=.1表示测试集占总数据的10%，训练集则占90%
#random_state=0确保了数据分割的随机性是可重复的，即每次运行代码时，分割结果都是一样的

# In[] 保存数据
savemat('result/data_process.mat',{'train_x':train_x,'test_x':test_x,
                                   'train_y':train_y,'test_y':test_y})
#将数据保存为 MATLAB 格式的 .mat 文件，以便 MATLAB 程序可以读取和处理这些数据