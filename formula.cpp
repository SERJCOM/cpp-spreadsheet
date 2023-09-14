#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

#include "sheet.h"

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}


namespace {
class Formula : public FormulaInterface {
public:
// Реализуйте следующие методы:
    explicit Formula(std::string expression): ast_(ParseFormulaAST(expression)) {
    }

    Value Evaluate(const SheetInterface& sheet) const override {

        const Sheet* _sheet = dynamic_cast<const Sheet*>(&sheet);

        auto lambda = [&_sheet](Position pos){
            return _sheet->GetValue(pos);
        };


        try{
            return ast_.Execute(lambda);
        }
        catch(const FormulaError& error){
            return error;
        }
    }

    std::string GetExpression() const override {
        std::stringstream str;
        ast_.PrintFormula(str);

        return str.str();
    }

    std::vector<Position> GetReferencedCells() const override{
        std::set<Position> temp;

        for(const auto& cell : ast_.GetCells()){
            temp.insert(cell);
        }

        std::vector<Position> res(temp.begin(), temp.end());

        return res;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try{
        return std::make_unique<Formula>(std::move(expression));
    }
    catch(...){
        throw FormulaException("ошибка в выражении" + expression);
    }
}