.PHONY: all
all: generator NewPrimeCounter primeCounter

generator:  generator.c
	gcc -o randomGenerator generator.c

NewPrimeCounterrimeCounter:	NewPrimeCounter.c
	gcc -o NewPrimeCounter NewPrimeCounter.c

primeCounter:  primeCounter.c
	gcc -o primeCounter primeCounter.c

.PHONY: clean
clean:
	-rm randomGenerator NewPrimeCounter primeCounter 2>/dev/null
