import string
import random


class Passenger:
    def __init__(self, db):
        self.db = db
        self.name = random.choice(string.ascii_letters)
        self.location, self.destination = random.sample([1, 2, 3, 4], 2)

    def __str__(self):
        return f'Ime: {self.name}, od: {self.location}, do: {self.destination}'

    def get_in(self):
        pass

    def get_out(self):
        pass
