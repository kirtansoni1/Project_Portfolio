#Importing python libraries
import matplotlib.pyplot as plt

#Plot the Grapg and Save it as png to display in the GUI
def Plt_Graph(timeframe,count_sitting,count_walking,count_running):
    x = ['Sitting','Walking','Running']
    y = [count_sitting,count_walking,count_running]
    plt.bar(x, y, color='blue')
    plt.xticks(fontsize=12)
    plt.yticks(fontsize=12)
    count_sitting=float("%.2f"%count_sitting)
    count_walking=float("%.2f"%count_walking)
    count_running=float("%.2f"%count_running)
    plt.text(0,count_sitting,count_sitting, ha = 'center',
                bbox = dict(facecolor = 'cyan', alpha =0.6))
    plt.text(1,count_walking,count_walking, ha = 'center',
                bbox = dict(facecolor = 'cyan', alpha =0.6)) 
    plt.text(2,count_running,count_running, ha = 'center',
                bbox = dict(facecolor = 'cyan', alpha =0.6))
    plt.xlabel("Movement")
    plt.ylabel("Hours")

    #If the user selected daily graph
    if timeframe=="Day":
        plt.title('Movement Analysis per Day')
        plt.savefig('plt_day_graph.png') #Saving the graph as .png
        plt.clf()
    #If the user selected weekly graph
    if timeframe=="Week":
        plt.title('Movement Analysis per Week')
        plt.savefig('plt_week_graph.png') #Saving the graph as .png
        plt.clf()
    #If the user selected monthly graph
    if timeframe=="Month":
        plt.title('Movement Analysis per Month')
        plt.savefig('plt_month_graph.png') #Saving the graph as .png
        plt.clf()