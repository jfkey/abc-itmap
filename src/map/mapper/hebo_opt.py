from hebo.design_space.design_space import DesignSpace
from hebo.optimizers.hebo import HEBO
import numpy as np
import pandas as pd 
from typing import List
import torch 


  
def obj(params : pd.DataFrame) -> np.ndarray:
    return ((params.values - 0.50)**2).sum(axis = 1).reshape(-1, 1)

def init_opt():
    np.random.seed(42)
    torch.manual_seed(42)
    params = [
        {'name' : 'delay_para0', 'type' : 'num', 'lb' : 0.0, 'ub' : 1.0},
        {'name' : 'delay_para1', 'type' : 'num', 'lb' : 0.0, 'ub' : 0.5},
        {'name' : 'delay_para2', 'type' : 'num', 'lb' : 0.0, 'ub' : 0.5},
        {'name' : 'delay_para3', 'type' : 'num', 'lb' : 0.0, 'ub' : 1.0},
        {'name' : 'delay_para4', 'type' : 'num', 'lb' : 0.5, 'ub' : 2.0},
        {'name' : 'delay_para5', 'type' : 'num', 'lb' : 0.0, 'ub' : 0.5},
        {'name' : 'delay_para6', 'type' : 'num', 'lb' : 0.0, 'ub' : 0.5},
        {'name' : 'delay_para7', 'type' : 'num', 'lb' : 0.0, 'ub' : 1.0},
        {'name' : 'delay_para8', 'type' : 'num', 'lb' : 0.5, 'ub' : 2.0},
        {'name' : 'delay_para9', 'type' : 'num', 'lb' : 0.0, 'ub' : 1.0}    
    ]
    space = DesignSpace().parse(params)
    #space.sample(5)
    # cfg = {
    #                     'lr'           : 0.01,
    #                     'num_epochs'   : 100,
    #                     'verbose'      : False,
    #                     'noise_lb'     : 8e-2, 
    #                     'pred_likeli'  : False
    #                     }
    # opt = HEBO(space, model_name='gpy', rand_sample= 8, model_config=cfg )
    opt = HEBO(space, model_name='rf', rand_sample= 5)
    return opt

 

def iterate_opt(opt, i_iter,  given_rec_x : List[float], given_rec_y : List[float]):
    if (i_iter == -1): 
        column_names = [f'delay_para{i}' for i in range(len(given_rec_x))]
        initial_points = pd.DataFrame([given_rec_x], columns=column_names)
        initial_evaluations = np.array(given_rec_y) 
        opt.observe(initial_points, initial_evaluations)
        return opt, given_rec_x 
    elif (i_iter ==0):
        rec_x = opt.suggest(n_suggestions = 1)
        rec_x_list = rec_x.values.tolist()[0] 
        return opt, rec_x_list
    else:
        column_names = [f'delay_para{i}' for i in range(len(given_rec_x))]
        given_rec_x = pd.DataFrame([given_rec_x], columns=column_names)
        given_rec_y = np.array(given_rec_y) 
        opt.observe(given_rec_x, given_rec_y)
        rec_x = opt.suggest(n_suggestions = 1)
        rec_x_list = rec_x.values.tolist()[0] 
        return opt, rec_x_list 
    

def best_x(opt):
    return opt.best_x

def best_y(opt):
    return opt.best_y
  
  
# def fun_a(): 

#     rec_x1 = [0.356135,0.548158,0.554104,0.592865,0.52982,0.56736,0.590789,0.571867,0.702422,0.915102]
#     rec_x2 = [0.356135,0.148158,0.454104,0.792865,0.762982,0.046736,0.390789,0.171867,0.702422,0.915102]
#     rec_y1 = [0.2666859]
#     rec_y2 = [0.8398567]


#     opt = init_opt() 
#     given_rec_x = []
#     given_rec_y = []
#     max_iter = 10
#     # init points
#     opt, _ = iterate_opt(opt, -1, rec_x1, rec_y1)
#     opt, _ = iterate_opt(opt, -1, rec_x2, rec_y2)


#     for i in range(0, max_iter):  
#         opt, rec_x = iterate_opt(opt, i, given_rec_x, given_rec_y)
#         given_rec_x = rec_x
#         given_rec_y = obj(pd.DataFrame([rec_x])) 
#         print("i {}, y {}".format(i,given_rec_y))
#         # opt, rec_x = iterate_opt(opt, i, given_rec_x, given_rec_y)
#         # given_rec_x = rec_x
#         # given_rec_y = obj(pd.DataFrame([rec_x])) 
#         # print("i {}, y {}".format(i,given_rec_y))
   
# if __name__ == '__main__':
#     while True:
#         user_input = input("Enter 1 to execute fun_a(), 2 to execute fun_a(), anything else to quit:\n")
#         if user_input == "1":
#             fun_a()
#         elif user_input == "2":
#             fun_a()
#         else:
#             print("Exiting.")
#             break

# conv_hebo_seq = np.minimum.accumulate(opt.y) 
# print(conv_hebo_seq)