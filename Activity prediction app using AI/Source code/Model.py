#Importing python libraries
import tensorflow as tf

#importing python files
import sqlite

#Defining global variables
input_shape=''
output_shape=''
interpreter=''

#Predicting the output class
def predict():

    reshaped_Record = sqlite.last_n_rows_acc(12) #Getting the lastest 12 rows of accelerometer database from sql
    interpreter.set_tensor(input_shape[0]['index'], reshaped_Record) #Feeding the value to the Model
    interpreter.invoke()
    output_data = interpreter.get_tensor(output_shape[0]['index']) #Storing the output prediction
    Predicted_Class = Return_Class(output_data) #Sending the prediction to compare function
    return Predicted_Class #Returning the final Class as a string

#Loading the .tflite model
def Load_Model():
    global input_shape
    global output_shape
    global interpreter

    interpreter = tf.lite.Interpreter(model_path="Final_Acc_model.tflite")
    interpreter.allocate_tensors()

    input_shape = interpreter.get_input_details()
    output_shape = interpreter.get_output_details()

#Comparing the probaility of all three predicted classes and returning the class with the highest probability
def Return_Class(output_data):
    if output_data[0][0] > output_data[0][1] and output_data[0][0] > output_data[0][2]:
        return "Sitting"
    elif output_data[0][1] > output_data[0][0] and output_data[0][1] > output_data[0][2]:
        return "Walking"
    elif output_data[0][2] > output_data[0][1] and output_data[0][2] > output_data[0][0]:
        return "Running"