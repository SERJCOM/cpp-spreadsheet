#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
    Cell(Sheet& sheet);
    ~Cell() = default;

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    void AddReferringCell(Cell* cell);

private:
    class Impl;
    class EmptyImpl;
    class TextImpl;
    class FormulaImpl;

    std::unique_ptr<Impl> impl_;

    void InvalidateCash();

    void CheckCyclicDependencies(std::unordered_set<const Cell*> processed_cells) const ;

    void CheckCyclicDependencies(std::vector<const Cell*> referring_cells) const ;

    bool HasCash() const ;

    Sheet* sheet_ = nullptr;
    std::vector<Cell*> referring_cells_;

    
    
};


class Cell::Impl{
public:
    void DeleteCash() ;

    bool HasCash() const ;

    virtual CellInterface::Value GetValue() const = 0 ;

    virtual std::string GetText() const = 0 ;

    virtual void Clear() = 0 ;

protected:
    Value GetCashValue() const {
        return cash_.value();
    }
    void SetCashValue(Value value){
        cash_ = value;
    }
private:
    mutable std::optional<Value> cash_ ;
};


class Cell::EmptyImpl : public Cell::Impl{
public:

    EmptyImpl() = default;

    CellInterface::Value GetValue() const override;

    std::string GetText() const override;

    void Clear() override;
};


class Cell::TextImpl : public Cell::Impl{
public:
    TextImpl(std::string text);

    CellInterface::Value GetValue() const override;

    std::string GetText() const override;

    void Clear() override;
private:
    std::string text_;
};


class Cell::FormulaImpl : public Cell::Impl{
public:

    FormulaImpl(std::string, const Sheet& sheet);

    std::vector<Position> GetReferencedCells() const;

    CellInterface::Value GetValue() const override;

    std::string GetText() const override ;

    void Clear() override;

private:
std::unique_ptr<FormulaInterface> formula_;
const Sheet& sheet_;
};

