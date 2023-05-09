#Pyhton libraries
import numpy as np
import datetime
import sqlite3 as sql

#Import python files
import RSA

today = datetime.datetime.now() #Getting the current time and date

#This function returns the count in hours of users three activites of current day
def get_day_data_acc():
    
    global today
    today_day=today.strftime('%Y-%m-%d') #Converting to date only format
    conn = sql.connect('acc.db')
    c = conn.cursor()
    c.execute("SELECT prediction FROM acc WHERE date='{}'".format(today_day)) #Select rows having todays date
    fetched_data = c.fetchall()
    fetched_data = np.array(fetched_data, dtype=str) #Converting the tuple to numpy array
    count_sitting = np.count_nonzero(fetched_data == "Sitting")/60 #Calculating the number of hours users was sitting
    count_walking = np.count_nonzero(fetched_data == "Walking")/60 #Calculating the number of hours users was walking
    count_running = np.count_nonzero(fetched_data == "Running")/60 #Calculating the number of hours users was running
    return count_sitting,count_walking,count_running

#This function returns the count in hours of users three activites of current week
def get_week_data_acc():

    global today
    today_day=today.strftime('%Y-%m-%d') #Converting to date only format
    dt = datetime.datetime.strptime(today_day, '%Y-%m-%d')
    week_start = dt - datetime.timedelta(days=dt.weekday()) #Calculating the current week's starting date
    week_end = week_start + datetime.timedelta(days=6) #Calculating the current week's ending date
    week_start = week_start.strftime('%Y-%m-%d')
    week_end = week_end.strftime('%Y-%m-%d')
    conn = sql.connect('acc.db')
    c = conn.cursor()
    #Selecting values of prediction between current week start and end date
    c.execute('SELECT prediction from acc WHERE date BETWEEN ? AND ?', (week_start, week_end))
    fetched_data = c.fetchall()
    fetched_data = np.array(fetched_data, dtype=str) #Converting the tuple to numpy array
    count_sitting = np.count_nonzero(fetched_data == "Sitting")/60 #Calculating the number of hours users was sitting
    count_walking = np.count_nonzero(fetched_data == "Walking")/60 #Calculating the number of hours users was walking
    count_running = np.count_nonzero(fetched_data == "Running")/60 #Calculating the number of hours users was running
    return count_sitting,count_walking,count_running

#This function returns the count in hours of users three activites of current month
def get_month_data_acc():

    global today
    today_day=today.strftime('%Y-%m-%d') #Converting to date only format
    start_month=datetime.datetime.strptime(today_day, '%Y-%m-%d').replace(day=1) #Calculating the current month's starting date
    next_month = datetime.datetime.strptime(today_day, '%Y-%m-%d').replace(day=28) + datetime.timedelta(days=4)
    end_month = next_month - datetime.timedelta(days=next_month.day) #Calculating the current month's ending date
    start_month = start_month.strftime('%Y-%m-%d')
    end_month = end_month.strftime('%Y-%m-%d')
    conn = sql.connect('acc.db')
    c = conn.cursor()
    #Selecting values of prediction between current month start and end date
    c.execute('SELECT prediction from acc WHERE date BETWEEN ? AND ?', (start_month, end_month))
    fetched_data = c.fetchall()
    fetched_data = np.array(fetched_data, dtype=str) #Converting the tuple to numpy array
    count_sitting = np.count_nonzero(fetched_data == "Sitting")/60 #Calculating the number of hours users was sitting
    count_walking = np.count_nonzero(fetched_data == "Walking")/60 #Calculating the number of hours users was walking
    count_running = np.count_nonzero(fetched_data == "Running")/60 #Calculating the number of hours users was running
    return count_sitting,count_walking,count_running

#Returns the number of rows of a particular column from main db
def Get_count_row_main(col_name):

    conn = sql.connect('data.db')
    c = conn.cursor()
    c.execute('SELECT COUNT({}) from users'.format(col_name))
    result = c.fetchone()
    N =result[0]
    conn.commit()
    conn.close()
    return N

#Returns the number of rows of a particular column from accelerometer db
def Get_count_row_acc(col_name):

    conn = sql.connect('acc.db')
    c = conn.cursor()
    c.execute('SELECT COUNT({}) from acc'.format(col_name))
    result = c.fetchone()
    N =result[0]
    conn.commit()
    conn.close()
    return N

#Creates a new main db file if not existing
def create_main_db():

    conn = sql.connect('data.db')
    c = conn.cursor()
    c.execute("""CREATE TABLE if not exists users(
        data_username text,
        data_password text,
        data_name text,
        data_age int,
        data_weight text,
        data_height text,
        data_job text,
        data_gender text
        )
        """)
    conn.commit()
    conn.close()

