from torch.utils.data import DataLoader
from util import netDataset
import torch
import os
import numpy as np
import matplotlib.pyplot as plt
from Ghostnet_v3 import ghostnetv3
from g_ghost_regnet import g_ghost_regnetx_002
from swin_transformer import SwimTransformer
from wavemlp import WaveMLP_B
import warnings
warnings.filterwarnings("ignore")

#用测试集评估模型的训练好坏
def eval(model,test_loader):#定义一个名为 eval 的函数，用于评估模型在测试集上的性能
    eval_loss=0.0#用于累加测试过程中的总损失。意义：通过累加每个批次的损失，最终计算平均损失
    total_acc=0.0#用于累加测试过程中的总正确预测数,通过累加每个批次的正确预测数，最终计算准确率
    model.eval()#将模型设置为评估模式,在评估模式下，模型不会进行梯度计算，也不会更新参数，以节省内存和计算资源
    # 遍历测试数据加载器中的每一个批次,i：当前批次的索引,batch：当前批次的数据，包含输入和标签,意义：逐批次处理测试数据，计算损失和准确率
    for i,batch in enumerate(test_loader):
#        batch = tuple(t.to(args.device) for t in batch)
        x, y = batch#将当前批次的输入数据和标签分开,意义：准备输入数据 x 和真实标签 y，用于模型推理和计算损失
        x = x.to(device)#将输入数据 x 移动到指定的设备（如GPU）上，确保输入数据与模型在同一个设备上进行计算，提高计算效率
        y = y.to(device)#将标签 y 移动到指定的设备（如GPU）上，意义：确保标签与输入数据在同一个设备上，以便进行计算
        with torch.no_grad():#在评估过程中，不需要计算梯度，以节省内存和计算资源
            logits= model(x)#model返回的是（bs,num_classes），是模型对输入数据的预测结果
            batch_loss=loss_function(logits,y)#计算当前批次的损失，logits：模型的预测结果，y：真实标签，意义：使用损失函数（如交叉熵损失）计算模型预测与真实标签之间的差异
            #记录误差
            eval_loss+=batch_loss.item()#将当前批次的损失累加到 eval_loss 中，意义：通过累加每个批次的损失，最终计算平均损失
            #记录准确率
            _,preds= logits.max(1)#找到 logits 中每个样本的最大值的索引，即预测的类别
            num_correct=(preds==y).sum().item()#计算当前批次中的正确预测数，意义：preds == y生成一个布尔张量，表示每个样本是否预测正确。sum() 计算总共有多少个正确的预测，item() 提取这个数值
            total_acc+=num_correct#将当前批次的正确预测数累加到 total_acc 中，通过累加每个批次的正确预测数，最终计算准确率

    loss=eval_loss/len(test_loader)#计算平均损失，将累加的总损失除以总批次数，得到平均损失
    acc=total_acc/(len(test_loader)*eval_batch_size)#计算准确率，total_acc：总的正确预测数。len(test_loader)：总批次数。eval_batch_size：每个批次的大小
                                                    #意义：总样本数是批次数乘以批次大小，准确率是总正确预测数除以总样本数
    return loss,acc#返回平均损失和准确率，意义：评估函数的输出，用于衡量模型在测试集上的性能



