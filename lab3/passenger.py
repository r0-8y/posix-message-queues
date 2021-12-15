import string
import random
import time


def spawn_passenger(db):
    random.seed()
    while True:
        p = Passenger(db)
        time.sleep(10)
        # time.sleep(random.randint(2, 12 - db.load * 3))
        if len(db.waiting[p.location]) == 10:
            continue
        else:
            db.waiting[p.location].append(p)
            db.elevator.target_floors.add(p.location)
        # time.sleep(random.randint(2, 12 - db.load * 3))


class Passenger:
    def __init__(self, db):
        self.db = db
        self.name = random.choice(string.ascii_letters)
        self.location, self.destination = random.sample([1, 2, 3, 4], 2)

    def __str__(self):
        return f'Ime: {self.name}, od: {self.location}, do: {self.destination}'

