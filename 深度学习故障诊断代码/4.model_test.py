from torch.utils.data import DataLoader
from util import netDataset
import torch
import os
import numpy as np
import matplotlib.pyplot as plt
from Ghostnet_v3 import ghostnetv3
from scipy.io import savemat
#用测试集评估模型的训练好坏
def eval(model,test_loader):
    eval_loss=0.0
    total_acc=0.0
    model.eval()
    loss_function = torch.nn.CrossEntropyLoss()

    for i,batch in enumerate(test_loader):
#        batch = tuple(t.to(args.device) for t in batch)
        x, y = batch
        x = x.to(device)
        y = y.to(device)
        with torch.no_grad():
            logits= model(x)#model返回的是（bs,num_classes
            batch_loss=loss_function(logits,y)
            #记录误差
            eval_loss+=batch_loss.item()
            #记录准确率
            _,preds= logits.max(1)
            num_correct=(preds==y).sum().item()
            total_acc+=num_correct

    loss=eval_loss/len(test_loader)
    acc=total_acc/(len(test_loader)*eval_batch_size)
    return loss,acc
# 提取分类层前一层网络的输出，作为判别特征
def extract(model,test_loader):
    model.eval()
    feature=np.zeros((0,0))
    feature=torch.tensor([])
    label=torch.tensor([])
    for i,batch in enumerate(test_loader):
        x, y = batch
        x = x.to(device)
        with torch.no_grad():
            fea= model(x,True)
            fea=fea.detach().cpu()
            
            feature=torch.cat([feature,fea])
            label=torch.cat([label,y])
            
    return feature.numpy(),label.numpy()



if __name__ == "__main__":
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print("load dataset.........................")
    #加载数据    
    trainpath='image/train/'
    valpath='image/test/'

    img_size=64
    eval_batch_size=4
    
    train_lines = os.listdir(trainpath)
    val_lines = os.listdir(valpath)
    model = ghostnetv3(num_classes=10,width=1.0).to(device)
    
    train_dataset = netDataset(trainpath,train_lines,img_size)
    val_dataset = netDataset(valpath,val_lines,img_size)
    train_loader = DataLoader(train_dataset, shuffle=False, batch_size=eval_batch_size,
                              num_workers=0, pin_memory=True,drop_last=True)
    test_loader = DataLoader(val_dataset, shuffle=False, batch_size=eval_batch_size, 
                             num_workers=0,pin_memory=True,drop_last=False)
    try:
        net_dict=model.state_dict()
        model_para=torch.load('output/best.pt').to(device)
    
        state_dict={k:v for k,v in model_para.named_parameters() if k in net_dict.keys()}
        net_dict.update(state_dict)
        model.load_state_dict(net_dict)
    except:
        model=torch.load('output/best.pt',map_location=torch.device('cpu'),weights_only=False).to(device)
    print('model test-----------------')
    eval_loss, eval_acc = eval( model, test_loader)
    print(eval_acc)
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
    
    