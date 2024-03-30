from hebo.design_space.design_space import DesignSpace
from hebo.optimizers.hebo import HEBO
import numpy as np
import pandas as pd 
from typing import List

  
def obj(params : pd.DataFrame) -> np.ndarray:
    return ((params.values - 0.50)**2).sum(axis = 1).reshape(-1, 1)

def init_opt():
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
    opt = HEBO(space, model_name='gp', rand_sample= None )
    return opt
 
 

def iterate_opt(opt, i_iter,  given_rec_x : List[float], given_rec_y : List[float]):
    if (i_iter ==0):
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
  
  
# opt = init_opt() 
# given_rec_x = [0.356135,0.148158,0.454104,0.792865,0.762982,0.046736,0.390789,0.171867,0.702422,0.915102]
# given_rec_y = obj(pd.DataFrame([given_rec_x])) 
# opt, rec_x =  iterate_opt(opt, -1, given_rec_x, given_rec_y)
# print("given_rec_y",given_rec_y)

# given_rec_x = rec_x
# given_rec_y = obj(pd.DataFrame([given_rec_x])) 

 
# opt = init_opt() 
# given_rec_x = []
# given_rec_y = []
# max_iter = 15
# for i in range(0, max_iter):  
#     if (i == 0):
#         rec_x = [0.356135,0.148158,0.454104,0.792865,0.762982,0.046736,0.390789,0.171867,0.702422,0.915102]
#         given_rec_x = rec_x
#         given_rec_y = obj(pd.DataFrame([rec_x]))
#         opt, rec_x =  iterate_opt(opt, -1, given_rec_x, given_rec_y)
        
#     else:

#         opt, rec_x = iterate_opt(opt, i, given_rec_x, given_rec_y)
#         given_rec_x = rec_x
#         given_rec_y = obj(pd.DataFrame([rec_x])) 
#         print("i {}, y {}".format(i,given_rec_y))
   
  
# conv_hebo_seq = np.minimum.accumulate(opt.y) 
# print(conv_hebo_seq)