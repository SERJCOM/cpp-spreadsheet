#pragma once

#include "cell.h"
#include "common.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>

#include <functional>

class Sheet : public SheetInterface {
public:

    ~Sheet() override = default;

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    const Cell* GetConcreteCell(Position pos) const;
    Cell* GetConcreteCell(Position pos);

    Cell* GetOrCreateCell(Position pos);

    CellInterface::Value GetValue(Position pos) const;

private:

    void SaveCell(std::unique_ptr<Cell> cell, Position pos);
    std::unordered_map<Position, std::unique_ptr<Cell>, PositionHash> position_cell_;

    std::map<int, std::unordered_set<int>> row_cell_;
    std::map<int, std::unordered_set<int>> col_cell_;


};