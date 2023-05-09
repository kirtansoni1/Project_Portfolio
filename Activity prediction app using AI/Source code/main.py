'''
To test this app on PC like realtime I have given .csv data file of three classes Sitting, Walking, Running
it will be automatically feeded to the AI model when check-in, every 60 seconds. If you want to convert this
project to .apk then do change debugging to False.
'''

#Importing python libraries
from kivymd.app import MDApp
from kivy.lang import Builder
from kivy.clock import Clock
from kivy.core.window import Window
from kivymd.uix.menu import MDDropdownMenu
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy import platform
from plyer import accelerometer
import csv

#Importing pyhton Files
import RSA
import Notify
import sqlite
import Model
import Plot_Graph
import MotivationTask

'''***********************************************'''
debugging = True  #True - to test this app on PC
'''***********************************************'''

if debugging == True:
    r=0.25
else:
    r=1

#Resizing the window screen
Window.size = (1080*r, 2300*r)

#Loading the Public and Private Keys
pubKey, privKey = RSA.load_keys()

#Global variables
sitting_monitor=0  #For monitoring the sitting activity
notification_is_on=True #For turning notification ON/OFF globally(Default ON)
Prediction="" #Store the predicted value
notification=False #For notification toggle code (Define it once)
notify_hr_bool=True #variable to execute the notification per hour requirement
N_freq=0.5 #Notification frequency by default 1 in 2 hours
User_Name=""#To Store the logged-in username for showing the appropriate settings page data
PassWord=""#To Store the logged-in password for showing the appropriate settings page data
Checkbox1=False #Motivation task check box bool
Checkbox2=False
Checkbox3=False
Checkbox4=False
Enable_MotivationTask=False #To toggle the motivation task manager
Screen_bool=False #to stop analysis when logout of the app(return to login page)
sensor_available=False
Task_chg1=False #type of task to display as per user details
Task_chg2=False
Task_chg3=False
Task_chg4=False
test_t=0
t1=0 #Boolean to control the clock time of 
t2=0
t3=0
t4=0
init_once=True #To execute a code once on initializing screen

# Setting Permisions
if debugging == False:
    if platform == "android":
        from android.permissions import request_permissions, Permission
        request_permissions([Permission.WRITE_EXTERNAL_STORAGE, 
    Permission.READ_EXTERNAL_STORAGE])

#Create new account screen
class CreateAcc(Screen):
    def init_CreateAcc(self):
        print("Screen - Create Account")
    def create_save(self):
        #Call RSA python file to encrypt data
        cipher_username = RSA.encrypt(self.ids.Cname.text, pubKey)
        cipher_pass = RSA.encrypt(self.ids.Cpass.text, pubKey)
        #Inserting values to db
        sqlite.Insert_items_main(cipher_username,cipher_pass,None,None,None,None,None,None)
        self.parent.current='login'

#Login Screen
class Login(Screen):
    global debugging
    def init_Login(self):
        global Screen_bool
        if Screen_bool==True:
            s2 = self.manager.get_screen('mainpage')
            s2.model_analysis_stop()
            Screen_bool=False
        if debugging == True:
            print("Screen - Login")
    def verify_credentials(self):
        global User_Name
        global PassWord
        N=sqlite.Get_count_row_main('data_username') #Gets number of rows in a particular column
        NAME = sqlite.Get_items_main('data_username') #Gets record of columns and store in a variable
        PASS = sqlite.Get_items_main('data_password')
        USERNAME=''
        PASSWORD=''
        matched=False
        #Iterates through each row of db
        for i in range(0, N ,1):
            USERNAME = NAME[i][0]
            PASSWORD = PASS[i][0]
            #Store decrypted data in a variable
            decrypted_uname = RSA.decrypt(USERNAME, privKey)
            decrypted_pass = RSA.decrypt(PASSWORD, privKey)
            #Checks whether the matching credentials are present in local database or not
            if decrypted_uname==self.ids.user.text and decrypted_pass==self.ids.Pass.text:
                matched=True
                User_Name=decrypted_uname
                PassWord=decrypted_pass
                decrypted_uname=''
                decrypted_pass=''
                break

        if matched==True:
            self.ids.approved.text ="Welcome!"
            self.ids.approved.text_color = 2/255,168/255,63/255,1
            #Call notification python file
            Notify.notify_me('SignIn approved','Looking Fit!','MainApp','app.ico')
            #Change the screen to mainpage
            self.parent.current='mainpage'
            matched=False
                
        else:
            self.ids.approved.text ="Wrong Username/Password"
            self.ids.approved.text_color = 1,0,0,1
            matched=False
                
