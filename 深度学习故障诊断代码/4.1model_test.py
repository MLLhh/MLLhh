from torch.utils.data import DataLoader
from util import netDataset
from thop import profile
import torch
import os
import numpy as np
import matplotlib.pyplot as plt
from Ghostnet_v3 import ghostnetv3
from g_ghost_regnet import g_ghost_regnetx_002
from swin_transformer import SwimTransformer
from scipy.io import savemat
import seaborn as sns
from sklearn.metrics import confusion_matrix, precision_recall_fscore_support
import pandas as pd
#用测试集评估模型的训练好坏
def eval(model,test_loader):
    eval_loss=0.0#用于累加每个batch的损失
    total_acc=0.0#用于累加每个batch的正确预测数量
    y_true = []#存储所有真实标签
    y_pred = []#存储所有预测标签
    model.eval()#将模型设置为评估模式
    loss_function = torch.nn.CrossEntropyLoss()#使用 CrossEntropyLoss作为损失函数，适用于分类任务

    for i,batch in enumerate(test_loader):#遍历测试集
#        batch = tuple(t.to(args.device) for t in batch)
        x, y = batch#对于每个batch，解包输入 x 和标签 y
        x = x.to(device)#将数据移动到设备（如GPU）上，以加速计算
        y = y.to(device)
        with torch.no_grad():#使用 torch.no_grad() 上下文管理器，禁用梯度计算，节省内存和计算资源
            logits= model(x)#model返回的是（bs,num_classes)
            batch_loss=loss_function(logits,y)#使用交叉熵损失函数计算当前batch的损失 batch_loss
            #记录误差
            eval_loss+=batch_loss.item()#将当前batch的损失 batch_loss，累加到 eval_loss
            #记录准确率
            _,preds= logits.max(1)#通过 logits.max(1) 得到预测类别 preds
            num_correct=(preds==y).sum().item()#计算当前batch的正确预测数量 num_correct
            total_acc+=num_correct#累加到 total_acc

            y_true.extend(y.cpu().numpy())
            y_pred.extend(preds.cpu().numpy())
            #将真实标签 y 和预测标签 preds从GPU移动到CPU，并转换为numpy数组，存储到 y_true 和 y_pred 中
    loss=eval_loss/len(test_loader)#loss 是所有batch损失的平均值
    acc=total_acc/(len(test_loader)*eval_batch_size)#acc 是所有batch正确预测数量的平均准确率，假设每个batch的大小相同

    # 混淆矩阵
    cm = confusion_matrix(y_true, y_pred)#使用 confusion_matrix 计算混淆矩阵，帮助分析模型在每个类别上的表现
    from sklearn.metrics import classification_report

    # 计算每一类的分类准确率
    report = classification_report(y_true, y_pred, digits=4)
    print(report)
    #导入 classification_report，生成详细分类报告，包括每个类别的精确率、召回率、F1分数和支持度，并打印出来
    # 模型指标
    precision, recall, f1, support = precision_recall_fscore_support(y_true, y_pred, average='weighted')
    # return loss,acc
    return loss, acc, cm, precision, recall, f1, support
    #返回损失、准确率、混淆矩阵、精确率、召回率、F1分数和支持度，供后续分析使用
import matplotlib.pyplot as plt
import seaborn as sns

# 原始标签和自定义标签的映射
label_mapping = {
        'C1','C2','C3','C4','C5','C6','C7','C8','C9','C10'
    }
# 画混淆矩阵图
def plot_confusion_matrix(cm,label_mapping ):
    plt.figure(figsize=(10, 10), dpi=300)
    sns.set_context('poster')
    sns.heatmap(cm, annot=True, fmt='d', cmap='Blues', xticklabels=label_mapping, yticklabels=label_mapping)
    plt.xlabel('Predicted')
    plt.ylabel('True')
    plt.title('Confusion Matrix')
    plt.savefig("cm.jpg")  # 混淆矩阵保存路径
    plt.show()
