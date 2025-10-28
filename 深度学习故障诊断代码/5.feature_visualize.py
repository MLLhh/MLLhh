# -*- coding: utf-8 -*-
from scipy.io import loadmat
import matplotlib.pyplot as plt
from sklearn.preprocessing import StandardScaler,MinMaxScaler
from sklearn.manifold import TSNE
from sklearn.decomposition import PCA
from mpl_toolkits.mplot3d import Axes3D

# In[] 加载原始数据
data=loadmat('result/data_process.mat')#这个是保存下来的原始数据
input_data=data['train_x']
label=data['train_y'].reshape(-1,)

# method=TSNE(n_components=3)
method=PCA(n_components=3)#降维处理：使用PCA（主成分分析）将原始数据（可能是高维数据）降维到3维，以便进行可视化
feature0=method.fit_transform(input_data) 
colors = ['black', 'blue', 'purple', 'yellow', 'cadetblue', 'red', 'lime', 'cyan', 'orange', 'gray']

plt.figure()
ax = plt.axes(projection='3d')  # 设置三维轴
for i in range(len(colors)):
    ax.scatter3D(feature0[:, 0][label==i], feature0[:, 1][label==i],feature0[:, 2][label==i], c=colors[i],label=str(i))
#    ax.text(np.mean(feature0[:, 0][label==i]), np.mean(feature0[:, 1][label==i]),np.mean(feature0[:, 2][label==i]), str(i))
plt.legend()
plt.title('train original data')
plt.savefig('result/train original data.jpg')
input_data=data['test_x']
label=data['test_y'].reshape(-1,)
#以上为训练数据的处理

#以下为测试数据的处理
feature1=method.fit_transform(input_data)
plt.figure()
ax = plt.axes(projection='3d')  # 设置三维轴
for i in range(len(colors)):
    ax.scatter3D(feature1[:, 0][label==i], feature1[:, 1][label==i],feature1[:, 2][label==i], c=colors[i],label=str(i))
    
plt.legend()
plt.title('test original data')
plt.savefig('result/test original data.jpg')




# In[] 加载特征数据

data=loadmat('output/feature.mat')#这个是保存下来的原始数据
input_data=data['train_feature']
label=data['train_label'].reshape(-1,)

# method=TSNE(n_components=3)
method=PCA(n_components=3)
feature0=method.fit_transform(input_data) 
colors = ['black', 'blue', 'purple', 'yellow', 'cadetblue', 'red', 'lime', 'cyan', 'orange', 'gray']

plt.figure()
ax = plt.axes(projection='3d')  # 设置三维轴
for i in range(len(colors)):
    ax.scatter3D(feature0[:, 0][label==i], feature0[:, 1][label==i],feature0[:, 2][label==i], c=colors[i],label=str(i))
#    ax.text(np.mean(feature0[:, 0][label==i]), np.mean(feature0[:, 1][label==i]),np.mean(feature0[:, 2][label==i]), str(i))
plt.legend()
plt.title('train feature data')
plt.savefig('result/train feature data.jpg')


input_data=data['test_feature']
label=data['test_label'].reshape(-1,)

feature1=method.fit_transform(input_data) 

plt.figure()
ax = plt.axes(projection='3d')  # 设置三维轴
for i in range(len(colors)):
    ax.scatter3D(feature1[:, 0][label==i], feature1[:, 1][label==i],feature1[:, 2][label==i], c=colors[i],label=str(i))
    
plt.legend()
plt.title('test feature data')
plt.savefig('result/test feature data.jpg')

plt.show()
