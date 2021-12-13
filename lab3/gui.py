def print_elevator(db, floor):
    if db.elevator.floor == floor:
        print('|[', end='')
        for p in db.elevator.passengers:
            print(p.name, end='')
        for i in range(6 - len(db.elevator.passengers)):
            print(end=' ')
        print(']|', end=' ')
    else:
        print('|        |', end=' ')


def print_state(db):
    print('               Lift1')
    print(f'Smjer/vrata:    {db.elevator.direction} {db.elevator.doors}')
    print('Stajanja:===== ', end='')
    for stop in db.elevator.stops:
        if stop:
            print('*', end='')
        else:
            print('-', end='')
    print(' == Izašli')

    for floor, passengers in reversed(list(db.waiting.items())):
        print(f'{floor}:', end='')
        for p in passengers:
            print(p.name, end='')
        for i in range(10 - len(passengers)):
            print(end=' ')
        print_elevator(db, floor)
        if len(db.arrived[floor]) != 0:
            for p in db.arrived[floor]:
                print(p.name, end='')
        print()
        if floor != 1:
            print('  ==========', end='')
            print_elevator(db, floor - 0.5)
        print()
    print('========================')

    all_passengers = []
    for passengers in db.waiting.values():
        for p in passengers:
            all_passengers.append(p)
    for passengers in db.arrived.values():
        for p in passengers:
            all_passengers.append(p)
    for p in db.elevator.passengers:
        all_passengers.append(p)
    print('Putnici:', end=' ')
    for p in all_passengers:
        print(p.name, end='')
    print('\n     od:', end=' ')
    for p in all_passengers:
        print(p.location, end='')
    print('\n     do:', end=' ')
    for p in all_passengers:
        print(p.destination, end='')
    print()