def create_motivation_task_db():

    conn = sql.connect('Mtask.db')
    c = conn.cursor()
    c.execute("""CREATE TABLE if not exists Mtask(
        AGE int,
        T1 text,
        T2 text,
        T3 text,
        T4 text
        )
        """)
    conn.commit()
    conn.close()

#Get tasks from motivation task database.
def get_motivation_task(col_name,age_row):
    conn = sql.connect('Mtask.db')
    c = conn.cursor()
    c.execute("SELECT {} FROM Mtask WHERE AGE = {}".format(col_name,age_row))
    fetched_data = c.fetchone()
    fetched_data = fetched_data[0]
    return fetched_data

#Get latest value of column from users database
def get_latest_item_main(col_name):
    conn = sql.connect('data.db')
    c = conn.cursor()
    c.execute("SELECT {} FROM users ORDER BY rowid DESC LIMIT 1".format(col_name))
    fetched_data = c.fetchone()
    fetched_data = fetched_data[0]
    return fetched_data

#Returns the data stored in a particular column from main db
def Get_items_main(item_name):

    conn = sql.connect('data.db')
    c = conn.cursor()   
    c.execute('SELECT {} FROM users'.format(item_name))
    fetched_data = c.fetchall()
    conn.commit()
    conn.close()
    return fetched_data

#Returns the data stored in a particular column from accelerometer db
def Get_items_acc(item_name):

    conn = sql.connect('acc.db')
    c = conn.cursor()   
    c.execute('SELECT {} FROM acc'.format(item_name))
    fetched_data = c.fetchall()
    conn.commit()
    conn.close()
    return fetched_data

#Creates a new accelerometer db file if not existing
def create_acc_db():

    conn = sql.connect('acc.db')
    c = conn.cursor()
    c.execute("""CREATE TABLE if not exists acc(
        date text,
        prediction text,
        X_val float,
        Y_val float,
        Z_val float
        )
        """)
    conn.commit()
    conn.close()

#Inserts data of particular column into main db
def Insert_items_main(username_cipher,password_cipher,name_cipher,age_cipher,weight_cipher,height_cipher,job_cipher,gen_cipher):

    conn = sql.connect('data.db')
    c = conn.cursor()
    c.execute("INSERT INTO users (data_username,data_password,data_name,data_age,data_weight,data_height,data_job,data_gender) VALUES(?,?,?,?,?,?,?,?)", 
        (username_cipher,password_cipher,name_cipher,age_cipher,weight_cipher,height_cipher,job_cipher,gen_cipher)
        )
    conn.commit()
    conn.close()

#Get saved accelerometer data form database
def last_n_rows_acc(N):

    x_val=[]
    y_val=[]
    z_val=[]
    fetched_data=[]
    conn = sql.connect('acc.db')
    c = conn.cursor()
    c.execute("SELECT X_val,Y_val,Z_val FROM acc ORDER BY rowid DESC LIMIT {} OFFSET 1".format(N)) #change here
    value = c.fetchall()
    # Coverting Data which is compatible with input of my GRU model
    for i in range(0,12):
        XX = value[i][0]
        YY = value[i][1]
        ZZ = value[i][2]
        x_val.append(XX)
        y_val.append(YY)
        z_val.append(ZZ)
    fetched_data.append([x_val, y_val, z_val]) #3D shaping
    fetched_data = np.array(fetched_data, dtype=np.float32) #Converting to 3D npArray with dtype=32
    #Reshaping accelerometer data which compatible with my GRU model
    reshaped_Record = np.asarray(fetched_data, dtype= np.float32).reshape(-1, 12, 3)
    return reshaped_Record #Returning the final 3D Array input

#Inserts data of accelerometer into accelerometer db
def Insert_items_acc(X_val,Y_val,Z_val):
    global today
    date=today.strftime('%Y-%m-%d')
    conn = sql.connect('acc.db')
    c = conn.cursor()
    c.execute("INSERT INTO acc (date,X_val,Y_val,Z_val) VALUES(?,?,?,?)", 
        (date,X_val,Y_val,Z_val)
        )
    conn.commit()
    conn.close()

#Inserts data of prediction into accelerometer db
def Insert_prediction_acc(prediction):
    global today
    date=today.strftime('%Y-%m-%d')
    conn = sql.connect('acc.db')
    c = conn.cursor()
    c.execute("INSERT INTO acc (date,prediction,X_val,Y_val,Z_val) VALUES(?,?,?,?,?)", 
        (date,prediction,None,None,None)
        )
    conn.commit()
    conn.close()

