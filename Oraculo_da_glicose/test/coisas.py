import numpy as np
import pandas as pd
import os 

os.chdir('/home/andre/Documents/Git/PICOW_Codes/SBESC/Oraculo_da_glicose/test')

data = pd.read_csv('bb_predictions.csv')

with open("teste.txt", "w") as f:
    print("uint_16_t glicose[] ={", file=f)

    for i in range(50):
        print(f'{data["Predicted Glucose (mg/dL)"][i]}, ',file=f)
    print("};", file=f)