1. pokretanje posluzitelja
    gcc -pthread -Wall posluzitelj.c -o posluzitelj -lrt
    ./posluzitelj 4 30

2. pokretanje generatora
    gcc -pthread -Wall generator.c -o generator -lrt
    ./generator 4 10

3. priprema okruzenja
    cd Documents/srsv/lab5/
    export SRSV_LAB5=lab5sim
    rm /dev/shm/* & rm /dev/mqueue/lab5sim