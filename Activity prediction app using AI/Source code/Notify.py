#Importing pyhton Libraries
from plyer import notification

#Generate notification upon call
def notify_me(Title,Message,app_name,app_icon):

    notification.notify(title=Title, message=Message,app_name=app_name,app_icon=app_icon)