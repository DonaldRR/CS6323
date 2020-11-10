import socket
import torch
from torch import optim
from tensorboardX import SummaryWriter

from config import *
from models import *
from environment import *
from utils import *
from wrappers import *

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

    # init target
    actor_net_target.load_state_dict(actor_net.state_dict())
    critic_net_target.load_state_dict(critic_net.state_dict())

    actor_net_target.eval()
    critic_net_target.eval()

    # optim
    actor_optim = optim.Adam(actor_net.parameters(), lr=0.005, weight_decay=1e-5)
    critic_optim = optim.Adam(critic_net.parameters(), lr=0.005, weight_decay=1e-5)

    # memory bank
    buffer = Buffer()

    # tensorboard logger
    tb = SummaryWriter()

    # create socket connection
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:

        # connect server
        s.connect((HOST, PORT))
        while True:

            # Run model
            for i in range(EPISODES):

                is_eval = (i % EVAL_PER_TRAIN) == 0

                rewards = []
                states = []
                actions = []
                Qs = []

                s.send(b'start')
                # print('sent1')
                model_env = ModelEnv(s.recv(1024))
                # print('received1')

                # Loop till the game ends
                T = 0
                while True:
                    actor_net.eval()
                    critic_net.eval()

                    T += 1
                    # Receive states of the game
                    msg = Message(s.recv(1024))
                    # print('received2')

                    cur_state = toTensor(msg.state, device) # s_t
                    prev_reward = [model_env.compute_reward(cur_state) if not msg.is_terminal else -1.0] # r_t
                    prev_reward = toTensor(np.array(prev_reward), device)
                    cur_action = actor_net(cur_state.unsqueeze(0))[0] # a_t
                    if not is_eval:
                        cur_action = Actor.sample_action(cur_action) # a_t + noise
                    #print('state:', cur_state, '->action:', cur_action)
                    cur_q = critic_net(cur_state.unsqueeze(0), cur_action.unsqueeze(0)) # q_t

                    if len(states) > 1:
                        buffer.append([states[-1], actions[-1], prev_reward, cur_state])

                    states.append(cur_state)
                    rewards.append(prev_reward)
                    actions.append(cur_action)
                    Qs.append(cur_q)

                    if msg.is_terminal:
                        break

                    # Send action of the game
                    #s.send(bytes(Actor.to_str(cur_action)))
                    dummy = Actor.to_str(cur_action)
                    s.send(bytes(dummy, 'utf-8'))
                    # print('sent2')

                    # train actor/critic
                    with torch.autograd.set_detect_anomaly(True):
                        if buffer.ready and not is_eval:
                            print(T)
                            s_t, a_t, r_t, s_t_ = buffer.get_batch()

                            critic_net.train()
                            #q_t = critic_net(s_t, a_t)
                            q_t_ = critic_net_target(s_t_, actor_net_target(s_t_))
                            q_t = critic_net(s_t, a_t)
                            y = r_t + gamma * q_t_
                            l_q = ((y - q_t)**2).mean()

                            critic_optim.zero_grad()
                            l_q.backward(retain_graph=True)
                            critic_optim.step()

                            critic_net.eval()
                            actor_net.train()
                            q = -critic_net(s_t, actor_net(s_t)).mean()
                            actor_optim.zero_grad()
                            q.backward()
                            actor_optim.step()

                            update_target(actor_net, actor_net_target, tau)
                            update_target(critic_net, critic_net_target, tau)

                # logger
                if is_eval:
                    tb.add_scalar('eval/game_t', T, i)
                else:
                    tb.add_scalar('train/game_t', T, i)