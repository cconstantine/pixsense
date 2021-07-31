import unittest
import time
import glm
from datetime import datetime, timedelta
from person import Person
import tracking

class TrackingTest():
    def test_single_person(self):
        pos_1 = glm.vec3(0.0, 0.0, 0.0)
        result = self.crowd.update([pos_1], datetime.now())
        result = glm.vec3(result["x"], result["y"], result["z"])
        self.assertEqual(pos_1, result)

        result = self.crowd.update([pos_1], datetime.now() + timedelta(milliseconds=10))
        result = glm.vec3(result["x"], result["y"], result["z"])
        self.assertEqual(pos_1, result)

        result = self.crowd.update([pos_1], datetime.now() + timedelta(minutes=10))
        result = glm.vec3(result["x"], result["y"], result["z"])
        self.assertEqual(pos_1, result)

    def test_two_people_sticks(self):
        pos_1 = glm.vec3(0.0, 0.0, 0.0)
        pos_2 = glm.vec3(1.0, 0.0, 0.0)
        result = self.crowd.update([pos_1, pos_2], datetime.now())
        tracked_1 = glm.vec3(result["x"], result["y"], result["z"])

        result = self.crowd.update([pos_1, pos_2], datetime.now() + timedelta(milliseconds=10))
        tracked_2 = glm.vec3(result["x"], result["y"], result["z"])

        self.assertEqual(tracked_1, tracked_2)

    def test_two_people_changes(self):
        for _ in range(1):
            pos_1 = glm.vec3(0.0, 0.0, 0.0)
            pos_2 = glm.vec3(1.0, 0.0, 0.0)
            result = self.crowd.update([pos_1, pos_2], datetime.now())
            tracked_1 = glm.vec3(result["x"], result["y"], result["z"])

            result = self.crowd.update([pos_1, pos_2], datetime.now() + timedelta(minutes=10))
            tracked_2 = glm.vec3(result["x"], result["y"], result["z"])

            self.assertNotEqual(tracked_1, tracked_2)

# class Sqlite3TrackingTest(TrackingTest, unittest.TestCase):
#     def setUp(self):
#         self.crowd = tracking.Sqlite3Tracking()

class PGTrackingTest(TrackingTest, unittest.TestCase):
    def setUp(self):
        self.crowd = tracking.PGTracking("test")