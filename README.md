## Introduction



## Problem Defination

constraint: mileage, load, work time

heterogenous vehicle

multi-trip

(multi-depot)

(time window)



## Hybrid Genetic Algorithm

[flow chart]

### Genome Coding

![image](./img/coding.JPG)

loop



### Initialize

#### Clark-Wright Saving



#### Random



### Crossover

![avatar](./img/crossover.JPG)



### Mutate

#### Customer-Based Mutate

* Swap

  ![avatar](./img/swap.JPG)

* Shift

  ![avatar](./img/shift.JPG)

* Reverse

  ![avatar](./img/reverse.JPG)



#### Vehicle-Based Mutate

* Change

  ![avatar](./img/change.JPG)

* Insert

  ![avatar](./img/insert.JPG)

* Remove

![avatar](./img/remove.JPG)



### Finetune

local search



### Select

50% elite

50% simulated annealing



### Migrate

![avatar](./img/migrate.JPG)

migrate 20% elite every 15 generations, select



## Hyperparameter

popsize

eliteRate

crossoverRate

mutationRate

finetuneRate

migrationRate



## Experiment

### Interface



### Benchmark



## References



