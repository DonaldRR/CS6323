import socket
from torch import optim
from tensorboardX import SummaryWriter

from config import *
from models import *
from environment import *
from utils import *
from wrappers import *

if __name__ == '__main__':

    actor_net = Actor()
    actor_net_target = Actor()
    actor_net_target.load_state_dict(actor_net.state_dict())
    actor_optim = optim.Adam(actor_net.parameters())

    critic_net = Critic()
    critic_net_target = Critic()
    critic_net_target.load_state_dict(critic_net.state_dict())
    critic_optim = optim.Adam(critic_net.parameters())

    buffer = Buffer()

    tb = SummaryWriter()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:

        s.connect((HOST, PORT))
        while True:

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
                    cur_state = toTensor(msg.state) # s_t
                    prev_reward = torch.tensor([model_env.compute_reward(cur_state.numpy()) if not msg.is_terminal else -1.0]) # r_t
                    cur_action = actor_net(cur_state.unsqueeze(0))[0] # a_t
                    if is_eval:
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
                    if buffer.ready and not is_eval:
                        actor_net.train()
                        critic_net.train()
                        s_t, a_t, r_t, s_t_ = buffer.get_batch()

                        #q_t = critic_net(s_t, a_t)
                        q_t = critic_net(s_t, a_t)
                        q_t_ = critic_net_target(s_t_, actor_net_target(s_t_))
                        y = r_t + gamma * q_t_
                        l_q = ((y - q_t)**2).mean()

                        critic_optim.zero_grad()
                        l_q.backward(retain_graph=True)
                        critic_optim.step()

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