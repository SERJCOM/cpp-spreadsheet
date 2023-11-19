# SpreadSheet
## An analogue of excel, working with formulas and cells, finding cyclic dependencies

<img src="https://profile-counter.glitch.me/{SERJCOM}/count.svg" alt="madushadhanushka :: Visitor's Count" />

## Features
+ Looping Search
+ Сalculation of formulas
+ Сell references
+ Checking the correctness of the cell

## TODO
+ Make a graphical interface
+ Add more functions for cells

## Examples
Example of creating a table and setting values for cells
```
auto sheet = CreateSheet();
sheet->SetCell("A1"_pos, "1");
sheet->SetCell("A2"_pos, "=A1");
sheet->SetCell("B2"_pos, "=A1");
```



