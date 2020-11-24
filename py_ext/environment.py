import math
import numpy as np

from utils import *
from config import *

class ModelEnv:

    def __init__(self, bytes):
        self.H = None
        self.W = None
        self.target_keypoints = None
        self.ball = None
        self.prey = None
        self.predator = None
        self.decode(bytes)

    def decode(self, input):

        input = input.decode('utf_8')
        input_list = input.split('\n')

        h_w = input_list[0].split(';')[:2]
        h_w = list(map(convert_nameValue, h_w))
        self.H = h_w[0]
        self.W = h_w[1]

        self.target_keypoints = []
        self.target_keypoints = list(map(convert_nameValue, input_list[1].split(';')[:-1]))
        self.target_keypoints = np.array(self.target_keypoints)

        ball_prey_predator = input_list[2].split(';')
        self.ball = convert_nameValue(ball_prey_predator[0])
        self.prey = convert_nameValue(ball_prey_predator[1])
        self.predator = convert_nameValue(ball_prey_predator[2])

    def find_nearest_target(self, position):

        nearest_d = self.H + self.W
        nearest_target = None

        for kp in self.target_keypoints:
            d = np.sqrt(np.sum((position - kp)**2))
            if d < nearest_d:
                nearest_d = d
                nearest_target = kp

        return nearest_target, nearest_d

    def compute_reward(self, cur_state, prev_state):

        """

        :param state: torch.tensor
        :return:
        """
        DIAG = np.sqrt(self.H**2 + self.W**2)

        cur_state = toNPArr(cur_state)
        prev_state = toNPArr(prev_state)

        rewards = []
        for i in range(1, N_PREY + 1):

            cur_pos = np.array(cur_state[(-i)*4:(-i)*4+2])
            prev_pos = np.array(prev_state[(-i)*4:(-i)*4+2])

            cur_target, cur_d = self.find_nearest_target(cur_pos)
            prev_target, prev_d = self.find_nearest_target(prev_pos)

            reward = cur_d - prev_d
            reward = reward / np.exp(cur_d / DIAG)
            rewards.append(reward)

        ret = np.average(rewards) / DIAG

        return ret

