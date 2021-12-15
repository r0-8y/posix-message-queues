import concurrent.futures
from gui import print_state
from passenger import spawn_passenger
from elevator import simulate_elevator
from elevator import Elevator


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


if __name__ == '__main__':
    db = DB(load=2)
    with concurrent.futures.ThreadPoolExecutor() as executor:
        gui = executor.submit(print_state, db)
        passenger = executor.submit(spawn_passenger, db)
        elevator = executor.submit(simulate_elevator, db)