#Main Page of the Application
class MainPage(Screen):

    global notification_is_on
    global Prediction
    global Checkbox1,Checkbox2,Checkbox3,Checkbox4

    def chng_cb(self):
        global Task_chg1,Task_chg2,Task_chg3,Task_chg4,N_freq,init_once,debugging,Screen_bool
        Screen_bool=True
        if debugging == True:
            print("Screen - MainPage")
        if init_once == True:
            s2 = self.manager.get_screen('setting11')
            s2.N_Hours(N_freq)
            init_once=False
        Record = sqlite.get_latest_item_main("data_age") #Get the settings of user to chnage Tasks 
        decrypted_Record = RSA.decrypt(Record, privKey) #Decrypt the settings
        if decrypted_Record is "": #If empty then load default tasks_update
            decrypted_Record=20
        if type(decrypted_Record) is not int: #If settings like age are in str then convert to int
            decrypted_Record=int(decrypted_Record)
        if decrypted_Record <= 14:
            Task_chg1 = True
            Task_chg2 = False
            Task_chg3 = False
            Task_chg4 = False
        elif decrypted_Record >14 and decrypted_Record <= 45:
            Task_chg2 = True
            Task_chg1 = False
            Task_chg3 = False
            Task_chg4 = False
        elif decrypted_Record > 45 and decrypted_Record <= 55:
            Task_chg3 = True
            Task_chg2 = False
            Task_chg1 = False
            Task_chg4 = False
        elif decrypted_Record > 55 and decrypted_Record <= 150:
            Task_chg4 = True
            Task_chg2 = False
            Task_chg3 = False
            Task_chg1 = False
        Task1 = sqlite.get_motivation_task("T1",decrypted_Record) #Get tasks_update from motivation task manager
        Task2 = sqlite.get_motivation_task("T2",decrypted_Record)
        Task3 = sqlite.get_motivation_task("T3",decrypted_Record)
        Task4 = sqlite.get_motivation_task("T4",decrypted_Record)
        self.ids.motivation_text1.text = str(Task1) #Change checkbox text in GUI
        self.ids.motivation_text2.text = str(Task2)
        self.ids.motivation_text3.text = str(Task3)
        self.ids.motivation_text4.text = str(Task4)

    #Called every 1 minute or 60 Seconds
    def read_accelerometer(self, dt):
        
        Tx=12 #Setting the Tx
        global Prediction, debugging, sensor_available
        global sitting_monitor, test_t

        if debugging == True:
            if test_t==3:
                test_t=0
            test_data=["sitting-test.csv","walking-test.csv","running-test.csv"]
            print("feeding - {} file to the model".format(test_data[test_t]))
            Tx=12+1

        # Accelerometer reading
        for i in range(0,Tx,1):
            if i<=Tx:
                if sensor_available == True:
                    #Get raw values from the accelerometer sensor
                    Acc_val = accelerometer.acceleration[:3]
                    #Only store Accelerometer values if they are available
                    if not Acc_val == (None, None, None):
                        Acc_x=Acc_val[0]
                        Acc_y=Acc_val[1]
                        Acc_z=Acc_val[2]
                        sqlite.Insert_items_acc(Acc_x,Acc_y,Acc_z)
                else:
                    with open(test_data[test_t], newline='') as f:
                        reader = csv.reader(f)
                        Acc_val = [tuple(row) for row in reader]
                        Acc_x=Acc_val[i][0]
                        Acc_y=Acc_val[i][1]
                        Acc_z=Acc_val[i][2]
                        sqlite.Insert_items_acc(Acc_x,Acc_y,Acc_z)
            else:
                pass
        test_t=test_t+1
        Prediction = Model.predict() #Predicting the movement of user
        sqlite.Insert_prediction_acc(Prediction) #Saving the prediction in the sqlite
        self.ids.prd_val.text ="You are actually [b]{}[/b].".format(Prediction) #Displaying the prediction in GUI

        #Notification will be generated if the user is sitting for 2 hours straight
        if Prediction=="Sitting":
            sitting_monitor = sitting_monitor + 1
            if sitting_monitor>120: #If user is in same profile for 120 minutes then send notification
                if notification_is_on==True: #Will only notify if notifications is set to True
                    Notify.notify_me('Sitting for too long!', 'Please standup and move','MainApp','app.ico')
                    #Will show a hint to user in GUI to change profile
                    self.ids.profile_hint.text="     Hint: You should change your profile."
                    sitting_monitor=0
                else:
                    pass
            else:
                pass
        else:
            sitting_monitor=0

    def on_checkbox1_active(self, checkbox, value):
        global Enable_MotivationTask,Checkbox1
        print('Task1 -',value)
        if value and Enable_MotivationTask:
            Checkbox1=True
        else:
            Checkbox1=False
            self.Motivationtask_remind("CB")
    def on_checkbox2_active(self, checkbox, value):
        global Enable_MotivationTask,Checkbox2
        print('Task2 -',value)
        if value and Enable_MotivationTask:
            Checkbox2=True
        else:
            Checkbox2=False
            self.Motivationtask_remind("CB")
    def on_checkbox3_active(self, checkbox, value):
        global Enable_MotivationTask,Checkbox3
        print('Task3 -',value)
        if value and Enable_MotivationTask:
            Checkbox3=True
        else:
            Checkbox3=False
            self.Motivationtask_remind("CB")
    def on_checkbox4_active(self, checkbox, value):
        global Enable_MotivationTask,Checkbox4
        print('Task4 -',value)
        if value and Enable_MotivationTask:
            Checkbox4=True
        else:
            Checkbox4=False
            self.Motivationtask_remind("CB")

    #Set reminder of motivational task
    def Motivationtask_remind(self,check):
        global Checkbox1,Checkbox2,Checkbox3,Checkbox4,Enable_MotivationTask,notification_is_on
        global debugging
        global t1,t2,t3,t4,notify_hr_bool
        if Enable_MotivationTask==True:
            if Checkbox1==True:
                t1,t2,t3,t4=self.tasks_update()
                MotivationTask.Clock_checkbox(1,True,t1,t2,t3,t4)
                if debugging ==True:
                    print("Reminder set of Task1")
            else:
                t1,t2,t3,t4=self.tasks_update()
                MotivationTask.Clock_checkbox(1,False,t1,t2,t3,t4)
                if debugging ==True:
                    print("Reminder removed of Task1")
            if Checkbox2==True:
                t1,t2,t3,t4=self.tasks_update()
                MotivationTask.Clock_checkbox(2,True,t1,t2,t3,t4)
                if debugging ==True:
                    print("Reminder set of Task2")
            else:
                t1,t2,t3,t4=self.tasks_update()
                MotivationTask.Clock_checkbox(2,False,t1,t2,t3,t4)
                if debugging ==True:
                    print("Reminder removed of Task2")
            if Checkbox3==True:
                t1,t2,t3,t4=self.tasks_update()
                MotivationTask.Clock_checkbox(3,True,t1,t2,t3,t4)
                if debugging ==True:
                    print("Reminder set of Task3")
            else:
                t1,t2,t3,t4=self.tasks_update()
                MotivationTask.Clock_checkbox(3,False,t1,t2,t3,t4)
                if debugging ==True:
                    print("Reminder removed of Task3")
            if Checkbox4==True:
                t1,t2,t3,t4=self.tasks_update()
                MotivationTask.Clock_checkbox(4,True,t1,t2,t3,t4)
                if debugging ==True:
                    print("Reminder set of Task4")
            else:
                t1,t2,t3,t4=self.tasks_update()
                MotivationTask.Clock_checkbox(4,False,t1,t2,t3,t4)
                if debugging ==True:
                    print("Reminder removed of Task4")

            if notification_is_on==True:
                if check is not "CB":
                    Notify.notify_me('Reminder Set', 'Lets do this!','MainApp','app.ico')
                else:
                    Notify.notify_me('Reminder removed', 'See you again!','MainApp','app.ico')
            else:
                pass
        else:
            pass
    
    def tasks_update(self):#Settng the tasks according to the user settings in motivation database
        global Task_chg1,Task_chg2,Task_chg3,Task_chg4
        global t1,t2,t3,t4
        if Task_chg1 == True:
            t1=5
            t2=10
            t3=5
            t4=10
            Task_chg1=False
        if Task_chg2 == True:
            t1=15
            t2=30
            t3=20
            t4=30
            Task_chg2=False
        if Task_chg3 == True:
            t1=15
            t2=20
            t3=30
            t4=40
            Task_chg3=False
        if Task_chg4 == True:
            t1=10
            t2=15
            t3=10
            t4=15
            Task_chg4=False
        return t1,t2,t3,t4

    def model_analysis_start(self):
        global Enable_MotivationTask
        global debugging,sensor_available
        try:
            #Enable accelerometer sensor
            accelerometer.enable()
        except:
            sensor_available=False
            print("Accelerometer sensor not available")
        else:
            sensor_available=True

        if debugging==True: 
            print("Movement Analysis - enabled")
            print("Task Manager - enabled")
        #Schedule a clock interval which will call accelerometer and AI model as per the time interval(s), 60 seconds in my case
        Clock.schedule_once(self.read_accelerometer)
        Clock.schedule_interval(self.read_accelerometer, 60)
        Enable_MotivationTask=True
        
    def model_analysis_stop(self):
        global Enable_MotivationTask,Checkbox1,Checkbox2,Checkbox3,Checkbox4,debugging,sensor_available
        try:
            #Disable accelerometer sensor
            accelerometer.disable()
        except:
            print("Accelerometer not available")
            sensor_available=False
        else:
            sensor_available=True

        if debugging==True:    
            print("Movement Analysis - disabled")
            print("Task Manager - Disabled")
            print("Reminder - Removed")
            
        #Unschedule a clock interval which will stop the AI model and accelerometer sensor
        Clock.unschedule(self.read_accelerometer)

        Checkbox1=False
        Checkbox2=False
        Checkbox3=False
        Checkbox4=False
        self.Motivationtask_remind("CB")
        Enable_MotivationTask=False #Disableing the Motivation Task Manager
        self.ids.cb1.state='normal' #Unchecking all the Checkboxes in GUI
        self.ids.cb2.state='normal'
        self.ids.cb3.state='normal'
        self.ids.cb4.state='normal'
    
    def Notification_Toggle(self):
        global notification_is_on
        global notification, debugging
        #Notification toggle button code
        if notification==True:
            self.ids.notify.text="Notifications ON"
            if debugging == True:
                print("Notification - ON")
            Notify.notify_me('Notifications are ON', 'Ding Dong!','MainApp','app.ico')  #Call notification python file
            notification = False #Notification toggle bool
            notification_is_on=True #Turn ON Notifications globally
            MotivationTask.Update_notification_state(notification_is_on,notify_hr_bool)
        else:
            self.ids.notify.text="Notifications OFF"
            if debugging == True:
                print("Notification - OFF")
            notification = True #Notification toggle bool
            notification_is_on=False #Turn OFF Notifications globally
            MotivationTask.Update_notification_state(notification_is_on,notify_hr_bool)
            

