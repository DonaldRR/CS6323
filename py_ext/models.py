import torch
from torch import nn

from config import *

class Critic(nn.Module):

    def __init__(self, state_size=4, action_size=3, hidden_size=32):
        super(Critic, self).__init__()

        self.state_trans = nn.Sequential(
            nn.Linear((N_PREY + N_PREDATOR) * state_size, hidden_size),
            nn.BatchNorm1d(hidden_size),
            nn.ReLU(),
            nn.Linear(hidden_size, hidden_size),
            nn.BatchNorm1d(hidden_size)
        )

        self.action_trans = nn.Sequential(
            nn.Linear(N_PREDATOR * action_size, hidden_size),
            nn.BatchNorm1d(hidden_size),
            nn.ReLU(),
            nn.Linear(hidden_size, hidden_size),
            nn.BatchNorm1d(hidden_size)
        )

        self.pred = nn.Sequential(
            nn.Linear(hidden_size * 2, hidden_size),
            nn.BatchNorm1d(hidden_size),
            nn.ReLU(),
            nn.Linear(hidden_size, 1)
        )

    def forward(self, state, action):
        x_s = self.state_trans(state)
        x_a = self.action_trans(action)

        x = torch.cat((x_s, x_a), 1)
        pred = self.pred(x)

        return pred


class Actor(nn.Module):

    def __init__(self, state_size=4, action_size=3, hidden_size=32):
        super(Actor, self).__init__()

        """

        input: environment state containing states of each agent, e.g. [pos_x_1, pos_y_1, speed_x_1, speed_y_1, pos_x_2, pos_y_2, speed_x_2, speed_y_2, ...] 
        output: actions for controlled agents, e.g. [action_x_1, action_y_1, velocity_1, action_x_2, action_y_2, velocity_2, ...]
        """

        self.state_size = state_size
        self.action_size = action_size
        self.hidden_size = hidden_size

        self.net = nn.Sequential(
            nn.Linear((N_PREY + N_PREDATOR) * self.state_size, self.hidden_size),
            nn.BatchNorm1d(self.hidden_size),
            nn.ReLU(),
            nn.Linear(self.hidden_size, self.hidden_size),
            nn.BatchNorm1d(self.hidden_size),
            nn.ReLU(),
            nn.Linear(self.hidden_size, N_PREDATOR * self.action_size),
            #nn.Linear(self.hidden_size, N_PREDATOR * self.action_size * 2),
            #nn.ReLU(),
            nn.Sigmoid()
        )

    def forward(self, state):
        batch_size = state.size()[0]

        x = self.net(state)  # (b, n * a * 2)
        #x = x.view(batch_size, N_PREDATOR, self.action_size, 2)
        #x = x[:, :, :, 0] / (x.sum(dim=3) + 1e-6)
        #x = x.view(batch_size, -1)

        return x

    @staticmethod
    def sample_action(action):

        noise = (torch.rand(action.size()) - 0.5) * 2 * 0.2
        action = action + noise
        action = torch.clamp(action, 0, 1)

        return action

    @staticmethod
    def to_str(action):
        action_str = "action\n"

        for i in range(N_PREDATOR):
            ax = action[i * 3] - 0.5
            ay = action[i * 3 + 1] - 0.5
            av = action[i * 3 + 2]
            action_str += "%f,%f,%f;" % (ax, ay, av)

        return action_str

