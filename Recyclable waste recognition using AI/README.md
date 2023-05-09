
#Follow this steps to run waste-recognition using AI project:
-----------------------------------------------------------

1) Create a new enviorment in conda with python version 3.7.0:
conda create --name object_detection python=3.7.0

2) Activate the enviorment:
conda activate object_detection

3) Install PyTorch version=1.7
conda install pytorch==1.7.0

4) Install all the required libraries from requirements.txt:
pip install -r requirements.txt

5) Run the python code for live video detection:
python Live_video.py

6) Run the python code for Image detection:
python Image.py

************************************************************
If you have a Cuda supported GPU device:
Change 'cpu' -------> 'gpu:0'
in, Live_video.py on line - 33
    Object detection\models\experimental.py on line - 81
************************************************************