#Setting Screen of the Application
class Settings11(Screen):
    global User_Name, notify_hr_bool,PassWord, debugging

    def add_settings(self):
        #Call RSA python file to encrypt data
        encrypted_name = RSA.encrypt(self.ids.kv_name.text, pubKey)
        encrypted_uname = RSA.encrypt(User_Name, pubKey)
        encrypted_pass = RSA.encrypt(PassWord, pubKey)
        encrypted_age = RSA.encrypt(self.ids.kv_age.text, pubKey)
        encrypted_weight = RSA.encrypt(self.ids.kv_weight.text, pubKey)
        encrypted_height = RSA.encrypt(self.ids.kv_height.text, pubKey)
        encrypted_job = RSA.encrypt(self.ids.kv_job.text, pubKey)
        encrypted_gen = RSA.encrypt(self.ids.kv_gen.text, pubKey)
        #Insert the encrypted data in db
        sqlite.Insert_items_main(encrypted_uname,encrypted_pass,encrypted_name,encrypted_age,encrypted_weight,encrypted_height,encrypted_job,encrypted_gen)
        if debugging == True:
            print("Settings - saved")

    def show_settings(self):
        global sensor_available,debugging
        if sensor_available==True:
            self.ids.sensor_avail.text = "Sensors: Accelerometer"
        else:
            self.ids.sensor_avail.text = "Sensors: None"
        if debugging==True:
            print("Screen - Settings")
        #Get number of rows of a particular column
        No=sqlite.Get_count_row_main('data_name')
        #Get all data from db
        Record = sqlite.Get_items_main('*')
        #Call RSA python file to decrypt data
        decrypted_name = RSA.decrypt(Record[No][2], privKey)
        decrypted_age = RSA.decrypt(Record[No][3], privKey)
        decrypted_weight = RSA.decrypt(Record[No][4], privKey)
        decrypted_height = RSA.decrypt(Record[No][5], privKey)
        decrypted_job = RSA.decrypt(Record[No][6], privKey)
        decrypted_gen = RSA.decrypt(Record[No][7], privKey)
        self.ids.kv_name.text = decrypted_name
        self.ids.kv_age.text = decrypted_age
        self.ids.kv_weight.text = decrypted_weight
        self.ids.kv_height.text = decrypted_height
        self.ids.kv_job.text = decrypted_job
        self.ids.kv_gen.text = decrypted_gen

    #Drop down list for selecting number of notificaztions per hour
    def Notification_Hr(self):
        self.menu_list = [
            {
                "viewclass": "OneLineListItem",
                "text": "1",
                "on_release": lambda x = 1: self.N_Hours(x)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "2",
                "on_release": lambda x = 2: self.N_Hours(x)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "3",
                "on_release": lambda x = 3: self.N_Hours(x)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "4",
                "on_release": lambda x = 4: self.N_Hours(x)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "5",
                "on_release": lambda x = 5: self.N_Hours(x)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "6",
                "on_release": lambda x = 6: self.N_Hours(x)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "7",
                "on_release": lambda x = 7: self.N_Hours(x)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "8",
                "on_release": lambda x = 8: self.N_Hours(x)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "9",
                "on_release": lambda x = 9: self.N_Hours(x)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "10",
                "on_release": lambda x = 10: self.N_Hours(x)
            }
        ]
        self.menu = MDDropdownMenu(
            caller = self.ids.NH,
            items = self.menu_list,
            width_mult = 1
        )
        self.menu.open()

    #Execute a toggel-bool to control the frequency of notifications per hour globally(set as per user)
    def set_notification_freq(self,dt):
        global notification_is_on,notify_hr_bool
        global Checkbox1,Checkbox2,Checkbox3,Checkbox4
        if notify_hr_bool == False:
            notify_hr_bool == True
        MotivationTask.Update_notification_state(notification_is_on,notify_hr_bool)

    #Change the notification frequency per hour
    def N_Hours(self,freq):
        global N_freq
        N_freq = freq
        Clock.unschedule(self.set_notification_freq)#Unschedule the previous clock
        #Schedule the clock that will change notification frequency to N/N_Hours
        Clock.schedule_interval(self.set_notification_freq, 3600/N_freq)
        if debugging == True:
            if N_freq == 0.5:
                print("By default Notification frequency - {} per hour".format(N_freq))
            else:
                print("Notification frequency - {} per hour".format(N_freq))
            
