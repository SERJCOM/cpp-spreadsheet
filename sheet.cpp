#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <vector>


using namespace std::literals;


void Sheet::SetCell(Position pos, std::string text) {
    if(!pos.IsValid()){
        throw InvalidPositionException("позиция ошибочна");
    }

    // if(IsCellDeleted(pos)){
    //     SaveCell(std::move(deleted_cells_.at(pos)), pos);
    //     deleted_cells_.erase(pos);
    // }

    if(position_cell_.count(pos) == 0){
        std::unique_ptr<Cell> cell = std::make_unique<Cell>(*this);

        SaveCell(std::move(cell), pos);

        position_cell_[pos]->Set(text);
    }
    else if(GetConcreteCell(pos)->IsEmpty()){
        std::unique_ptr<Cell> cell = std::make_unique<Cell>(*this);

        SaveCell(std::move(cell), pos);

        position_cell_[pos]->Set(text);
    }
    else{
        position_cell_.at(pos)->Set(text);
    }

}

const CellInterface* Sheet::GetCell(Position pos) const {
    if(!pos.IsValid()){
        throw InvalidPositionException("позиция ошибочна");
    }
    if(position_cell_.count(pos) == 0 ){
        return nullptr;
    }

    return position_cell_.at(pos).get();
}

CellInterface* Sheet::GetCell(Position pos) {
    if(!pos.IsValid()){
        throw InvalidPositionException("позиция ошибочна");
    }
    if(position_cell_.count(pos) == 0 ){
        return nullptr;
    }

    return position_cell_.at(pos).get();
}

void Sheet::ClearCell(Position pos) {
    if(!pos.IsValid()){
        throw InvalidPositionException("позиция ошибочна");
    }
    if(GetCell(pos)){
        if(!GetConcreteCell(pos)->IsReferenced()){
            position_cell_.erase(pos);
        }
        else{
            GetConcreteCell(pos)->Clear();
        }
        
        row_cell_.at(pos.row).erase(pos.col);
        if(row_cell_.at(pos.row).size() == 0){
            row_cell_.erase(pos.row);
        }
        col_cell_.at(pos.col).erase(pos.row);
        if(col_cell_.at(pos.col).size() == 0){
            col_cell_.erase(pos.col);
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size size{0, 0};

    if(row_cell_.size() > 0){
        int row_max = row_cell_.rbegin()->first + 1;
        size.rows = row_max  ;
    }
    if(col_cell_.size() > 0){
        int col_max = col_cell_.rbegin()->first + 1;
        size.cols = col_max  ;
    }
    
    return size;
}

void PrintValue(std::ostream& output, double value){
    output << value;
}

void PrintValue(std::ostream& output, const std::string& value){
    output << value;
}

void PrintValue(std::ostream& output, FormulaError value){
    output << value;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();

    for(int row = 0; row < size.rows; row++){
        for(int col = 0; col < size.cols; col++){
            Position pos{row, col};
            auto cell = GetCell(pos);
            if(cell){
                auto value = cell->GetValue();
                std::visit([&output](auto value){
                    PrintValue(output, value);
                }, value);
            }
            if(col != size.cols - 1)
                output << '\t';
        }

        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    for(int row = 0; row < size.rows; row++){
        for(int col = 0; col < size.cols; col++){
            Position pos{row, col};
            auto cell = GetCell(pos);
            if(cell){
                output << cell->GetText();
            }
            if(col != size.cols - 1)
                output << '\t';
        }

        output << '\n';
    }
}

const Cell *Sheet::GetConcreteCell(Position pos) const
{
    return position_cell_.at(pos).get();
}

Cell *Sheet::GetConcreteCell(Position pos)
{
    return position_cell_.at(pos).get();
}

Cell *Sheet::GetOrCreateCell(Position pos)
{
    if(position_cell_.count(pos) == 0 ){
        SetCell(pos, "");
    }

    return GetConcreteCell(pos);
}

CellInterface::Value Sheet::GetValue(Position pos) const
{

    if(!pos.IsValid()){
        throw FormulaError(FormulaError::Category::Ref);
    }

    if(position_cell_.count(pos) == 0){
        return 0.0;
    }

    return GetConcreteCell(pos)->GetValue();
}

void Sheet::SaveCell(std::unique_ptr<Cell> cell, Position pos)
{
    position_cell_[pos] = std::move(cell);
    row_cell_[pos.row].insert(pos.col);
    col_cell_[pos.col].insert(pos.row);
}



std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}