import socket
import os
from datetime import datetime
from tqdm import tqdm
import torch
import numpy as np
from torch import optim
from tensorboardX import SummaryWriter

from config import *
from models import *
from environment import *
from utils import *
from wrappers import *

def propogate_history(history):

    if len(history.shape) == 2:
        history = history.unsqueeze(0)

    B, T, _ = history.shape

    propogated_h = torch.zeros((N_PREDATOR * B, T, 2 + (N_PREDATOR + N_PREY) * 2 + 4))
    for b in range(B):
        for i in range(N_PREDATOR):
            propogated_h[N_PREDATOR * b + i, :, :2] = history[b, :, i*2:(i+1)*2]
            propogated_h[N_PREDATOR * b + i, :, 2:2+(N_PREDATOR + N_PREY)*2] = history[b, :, N_PREDATOR * 2:]
            propogated_h[N_PREDATOR * b + i, :, 2+(N_PREDATOR + N_PREY)*2 + i] = 1

    return propogated_h

def propogate_mask(mask):

    return mask.unsqueeze(0).repeat(4, 1, 1).permute(1, 0, 2).reshape(-1, SEQUENCE_LEN)


def pad_sequence(tensor):

    if isinstance(tensor, list):
        tensor = torch.stack(tensor, dim=0)

    seq_len = tensor.shape[0]
    if len(tensor.shape) == 2:
        tensor = tensor.unsqueeze(dim=0)

    tensor = torch.nn.functional.pad(tensor, (0, 0, 0, SEQUENCE_LEN - seq_len, 0, 0), "constant", 0)
    mask = torch.zeros((1, SEQUENCE_LEN))
    mask[0, seq_len - 1] = 1

    return tensor, mask


def sub_tensor(tensor, group_size, start, end):

    if len(tensor.shape) == 1:
        tensor = tensor.unsqueeze(0)

    B, C = tensor.shape
    G = C // group_size
    ret = []
    for b in range(B):
        tmp = []
        for g in range(G):
            tmp.append(tensor[b, g*group_size:(g+1)*group_size][start:end])
        ret.append(torch.cat(tmp))

    ret = torch.stack(ret)

    return ret

def propagate_state(state, state_size=2):

    """
    :param state: torch.Tensor
    :return:
    """

    def index_list(l, i):

        ret = []
        for j in range(state_size):
            ret.append(l[i*4+j])

        return ret

    states = []
    for i in range(N_PREDATOR):
        tmp = index_list(state, i)
        #tmp = [state[i*4], state[i*4+1], state[i*4+2], state[i*4+3]]
        for j in range(N_PREDATOR):
            if i != j:
                #tmp.extend([state[j*4], state[j*4+1], state[j*4+2], state[j*4+3]])
                tmp.extend(index_list(state, j))

        for j in range(N_PREY):
            #tmp.extend([state[(N_PREDATOR + j)*2], state[(N_PREDATOR + j)*2 + 1],state[(N_PREDATOR + j)*2 + 2], state[(N_PREDATOR + j)*2 + 3]])
            tmp.extend(index_list(state, N_PREDATOR + j))

        states.append(torch.tensor(tmp))
    states = torch.stack(states, dim=0)

    return states

def batch_propagate(states):

    batch_states = [propagate_state(states[i]) for i in range(states.shape[0])]
    batch_states = torch.cat(batch_states, dim=0)

    return batch_states


def train(actor, critic, actor_target, critic_target, actor_optim, critic_optim, memory):

    actor.train()
    critic.train()

    # train actor/critic
    with torch.autograd.set_detect_anomaly(True):
        for _ in tqdm(range(100)):
            H_t, A_t, R_t, H_t_, mask_t, mask_t_ = memory.get_batch_tracks()

            critic_optim.zero_grad()
            target_A_t_ = actor_target.forward_(propogate_history(H_t_), propogate_mask(mask_t_)).detach().reshape(mask_t_.shape[0], -1)
            target_Q_t_ = critic_target.forward_(H_t_, target_A_t_, mask_t_).detach()
            Q_t = critic.forward_(H_t, A_t, mask_t)
            y = R_t + target_Q_t_ * gamma
            y = torch.clamp(y, -5.0, 5.0)
            l_q = ((y - Q_t) ** 2).mean()
            l_q.backward()
            torch.nn.utils.clip_grad_value_(critic.parameters(), 0.5) # gradients clipping
            critic_optim.step()

            actor_optim.zero_grad()
            pred_A_t = actor.forward_(propogate_history(H_t), propogate_mask(mask_t)).reshape(mask_t.shape[0], -1)
            pred_A_t_l1 = (pred_A_t** 2).mean()
            pred_Q_t = critic.forward_(H_t, pred_A_t, mask_t).mean() - pred_A_t_l1 * 0.1
            (-pred_Q_t).backward()
            torch.nn.utils.clip_grad_value_(actor.parameters(), 0.5) # gradients clipping
            actor_optim.step()

            # update target networks
            soft_update(actor_target, actor, tau)
            soft_update(critic_target, critic, tau)


