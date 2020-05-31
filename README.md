## Introduction



## Problem Defination

constraint: mileage, load, work time

heterogenous vehicle

multi-trip

(multi-depot)

(time window)



## Hybrid Genetic Algorithm



### Genome Coding

![image](http://github.com/btnkij/VehicleRouting/raw/master/img/coding.jpg)

loop



### Initialize

#### Clark-Wright Saving



#### Random



### Crossover

![avatar](./img/crossover.jpg)



### Mutate

#### Customer-Based Mutate

* Swap

  ![avatar](./img/swap.jpg)

* Shift

  ![avatar](./img/shift.jpg)

* Reverse

  ![avatar](./img/reverse.jpg)



#### Vehicle-Based Mutate

* Change

  ![avatar](./img/change.jpg)

* Insert

  ![avatar](./img/insert.jpg)

* Remove

![avatar](./img/remove.jpg)



### Finetune

local search



### Select

50% elite

50% simulated annealing



### Migrate

![avatar](./img/migrate.jpg)

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



