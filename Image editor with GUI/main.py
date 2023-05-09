#Kivy/KivyMD imports
from kivy.core.window import Window
from kivy.lang import Builder
from kivy.uix.screenmanager import Screen
from kivymd.uix.menu import MDDropdownMenu
from kivymd.app import MDApp
from kivy.metrics import dp
from kivymd.uix.dialog import MDDialog
from kivymd.uix.button import MDFlatButton

#Other lib imports
import cv2
import os
from plyer import notification, filechooser
import numpy as np

#To Maximize the GUI window
Window.maximize()


#KV script string
KV = '''
ScreenManager:
    First:

<First>:
    name:"First"
    canvas.before:
        Rectangle:
            size: self.size
            pos: self.pos
            source: "background.jpg"
    MDScreen:
        size:root.height,root.width
        MDLabel:
            text:"We support LGBTQ+"
            font_size: "12dp"
            color: (1,1,1,1)
            pos_hint:{'center_x':0.96, 'center_y':0.02}
            halign: 'center'
            valign: 'center'
            bold: True
        MDLabel:
            text:"Original Image:"
            font_size: "15dp"
            color: (0,0,0,0.7)
            pos_hint: {'center_x': .22, 'center_y': .2}
            halign: 'center'
            valign: 'center'
        MDLabel:
            text:"Modified Image:"
            font_size: "15dp"
            color: (0,0,0,0.7)
            pos_hint: {'center_x': .805, 'center_y': .2}
            halign: 'center'
            valign: 'center'
        MDLabel:
            id: cur_filter
            text:""
            font_size: "18dp"
            color: (0,0,0,0.7)
            pos_hint: {'center_x': .5, 'center_y': .7}
            halign: 'center'
            valign: 'center'
        MDLabel:
            id: intensity_label
            text:""
            font_size: "15dp"
            color: (0,0,0,0.7)
            pos_hint: {'center_x': .5, 'center_y': .47}
            halign: 'center'
            valign: 'center'
        MDSlider:
            id: slider
            min:0
            max:50
            step:10
            orientation: "horizontal"
            pos_hint: {'center_x': .5, 'center_y': .5}
            size_hint:0.1,0.05
            hint: True
            hint_bg_color: 105/255,189/255,210/255,1
            hint_text_color: "white"
            disabled: True
            on_value: root.apply_slider(*args)
            show_off: False
        MDRoundFlatIconButton:
            text: "Select Image"
            icon: "folder"
            font_size: "18dp"
            size_hint:0.11,0.05
            theme_text_color: "Custom"
            text_color: 0, 0, 0, 1
            pos_hint: {'center_x': .15, 'center_y': .1}
            on_release: root.file_manager_open()
        MDRoundFlatIconButton:
            text: "Save output"
            icon: "content-save"
            font_size: "18dp"
            size_hint:0.11,0.05
            theme_text_color: "Custom"
            text_color: 0, 0, 0, 1
            pos_hint: {'center_x': .5, 'center_y': .1}
            on_release: root.save_image()
        MDRoundFlatIconButton:
            text: "Rotate"
            icon: "rotate-3d-variant"
            font_size: "18dp"
            size_hint:0.08,0.05
            theme_text_color: "Custom"
            text_color: 0, 0, 0, 1
            pos_hint: {'center_x': .3, 'center_y': .1}
            on_release: root.rotate_img1()
        Rot_img1:
            id:rotate1
            angle: 0
            source: 'photo.png'
            pos_hint: {'center_x': .22, 'center_y': .55}
        Rot_img2:
            id:rotate2
            angle: 0
            pos_hint: {'center_x': .8, 'center_y': .55}
            source: 'photo.png'
        MDRoundFlatIconButton:
            text: "Rotate"
            icon: "rotate-3d-variant"
            font_size: "18dp"
            size_hint:0.08,0.05
            theme_text_color: "Custom"
            text_color: 0, 0, 0, 1
            pos_hint: {'center_x': .805, 'center_y': .1}
            on_release: root.rotate_img2()
        MDRectangleFlatIconButton:
            id: NH
            icon: "menu"
            text: "Select to apply"
            font_size: "18dp"
            theme_text_color: "Custom"
            text_color: 0,0,0,1
            pos_hint: {'center_x':0.5, 'center_y':0.3}
            on_release: root.Apply_Filter()

        MDTopAppBar:
            title: "Computer Vision Mini-Project - Convert image to Grey, Smooth and finding Edge"
            right_action_items: [["exit-to-app", lambda x: app.stop()]]
            left_action_items: [["menu", lambda x: root.makers()]]
            pos_hint: {'center_x': 0.5, 'center_y': 0.97}
            elevation: 4
            
        MDNavigationDrawer:
            id:nav_drawer
            md_bg_color: 1,1,1,0.8
            BoxLayout:
                orientation: "vertical"
                pos_hint: {"center_x": .5, 'center_y': 0.5}
                padding: "30dp"
                spacing: "20dp"
                
                MDCard:
                    orientation: "vertical"
                    padding: "30dp"
                    spacing: "40dp"
                    size_hint: None, None
                    size: "285dp",  self.minimum_height
                    pos_hint: {"center_x": .5, 'top': 0.9}
                    line_color: 0.2, 0.2, 0.2, 0.3
                    md_bg_color: 89/255, 163/255, 168/255,0.9
                    MDLabel:
                        text:"Guide:"
                        font_size: "20dp"
                        color: (0,0,0,1)
                        bold: True
                    MDSeparator:
                        height:"2dp"
                        color: 0,0,0,0.5
                    MDLabel:
                        text:"1) Select any Image from your computer."
                        font_size: "15dp"
                        color: (0,0,0,1)
                    MDLabel:
                        text:"2) Click on the Select to Apply button."
                        font_size: "15dp"
                        color: (0,0,0,1)
                    MDLabel:
                        text:"3) Pick any mentioned filter you want to apply."
                        font_size: "15dp"
                        color: (0,0,0,1)
                    MDLabel:
                        text:"4) Adjust the slider for intensity of filter."
                        font_size: "15dp"
                        color: (0,0,0,1)
                    MDLabel:
                        text:"5) Modified Image will appear on the output side."
                        font_size: "15dp"
                        color: (0,0,0,1)
                    MDLabel:
                        text:"6) Click on Save button to save the image."
                        font_size: "15dp"
                        color: (0,0,0,1)
                MDCard:
                    orientation: "vertical"
                    padding: "30dp"
                    spacing: "20dp"
                    size_hint: None, None
                    size: "285dp",  self.minimum_height
                    pos_hint: {"center_x": .5, 'top': 0.5}
                    line_color: 0.2, 0.2, 0.2, 0.3
                    md_bg_color: 89/255, 163/255, 168/255,0.9
                    MDLabel:
                        text:"Group members:"
                        font_size: "20dp"
                        color: (0,0,0,1)
                        bold: True
                    MDSeparator:
                        height:"2dp"
                        color: 0,0,0,0.5
                    MDLabel:
                        text:"Kirtan Soni (12201189)"
                        font_size: "15dp"
                        color: (0,0,0,1)
                    MDLabel:
                        text:"Gaurav Sharma (12200584)"
                        font_size: "15dp"
                        color: (0,0,0,1)
                    MDLabel:
                        text:"Anmol Sharma (661264)"
                        font_size: "15dp"
                        color: (0,0,0,1)
                    MDLabel:
                        text:"Azhar Mehmood (12202335)"
                        font_size: "15dp"
                        color: (0,0,0,1)
                    MDLabel:
                        text:"Hiral Gondaliya (22111961)"
                        font_size: "15dp"
                        color: (0,0,0,1)
                Image:
                    source: "lgbtq.png"
<Rot_img1@Image>:
    angle: 0
    source: ''
    size_hint: None,None
    size: 552,552
    canvas.before:
        PushMatrix
        Rotate:
            angle: root.angle
            origin: self.center
    canvas.after:
        PopMatrix

<Rot_img2@Image>:
    angle: 0
    source: ''
    size_hint: None,None
    size: 552,552
    canvas.before:
        PushMatrix
        Rotate:
            angle: root.angle
            origin: self.center
    canvas.after:
        PopMatrix
'''

