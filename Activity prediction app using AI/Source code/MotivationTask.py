#Importing python libraries
from kivy.clock import Clock

#Importing python files
import Notify
import Model

#Defining Global Variables
cb1_monitor=0
cb2_monitor=0
cb3_monitor=0
cb4_monitor=0
notification_is_on=False
notify_hr_bool=True
m_task1=0
m_task2=0
m_task3=0
m_task4=0

#Motivation tasks clock manager
def Clock_checkbox(CB_N,CB_state,t1,t2,t3,t4):
    global cb1_monitor,cb2_monitor,cb3_monitor,cb4_monitor,notification_is_on
    global m_task1,m_task2,m_task3,m_task4
    global notify_hr_bool
    m_task1=t1
    m_task2=t2
    m_task3=t3
    m_task4=t4
    if CB_N==1:
        if CB_state==True:
            Clock.schedule_interval(CB1, 60)
        else:
            CB_Unschedule("1")
    elif CB_N==2:
        if CB_state==True:
            Clock.schedule_interval(CB2, 60)
        else:
            CB_Unschedule("2")
    elif CB_N==3:
        if CB_state==True:
            Clock.schedule_interval(CB3, 60)
        else:
            CB_Unschedule("3")
    elif CB_N==4:
        if CB_state==True:
            Clock.schedule_interval(CB4, 60)
        else:
            CB_Unschedule("4")

def Update_notification_state(Notification_state,notify_hr):
    global notification_is_on,notify_hr_bool
    notification_is_on=Notification_state
    notify_hr_bool=notify_hr


def CB1(dt):
    global cb1_monitor,notification_is_on,m_task1,notify_hr_bool
    Prediction = Model.predict()
    cb1_monitor = cb1_monitor + 1
    if cb1_monitor<=m_task1:
        if Prediction is not "Running":
            if notification_is_on==True and notify_hr_bool==True:
                Notify.notify_me('Hello!','You are doing great, just a bit further!','MainApp','app.ico')
                notify_hr_bool=False
            else:
                pass
    else:
        if notification_is_on==True: #Will only notify if notifications is set to True
            Notify.notify_me('Congratulations!', 'You have completed the Task','MainApp','app.ico')
            cb1_monitor=0
            CB_Unschedule("1")
        else:
            pass

def CB2(dt):
    global cb2_monitor,notification_is_on,m_task2,notify_hr_bool
    Prediction = Model.predict()
    cb2_monitor = cb2_monitor + 1
    if cb2_monitor<=m_task2:
        if Prediction is not "Running":
            if notification_is_on==True and notify_hr_bool==True:
                Notify.notify_me('Hello!','You are doing great, just a bit further!','MainApp','app.ico')
                notify_hr_bool=False
            else:
                pass
    else:
        if notification_is_on==True: #Will only notify if notifications is set to True
            Notify.notify_me('Congratulations!', 'You have completed the Task','MainApp','app.ico')
            cb2_monitor=0
            CB_Unschedule("2")
        else:
            pass

def CB3(dt):
    global cb3_monitor,notification_is_on,m_task3,notify_hr_bool
    Prediction = Model.predict()
    cb3_monitor = cb3_monitor + 1
    if cb3_monitor<=m_task3:
        if Prediction is not "Walking":
            if notification_is_on==True and notify_hr_bool==True:
                Notify.notify_me('Hello!','You are doing great, just a bit further!','MainApp','app.ico')
                notify_hr_bool=False
            else:
                pass
    else:
        if notification_is_on==True: #Will only notify if notifications is set to True
            Notify.notify_me('Congratulations!', 'You have completed the Task','MainApp','app.ico')
            cb3_monitor=0
            CB_Unschedule("3")
        else:
            pass

def CB4(dt):
    global cb4_monitor,notification_is_on,m_task4,notify_hr_bool
    Prediction = Model.predict()
    cb4_monitor = cb4_monitor + 1
    if cb4_monitor<=m_task4:
        if Prediction is not "Walking":
            if notification_is_on==True and notify_hr_bool==True:
                Notify.notify_me('Hello!','You are doing, just a bit further!','MainApp','app.ico')
                notify_hr_bool=False
            else:
                pass
    else:
        if notification_is_on==True: #Will only notify if notifications is set to True
            Notify.notify_me('Congratulations!', 'You have completed the Task','MainApp','app.ico')
            cb4_monitor=0
            CB_Unschedule("4")
        else:
            pass

#For unscheduling the clocks of motivation tasks
def CB_Unschedule(N):
    global cb1_monitor,cb2_monitor,cb3_monitor,cb4_monitor
    if N=="1":
        Clock.unschedule(CB1)
        cb1_monitor=0
    elif N=="2":
        Clock.unschedule(CB2)
        cb2_monitor=0
    elif N=="3":
        Clock.unschedule(CB3)
        cb3_monitor=0
    elif N=="4":
        Clock.unschedule(CB4)
        cb4_monitor=0