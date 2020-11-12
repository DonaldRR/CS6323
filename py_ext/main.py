import socket
import torch
from torch import optim
from tensorboardX import SummaryWriter

from config import *
from models import *
from environment import *
from utils import *
from wrappers import *

def propagate_state(state):

    """
    :param state: torch.Tensor
    :return:
    """

    states = []
    for i in range(N_PREDATOR):
        tmp = [state[i*2], state[i*2+1]]
        for j in range(N_PREDATOR):
            if i != j:
                tmp.extend([state[j*2], state[j*2+1]])

        for j in range(N_PREY):
            tmp.extend([state[(N_PREDATOR + j)*2], state[(N_PREDATOR + j)*2 + 1]])

        states.append(torch.tensor(tmp))
    states = torch.stack(states, dim=0)

    return states

def batch_propagate(states):

    batch_states = [propagate_state(states[i]) for i in range(states.shape[0])]
    batch_states = torch.cat(batch_states, dim=0)

    return batch_states

if __name__ == '__main__':

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

    lr_scheduler = LRScheduler(0.001, 20000)
    noiser = NoiseScheduler(NOISE_SCALE, 0.05, 20000)

    # init target
    hard_update(actor_net_target, actor_net)
    hard_update(critic_net_target, critic_net)

    actor_net_target.eval()
    critic_net_target.eval()

    # optim
    actor_optim = optim.Adam(actor_net.parameters(), lr=0.001, weight_decay=1e-5)
    critic_optim = optim.Adam(critic_net.parameters(), lr=0.001, weight_decay=1e-5)

    # memory bank
    buffer = Buffer(batch_size=BATCH_SIZE)

    # tensorboard logger
    tb = Logger('log').writer

    # create socket connection
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:

        # connect server
        s.connect((HOST, PORT))
        while True:

            # Run model
            iteration = 0
            for i in range(EPISODES):

                is_eval = (i % EVAL_PER_TRAIN) == 0

                s.send(b'start')
                # print('sent1')
                model_env = ModelEnv(s.recv(1024))
                # print('received1')

                # Loop till the game ends
                T = 0
                prev_state = None
                prev_action = None
                cur_state = None
                cur_action = None
                while True:
                    actor_net.eval()
                    critic_net.eval()

                    T += 1
                    # Receive states of the game
                    msg = Message(s.recv(1024))
                    # print('received2')

                    if msg.require_action or msg.is_terminal:

                        # retrieve current state
                        cur_state = toTensor(msg.state, device) # s_t

                        # add training pair
                        if T > 1:
                            prev_reward = [model_env.compute_reward(cur_state, prev_state) if not msg.is_terminal else -1.0]  # r_t
                            prev_reward = toTensor(np.array(prev_reward), device)
                            buffer.append(T, [prev_state, prev_action, prev_reward, cur_state])

                        # game terminated
                        if msg.is_terminal:
                            break

                        # compute next action
                        cur_action = actor_net(propagate_state(cur_state)).detach().view(-1)
                        #cur_action = actor_net(cur_state.unsqueeze(0)).detach()[0] # a_t
                        if not is_eval:
                            # exploration with noise added to action
                            print('s', cur_state, 'a', cur_action)
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
                                actor_optim.lr = lr
                                critic_optim.lr = lr
                                s_t, a_t, r_t, s_t_ = buffer.get_batch()

                                critic_net.train()
                                #q_t = critic_net(s_t, a_t)
                                #q_t_ = critic_net_target(s_t_, actor_net_target(s_t_).detach()).detach()
                                q_t_ = critic_net_target(s_t_, actor_net_target(batch_propagate(s_t_)).detach().view(BATCH_SIZE, -1)).detach()
                                q_t = critic_net(s_t, a_t)
                                y = r_t + gamma * q_t_
                                y = torch.clamp(y, -1.0, 1.0)
                                print('q_t_', q_t_, 'q_t', q_t)
                                l_q = ((y - q_t)**2).mean()

                                critic_optim.zero_grad()
                                l_q.backward()
                                torch.nn.utils.clip_grad_value_(critic_net.parameters(), 0.5)
                                critic_optim.step()

                                critic_net.eval()
                                actor_net.train()
                                #q = -critic_net(s_t, actor_net(s_t)).mean()
                                q = -critic_net(s_t, actor_net(batch_propagate(s_t)).view(BATCH_SIZE, -1)).mean()
                                actor_optim.zero_grad()
                                q.backward()
                                torch.nn.utils.clip_grad_value_(actor_net.parameters(), 0.5)
                                actor_optim.step()

                                soft_update(actor_net_target, actor_net, tau)
                                soft_update(critic_net_target, critic_net, tau)

                                tb.add_scalar('train/l_q', l_q, iteration)
                                iteration += 1

                # logger
                if is_eval:
                    tb.add_scalar('eval/game_t', T, i)
                else:
                    tb.add_scalar('train/game_t', T, i)