import math

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

    def compute_reward(self, state):

        nearest_d = math.sqrt(self.H ** 2 + self.W ** 2)
        for i in range(N_PREY):

            pos = np.array(state[i*4:i*4+2])

            for kp in self.target_keypoints:
                d = np.sqrt(np.sum((pos - kp)**2))
                if d < nearest_d:
                    nearest_d = d

        reward = 0
        if nearest_d < 0.8 * self.H:
            reward -= 0.1
        if nearest_d < 0.6 * self.H:
            reward -= 0.1
        if nearest_d < 0.4 * self.H:
            reward -= 0.1
        if nearest_d < 0.2 * self.H:
            reward -= 0.1

        return reward