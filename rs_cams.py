import pyrealsense2.pyrealsense2 as rs
import numpy as np
import cv2
import poser
import time
from datetime import datetime
from statistics import mean
import logging
logger = logging.getLogger(__name__)

import collections
import PIL.Image

import psycopg2
import glm

import tracking
from person import Person

class Timer:
    def __init__(self, size):
        self.size = size
        self.times = collections.deque()
        self.sum = 0
        self.time = time.time()

    def tick(self):
        t = time.time()
        delta = t - self.time
        self.time = t

        self.sum += delta
        self.times.append(delta)
        if len(self.times) > self.size:
            self.sum -= self.times.popleft()

    def fps(self):
        return len(self.times) / self.sum

    def latency(self):
        sum(self.times) / len(self.times)

class RSCamera:
    def __init__(self, camera_id, model, optimize, offset, rotate):
        self.camera_id = camera_id
        self.model = model
        self.optimize = optimize
        self.offset = offset
        self.rotate = rotate
    
    def enable(self):
        logger.info("Enabling camera {0}".format(self.camera_id))
        self.pipeline = rs.pipeline()
        config = rs.config()
        config.enable_device(self.camera_id)
        config.enable_stream(rs.stream.infrared, 848, 480, rs.format.y8, 60)
        config.enable_stream(rs.stream.depth,    848, 480, rs.format.z16, 60)
        pipeline_profile=self.pipeline.start(config)
        pipeline_profile.get_device().sensors[0].set_option(rs.option.emitter_enabled, 0)
        if self.model == 'resnet':
            self.resn = poser.DetectorResnet((848, 480), self.optimize)
        elif self.model == 'densenet':
            self.resn = poser.DetectorDensenet((848, 480), self.optimize)
        else:
            raise RuntimeError(f"Unknown model: {self.model}")

    def wait_for_frames(self):
        frames = self.pipeline.wait_for_frames()
        depth_frame = frames.get_depth_frame()
        color_frame = frames.get_infrared_frame(0)
        if not depth_frame or not color_frame:
            logger.warning("Missing frames from camera {0}:".format(self.camera_id))
            if not depth_frame:
                logger.warning(" - depth")
            if not color_frame:
                logger.warning(" - visible")
            return (None, None)
        # Convert images to numpy arrays
        # depth_image = np.asanyarray(depth_frame.get_data())
        color_image = np.asanyarray(color_frame.get_data())

        # cv2.imshow(PIL.Image.fromarray(color_image))
        color_image = cv2.cvtColor(color_image,cv2.COLOR_GRAY2BGR)

        return (color_image, depth_frame)
    
    def min_distance(self, depth_frame, x, y, window = 5):
        min = 0
        for i in range(-window + 1, window):
            x_i = x+i
            if x_i >= 0 and x_i < depth_frame.width:
                for j in range(-window + 1, window):
                    y_j = y+j
                    if y_j >= 0 and y_j < depth_frame.height:
                        distance = depth_frame.get_distance(x_i, y_j)
                        if distance > 0 and distance < min or min == 0:
                            min = distance
        return min
    
    def keypoints_to_people(self, keypoints, depth_frame):
        depth_intrinsics = depth_frame.profile.as_video_stream_profile().intrinsics
        people = []
        now = datetime.now()
        for keypoint in keypoints:
            uv = keypoint[0][1:3]
            if uv:
                xyz = glm.vec3(rs.rs2_deproject_pixel_to_point(depth_intrinsics, uv, self.min_distance(depth_frame, *uv)))
                if xyz != glm.vec3(0.0):
                    xyz.y = -xyz.y
                    xyz.x = -xyz.x
                    xyz += self.offset
                    xyz = glm.rotate(xyz, glm.radians(self.rotate), glm.vec3(0.0, 1.0, 0.0))
                    people.append(Person(xyz, updated_at=now))
        return people

    def detect(self, color_image, depth_frame):
        keypoints = camera.resn.detect(color_image)
        return self.keypoints_to_people(keypoints, depth_frame)
        
if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser(description='TensorRT pose estimation run')
    parser.add_argument('--device', type=str, help = 'Device id (049222073570 or 038122250538)' )
    parser.add_argument('--log_level', default="INFO", type=str, help = 'Logging level' )
    parser.add_argument('--model', type=str, default='densenet', help = 'resnet or densenet' )
    parser.add_argument('--optimize', default=False, action='store_true', help = 'Generate a new optimized trt module' )
    parser.add_argument('-x', type=float, default=0, help="x offset")
    parser.add_argument('-y', type=float, default=0, help="y offset")
    parser.add_argument('-z', type=float, default=0, help="z offset")
    parser.add_argument('-r', type=float, default=0, help="camera rotation (degrees)")
    parser.add_argument('--fps', type=float, default=10, help="non-tracking fps limit")

    #cd.rotation = glm::rotate(glm::radians(dlib::get_option(config, "rotation", 0.0f)), glm::vec3(0.0f, 1.1f, 0.0f));

    args = parser.parse_args()
    fps_time = 1.0/args.fps
    logging.basicConfig(level=getattr(logging, args.log_level.upper()))

    logger.info("Streaming...")
    camera = RSCamera(args.device, args.model, args.optimize, offset=glm.vec3(args.x, args.y, args.z), rotate=args.r)
    camera.enable()
    fps = Timer(10)
    con = psycopg2.connect('')

    crowd = tracking.PGTracking('pixo-16')
    while True:
        t0 = time.time()
        fps.tick()
        color_image, depth_frame = camera.wait_for_frames()

        t1 = time.time()
        people = camera.detect(color_image, depth_frame)
        is_tracking = crowd.update([ person.xyz for person in people ])
        t2 = time.time()

        latency = (t2 - t1)/2
        num_people = len(people)
        logger.info("{0} |{1}| {2:7.2f}fps ({3: 7.4f}ms)".format(args.device, "*"*num_people + " "* (10-num_people), fps.fps(), 1000*latency))
        td = fps_time - (time.time() - t0)
        if not is_tracking and td > 0:
            time.sleep(td)
