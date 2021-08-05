import sqlite3
import glm
import datetime
import psycopg2
import psycopg2.extras
import logging
logger = logging.getLogger(__name__)

import uuid

class PGTracking:
    def __init__(self, sculpture_name):
        self.con = psycopg2.connect('')
        self.con.cursor().execute("SET statement_timeout = 16")
        self.con.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_SERIALIZABLE)
        self.sculpture_name = sculpture_name

        self.window = 0.1

    def update(self, people, now = None):
        try:
            self._update(people, now)
        except psycopg2.errors.SerializationFailure:
            pass
        except psycopg2.errors.DeadlockDetected as e:
            logger.error(e)
    
    def _update(self, people, now = None):
        if not now:
            now = datetime.datetime.now()
        with self.con:
            with self.con.cursor() as cur:
                cur.execute("SET synchronous_commit = 'off'")

                for person in people:
                    cur.execute("""
                        UPDATE tracking_locations
                        SET x = %s, y = %s, z = %s, updated_at = %s
                        WHERE name IN (
                            SELECT name
                            FROM tracking_locations
                            WHERE
                            x BETWEEN %s AND %s AND
                            y BETWEEN %s AND %s AND
                            z BETWEEN %s AND %s
                            LIMIT 1
                        )
                        RETURNING 1
                        """,
                        (person.x, person.y, person.z, now,
                        person.x - self.window, person.x + self.window,
                        person.y - self.window, person.y + self.window,
                        person.z - self.window, person.z + self.window ))
                    if len(cur.fetchall()) == 0:
                        cur.execute("""
                        INSERT INTO tracking_locations(name, x, y, z, updated_at)
                        VALUES(%s, %s, %s, %s, %s)
                        """,
                        (str(uuid.uuid1()), person.x, person.y, person.z, now))
                
                expiry = now - datetime.timedelta(seconds=1)
                cur.execute("DELETE FROM tracking_locations WHERE updated_at < %s", ( expiry, ))
        with self.con:
            with self.con.cursor(cursor_factory = psycopg2.extras.RealDictCursor) as cur:
                cur.execute("SET synchronous_commit = 'off'")
                expiry = now - datetime.timedelta(seconds=30)
                tracked = self.get_tracked(cur)
                if not tracked or tracked["tracked_at"] < expiry:
                    tracked_uuid = str(uuid.uuid1())
                    cur.execute("UPDATE tracking_locations SET name = %s WHERE name = %s", (tracked_uuid,self.sculpture_name))
                    cur.execute("""
                    UPDATE tracking_locations
                    SET tracked_at = %s, name = %s
                    WHERE name in (SELECT name from tracking_locations WHERE name != %s ORDER BY RANDOM() limit 1)
                    """, ( now, self.sculpture_name, tracked_uuid))
                return self.get_tracked(cur)

    def get_tracked(self, cur=None):
        if cur:
            cur.execute("SELECT * from tracking_locations where tracked_at is not null order by tracked_at desc limit 1")
            return cur.fetchone()
        with self.con.cursor(cursor_factory = psycopg2.extras.RealDictCursor) as cur:
            cur.execute("SELECT * from tracking_locations where tracked_at is not null order by tracked_at desc limit 1")
            return cur.fetchone()

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
    t = PGTracking("benchtest")
    window = datetime.timedelta(seconds=1)

    def timing():
        start_time = datetime.datetime.now()
        for i in range(1,1000000):
            tracked = t.update([glm.vec3(x, 2, 3) for x in range(100)])
            delta = datetime.datetime.now() - start_time
            # if tracked:
            #     print(f"{i}: {dict(tracked)}")
            if delta > window:
                print(10 / i)
                break
            

    while True:
        timing()
