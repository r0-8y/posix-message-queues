import time


def arrived(db):
    arrived_passengers = list(filter(lambda passenger: passenger.destination == db.elevator.floor, db.elevator.passengers))
    if len(arrived_passengers) > 0:
        # otvaranje vrata
        time.sleep(1)
        db.elevator.doors = 'O'
        for passenger in arrived_passengers:
            db.elevator.passengers.remove(passenger)
            db.arrived[db.elevator.floor].append(passenger)
        # cekamo da izadu
        time.sleep(1)
        if len(db.waiting[db.elevator.floor]) > 0:
            while len(db.waiting[db.elevator.floor]) != 0:
                if len(db.elevator.passengers) < db.elevator.capacity:
                    p = db.waiting[db.elevator.floor].pop()
                    db.elevator.passengers.append(p)
                    db.elevator.target_floors.add(p.destination)
        # zatvaranje vrata
        time.sleep(1)
        db.elevator.doors = 'Z'
    else:
        if len(db.waiting[db.elevator.floor]) > 0:
            # otvaranje vrata
            time.sleep(1)
            db.elevator.doors = 'O'
            while len(db.waiting[db.elevator.floor]) != 0:
                if len(db.elevator.passengers) < db.elevator.capacity:
                    p = db.waiting[db.elevator.floor].pop()
                    db.elevator.passengers.append(p)
                    db.elevator.target_floors.add(p.destination)
            # zatvaranje vrata
            time.sleep(1)
            db.elevator.doors = 'Z'
    db.elevator.stops = [False, False, False, False]
    for passenger in db.elevator.passengers:
        db.elevator.stops[passenger.destination - 1] = True


def simulate_elevator(db):
    while True:
        if len(db.elevator.passengers) > 0:
            for passenger in db.elevator.passengers:
                db.elevator.target_floors.add(passenger.destination)
        if len(db.elevator.target_floors) > 0:
            target_floor = db.elevator.target_floors.pop()
            if target_floor == db.elevator.floor:
                arrived(db)
            while db.elevator.floor != target_floor:
                if target_floor < db.elevator.floor:
                    db.elevator.direction = 'D'
                    db.elevator.floor -= 0.5
                    if db.elevator.floor % 1 == 0:
                        arrived(db)
                elif target_floor > db.elevator.floor:
                    db.elevator.direction = 'G'
                    db.elevator.floor += 0.5
                    if db.elevator.floor % 1 == 0:
                        arrived(db)
                else:
                    arrived(db)
                time.sleep(1)
        else:
            time.sleep(1)


class Elevator:
    def __init__(self, db, capacity=6):
        self.db = db
        self.passengers = []
        self.stops = [False, False, False, False]
        self.direction = ' '
        self.doors = 'Z'
        self.floor = 1
        self.capacity = capacity
        self.target_floors = set()