def run_game(actor, server_socket, noiser, device='cpu', mode='train'):
    actor.eval()

    server_socket.send(b'start')
    model_env = ModelEnv(server_socket.recv(1024))

    # Loop till the game ends
    t = 0
    prev_state = None
    reward_hist = []
    history = torch.zeros((SEQUENCE_LEN, N_PREDATOR * 2 + (N_PREDATOR + N_PREY) * 2))
    while True:

        # Receive states of the game
        msg = Message(server_socket.recv(1024))

        if msg.require_action or msg.is_terminal:

            # retrieve current state
            cur_state = toTensor(msg.state, device)
            history[t][N_PREDATOR * 2:] = sub_tensor(cur_state, 4, 0, 2)[0]

            # compute reward
            if t >= 1:
                prev_reward = model_env.compute_reward(cur_state, prev_state) if not msg.is_terminal else -10.0  # r_t
                prev_reward = toTensor(np.array([prev_reward]), device)
                reward_hist.append(prev_reward)

            # game terminated
            if msg.is_terminal:
                break

            # compute next action
            tmp_mask = torch.zeros((N_PREDATOR, SEQUENCE_LEN))
            tmp_mask[:, t] = 1
            cur_action = actor.forward_(propogate_history(history), tmp_mask).detach().view(-1)
            print('a', cur_action)

            if mode == 'train':
                # exploration with noise added to action
                cur_action = Actor.sample_action(cur_action, noiser.step())  # a_t + noise
            history[t + 1][:N_PREDATOR * 2] = cur_action

            # Send action of the game
            str_cur_action = Actor.to_str(cur_action)
            server_socket.send(bytes(str_cur_action, 'utf-8'))

            prev_state = cur_state
            t += 1

    return history, reward_hist


def main():

    # choose device
    if torch.cuda.is_available():
        device = torch.device('cuda')
    else:
        device = torch.device('cpu')

    print('[*] Current device:', device)

    # actor
    actor_net = Actor()
    actor_net.to(device=device)
    # target actor
    actor_net_target = Actor()
    actor_net_target.to(device=device)

    # critic
    critic_net = Critic()
    critic_net.to(device=device)
    # target critic
    critic_net_target = Critic()
    critic_net_target.to(device=device)

    # init target
    hard_update(actor_net_target, actor_net)
    hard_update(critic_net_target, critic_net)

    print('[*] Networks initialized')

    # optim
    actor_optim = optim.Adam(actor_net.parameters(), lr=1e-4, weight_decay=1e-5)
    critic_optim = optim.Adam(critic_net.parameters(), lr=1e-4, weight_decay=1e-5)

    noiser = NoiseScheduler(NOISE_SCALE, 0.05, 50000)
    memory = Buffer(buffer_size=MEMORY_SIZE, batch_size=BATCH_SIZE)

    bank_dir = os.path.join('./bank/%s' % datetime.now().strftime("%Y%m%d-%H%M%S"))
    os.mkdir(bank_dir)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.connect((HOST, PORT))
        print('[*] Server connected')

        print('[*] Running')
        for i in range(EPISODES):
            for j in range(MEMORY_SIZE):
                history, rewards = run_game(actor_net, server_socket, noiser, device)
                memory.append_track(history, rewards)
            train(actor_net, critic_net, actor_net_target, critic_net_target, actor_optim, critic_optim, memory)

            eval_history, eval_rewards = run_game(actor_net, server_socket, noiser, device, 'eval')
            print('E%d:%d' % (i, len(eval_rewards)))

            memory.save_tracks(os.path.join(bank_dir, '%d-%d.npy' % (i, j)))
            memory.clear_tracks()

