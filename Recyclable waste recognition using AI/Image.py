import random
import cv2
import torch
import numpy as np

from models.experimental import attempt_load
from utils.dataloaders import letterbox
from utils.general import check_img_size, non_max_suppression, scale_coords
from utils.torch_utils import select_device

label = {
    0: "electronics ",
    1: "batteries",
    2: "clothes",
    3: "glass",
    4: "organic",
    5: "metal",
    6: "paper",
    7: "plastic"
}


class YOLOV5_Detector:
    def __init__(self, weights, img_size, confidence_thres, iou_thresh, agnostic_nms, augment):
        self.weights = weights
        self.imgsz = img_size
        self.conf_thres = confidence_thres
        self.iou_thres = iou_thresh

        self.agnostic_nms = agnostic_nms
        self.augment = augment

        self.device = select_device('cpu')
        self.model = attempt_load(weights)  # load FP32 model
        self.stride = int(self.model.stride.max())  # model stride
        self.imgsz = check_img_size(img_size, s=self.stride)  # check img_size

    def plot_one_box(self, x, img, color=None, label=None, line_thickness=3):
        # Plots one bounding box on image img
        tl = line_thickness or round(0.002 * (img.shape[0] + img.shape[1]) / 2) + 1  # line/font thickness
        color = color or [random.randint(0, 255) for _ in range(3)]
        c1, c2 = (int(x[0]), int(x[1])), (int(x[2]), int(x[3]))
        cv2.rectangle(img, c1, c2, color, thickness=tl, lineType=cv2.LINE_AA)
        if label:
            tf = max(tl - 1, 1)  # font thickness
            t_size = cv2.getTextSize(label, 0, fontScale=tl / 3, thickness=tf)[0]
            c2 = c1[0] + t_size[0], c1[1] - t_size[1] - 3
            cv2.rectangle(img, c1, c2, color, -1, cv2.LINE_AA)  # filled
            cv2.putText(img, label, (c1[0], c1[1] - 2), 0, tl / 3, [225, 255, 255], thickness=tf, lineType=cv2.LINE_AA)

    def add_margin(pil_img, top, right, bottom, left, color):
        width, height = pil_img.size
        new_width = width + right + left
        new_height = height + top + bottom
        result = Image.new(pil_img.mode, (new_width, new_height), color)
        result.paste(pil_img, (left, top))
        return result

    def Detect(self, img0):

        img = letterbox(img0, self.imgsz, stride=self.stride)[0]
        img = img[:, :, ::-1].transpose(2, 0, 1)
        img = np.ascontiguousarray(img)

        # Get names and colors
        names = self.model.module.names if hasattr(self.model, 'module') else self.model.names
        # colors = [[random.randint(0, 255) for _ in range(3)] for _ in names]

        img = torch.from_numpy(img).to(self.device)
        img = img.float()
        img /= 255.0

        if img.ndimension() == 3:
            img = img.unsqueeze(0)

        pred = self.model(img, augment=self.augment)[0]

        # Apply NMS
        pred = non_max_suppression(pred, self.conf_thres, self.iou_thres, agnostic=self.agnostic_nms)

        # Process detections
        for i, det in enumerate(pred):  # detections per image
            s = '%gx%g ' % img.shape[2:]  # print string
            gn = torch.tensor(img0.shape)[[1, 0, 1, 0]]  # normalization gain whwh
            if len(det):
                # Rescale boxes from img_size to im0 size
                det[:, :4] = scale_coords(img.shape[2:], det[:, :4], img0.shape).round()

                # Print results
                for c in det[:, -1].unique():
                    n = (det[:, -1] == c).sum()  # detections per class
                    s += f"{n} {names[int(c)]}{'s' * (n > 1)}, "  # add to string

                for *xyxy, conf, cls in reversed(det):
                    if int(cls) in list(label.keys()):
                        self.plot_one_box(xyxy, img0, color=(0, 255, 0), label=label[int(cls)], line_thickness=3)

        cv2.imshow("Result", img0)
        cv2.imwrite('Result.jpg', img0)
        cv2.waitKey(0)

        cv2.destroyAllWindows()
        return img0


detector = YOLOV5_Detector(weights='best.pt',
                           img_size=640,
                           confidence_thres=0.2,
                           iou_thresh=0.9,
                           agnostic_nms=True,
                           augment=True)

img1 = cv2.imread(r"C:\Users\kirta\OneDrive\Desktop\Object detection\Object detection\Test Images\15.jpg")
img = cv2.resize(img1, (640, 640))
# img = cv2.imread(r"TestImages/Test (2).jpg")
# img = cv2.imread(r"TestImages/Test (3).jpg")

detector.Detect(img)
