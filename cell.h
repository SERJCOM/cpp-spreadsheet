#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>


class Sheet;

class Cell : public CellInterface {
public:
    explicit Cell(Sheet& sheet);
    ~Cell() = default;

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

    bool IsReferenced() const;

    bool IsEmpty() const ;

private:
    class Impl;

    std::unique_ptr<Impl> impl_;
    Sheet* sheet_ = nullptr;
    std::vector<Cell*> referring_cells_;

    void AddReferringCell(Cell* cell);

    void DeleteReferringCell(Cell* cell);

    void InvalidateCache();

    void RemoveThisCellFromDependentCells();

    using CellsContainer = std::vector<const Cell*>;
    void CheckCyclicDependenciesRecursion(const Cell* starting_cell, std::unordered_set<const Cell*>& processed_cells) const ;

    void CheckCyclicDependencies(const CellsContainer& referring_cells) const ;

    bool HasCache() const ;

    class Impl{
    public:

        virtual ~Impl() = default;

        virtual void DeleteCache() {}

        virtual bool HasCache() const {return false;}

        virtual CellInterface::Value GetValue() const = 0 ;

        virtual std::string GetText() const = 0 ;

        virtual void Clear() = 0 ;

    protected:
        virtual Value GetCacheValue() const {return Value{};}

        virtual void SetCacheValue(Value value) const {}
    };


    class EmptyImpl final: public Impl{
    public:

        EmptyImpl() = default;

        CellInterface::Value GetValue() const override;

        std::string GetText() const override;

        void Clear() override;
    };


    class TextImpl final: public Impl{
    public:
        TextImpl(std::string text);

        CellInterface::Value GetValue() const override;

        std::string GetText() const override;

        void Clear() override;
    private:
        std::string text_;
    };


    class FormulaImpl final : public Impl{
    public:

        FormulaImpl(std::string, const Sheet& sheet);

        std::vector<Position> GetReferencedCells() const;

        CellInterface::Value GetValue() const override;

        std::string GetText() const override ;

        void Clear() override;

        void DeleteCache() override {
            cache_.reset();
        }

        bool HasCache() const override {
            return cache_.has_value();
        }

    private:

        Value GetCacheValue() const override{
            return cache_.value();
        }

        void SetCacheValue(Value value) const override {
            cache_ = value;
        }


        std::unique_ptr<FormulaInterface> formula_;
        const Sheet& sheet_;
        mutable std::optional<Value> cache_ ;
    };


    
};