if __name__ == '__main__':

    #main()

    # choose device
    if torch.cuda.is_available():
        device = torch.device('cuda')
    else:
        device = torch.device('cpu')

    # actor
    actor_net = Actor()
    actor_net.to(device=device)

    # target actor
    actor_net_target = Actor()
    actor_net_target.to(device=device)

    # critic
    critic_net = Critic()
    critic_net.to(device=device)

    # target critic
    critic_net_target = Critic()
    critic_net_target.to(device=device)

    lr_scheduler = LRScheduler(0.0001, 50000)
    noiser = NoiseScheduler(NOISE_SCALE, 0.05, 50000)

    # init target
    hard_update(actor_net_target, actor_net)
    hard_update(critic_net_target, critic_net)

    actor_net_target.eval()
    critic_net_target.eval()

    # optim
    actor_optim = optim.Adam(actor_net.parameters(), lr=0.0001, weight_decay=1e-5)
    critic_optim = optim.Adam(critic_net.parameters(), lr=0.0001, weight_decay=1e-5)

    # memory bank
    buffer = Buffer(buffer_size=MEMORY_SIZE, batch_size=BATCH_SIZE)

    # tensorboard logger
    tb = Logger('log').writer

    # create socket connection
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:

        # connect server
        s.connect((HOST, PORT))
        while True:

            # Run model
            iteration = 0
            N_tracks = 0
            for i in range(EPISODES):

                is_eval = (i % EVAL_PER_TRAIN) == 0

                s.send(b'start')
                model_env = ModelEnv(s.recv(1024))

                # Loop till the game ends
                T = 0
                N_timestamps = 0
                prev_state = None
                prev_action = None
                cur_state = None
                cur_action = None
                R = 0
                while True:
                    actor_net.eval()

                    T += 1
                    # Receive states of the game
                    msg = Message(s.recv(1024))

                    if msg.require_action or msg.is_terminal:

                        # retrieve current state
                        cur_state = toTensor(msg.state, device) # s_t

                        # add training pair
                        if T > 1:
                            prev_reward = model_env.compute_reward(cur_state, prev_state) if not msg.is_terminal else -1.0  # r_t
                            w = math.pow(2, prev_reward)
                            w = w + 2 if w >= 1.0 else w
                            prev_reward = toTensor(np.array([prev_reward]), device) + 0.2
                            R = prev_reward + gamma * R
                            buffer.append(w, [prev_state, prev_action, prev_reward, cur_state])

                        # game terminated
                        if msg.is_terminal:
                            break

                        # compute next action
                        cur_action = actor_net(propagate_state(cur_state)).detach().view(-1)
                        #cur_action = actor_net.forward_(*pad_sequence(history)).detach()[0]
                        #cur_action = actor_net(cur_state.unsqueeze(0)).detach()[0] # a_t
                        if not is_eval:
                            # exploration with noise added to action
                            cur_action = Actor.sample_action(cur_action, noiser.step()) # a_t + noise

                        # Send action of the game
                        str_cur_action = Actor.to_str(cur_action)
                        s.send(bytes(str_cur_action, 'utf-8'))

                        prev_state = cur_state
                        prev_action = cur_action

                        # train actor/critic
                        with torch.autograd.set_detect_anomaly(True):
                            if buffer.ready and not is_eval:
                                lr = lr_scheduler.step()
                                actor_optim.lr = lr * 0.1
                                critic_optim.lr = lr
                                s_t, a_t, r_t, s_t_ = buffer.get_batch()

                                #q_t = critic_net(s_t, a_t)
                                #q_t_ = critic_net_target(s_t_, actor_net_target(s_t_).detach()).detach()
                                critic_optim.zero_grad()
                                q_t_ = critic_net_target(sub_tensor(s_t_, 4, 0, 2), actor_net_target(batch_propagate(s_t_)).detach().view(BATCH_SIZE, -1)).detach()
                                q_t = critic_net(sub_tensor(s_t, 4, 0, 2), a_t)
                                y = r_t + q_t_ * gamma
                                y = torch.clamp(y, -5.0, 5.0)
                                l_q = ((y - q_t)**2).mean()
                                l_q.backward()
                                torch.nn.utils.clip_grad_value_(critic_net.parameters(), 0.5)
                                critic_optim.step()

                                actor_net.train()
                                actor_optim.zero_grad()
                                #q = -critic_net(s_t, actor_net(s_t)).mean()
                                l_v = -critic_net(sub_tensor(s_t, 4, 0, 2), actor_net(batch_propagate(s_t)).view(BATCH_SIZE, -1)).mean()
                                l_v.backward()
                                torch.nn.utils.clip_grad_value_(actor_net.parameters(), 0.5)
                                actor_optim.step()

                                soft_update(actor_net_target, actor_net, tau)
                                soft_update(critic_net_target, critic_net, tau)

                                positive_weight_ratio, positive_n_ratio = buffer.statistics()
                                tb.add_scalar('p_w', positive_weight_ratio, iteration)
                                tb.add_scalar('p_n', positive_n_ratio, iteration)
                                tb.add_scalar('train/l_q', l_q, iteration)
                                tb.add_scalar('train/l_v', l_v, iteration)
                                iteration += 1
                        N_timestamps += 1
                N_tracks += 1

                # logger
                if is_eval:
                    tb.add_scalar('eval/game_t', T, i)
                    tb.add_scalar('eval/r', R, i)
                else:
                    tb.add_scalar('train/game_t', T, i)
                    tb.add_scalar('train/r', R, i)
                tb.add_scalar('n_actions', N_timestamps / N_tracks, i)
