# SpreadSheet
## An analogue of excel, working with formulas and cells, finding cyclic dependencies


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
sheet->SetCell(Position::FromString("A1"), "1");
sheet->SetCell(Position::FromString("A2"), "=A1");
sheet->SetCell(Position::FromString("B2"), "=A1");
```



