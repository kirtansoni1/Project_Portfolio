# Sensor Life time prediction using AI
- My second semester case study project in which my task was to develop a mdoel which can predict Infenion's TLE5501 E001 GMR based angle sensor.
- The sensor data was collected using HTMOL process with 0, 500, 1000, 1500 hours treatment.
- The data given to me was in the excel format so first I developed a python script that can automatically fetch the necessary data like angle error, hysteresis, raw measurements. Then I clean the outliers in the data and trained the GRU model for prediction. I also did alternative method with using Linear regrresion and Arrhenius equation to predict life time in hours.
