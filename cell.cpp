#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <utility>

#include "sheet.h"

Cell::Cell(Sheet &sheet)
{
    sheet_ = &sheet;
    impl_ = std::make_unique<EmptyImpl>();
}

void Cell::Set(std::string text)
{
    if(text == impl_->GetText()){
        return;
    }


    if(IsReferenced()){
        InvalidateCache();
    }

    if(!text.empty()){
        if(text.at(0) == FORMULA_SIGN && text.size() > 1){
            std::unique_ptr<FormulaImpl> temp_impl = std::make_unique<FormulaImpl>(text.substr(1), *sheet_);
            std::vector<const Cell*> vector_cells;

            if(temp_impl->GetReferencedCells().size() > 0)
                for(auto pos : temp_impl->GetReferencedCells()){
                    vector_cells.push_back(sheet_->GetOrCreateCell(pos));
                }
            

            CheckCyclicDependencies(vector_cells);

            if(temp_impl->GetReferencedCells().size() > 0)
                for(auto pos : temp_impl->GetReferencedCells()){
                    sheet_->GetConcreteCell(pos)->AddReferringCell(this);
                }
            
            impl_ = std::move(temp_impl);
        }
        else{
            impl_ = std::make_unique<TextImpl>(text);
        }
    }
    else{
        impl_ = std::make_unique<EmptyImpl>();
    }
}

void Cell::Clear()
{
    if(IsReferenced())  InvalidateCache();
    
    impl_ = std::make_unique<EmptyImpl>();

}

Cell::Value Cell::GetValue() const
{

    try{
        return impl_->GetValue();
    }
    catch(const FormulaError& err){
        return err;
    }
}

std::string Cell::GetText() const
{
    return impl_->GetText();
}
 
std::vector<Position> Cell::GetReferencedCells() const
{
    if(FormulaImpl* form = dynamic_cast<Cell::FormulaImpl*>(impl_.get())){
        return form->GetReferencedCells();
    }
    else{
        return std::vector<Position>{};
    }
}

bool Cell::IsReferenced() const
{
    return !referring_cells_.empty();
}

bool Cell::IsEmpty() const
{
    return ( dynamic_cast<EmptyImpl*>(impl_.get()) != nullptr );
}




void Cell::AddReferringCell(Cell* cell)
{
    referring_cells_.push_back(cell);
}

void Cell::DeleteReferringCell(Cell *cell)
{
    std::remove(referring_cells_.begin(), referring_cells_.end(), cell);
}

void Cell::InvalidateCache()
{
    if(HasCache()){
        impl_->DeleteCache();
        for(auto cell : referring_cells_){
            cell->InvalidateCache();
        }
    }

    for(auto cell : GetReferencedCells()){
        Cell* cell_ptr = sheet_->GetConcreteCell(cell);
        if(cell_ptr)    cell_ptr->DeleteReferringCell(this);
    
    }

}



void Cell::CheckCyclicDependenciesRecursion(const Cell* starting_cell, std::unordered_set<const Cell*>& processed_cells) const 
{
    if(processed_cells.count(this) != 0){
        return;
    }

    if(starting_cell == this){
        throw CircularDependencyException("circular dependency");
    }

    for(auto ref_cell : GetReferencedCells()){
        const Cell* _cell = sheet_->GetConcreteCell(ref_cell);
        _cell->CheckCyclicDependenciesRecursion(starting_cell, processed_cells);
        processed_cells.insert(_cell);
    }

}



void Cell::CheckCyclicDependencies(const CellsContainer& referring_cells) const
{
    auto it = std::find(referring_cells.begin(), referring_cells.end(), this);
    if(it != referring_cells.end()){
        throw CircularDependencyException("circular dependency");
    }

    std::unordered_set<const Cell*> empty_processed_cell;

    for(auto ref_cell : referring_cells){
        ref_cell->CheckCyclicDependenciesRecursion(this, empty_processed_cell);
    }
}



bool Cell::HasCache() const
{
    return impl_->HasCache();
}

CellInterface::Value Cell::EmptyImpl::GetValue() const
{
    return FormulaError(FormulaError::Category::Value);
}

std::string Cell::EmptyImpl::GetText() const
{
    return std::string();
}

void Cell::EmptyImpl::Clear()
{
}

Cell::TextImpl::TextImpl(std::string text)
{
    text_ = std::move(text);
}



CellInterface::Value Cell::TextImpl::GetValue() const
{
    std::string res = text_;
    if(text_.find(ESCAPE_SIGN) == 0){
        res = text_.substr(1);
    }

    return res;
}

std::string Cell::TextImpl::GetText() const
{
    return text_;
}

void Cell::TextImpl::Clear()
{
    text_ = "";
}

Cell::FormulaImpl::FormulaImpl(std::string text, const Sheet& sheet) : sheet_(sheet)
{
    formula_ = ParseFormula(text);
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const
{
    return formula_->GetReferencedCells();
}

CellInterface::Value Cell::FormulaImpl::GetValue() const
{
    if(HasCache()){
        return GetCacheValue();
    }
    else{
        CellInterface::Value value;
        auto result = formula_->Evaluate(sheet_);
        if(std::holds_alternative<double>(result)){
            value = std::get<double>(result);
            SetCacheValue(value);
            return value;
        }
        else{
            value = std::get<FormulaError>(result);
            SetCacheValue(value);
            return value;
        }   
    }
}

std::string Cell::FormulaImpl::GetText() const
{
    return std::string(1, FORMULA_SIGN) + formula_->GetExpression();
}

void Cell::FormulaImpl::Clear()
{
    formula_.reset();
    formula_ = ParseFormula("");
}