#Create screen
class First(Screen):

    #Initial function
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.rot1_count=0
        self.rot2_count=0
        self.pic_path = ""
        self.init_image = True
        self.filter_selected = False
        self.count = 0
        self.current_filter = ""
        self.image_to_save = []
        self.save_image_counter = 0
        self.parent_dir = os.getcwd()

    #For Navigation Menu
    def makers(self):
        self.ids.nav_drawer.set_state("toggle")

    #To open filme manager popup
    def file_manager_open(self):
        filechooser.open_file(on_selection = self.selected)

    #To Save the image selection path
    def selected(self, selection):
        if selection:
            self.pic_path = str(selection[0])
            if self.pic_path.find(".jpg") < 0 and self.pic_path.find(".png") < 0 and self.pic_path.find(".jpeg") < 0 and self.pic_path.find(".JPG") < 0 and self.pic_path.find(".JPEG") < 0 and self.pic_path.find(".PNG") < 0:
                self.dialog = MDDialog(
                title = "Sorry!",
                text = "This app only supports image with '.jpg','.jpeg','.png' extention.",
                buttons =[
                    MDFlatButton(
                        text="Understood", on_release = self.close_dialog, font_size = dp(15)
                        ),
                    ]
                )
                self.dialog.open()
            else:
                self.ids.rotate1.source = str(selection[0])
                self.ids.cur_filter.text = ""
                self.ids.intensity_label.text = ""
                self.init_image = False
                self.filter_selected = False
                self.ids.slider.value = 0
                self.ids.slider.disabled = True
                self.ids.rotate2.source = 'photo.png'

    #To rotate input image
    def rotate_img1(self):
        if self.init_image == False:
            self.rot1_count += 1
            if self.rot1_count==1:
                self.ids.rotate1.angle = 90
            if self.rot1_count==2:
                self.ids.rotate1.angle = 180
            if self.rot1_count==3:
                self.ids.rotate1.angle = 270
            if self.rot1_count==4:
                self.ids.rotate1.angle = 0
                self.rot1_count = 0

    #To rotate output image
    def rotate_img2(self):
        if self.init_image == False and self.filter_selected == True:
            self.rot2_count += 1
            if self.rot2_count==1:
                self.ids.rotate2.angle = 90
            if self.rot2_count==2:
                self.ids.rotate2.angle = 180
            if self.rot2_count==3:
                self.ids.rotate2.angle = 270
            if self.rot2_count==4:
                self.ids.rotate2.angle = 0
                self.rot2_count = 0
    
    #Create a drop down menu of filters
    def Apply_Filter(self):
        self.menu_list = [
            {
                "viewclass": "OneLineListItem",
                "text": "",
                "height": dp(1)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Blur",
                "on_release": lambda x = "Blur": self.applyFilter(x,0),
                "height": dp(50)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Gaussian Blur",
                "on_release": lambda x = "GaussianBlur": self.applyFilter(x,0),
                "height": dp(50)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Sharpness",
                "on_release": lambda x = "Sharpness": self.applyFilter(x,0),
                "height": dp(50)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Brightness",
                "on_release": lambda x = "Brightness": self.applyFilter(x,0),
                "height": dp(50)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Contrast",
                "on_release": lambda x = "Contrast": self.applyFilter(x,0),
                "height": dp(50)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Edge Detection",
                "on_release": lambda x = "ED": self.applyFilter(x,0),
                "height": dp(50)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Gray Scale",
                "on_release": lambda x = "GS": self.applyFilter(x,0),
                "height": dp(50)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Show region in Green Color",
                "on_release": lambda x = "SGC": self.applyFilter(x,0),
                "height": dp(50)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Show region in Yellow Color",
                "on_release": lambda x = "SYC": self.applyFilter(x,0),
                "height": dp(50)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Show region in Blue Color",
                "on_release": lambda x = "SBC": self.applyFilter(x,0),
                "height": dp(50)
            },
            {
                "viewclass": "OneLineListItem",
                "text": "Show region in Red Color",
                "on_release": lambda x = "SRC": self.applyFilter(x,0),
                "height": dp(50)
            }
        ]
        self.menu = MDDropdownMenu(
            caller = self.ids.NH,
            items = self.menu_list,
            width_mult = 4,
            max_height= dp(283)
        )
        self.menu.open()

    #To get the slider values
    def apply_slider(self,*args):
        intensity = args[1]
        self.applyFilter(self.current_filter,intensity)
    
    #To Save the result image
    def save_image(self):
        if self.init_image == False and self.filter_selected == True:

            os.chdir(self.parent_dir)
            self.save_image_counter += 1
            cv2.imwrite('output_{}.jpeg'.format(self.save_image_counter),self.image_to_save)

            self.dialog = MDDialog(
            title = "Saved!",
            text = "Your image is saved as output_{}.jpeg".format(self.save_image_counter),
            buttons =[
                MDFlatButton(
                    text="Okay", on_release = self.close_dialog, font_size = dp(15)
                    ),
                ]
            )
            self.dialog.open()
        elif self.init_image == True or self.filter_selected == False:
            os.chdir(self.parent_dir)
            notification.notify(title="Please select an Image/Filter", message="Computer vision project", app_name = 'MainApp', app_icon = 'LGBTQ.ico', timeout = 1)

    #Click Cancel Button
    def close_dialog(self, obj):
        #Close alert box
        self.dialog.dismiss()
    
    #Apply filter on image
    def applyFilter(self, msg, intensity):
        if self.init_image == False:
            self.filter_selected = True
            self.current_filter = msg
            self.count += 1
            if self.count == 0:
                pass
            else:
                try:
                    os.remove('new{}.jpeg'.format(self.count-1))
                except:
                    pass
            if self.pic_path == '':
                pass
            else:
                original_image = cv2.imread(self.pic_path)
                if msg == "Blur":
                    self.ids.cur_filter.text = "Simple Blur filter:"
                    self.ids.intensity_label.text = "Simple Blur intensity:"
                    if intensity == 0:
                        intensity = 1
                    self.ids.slider.disabled = False
                    #Kernel - 1/(intensity*intensity) * [[1,1,1],[1,1,1],[1,1,1]], Matrix size = [intensity x intensity]
                    new_image = cv2.blur(original_image,(int(intensity),int(intensity)),0)
                elif msg == "GaussianBlur":
                    self.ids.cur_filter.text = "Gaussian Blur filter:"
                    self.ids.intensity_label.text = "Gaussian Blur intensity:"
                    if intensity == 0:
                        intensity = 0
                    self.ids.slider.disabled = False
                    #Kernel - 1/16 * [[1,2,1],[2,4,2],[1,2,1]], Matrix size = [intensity x intensity]
                    new_image = cv2.GaussianBlur(original_image,(int(intensity+23),int(intensity+23)),0)
                elif msg == "Sharpness":
                    self.ids.cur_filter.text = "Sharpness filter:"
                    self.ids.intensity_label.text = "Sharpness intensity:"
                    if intensity == 0:
                        intensity = 10
                    self.ids.slider.disabled = False
                    #Defining the kernel
                    kernel_Sharpness = np.array([[0,-1,0],
                                                [-1,5,-1],
                                                [0,-1,0]])
                    #Doing convolution and pooling
                    new_image = cv2.filter2D(src=original_image, ddepth=-1, kernel=kernel_Sharpness*int(intensity/10))
                elif msg == "Brightness":
                    self.ids.slider.disabled = False
                    self.ids.cur_filter.text = "Brightness filter:"
                    self.ids.intensity_label.text = "Brightness intensity:"
                    #Converting image to HSV
                    hsv = cv2.cvtColor(original_image, cv2.COLOR_BGR2HSV)
                    #splitting HSV values
                    h, s, v = cv2.split(hsv)
                    #Setting the limit
                    lim = 255 - int(intensity+10)
                    #Adjusting the vibrance according to the limit
                    v[v > lim] = 255
                    v[v <= lim] += int(intensity+10)
                    #Merging the H S V values
                    final_hsv = cv2.merge((h, s, v))
                    #Converting back to BGR format
                    new_image = cv2.cvtColor(final_hsv, cv2.COLOR_HSV2BGR)
                elif msg == "Contrast":
                    self.ids.slider.disabled = False
                    self.ids.cur_filter.text = "Contrast filter:"
                    self.ids.intensity_label.text = "Contrast intensity:"
                    # converting to LAB color space
                    lab= cv2.cvtColor(original_image, cv2.COLOR_BGR2LAB)
                    l_channel, a, b = cv2.split(lab)
                    # Applying CLAHE to L-channel
                    # feel free to try different values for the limit and grid size:
                    clahe = cv2.createCLAHE(clipLimit=int((intensity+10)/10), tileGridSize=(8,8))
                    cl = clahe.apply(l_channel)
                    # merge the CLAHE enhanced L-channel with the a and b channel
                    limg = cv2.merge((cl,a,b))
                    # Converting image from LAB Color model to BGR color spcae
                    new_image = cv2.cvtColor(limg, cv2.COLOR_LAB2BGR)
                elif msg == "ED":
                    self.ids.cur_filter.text = "Edge Detection filter:"
                    self.ids.intensity_label.text = ""
                    self.ids.slider.disabled = True
                    #Defining the kernel
                    kernel_ED = np.array([[-1,-1,-1],
                                        [-1,8,-1],
                                        [-1,-1,-1]])
                    #Doing convolution and pooling
                    new_image = cv2.filter2D(src=original_image, ddepth=-1, kernel=kernel_ED)
                elif msg == "GS":
                    self.ids.cur_filter.text = "Gray Scale filter:"
                    self.ids.intensity_label.text = ""
                    self.ids.slider.disabled = True
                    new_image = cv2.cvtColor(original_image, cv2.COLOR_BGR2GRAY)
                elif msg == "SGC":
                    self.ids.cur_filter.text = "Highlight region with green color:"
                    self.ids.intensity_label.text = ""
                    self.ids.slider.disabled = True
                    #Converting imgae to HSV format
                    image = cv2.cvtColor(original_image, cv2.COLOR_BGR2HSV)
                    #Defining the mask low and high range in (r,g,b) format
                    mask = cv2.inRange(image,(36, 0, 0),(86, 255,255))
                    #Creating the new mask with previous mask
                    imask = mask>0
                    #Creating a black image of same size
                    new_image = np.zeros_like(image, np.uint8)
                    #Overlapping the two images
                    new_image[imask] = image[imask]
                    #Converting image back to BGR format
                    new_image = cv2.cvtColor(new_image, cv2.COLOR_HSV2BGR)
                elif msg == "SYC":
                    self.ids.cur_filter.text = "Highlight region with yellow color:"
                    self.ids.intensity_label.text = ""
                    self.ids.slider.disabled = True
                    #Converting imgae to HSV format
                    image = cv2.cvtColor(original_image, cv2.COLOR_BGR2HSV)
                    #Defining the mask low and high range in (r,g,b) format
                    mask = cv2.inRange(image,(15, 0, 0),(36, 255,255))
                    #Creating the new mask with previous mask
                    imask = mask>0
                    #Creating a black image of same size
                    new_image = np.zeros_like(image, np.uint8)
                    #Overlapping the two images
                    new_image[imask] = image[imask]
                    #Converting image back to BGR format
                    new_image = cv2.cvtColor(new_image, cv2.COLOR_HSV2BGR)
                elif msg == "SBC":
                    self.ids.cur_filter.text = "Highlight region with blue color:"
                    self.ids.intensity_label.text = ""
                    self.ids.slider.disabled = True
                    #Converting imgae to HSV format
                    image = cv2.cvtColor(original_image, cv2.COLOR_BGR2HSV)
                    #Defining the mask low and high range in (r,g,b) format
                    mask = cv2.inRange(image,(90, 0, 0),(110, 255,255))
                    #Creating the new mask with previous mask
                    imask = mask>0
                    #Creating a black image of same size
                    new_image = np.zeros_like(image, np.uint8)
                    #Overlapping the two images
                    new_image[imask] = image[imask]
                    #Converting image back to BGR format
                    new_image = cv2.cvtColor(new_image, cv2.COLOR_HSV2BGR)
                elif msg == "SRC":
                    self.ids.cur_filter.text = "Highlight region with red color:"
                    self.ids.intensity_label.text = ""
                    self.ids.slider.disabled = True
                    #Converting imgae to HSV format
                    image = cv2.cvtColor(original_image, cv2.COLOR_BGR2HSV)
                    #Defining the mask low and high range in (r,g,b) format
                    mask = cv2.inRange(image,(161, 155, 84),(179, 255, 255))
                    #Creating the new mask with previous mask
                    imask = mask>0
                    #Creating a black image of same size
                    new_image = np.zeros_like(image, np.uint8)
                    #Overlapping the two images
                    new_image[imask] = image[imask]
                    #Converting image back to BGR format
                    new_image = cv2.cvtColor(new_image, cv2.COLOR_HSV2BGR)
                self.image_to_save = new_image
                os.chdir(self.parent_dir)
                cv2.imwrite('new{}.jpeg'.format(self.count),new_image)
                self.ids.rotate2.source = 'new{}.jpeg'.format(self.count)
                try:
                    os.remove('new{}.jpeg'.format(self.count))
                except:
                    pass
        elif self.init_image == True:
            notification.notify(title="Please select an Image", message="Computer vision project", app_name = 'MainApp', app_icon = 'LGBTQ.ico', timeout = 1)

#Root of KivyMD app
class MainApp(MDApp):
    def build(self):
        notification.notify(title="Welcome to Computer vision project", message="Made by Kirtan Soni, Gaurav Sharma, Anmol Sharma, Azhar Mehmood, Hiral Gondaliya", app_name = 'MainApp', app_icon = 'LGBTQ.ico', timeout = 2)
        return Builder.load_string(KV)

if __name__ == "__main__":
    MainApp().run()