if __name__ == "__main__":
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")#功能：检查是否可以使用GPU（CUDA），如果可以，则将设备设置为GPU，否则使用CPU。
                                                                         #意义：确保模型能够在支持GPU的环境中加速训练
    print("load dataset.........................")
    #加载数据    
    trainpath='image/train/'
    valpath='image/test/'

    img_size=64#定义图像的大小为64x64像素，意义：确保输入到模型中的图像具有统一的尺寸，方便模型处理
    train_batch_size=8#定义训练和验证时的批次大小
    eval_batch_size=8#意义：批次大小决定了每次训练或验证时处理的样本数量，过大的批次大小可能会导致内存不足，过小则可能降低训练效率
    learning_rate=1e-3#定义学习率为0.001，学习率决定了优化器更新模型参数的步长，过大的学习率可能导致模型无法收敛，过小则可能使训练过程过慢
    weight_decay=1e-5#定义权重衰减系数为0.00001，意义：权重衰减用于防止模型过拟合，通过在损失函数中添加L2正则化项，限制模型参数的大小
    total_epoch=50#定义总训练 epochs 数为50，训练的轮数
    
    train_lines = os.listdir(trainpath)#列出训练集和验证集目录下的所有文件
    val_lines = os.listdir(valpath)#意义：获取数据集中的所有样本文件，用于后续的数据加载
    'model = ghostnetv3(num_classes=4,width=1.0).to(device)'
    'model = SwimTransformer(num_classes=4).to(device)'
    model = WaveMLP_B(num_classes=4).to(device)#定义模型为 SwimTransformer，并将其移动到指定的设备（GPU或CPU）上
    'model = g_ghost_regnetx_002(num_classes=10).to(device)'
    train_dataset = netDataset(trainpath,train_lines,img_size)#创建训练集和验证集的数据集对象
    val_dataset = netDataset(valpath,val_lines,img_size)#意义：定义数据集的加载方式，包括数据路径、样本列表和图像大小
    train_loader = DataLoader(train_dataset, shuffle=True, batch_size=train_batch_size,
                              num_workers=0, pin_memory=True,drop_last=True)
    #创建数据加载器，用于按批次加载数据
    # 意义：DataLoader 将数据集划分为多个批次，方便模型训练和验证。shuffle=True 表示训练时会打乱数据顺序，以提高模型的泛化能力。
    test_loader = DataLoader(val_dataset, shuffle=False, batch_size=eval_batch_size, 
                             num_workers=0,pin_memory=True,drop_last=True)
    loss_function = torch.nn.CrossEntropyLoss()#定义损失函数为交叉熵损失，交叉熵损失常用于分类任务，衡量模型预测与真实标签之间的差异
    # Prepare optimizer and scheduler

    # 定义优化器为随机梯度下降（SGD），并设置学习率、动量和权重衰减。
    # 意义：优化器用于更新模型参数，以最小化损失函数。
    optimizer = torch.optim.SGD(model.parameters(),
                                lr=learning_rate,
                                momentum=0.9,
                                weight_decay=weight_decay)
    print("training.........................")
    #设置测试损失list,和测试acc 列表
    val_loss_list=[]
    val_acc_list=[]
    #设置训练损失list
    train_loss_list=[]
    train_acc_list=[]
    #意义：记录每次 epoch 的训练和验证结果，以便后续分析和绘制曲线。
    max_acc = 0#初始化最大准确率为0，意义：用于跟踪训练过程中验证集上的最佳准确率，以便保存最优模型
    for i in range(total_epoch):#开始训练循环，循环次数为 total_epoch
        model.train()#设置模型为训练模式
        train_loss=0#初始化当前 epoch 的训练损失为0，意义：用于累加每个批次的损失，最终计算平均损失
        for step, batch in enumerate(train_loader):#遍历训练数据加载器中的每个批次，意义：逐批次处理训练数据，更新模型参数
            x, y = batch#将当前批次的输入数据和标签分开，意义：准备输入数据 x 和真实标签 y，用于模型训练
            #print(y)
            x = x.to(device)#将输入数据和标签移动到指定的设备（GPU或CPU）上
            y = y.to(device)#确保输入数据与模型在同一个设备上进行计算，提高计算效率
            logits = model(x)#将输入数据 x传入模型，得到模型的输出 logits
            loss=loss_function(logits,y)#损失函数计算
            train_loss +=loss.item()
            loss.backward()#反向传播
            optimizer.step()#参数更新
            optimizer.zero_grad()#意义：逐步优化模型参数，降低训练损失，提高模型性能

        #每训练一个epoch,记录一次训练损失
        train_loss = train_loss / len(train_loader)#计算平均损失
        train_loss_list.append(train_loss)#记录平均损失
        _, train_acc = eval(model, train_loader)#评估准确率
        train_acc_list.append(train_acc)#记录准确率
        
        print("train Epoch:{},loss:{:.4f},train_acc:{:.4f} ".format(i, train_loss, train_acc), end='')


        # 每训练一个epoch,用当前训练的模型对验证集进行测试
        eval_loss, eval_acc = eval( model, test_loader)
        #将每一个测试集验证的结果加入列表
        val_loss_list.append(eval_loss)
        val_acc_list.append(eval_acc)

        print("val Epoch:{},eval_loss:{:.4f},eval_acc:{:.4f}".format(i, eval_loss, eval_acc))
        if eval_acc > max_acc:
            max_acc=eval_acc
            # 保存最优模型参数
            torch.save(model, 'output/best.pt')
    torch.save(model, 'output/last.pt') #保存最后一个epoch的模型
    np.savetxt("output/train_loss_list.txt", train_loss_list)
    np.savetxt("output/train_acc_list.txt", train_acc_list)
    np.savetxt("output/val_loss_list.txt",val_loss_list)
    np.savetxt("output/val_acc_list.txt",val_acc_list)

    # 从文件 output/train_loss_list.txt 中读取训练损失值的列表
    # 将文件中的每行数据转换为浮点数，存储在 train_loss列表中，用于后续绘制训练损失曲线
    with open('output/train_loss_list.txt','r') as f:
        train_loss_list=f.readlines()
    train_loss=[float(i.strip()) for i in train_loss_list]
    with open('output/val_loss_list.txt','r') as f:
        val_loss_list=f.readlines()     
    val_loss=[float(i.strip()) for i in val_loss_list]
    #从文件 output/train_acc_list.txt 中读取训练准确率值的列表
    #将文件中的每行数据转换为浮点数，存储在 train_acc 列表中，用于后续绘制训练准确率曲线
    with open('output/train_acc_list.txt','r') as f:
        train_acc_list=f.readlines()    
    train_acc=[float(i.strip()) for i in train_acc_list]
    with open('output/val_acc_list.txt','r') as f:
        val_acc_list=f.readlines()    
    val_acc=[float(i.strip()) for i in val_acc_list]

    #创建一个图形窗口，绘制训练损失和验证损失的曲线
    plt.figure()
    plt.plot(train_loss,label='train_loss')
    plt.plot(val_loss,label='val_loss')
    plt.legend()
    plt.savefig('result/loss curve.jpg')
    plt.figure()
    plt.plot(train_acc,label='train_acc')
    plt.plot(val_acc,label='val_acc')
    plt.legend()
    plt.savefig('result/accuracy curve.jpg')
    plt.show()