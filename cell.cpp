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
}

void Cell::Set(std::string text)
{
    if(IsReferenced()){
        InvalidateCash();
    }

    if(text.size() > 0){
        if(text.at(0) == '=' && text.size() > 1){
            std::unique_ptr<FormulaImpl> temp_impl = std::make_unique<FormulaImpl>(text.substr(1), *sheet_);
            std::vector<const Cell*> vector_cells;

            if(temp_impl->GetReferencedCells().size() > 0){
                for(auto pos : temp_impl->GetReferencedCells()){
                    vector_cells.push_back(sheet_->GetOrCreateCell(pos));
                }
            }

            CheckCyclicDependencies(vector_cells);

            if(temp_impl->GetReferencedCells().size() > 0){
                for(auto pos : temp_impl->GetReferencedCells()){
                    sheet_->GetConcreteCell(pos)->AddReferringCell(this);
                }
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
    if(IsReferenced()){
        InvalidateCash();
    }

    impl_->Clear();
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
    return referring_cells_.size() > 0;
}

void Cell::AddReferringCell(Cell* cell)
{
    referring_cells_.push_back(cell);
}

void Cell::InvalidateCash()
{
    if(HasCash()){
        impl_->DeleteCash();
        for(auto cell : referring_cells_){
            cell->InvalidateCash();
        }
    }
}



void Cell::CheckCyclicDependencies(std::unordered_set<const Cell *> processed_cells) const 
{
    if(processed_cells.count(this) != 0){
        throw CircularDependencyException("circular dependency");
    }

    processed_cells.insert(this);

    for(auto ref_cell : GetReferencedCells()){
        sheet_->GetConcreteCell(ref_cell)->CheckCyclicDependencies(processed_cells);
    }

}



void Cell::CheckCyclicDependencies(std::vector<const Cell*> referring_cells) const
{
    auto it = std::find(referring_cells.begin(), referring_cells.end(), this);
    if(it != referring_cells.end()){
        throw CircularDependencyException("circular dependency");
    }

    std::unordered_set<const Cell*> stop_set;
    stop_set.insert(this);

    for(auto ref_cell : referring_cells){
        ref_cell->CheckCyclicDependencies(stop_set);
    }
}



bool Cell::HasCash() const
{
    return impl_->HasCash();
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
    if(text_.find("'") == 0){
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
    if(HasCash()){
        return GetCashValue();
    }
    else{
        CellInterface::Value value;
        auto result = formula_->Evaluate(sheet_);
        if(std::holds_alternative<double>(result)){
            return std::get<double>(result);
        }
        else{
            return std::get<FormulaError>(result);
        }   
    }
}

std::string Cell::FormulaImpl::GetText() const
{
    return std::string("=") + formula_->GetExpression();
}

void Cell::FormulaImpl::Clear()
{
    formula_.reset();
    formula_ = ParseFormula("");
}

void Cell::Impl::DeleteCash()
{
    cash_.reset();
}

bool Cell::Impl::HasCash() const
{
    return cash_.has_value();
}
