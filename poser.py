import json
import trt_pose.coco
import trt_pose.models
import torch
import torch2trt
from torch2trt import TRTModule
import time, sys
import cv2
import torchvision.transforms as transforms
import PIL.Image
from trt_pose.draw_objects import DrawObjects
from trt_pose.parse_objects import ParseObjects
import argparse
import os.path
import pyrealsense2.pyrealsense2 as rs
import numpy as np

'''
hnum: 0 based human index
kpoint : keypoints (float type range : 0.0 ~ 1.0 ==> later multiply by image width, height
'''
def get_keypoint(humans, hnum, peaks):
    #check invalid human index
    kpoint = []
    human = humans[0][hnum]
    C = human.shape[0]
    for j in range(C):
        k = int(human[j])
        if k >= 0:
            peak = peaks[0][j][k]   # peak[1]:width, peak[0]:height
            peak = (j, float(peak[0]), float(peak[1]))
            kpoint.append(peak)
            #print('index:%d : success [%5.3f, %5.3f]'%(j, peak[1], peak[2]) )
        else:    
            peak = (j, None, None)
            kpoint.append(peak)
            #print('index:%d : None %d'%(j, k) )
    return kpoint


parser = argparse.ArgumentParser(description='TensorRT pose estimation run')
parser.add_argument('--model', type=str, default='resnet', help = 'resnet or densenet' )
args = parser.parse_args()

with open('human_pose.json', 'r') as f:
    human_pose = json.load(f)

topology = trt_pose.coco.coco_category_to_topology(human_pose)

num_parts = len(human_pose['keypoints'])
num_links = len(human_pose['skeleton'])


if 'resnet' in args.model:
    print('------ model = resnet--------')
    MODEL_WEIGHTS = 'resnet18_baseline_att_224x224_A_epoch_249.pth'
    OPTIMIZED_MODEL = 'resnet18_baseline_att_224x224_A_epoch_249_trt.pth'
    model = trt_pose.models.resnet18_baseline_att(num_parts, 2 * num_links).cuda().eval()
    WIDTH = 224
    HEIGHT = 224

else:    
    print('------ model = densenet--------')
    MODEL_WEIGHTS = 'densenet121_baseline_att_256x256_B_epoch_160.pth'
    OPTIMIZED_MODEL = 'densenet121_baseline_att_256x256_B_epoch_160_trt.pth'
    model = trt_pose.models.densenet121_baseline_att(num_parts, 2 * num_links).cuda().eval()
    WIDTH = 256
    HEIGHT = 256

d1 = torch.zeros((1, 3, HEIGHT, WIDTH)).cuda()

if os.path.exists(OPTIMIZED_MODEL) == False:
    model.load_state_dict(torch.load(MODEL_WEIGHTS))
    model_trt = torch2trt.torch2trt(model, [d1], fp16_mode=True)
    torch.save(model_trt.state_dict(), OPTIMIZED_MODEL)

model_trt = TRTModule()
model_trt.load_state_dict(torch.load(OPTIMIZED_MODEL))

mean = torch.Tensor([0.485, 0.456, 0.406]).cuda()
std = torch.Tensor([0.229, 0.224, 0.225]).cuda()
device = torch.device('cuda')


def timing():
    t0 = time.time()
    torch.cuda.current_stream().synchronize()
    for i in range(500):
        y = model_trt(d1)
    torch.cuda.current_stream().synchronize()
    t1 = time.time()

    print(500.0 / (t1 - t0))

while True:
    timing()

exit(1)

def execute(img):
    image = cv2.resize(img, dsize=(WIDTH, HEIGHT), interpolation=cv2.INTER_AREA)
    image = PIL.Image.fromarray(image)
    image = transforms.functional.to_tensor(image).to(device)
    image.sub_(mean[:, None, None]).div_(std[:, None, None])

    cmap, paf = model_trt(image[None, ...])
    cmap, paf = cmap.detach().cpu(), paf.detach().cpu()

    counts, objects, peaks = parse_objects(cmap, paf)#, cmap_threshold=0.15, link_threshold=0.15)
    print(counts)
    draw_objects(img, counts, objects, peaks)
    if counts[0] > 0:
        human = objects[0][0]
        left_eye_idx = 0
        human_idx = int(human[left_eye_idx])
        if human_idx >= 0:
            left_eye_peak = peaks[0][left_eye_idx][human_idx] 
            #print(left_eye_peak)
            return left_eye_peak
    return None
               
 

pipeline = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.color,    640, 480, rs.format.bgr8, 60)
config.enable_stream(rs.stream.infrared, 640, 480, rs.format.y8, 60)
config.enable_stream(rs.stream.depth,    640, 480, rs.format.z16, 60)

# Start streaming
pipeline_profile = pipeline.start(config)
device = pipeline_profile.get_device()
depth_sensor = device.query_sensors()[0]
depth_sensor.set_option(rs.option.emitter_enabled, 0)

parse_objects = ParseObjects(topology)
draw_objects = DrawObjects(topology)

device = torch.device('cuda')

print("Streaming...")
while True:
    frames = pipeline.wait_for_frames()
    img_frame = frames.get_infrared_frame()
    depth_frame = frames.get_depth_frame()
    t = time.time()

    img = np.asanyarray(img_frame.get_data())
    img = cv2.cvtColor(img,cv2.COLOR_GRAY2BGR)
    #cv2.imshow('image',img)
    #continue
    part = execute(img)
    fps = 1.0 / (time.time() - t)
    print("FPS:%f "%(fps))

    if part != None:
        point = (part[1]*640, part[0]*480)
        img = cv2.circle(img, point, radius=9, color=(0, 0, 255), thickness=-1)

        depth_intrinsics = depth_frame.profile.as_video_stream_profile().intrinsics
        xyz = rs.rs2_deproject_pixel_to_point(depth_intrinsics, [point[0], point[1]], depth_frame.get_distance(point[0], point[1]))
        print(xyz)

    #cv2.imshow('image',img)
    #cv2.waitKey(1)


cv2.destroyAllWindows()

