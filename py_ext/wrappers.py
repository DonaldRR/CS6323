import torch
import numpy as np
from utils import *

class Buffer:

    def __init__(self, buffer_size=5000, batch_size=16):
        self.buffer_size = buffer_size
        self.batch_size = batch_size
        self.memory = []

    def append(self, l):

        """
        :param l: [state_t, action_t, reward_t, state_t+1]
        :return:
        """

        if len(self.memory) > self.buffer_size:
            self.memory.pop(0)

        self.memory.append(l)

    def get_batch(self):

        idxs = np.random.choice(list(range(len(self.memory))), self.batch_size)
        s_t, a_t, r_t, s_t_ = [], [], [], []

        for idx in idxs:
            s_t.append(self.memory[idx][0])
            a_t.append(self.memory[idx][1])
            r_t.append(self.memory[idx][2])
            s_t_.append(self.memory[idx][3])

        s_t = torch.stack(tuple(s_t), dim=0)
        a_t = torch.stack(tuple(a_t), dim=0)
        r_t = torch.stack(tuple(r_t), dim=0)
        s_t_ = torch.stack(tuple(s_t_), dim=0)

        return s_t, a_t, r_t, s_t_

    @property
    def ready(self):

        return len(self.memory) >= self.batch_size

class Message:

    def __init__(self, bytes):
        self.preys = None
        self.predators = None
        self.termination = None
        self.decode(bytes)

    def decode(self, input):

        """
        message format:

        """

        input = input.decode('utf_8')
        input_list = input.split("\n")[:-1]
        self.preys = []
        self.predators = []
        for i in range(len(input_list) - 1):
            t = input_list[i].split(';')
            type = int(t[0].split(':')[1])
            pos = t[1].split(':')[1][1:-1]
            pos = list(map(float, pos.split(',')))
            speed = t[2].split(':')[1][1:-1]
            speed = list(map(float, speed.split(',')))
            tmp = {
                'type': type,
                'pos': pos,
                'speed': speed
            }
            if type == 0:
                self.preys.append(tmp)
            else:
                self.predators.append(tmp)

        self.termination = int(input_list[-1].split(':')[1])

    @property
    def is_terminal(self):

        return self.termination == 1

    @property
    def reward(self):

        return 0

    @property
    def state(self):

        state = []
        for _, agent_state in enumerate(self.preys):
            state.extend(agent_state['pos'])
            state.extend(agent_state['speed'])

        for _, agent_state in enumerate(self.predators):
            state.extend(agent_state['pos'])
            state.extend(agent_state['speed'])

        return state
