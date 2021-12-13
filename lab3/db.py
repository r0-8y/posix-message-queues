from passenger import Passenger
from elevator import Elevator
from gui import print_state
import time


class DB:
    def __init__(self, eleveator=None, load=1):
        if eleveator is None:
            self.elevator = Elevator(self)
        else:
            self.elevator = eleveator
        self.load = load
        self.waiting = {}
        self.arrived = {}
        for i in range(1, 5):
            self.waiting[i] = []
            self.arrived[i] = []

    def simulate(self):
        while True:
            p = Passenger(self)
            if len(self.waiting[p.location]) == 10:
                continue
            else:
                self.waiting[p.location].append(p)
            for i in range(20 - self.load * 5):
                print_state(self)
                all_passengers = []
                for passengers in self.waiting.values():
                    for p in passengers:
                        all_passengers.append(p)
                for passengers in self.arrived.values():
                    for p in passengers:
                        all_passengers.append(p)
                for p in self.elevator.passengers:
                    all_passengers.append(p)
                time.sleep(1)
