
from dataclasses import dataclass, field
import glm
from datetime import datetime

@dataclass
class Person:
    xyz: glm.vec3
    tracked_at: datetime = None
    updated_at: datetime = field(default_factory=datetime.now)

    def __eq__(self, other):
        return self.xyz == other.xyz
