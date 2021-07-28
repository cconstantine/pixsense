import sqlite3
import glm
import datetime

class Sqlite3Tracking:
    def __init__(self):
        self.db = sqlite3.connect(":memory:")
        self.db.row_factory = sqlite3.Row
        c = self.db.cursor()
        c.execute("CREATE TABLE people ( x float not null, y float not null, z float not null, track_at timestamp, updated_at timestamp not null)")
        c.execute("CREATE INDEX people_loc ON people ( x , y, z)")
        c.execute("CREATE INDEX people_time ON people ( updated_at)")
        c.execute("CREATE INDEX people_track ON people(track_at)")
        c.execute('''PRAGMA synchronous = OFF''')
        c.execute('''PRAGMA journal_mode = OFF''')

        self.window = 0.100
        self.tracked = None

    def update(self, people, now = None):
        if not now:
            now = datetime.datetime.now()
        c = self.db.cursor()
        for person in people:
            c.execute("""
                UPDATE people
                SET x = ?, y = ?, z = ?, updated_at = ?
                WHERE rowid IN (
                    SELECT rowid
                    FROM people
                    WHERE
                    x BETWEEN ? AND ? AND
                    y BETWEEN ? AND ? AND
                    z BETWEEN ? AND ?
                    LIMIT 1)
            """,
            (person.x, person.y, person.z, now,
            person.x - self.window, person.x + self.window,
            person.y - self.window, person.y + self.window,
            person.z - self.window, person.z + self.window ))

            if c.rowcount == 0:
                c.execute("INSERT INTO people (x, y, z, updated_at) VALUES(?, ?, ?, ?)", (person.x, person.y, person.z, now))
            elif c.rowcount > 1:
                print(c.rowcount)
        
        expiry = now - datetime.timedelta(seconds=1)
        c.execute("DELETE FROM people WHERE updated_at < ?", ( expiry, ))

        expiry = now - datetime.timedelta(seconds=30)
        c.execute("SELECT 1 from people where track_at > ? limit 1", ( expiry, ))
        if not c.fetchall():
            if self.tracked:
                c.execute(f"UPDATE people SET track_at = ? WHERE rowid != ? ORDER BY RANDOM() limit 1", ( now, self.tracked["rowid"]))
            else:
                c.execute(f"UPDATE people SET track_at = ? ORDER BY RANDOM() limit 1", ( now, ))

        newly_tracked = self.get_tracked()
        if not newly_tracked:
            return None
        if not self.tracked or self.tracked["updated_at"] < newly_tracked["updated_at"]:
            self.tracked = newly_tracked
            return self.tracked
        return None

    def get_tracked(self):
        c = self.db.cursor()
        c.execute("SELECT rowid, * from people where track_at not null order by track_at desc limit 1")
        return c.fetchone()

if __name__ == '__main__':
    t = Tracking()
    window = datetime.timedelta(seconds=60)

    def timing():
        start_time = datetime.datetime.now()
        for i in range(1,1000000):
            tracked = t.update(glm.vec3(i, 2, 3))
            delta = datetime.datetime.now() - start_time
            if tracked:
                print(f"{i}: {dict(tracked)}")
            if delta > window:
                break
            

    while True:
        timing()
