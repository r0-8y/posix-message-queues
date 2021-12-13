class Elevator:
    def __init__(self, db, capacity=6):
        self.db = db
        self.passengers = []
        self.stops = [False, False, False, False]
        self.direction = ' '
        self.doors = 'Z'
        self.floor = 1
        self.capacity = capacity


