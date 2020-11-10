import torch

import numpy as np


def toTensor(data, gpu=-1):

    data = torch.from_numpy(np.array(data)).float()
    if gpu >= 0:
        data = data.to(gpu)

    return data

def convert_nameValue(x):

    name, value = x.split(':')

    if len(value) > 2 and value[0] == '(' and value[-1] == ')':
        value = list(map(float, value[1:-1].split(',')))
    else:
        value = float(value)

    return value

def update_target(train, target, tau=0.1):

    target_state_dict = target.state_dict()
    state_dict = train.state_dict()

    for k, v in state_dict.items():
        target_state_dict[k] = tau * v + (1 - tau) * target_state_dict[k]

    target.load_state_dict(target_state_dict)
