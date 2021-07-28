import pyrealsense2.pyrealsense2 as rs
import numpy as np
import cv2
import poser
import time
from datetime import datetime

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
    def __init__(self, camera_id):
        self.camera_id = camera_id
    
    def enable(self):
        print("Enabling camera {0}".format(self.camera_id))
        self.pipeline = rs.pipeline()
        config = rs.config()
        config.enable_device(self.camera_id)
        config.enable_stream(rs.stream.infrared, 848, 480, rs.format.y8, 60)
        config.enable_stream(rs.stream.depth,    848, 480, rs.format.z16, 60)
        pipeline_profile=self.pipeline.start(config)
        pipeline_profile.get_device().sensors[0].set_option(rs.option.emitter_enabled, 0)

        self.resn = poser.DetectorResnet((848, 480), False)

    def wait_for_frames(self):
        frames = self.pipeline.wait_for_frames()
        depth_frame = frames.get_depth_frame()
        color_frame = frames.get_infrared_frame(0)
        if not depth_frame or not color_frame:
            print("Missing frames from camera {0}:".format(self.camera_id))
            if not depth_frame_1:
                print(" - depth")
            if not color_frame_1:
                print(" - visible")
            return (None, None)
        # Convert images to numpy arrays
        # depth_image = np.asanyarray(depth_frame.get_data())
        color_image = np.asanyarray(color_frame.get_data())

        # cv2.imshow(PIL.Image.fromarray(color_image))
        color_image = cv2.cvtColor(color_image,cv2.COLOR_GRAY2BGR)

        return (color_image, depth_frame)
    
    def keypoints_to_people(self, keypoints, depth_frame):
        depth_intrinsics = depth_frame.profile.as_video_stream_profile().intrinsics
        people = []
        now = datetime.now()
        for keypoint in keypoints:
            uv = keypoint[0][1:3]
            if uv:
                distance = depth_frame.get_distance(*uv)
                xyz = glm.vec3(rs.rs2_deproject_pixel_to_point(depth_intrinsics, uv, distance))
                if xyz != glm.vec3(0.0):
                    xyz.y = -xyz.y
                    xyz.x = -xyz.x
                    people.append(Person(xyz, updated_at=now))
        return people

    def detect(self, color_image, depth_frame):
        keypoints = camera.resn.detect(color_image)
        return self.keypoints_to_people(keypoints, depth_frame)
        
if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='TensorRT pose estimation run')
    parser.add_argument('--device', type=str, help = 'Device id (049222073570 or 038122250538)' )
    args = parser.parse_args()


    print("Streaming...")
    camera = RSCamera(args.device)
    camera.enable()
    fps = Timer(10)
    latency = 0.0
    con = psycopg2.connect('')

    crowd = tracking.Sqlite3Tracking()
    while True:
        fps.tick()
        color_image, depth_frame = camera.wait_for_frames()
        t1 = time.time()
        people = camera.detect(color_image, depth_frame)
        target = crowd.update([ person.xyz for person in people ])
        if target:
            with con.cursor() as cur:
                cur.execute("SET synchronous_commit = 'off'")
                cur.execute("""
                    INSERT INTO tracking_locations(name, x, y, z)
                    VALUES ('pixo-8', %s, %s, %s)
                    ON CONFLICT (name) DO UPDATE set x = EXCLUDED.x, y = EXCLUDED.y, z = EXCLUDED.z
                    RETURNING name, x, y, z
                    """,
                    (target["x"], target["y"] - 0.30, target["z"] - 0.18))
                con.commit()

        t2 = time.time()
        # for i, person in enumerate(people):
        #     #cv2.circle(camera.resn.image, person.uv, radius=9, color=(0, 0, 255), thickness=-1)
        #     if i > 0:
        #         continue
        #     with con.cursor() as cur:
        #         cur.execute("SET synchronous_commit = 'off'")
        #         cur.execute("""
        #             INSERT INTO tracking_locations(name, x, y, z)
        #             VALUES ('pixo-16', %s, %s, %s)
        #             ON CONFLICT (name) DO UPDATE set x = EXCLUDED.x, y = EXCLUDED.y, z = EXCLUDED.z
        #             RETURNING name, x, y, z
        #             """,
        #             person.xyz)
        #         con.commit()


        #cv2.imshow(args.device, camera.resn.image)
        #cv2.waitKey(1)
            
        latency = (1.9*latency + 0.1*(t2 - t1)) / 2
        print("{0}: {1: 2d} :: {2:7.2f}fps ({3: 7.4f}ms)".format(args.device, len(people), fps.fps(), 1000*latency))
