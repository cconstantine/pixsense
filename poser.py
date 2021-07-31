import json
# import trt_pose.coco

import torch
import torch.utils.data
import torch.nn
import os
import PIL.Image
import json
import tqdm
import trt_pose
import trt_pose.models
import torch
import torch2trt
from torch2trt import TRTModule
import time, sys
import logging
logger = logging.getLogger(__name__)

import cv2
import torchvision.transforms as transforms
import PIL.Image
from trt_pose.draw_objects import DrawObjects
from trt_pose.parse_objects import ParseObjects
import argparse
import os.path
import pyrealsense2.pyrealsense2 as rs
import numpy as np


# pulled in from trt_pose.coco to avoid an import of matplotlib with the wrong gtk version
def coco_category_to_topology(coco_category):
    """Gets topology tensor from a COCO category
    """
    skeleton = coco_category['skeleton']
    K = len(skeleton)
    topology = torch.zeros((K, 4)).int()
    for k in range(K):
        topology[k][0] = 2 * k
        topology[k][1] = 2 * k + 1
        topology[k][2] = skeleton[k][0] - 1
        topology[k][3] = skeleton[k][1] - 1
    return topology


class Detector:
    def __init__(self, net, image_dimensions, network_dimensions, weights_filename, optimize):
        with open('models/human_pose.json', 'r') as f:
            self.human_pose = json.load(f)

        self.topology = coco_category_to_topology(self.human_pose)

        self.parse_objects = ParseObjects(self.topology)
        self.draw_objects = DrawObjects(self.topology)
        self.num_parts = len(self.human_pose['keypoints'])
        self.num_links = len(self.human_pose['skeleton'])

        self.weights_filename = weights_filename
        self.weights_filename_opt = '{0}.trt'.format(weights_filename)
        self.model = net(self.num_parts, 2 * self.num_links).cuda().eval()
        self.image_dimensions = image_dimensions
        self.network_dimensions = network_dimensions
        # data = torch.zeros((1, 3, self.height, self.width)).cuda()
            
        if optimize or os.path.exists(self.weights_filename_opt) == False:
            logger.info("Optimizing network for trt")
            self.model.load_state_dict(torch.load(self.weights_filename))
            self.model_trt = torch2trt.torch2trt(self.model, [torch.zeros((1, 3, *self.network_dimensions)).cuda()], fp16_mode=True)
            torch.save(self.model_trt.state_dict(), self.weights_filename_opt)
        else:
            self.model_trt = TRTModule()
            self.model_trt.load_state_dict(torch.load(self.weights_filename_opt))

        self.mean = torch.Tensor([0.485, 0.456, 0.406]).cuda()
        self.std = torch.Tensor([0.229, 0.224, 0.225]).cuda()
        self.device = torch.device('cuda')
        self.preprocess = transforms.Compose([
            transforms.Resize(self.network_dimensions),
            transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
            self.model_trt
        ])


    def detect(self, image):
        # image = cv2.copyMakeBorder( image, 0, self.image_dimensions[0] - self.image_dimensions[1], 0, 0, cv2.BORDER_CONSTANT, [0, 0, 0])
        #image = cv2.resize(image, dsize=self.network_dimensions, interpolation=cv2.INTER_AREA)

        image_tensor = transforms.functional.to_tensor(image).to(self.device)
        cmap, paf = self.preprocess(image_tensor[None, ...])
        cmap, paf = cmap.detach().cpu(), paf.detach().cpu()
        counts, objects, peaks = self.parse_objects(cmap, paf)

        # self.draw_objects(image, counts, objects, peaks)
        # # self.image = cv2.resize(image, dsize=(self.image_dimensions[0], self.image_dimensions[1]), interpolation=cv2.INTER_AREA)
        # cv2.imshow(f"{self.camera_id}", image)
        # cv2.waitKey(1)
        return [ self.get_keypoint(objects, i, peaks) for i in range(counts[0]) ]

    '''
    hnum: 0 based human index
    kpoint : keypoints (float type range : 0.0 ~ 1.0 ==> later multiply by image width, height
    '''
    def get_keypoint(self, humans, hnum, peaks):
        #check invalid human index
        kpoint = []
        human = humans[0][hnum]
        C = human.shape[0]
        for j in range(C):
            k = int(human[j])
            if k >= 0:
                peak = peaks[0][j][k]   # peak[1]:width, peak[0]:height
                peak = (j, int(float(peak[1])*self.image_dimensions[0]), int(float(peak[0])*self.image_dimensions[1]))
                kpoint.append(peak)
                #print('index:%d : success [%5.3f, %5.3f]'%(j, peak[1], peak[2]) )
            else:    
                peak = [j]
                kpoint.append(peak)
                #print('index:%d : None %d'%(j, k) )
        return kpoint


class DetectorResnet(Detector):
    def __init__(self, image_dimensions, optimize):
        super().__init__(trt_pose.models.resnet18_baseline_att, image_dimensions, (224, 224), 'models/resnet18_baseline_att_224x224_A_epoch_249.pth', optimize)

class DetectorDensenet(Detector):
    def __init__(self, image_dimensions, optimize):
        super().__init__(trt_pose.models.densenet121_baseline_att, image_dimensions, (256, 256), 'models/densenet121_baseline_att_256x256_B_epoch_160.pth', optimize)

    
# if __name__ == "__main__":
#     parser = argparse.ArgumentParser(description='TensorRT pose estimation run')
#     parser.add_argument('--model', type=str, default='resnet', help = 'resnet or densenet' )
#     parser.add_argument('--optimize', default=False, action='store_true', help = 'Generate a new optimized trt module' )
#     args = parser.parse_args()

#     resn = DetectorDensenet(args.optimize)

#     pipeline = rs.pipeline()
#     config = rs.config()
#     #config.enable_stream(rs.stream.color,    640, 480, rs.format.bgr8, 60)
#     config.enable_stream(rs.stream.infrared, 640, 480, rs.format.y8, 30)
#     config.enable_stream(rs.stream.depth,    640, 480, rs.format.z16, 30)

#     # Start streaming
#     pipeline_profile = pipeline.start(config)
#     device = pipeline_profile.get_device()
#     depth_sensor = device.query_sensors()[0]
#     depth_sensor.set_option(rs.option.emitter_enabled, 0)



#     print("Streaming...")
#     while True:
#         frames = pipeline.wait_for_frames()
#         img_frame = frames.get_infrared_frame()
#         depth_frame = frames.get_depth_frame()
#         t = time.time()

#         img = np.asanyarray(img_frame.get_data())
#         cv2.imshow('image',img)
#         img = cv2.cvtColor(img,cv2.COLOR_GRAY2BGR)
#         #cv2.imshow('image',img)
#         #continue
#         part = resn.detect(img)
#         fps = 1.0 / (time.time() - t)
#         print("FPS:{0:7.2f} ".format(fps))

#         if part != None:
#             point = (part[1]*640, part[0]*480)
#             img = cv2.circle(img, point, radius=9, color=(0, 0, 255), thickness=-1)

#             depth_intrinsics = depth_frame.profile.as_video_stream_profile().intrinsics
#             xyz = rs.rs2_deproject_pixel_to_point(depth_intrinsics, [point[0], point[1]], depth_frame.get_distance(point[0], point[1]))
#             print(xyz)

#         #cv2.imshow('image',PIL.Image.fromarray(img))
#         cv2.waitKey(1)


#     cv2.destroyAllWindows()

