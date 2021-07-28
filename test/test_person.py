import unittest
import time
import glm
from datetime import datetime
from person import Person

class PersonTest(unittest.TestCase):
    def test_updated_at_changes(self):
        p1 = Person(glm.vec3())
        time.sleep(0.01)
        p2 = Person(glm.vec3())

        self.assertNotEqual(p1.updated_at, p2.updated_at)

    def test_updated_at_set(self):
        now = datetime.now()
        p1 = Person(glm.vec3(), updated_at=now)
        time.sleep(0.01)
        p2 = Person(glm.vec3(), updated_at=now)
        
        self.assertEqual(p1.updated_at, p2.updated_at)

    def test_eq_equal_xyz(self):
        p1 = Person(glm.vec3(1, 1, 1))
        time.sleep(0.01)
        p2 = Person(glm.vec3(1, 1, 1))

        self.assertNotEqual(p1.updated_at, p2.updated_at)
        self.assertEqual(p1, p2)
        