# 提取分类层前一层网络的输出，作为判别特征
def extract(model,test_loader):
    model.eval()#将模型设置为评估模式，通常在这种模式下，模型会关闭如Dropout和BatchNorm等正则化层，以确保推理结果的一致性
    feature=np.zeros((0,0))#两种方法尝试初始化特征数组。第一个使用NumPy的零张量np.zeros((0,0))
    feature=torch.tensor([])#第二个使用PyTorch的空张量torch.tensor([])
    label=torch.tensor([])#初始化一个空的PyTorch张量，用于存储所有批次的标签
    for i,batch in enumerate(test_loader):#循环遍历测试加载器中的每一个批次。enumerate提供了批次的索引i和对应的批次数据batch
        x, y = batch#从批次中解包出输入数据x和标签y
        x = x.to(device)#然后将输入数据x移动到与模型相同的设备上（如GPU或CPU），以便后续的模型推理
        with torch.no_grad():#一个上下文管理器，表明在其内部的代码不需要计算梯度。这可以节省内存和计算资源，因为在推理阶段不需要反向传播
            fea= model(x)#将输入x传递到模型model中，第二个参数True可能是在模型中定义的一种标记，用于指示只返回特征而不是完整的输出结果。
            fea=fea.detach().cpu()#从模型的输出中detach，以分离计算图，释放内存。然后将特征移动到CPU上，这有助于后续的数据处理和存储，因为大部分后续处理会在CPU上进行
            
            feature=torch.cat([feature,fea])#将当前批次的特征fea与之前的特征feature连接起来，逐渐构建完整的特征集合
            label=torch.cat([label,y])#将当前批次的标签y与之前的标签label连接起来，逐渐构建完整的标签集合

    return feature.numpy(),label.numpy()#将PyTorch张量转换为NumPy数组，返回提取到的特征和对应的标签


if __name__ == "__main__":
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print("load dataset.........................")
    #加载数据    
    trainpath='image/train/'
    valpath='image/test/'

    img_size=64#设置图像的大小为64x64像素
    eval_batch_size=4#验证时的批次大小
    
    train_lines = os.listdir(trainpath)#读取训练集目录下的所有文件名
    val_lines = os.listdir(valpath)#读取验证集目录下的所有文件名
    'model = ghostnetv3(num_classes=4,width=1.0).to(device)'
    model = SwimTransformer(num_classes=4,width=1.0).to(device)#加载了一个名为SwimTransformer的模型
    'model = g_ghost_regnetx_002(num_classes=10).to(device)'
    
    train_dataset = netDataset(trainpath,train_lines,img_size)
    val_dataset = netDataset(valpath,val_lines,img_size)
    train_loader = DataLoader(train_dataset, shuffle=False, batch_size=eval_batch_size,
                              num_workers=0, pin_memory=True,drop_last=True)
    test_loader = DataLoader(val_dataset, shuffle=False, batch_size=eval_batch_size, 
                             num_workers=0,pin_memory=True,drop_last=False)
    #drop_last是一个布尔参数,设置为True，则会丢弃最后一个不完整的批次,设置为False，则会保留最后一个不完整的批次
    #num_workers用于设置加载数据时使用的子进程数目。它能够同时加载多个批次的数据，从而提高数据加载的效率,将num_workers设置为0，则数据加载会在主进程中进行，这会阻塞主进程，降低程序的效率
    #shuffle=False：在训练时不打乱数据的顺序，可能会导致模型过拟合
    try:
        net_dict=model.state_dict()
        model_para=torch.load('output/best.pt').to(device)
    
        state_dict={k:v for k,v in model_para.named_parameters() if k in net_dict.keys()}
        net_dict.update(state_dict)
        model.load_state_dict(net_dict)
    #当程序执行到“try”块中的代码时，如果某行代码引发了指定的异常类型，程序会暂停执行“try”块中的代码，并跳转到对应的“except”块
    #如果在“try”块中没有发生任何异常，则程序会跳过所有的“except”块，继续执行“try”块之后的代码
    except:
        model=torch.load('output/best.pt',map_location=torch.device('cpu'),weights_only=False).to(device)
    print("计算模型FLOPS...")
    # 构造输入示例（batch_size=1，3通道，64x64图像）
    input_example = torch.randn(1, 3, 64, 64).to(device)
    # 计算FLOPS和参数数量
    flops, params = profile(model, inputs=(input_example,))
    print(f"模型FLOPS: {flops / 1e9:.2f} G")  # 转换为G FLOPS
    print(f"模型参数数量: {params / 1e6:.2f} M")  # 转换为M参数
    print('model test-----------------')
    # eval_loss, eval_acc = eval( model, test_loader)
    loss, acc, cm, precision, recall, f1, support = eval(model, test_loader)

    # Print metrics in a table
    metrics_table = pd.DataFrame({
        'Metric': ['Loss', 'Accuracy', 'Precision', 'Recall', 'F1-Score'],
        'Value': [loss, acc, precision, recall, f1]
    })
    print(metrics_table)
    plot_confusion_matrix(cm, label_mapping)
    # print(eval_acc)
    print('feature extraction-----------------')
    train_feature,train_label=extract(model,train_loader)
    test_feature,test_label=extract(model,test_loader)
    print('train_feature.shape:"',train_feature.shape)
    print('train_label.shape:"',train_label.shape)
    print('test_feature.shape:"',test_feature.shape)
    print('test_label.shape:"',test_label.shape)
    
    savemat('output/feature.mat',{'train_feature':train_feature,
                                  'train_label':train_label,
                                  'test_feature':test_feature,
                                  'test_label':test_label})
    
    