#Screen to display Graph
class Graph(Screen):
    global debugging

    def Load_Graph(self):#Load graph on pre entering the Graph screen
        print("Screen - Graph")
        self.daily()

    def daily(self):
        if debugging == True:
            print("Graph - Daily")
        self.ids.pic.source=""
        count_sitting,count_walking,count_running=sqlite.get_day_data_acc() #Getting the counts in hours of user's activity/day
        Plot_Graph.Plt_Graph("Day",count_sitting,count_walking,count_running) #plotting the graph
        self.ids.pic.source="plt_day_graph.png" #Updating graph in GUI
    def weekly(self):
        if debugging == True:
            print("Graph - Weekly")
        self.ids.pic.source=""
        count_sitting,count_walking,count_running=sqlite.get_week_data_acc() #Getting the counts in hours of user's activity/week
        Plot_Graph.Plt_Graph("Week",count_sitting,count_walking,count_running) #plotting the graph
        self.ids.pic.source="plt_week_graph.png" #Updating graph in GUI
    def monthly(self):
        if debugging == True:
            print("Graph - Monthly")
        self.ids.pic.source=""
        count_sitting,count_walking,count_running=sqlite.get_month_data_acc() #Getting the counts in hours of user's activity/month
        Plot_Graph.Plt_Graph("Month",count_sitting,count_walking,count_running) #plotting the graph
        self.ids.pic.source="plt_month_graph.png" #Updating graph in GUI

    #DropDown list menu to select the type of graph (daily,weekly,monthly)
    def dropdown(self):
        self.menu_list = [
            {
                "viewclass": "OneLineListItem",
                "text": "Daily",
                "on_release": lambda x = "Daily": self.daily()
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Weekly",
                "on_release": lambda x = "Weekly": self.weekly()
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Monthly",
                "on_release": lambda x = "Monthly": self.monthly()
            }
        ]
        self.menu = MDDropdownMenu(
            caller = self.ids.menu_id,
            items = self.menu_list,
            width_mult = 3
        )
        self.menu.open()


#Class for Screen Manager
class WindowManager(ScreenManager):
    pass

#Root of KivyMD app
class MainApp(MDApp):
    global debugging

    if debugging == False:
        def on_start(self, **kwargs):
            if platform == "android":
                from android.permissions import request_permissions, Permission
                request_permissions([Permission.WRITE_EXTERNAL_STORAGE, Permission.READ_EXTERNAL_STORAGE])

    def build(self):
        #Adding screens with screen manager
        sm = ScreenManager()
        sm.add_widget(Login(name='login'))
        sm.add_widget(MainPage(name='mainpage'))
        sm.add_widget(Settings11(name='setting11'))
        sm.add_widget(CreateAcc(name='createacc'))
        sm.add_widget(Graph(name='graph'))
        #creating sql databases
        sqlite.create_main_db()
        sqlite.create_acc_db()
        sqlite.create_motivation_task_db()
        #Loading the GRU model
        Model.Load_Model()
        #Setting the application theme
        self.theme_cls.theme_style = "Dark"
        self.theme_cls.primary_palette = "BlueGray"
        #Returns app with main .kv file
        return Builder.load_file('main2.kv')

MainApp().run()