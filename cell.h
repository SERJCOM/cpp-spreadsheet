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
    std::string text_;

    void AddReferringCell(Cell* cell);

    void DeleteReferringCell(Cell* cell);

    void InvalidateCache();

    using CellsContainer = std::vector<const Cell*>;
    void CheckCyclicDependenciesRecursion(CellsContainer& cells_stack, std::unordered_set<const Cell*>& processed_cells) const ;

    void CheckCyclicDependencies(const CellsContainer& referring_cells) const ;

    bool HasCache() const ;

    class Impl{
    public:

        virtual ~Impl() = default;

        void DeleteCache() ;

        bool HasCache() const ;

        virtual CellInterface::Value GetValue() const = 0 ;

        virtual std::string GetText() const = 0 ;

        virtual void Clear() = 0 ;

    protected:
        Value GetCacheValue() const {
            return cache_.value();
        }
        void SetCacheValue(Value value) const {
            cache_ = value;
        }
    private:
        mutable std::optional<Value> cache_ ;
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

    private:
        std::unique_ptr<FormulaInterface> formula_;
        const Sheet& sheet_;
    };


    
};



