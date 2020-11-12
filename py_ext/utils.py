import torch

import numpy as np


def toNPArr(data):

    if isinstance(data, torch.Tensor):
        if data.device == torch.device('cpu'):
            data = data.numpy()
        else:
            data = data.detach().cpu().numpy()
    else:
        data = np.array(data)

    return data

def toTensor(data, gpu=None):

    data = torch.from_numpy(np.array(data)).float()
    if gpu:
        data = data.to(device=gpu)

    return data

def convert_nameValue(x):

    name, value = x.split(':')

    if len(value) > 2 and value[0] == '(' and value[-1] == ')':
        value = list(map(float, value[1:-1].split(',')))
    else:
        value = float(value)

    return value

def soft_update(target, source, tau):
    for target_param, param in zip(target.parameters(), source.parameters()):
        target_param.data.copy_(target_param.data * (1.0 - tau) + param.data * tau)

def hard_update(target, source):
    for target_param, param in zip(target.parameters(), source.parameters()):
        target_param.data.copy_(param.data)
