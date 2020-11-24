import torch
from torch import nn

from config import *

NORMALIZATION = nn.LayerNorm
def Normalizer(*args):

    return NORMALIZATION(args)

class Critic(nn.Module):

    def __init__(self, state_size=2, action_size=2, hidden_size=64):
        super(Critic, self).__init__()

        self.state_size = state_size
        self.action_size = action_size
        self.hidden_size = hidden_size

        self.state_trans = nn.Sequential(
            nn.Linear((N_PREY + N_PREDATOR) * self.state_size, self.hidden_size),
            Normalizer(self.hidden_size),
            #nn.BatchNorm1d(hidden_size),
            nn.ReLU()
        )

        self.action_trans = nn.Sequential(
            nn.Linear(N_PREDATOR * self.action_size, self.hidden_size),
            Normalizer(self.hidden_size),
            #nn.BatchNorm1d(hidden_size),
            nn.ReLU()
        )

        self.pred = nn.Sequential(
            nn.Linear(self.hidden_size * 2, self.hidden_size),
            Normalizer(self.hidden_size),
            #nn.BatchNorm1d(hidden_size),
            nn.Dropout(0.8),
            nn.Linear(self.hidden_size, 1)
        )

        self.gru = nn.GRU(input_size=(N_PREY + N_PREDATOR) * self.state_size + N_PREDATOR * self.action_size,
                          hidden_size=self.hidden_size)

    def forward_(self, sequence, action, mask):

        """
        :param sequence: torch.Tensor, (B, L, C_i)
        :param action: torch.Tensor, (B, N * action_size)
        :param mask: torch.Tensor, (B, L)
        :return:
        """


        B = sequence.shape[0]

        sequence = sequence.permute(1, 0, 2).float() # (L, B, C_i)
        mask = mask.permute(1, 0).float() # (L, B)

        h0 = torch.zeros((1, B, self.hidden_size))
        output, _ = self.gru(sequence, h0) # (L, B, C_o)
        output = output[mask == 1, :] # (B, C_o)

        a_state = self.action_trans(action.float()) # (B, C_o)
        pred = self.pred(torch.cat([output, a_state], dim=1))

        return pred

    def forward(self, state, action):
        x_s = self.state_trans(state)
        x_a = self.action_trans(action)

        x = torch.cat((x_s, x_a), 1)
        pred = self.pred(x)

        return pred


class Actor(nn.Module):

    def __init__(self, state_size=2, action_size=2, hidden_size=64):
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
            Normalizer(self.hidden_size),
            #nn.BatchNorm1d(self.hidden_size),
            nn.ReLU(),
            nn.Linear(self.hidden_size, self.hidden_size),
            Normalizer(self.hidden_size),
            #nn.BatchNorm1d(self.hidden_size),
            nn.ReLU(),
            nn.Dropout(0.8),
            nn.Linear(self.hidden_size, self.action_size),
            #nn.Linear(self.hidden_size, N_PREDATOR * self.action_size),
            #nn.Linear(self.hidden_size, N_PREDATOR * self.action_size * 2),
            #nn.ReLU(),
            #nn.Sigmoid(),
            nn.Tanh()
        )

        self.gru = nn.GRU(input_size=self.action_size + (N_PREY + N_PREDATOR) * self.state_size + 4,
                          hidden_size=self.hidden_size)
        self.header = nn.Sequential(
            nn.Linear(self.hidden_size, self.hidden_size),
            Normalizer(self.hidden_size),
            #nn.ReLU(),
            nn.Linear(self.hidden_size, self.action_size),
            #nn.Sigmoid(),
            nn.Tanh()
        )

    def forward_(self, sequence, mask):

        """
        :param sequence: torch.Tensor, (B, L, C_i)
        :param mask: torch.Tensor, (B, L)
        :return:
        """

        B = sequence.shape[0]

        sequence = sequence.permute(1, 0, 2).float() # (L, B, C_i)
        mask = mask.permute(1, 0).float() # (L, B)

        h0 = torch.zeros((1, B, self.hidden_size)) # (1, B, C_o)
        output, _ = self.gru(sequence, h0) # (L, B, C_o)
        output = output[mask == 1, :] # (B, C_o)
        pred = self.header(output) # (B, action_size)

        return pred

    def forward(self, state):
        batch_size = state.size()[0]

        x = self.net(state)  # (b, n * a * 2)
        #x = x.view(batch_size, N_PREDATOR, self.action_size, 2)
        #x = x[:, :, :, 0] / (x.sum(dim=3) + 1e-6)
        #x = x.view(batch_size, -1)

        return x

    @staticmethod
    def sample_action(action, noise):

        noise = (torch.rand(action.size()) * 2 - 1) * noise
        noise = noise.to(action.device)

        action = action + noise
        action = torch.clip(action, -1, 1)

        return action

    @staticmethod
    def to_str(action):
        action_str = "action\n"

        for i in range(N_PREDATOR):
            ax = action[i * 2]
            ay = action[i * 2 + 1]
            #av = action[i * 3 + 2]
            #action_str += "%f,%f,%f;" % (ax, ay, av)
            action_str += "%f,%f;" % (ax, ay)

        return action_str

